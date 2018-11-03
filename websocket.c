#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include "nanomsg/nn.h"
#include "nanomsg/pubsub.h"
#include "nanomsg/ws.h"

int fillBuffer(char* buf, int max_size, float* data, int numpts) {
    int cnt = 0;
    int idx = 0;
    //idx += sprintf(&buf[idx], "{\"x\":0,\"y\",1}");
    //return idx;

    idx += sprintf(&buf[idx], "{\"x\":[");

    for (cnt = 0; cnt < numpts-1; cnt++) {
        idx += sprintf(&buf[idx], "%d,", cnt);
        if (idx >= max_size - 100) {
            return idx;
        }
    }
    idx += sprintf(&buf[idx], "%d],\"y\":[", cnt);
    if (idx >= max_size - 100) {
        return idx;
    }
 
    for (cnt = 0; cnt < numpts-1; cnt++) {
        idx += sprintf(&buf[idx], "%f,", data[cnt]);
    }
    idx += sprintf(&buf[idx], "%f]}", data[cnt]);
    if (idx >= max_size - 100) {
        return idx;
    }
 
    return idx;
}

int plotData(int sock, float* data, int numpts) {
    char buf[10000];
    int sz_d = fillBuffer(buf, 10000, data, numpts);
    if (sz_d >= 10000 - 100) {
        return -1;
    }
    //printf("SERVER: PUBLISHING DATE %s\n", buf);
    int bytes = nn_send(sock, buf, sz_d, 0);
    if (bytes < 0) {
        fprintf(stderr, "Could not send data");
        return -2;
    }
    return 0;
}

int main (int argc, const char *argv[])
{
    int rc;
    int sock;
    int N = 200;
    int cnt;
    float phase = 0;

     /*  Open the socket anew. */
    sock = nn_socket (AF_SP, NN_PUB);

    rc = nn_bind (sock, "ws://0.0.0.0:3001");
    //nn_setsockopt (sock, NN_WS_MSG_TYPE, NN_WS_MSG_TYPE_TEXT, NULL, 0);

    for (;;) {
        // make sine wave
        float data[N];
        for (cnt = 0; cnt < N; cnt++) {
            data[cnt] = (float)sin(2.0*M_PI*cnt/(float)N + phase);
        }
        int val = plotData(sock, data, N);
        if (val) {
            fprintf(stderr, "error sending data\n");
            break;
        }
        phase += 0.1;
        if (phase > 2*M_PI) {
            phase -= 2*M_PI;
        }
        usleep(100000);
    }

    return 0;
}

