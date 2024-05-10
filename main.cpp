#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "src/utils.h"
#include "src/source.h"


// TODO transfer below parameters to config and inputs
#define NUM_AUDIO_SOURCE 2
#define NUM_VIDEO_SOURCE 1
#define NUM_DATA_SOURCE 1

source_parameters a = source_parameters{
        name: "AUDIO",
        mean_on_time: 0.36,
        mean_off_time: 0.64,
        peak_bit_rate: 64,
        packet_size: 120
};

source_parameters v = source_parameters{
        name: "VIDEO",
        mean_on_time: 0.27,
        mean_off_time: 0.73,
        peak_bit_rate: 384,
        packet_size: 1000
};

source_parameters d = source_parameters{
        name: "DATA",
        mean_on_time: 0.35,
        mean_off_time: 0.65,
        peak_bit_rate: 256,
        packet_size: 583
};

#define INIT_SOURCE_STATE OFF

int send_packet() {
    // TODO Feltejae@lakeheadu.ca ctiwari@lakeheadu.ca
    // we need a packet object, for packet generating
    // private properties may have: size, type....
    return 0;
}

void *source_generator(void *arg) {
    source_parameters *p = (source_parameters *) arg;

    int peak_packets_per_sec = (int) (p->peak_bit_rate * 1000 / 8 / p->packet_size);

    printf("%s source generator running, mean_on_time: %f, mean_off_time: %f, peak_bit_rate, %f, packet_size: %d, peak_packets_per_sec: %d.\n",
           p->name,
           p->mean_on_time,
           p->mean_off_time,
           p->peak_bit_rate,
           p->packet_size,
           peak_packets_per_sec
    );

    source_state state = INIT_SOURCE_STATE;

    while (true) {
        if (state == OFF) {
            // TODO
            // why * 100? just make it bigger, simulate ms
            int next_on = (int) (exponential_random(p->mean_on_time) * 100);
            printf("%s source generator is on state \"OFF\", will sleep %d ms.\n", p->name, next_on);
            // milliseconds to microseconds
            usleep(next_on * 1000);
            state = ON;
            continue;
        } else {
            printf("%s source generator is on state \"ON\", sending packet %d.\n", p->name, 1);
            // since we are on, send one first
            // TODO handle ack
            int ack = send_packet();
            // calculate how many packets left
            int next_off = (int) (exponential_random(p->mean_off_time) * 100);
            int packets_to_send = peak_packets_per_sec * next_off / 1000;
            if (packets_to_send == 0) {
                printf("%s source generator is on state \"ON\", no more packet to send, will sleep %d ms.\n",
                       p->name,
                       next_off);
                // still need to sleep
                usleep(next_off * 1000);
            } else {
                printf("%s source generator is on state \"ON\", %d packets left to send.\n", p->name, packets_to_send);
                int packet_interval = next_off / packets_to_send;
                for (int i = 0; i < packets_to_send; i++) {
                    printf("%s source generator is on state \"ON\", sending packet %d.\n", p->name, i + 2);
                    ack = send_packet();
                    // milliseconds to microseconds
                    usleep(packet_interval * 1000);
                }
            }
            state = OFF;
            continue;
        }
    }

    pthread_exit(0);
}


// Driver Code

int main() {
    srand((unsigned) time(NULL));

    create_sources(NUM_AUDIO_SOURCE, &a, NUM_VIDEO_SOURCE, &v, NUM_DATA_SOURCE, &d, source_generator);

    /// ----

    for (int i = 0; i < 20; i++)
        printf("%f\n", exponential_random(0.35));

    return EXIT_SUCCESS;
}