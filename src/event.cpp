#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "event.h"
#include "utils.h"


#define INIT_SOURCE_STATE OFF


enum source_state {
    ON,
    OFF
};

void place_event(Event *events, int i, PacketType t, double clock) {
    Packet p;
    switch (t) {
        case AUDIO:
            p = Packet{.t=AUDIO};
            break;
        case VIDEO:
            p = Packet{.t=VIDEO};
            break;
        case DATA:
            p = Packet{.t=DATA};
            break;
        default:
            printf("???");
    }

    events[i].packet = &p;
    events[i].clock = clock;
}

void fill(Event *events, int length, SourceConfig *c) {
    source_state state = INIT_SOURCE_STATE;

    int peak_packets_per_sec = (int) (c->peak_bit_rate * 1000 / 8 / c->size);

    double clock = 0.0;
    int i = 0;

    while (i < length) {
        if (state == OFF) {
            double next_on = exponential_random(c->mean_on_time);
            clock += next_on;
            state = ON;
            continue;
        } else {
            // TODO if edge cases want to be handled, do here
            double next_off = exponential_random(c->mean_off_time);
            double on_end = clock + next_off;

            int packets_to_send = (int) (peak_packets_per_sec * next_off);
            double packet_interval = 1.0 / peak_packets_per_sec;
            do {
                place_event(events, i, c->t, clock);
                i++;
                clock += packet_interval;
                packets_to_send--;
            } while (packets_to_send > 0 && clock < on_end && i < length);
        }
    }
}

Event *prepare_events(SourceConfig c[], int size, int total) {
    int line_num = 0;
    for (int i = 0; i < size; i++) {
        line_num += c[i].num;
    }
    Event *lines[line_num];

    int m = 0;
    for (int i = 0; i < size; i++) {
        for (int n = 0; n < c[i].num; n++) {
            Event *events = (Event *) malloc(total * sizeof(Event));
            memset(events, 0, total * sizeof(Event));
            if (!events)
                return NULL;

            // register to lines table
            lines[m] = events;

            // fill events
            fill(events, total, &c[i]);

            m++;
        }
    }

    /* print
    for (int i = 0; i < line_num; i++) {
        for (int j = 0; j < total; j++) {
            printf("line: %d, event: %d, clock: %f, type: %d\n", i + 1, j + 1, lines[i][j].clock,
                   lines[i][j].packet->t);
        }
    }
    */

    Event *merge_line = (Event *) malloc(total * sizeof(Event));
    memset(merge_line, 0, total * sizeof(merge_line));
    if (!merge_line)
        return NULL;

    // merge all lines
    int line_index[line_num];
    for (int i = 0; i < line_num; i++) {
        line_index[i] = 0;
    }
    int n = 0;
    while (n < total) {
        double min_clock = 1.0e+30;
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

    for (int j = 0; j < total; j++) {
        char *t = (char *) malloc(256);
        switch (merge_line[j].packet->t) {
            case AUDIO:
                strcpy(t, "AUDIO");
                break;
            case VIDEO:
                strcpy(t, "VIDEO");
                break;
            case DATA:
                strcpy(t, "DATA");
                break;
            default:
                printf("----\n");
        }
        printf("merge line, event: %d, clock: %f, type: %s\n", j + 1, merge_line[j].clock, t);
    }

    return merge_line;
}