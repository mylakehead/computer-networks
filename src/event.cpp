#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>

#include "event.h"
#include "utils.h"


#define INIT_SOURCE_STATE OFF


enum source_state {
    ON,
    OFF
};

void place_event(Event *events, int i, PacketType t, float clock, float cost) {
    events[i].packet = Packet{.t=t};
    events[i].clock = clock;
    events[i].cost = cost;
}

void fill(Event *events, int length, float server_rate, Flow *f) {
    source_state state = INIT_SOURCE_STATE;

    int peak_packets_per_sec = (int) (f->peak_bit_rate * 1000 / 8 / f->packet_size);

    float clock = 0.0;
    int i = 0;
    float cost = f->packet_size / server_rate;

    while (i < length) {
        if (state == OFF) {
            float next_on = exponential_random(f->mean_on_time);
            clock += next_on;
            state = ON;
            continue;
        } else {
            // TODO if edge cases want to be handled, do here
            float next_off = exponential_random(f->mean_off_time);
            float on_end = clock + next_off;

            int packets_to_send = (int) (peak_packets_per_sec * next_off);
            float packet_interval = 1.0 / peak_packets_per_sec;
            do {
                place_event(events, i, f->t, clock, cost);
                i++;
                clock += packet_interval;
                packets_to_send--;
            } while (packets_to_send > 0 && clock < on_end && i < length);
        }
    }
}

Event *prepare_events(Config *config) {
    int line_num = 0;
    for (int i = 0; i < config->source.flows.size(); i++) {
        line_num += config->source.flows[i].streams;
    }
    Event *lines[line_num];

    int m = 0;
    for (int i = 0; i < config->source.flows.size(); i++) {
        for (int n = 0; n < config->source.flows[i].streams; n++) {
            Event *events = (Event *) malloc(config->source.event_count * sizeof(Event));
            memset(events, 0, config->source.event_count * sizeof(Event));
            if (!events)
                return NULL;

            // register to lines table
            lines[m] = events;

            // fill events
            fill(events, config->source.event_count, config->server.rate, &(config->source.flows[i]));

            m++;
        }
    }

    /* print
    for (int i = 0; i < line_num; i++) {
        for (int j = 0; j < total; j++) {
            printf("line: %d, event: %d, clock: %f, type: %d\n", i + 1, j + 1, lines[i][j].clock,
                   lines[i][j].packet.t);
        }
    }
    */

    Event *merge_line = (Event *) malloc(config->source.event_count * sizeof(Event));
    memset(merge_line, 0, config->source.event_count * sizeof(merge_line));
    if (!merge_line)
        return NULL;

    // merge all lines
    int line_index[line_num];
    for (int i = 0; i < line_num; i++) {
        line_index[i] = 0;
    }
    int n = 0;
    while (n < config->source.event_count) {
        float min_clock = 1.0e+30;
        int which_line = 0;
        for (int i = 0; i < line_num; i++) {
            if (lines[i][line_index[i]].clock < min_clock) {
                min_clock = lines[i][line_index[i]].clock;
                which_line = i;
            }
        }
        merge_line[n] = lines[which_line][line_index[which_line]];
        n++;
        line_index[which_line] += 1;
    }

    return merge_line;
}