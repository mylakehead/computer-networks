#include <cstdlib>
#include <cstring>
#include <vector>

#include "source.h"
#include "utils.h"


void place_event(Event *events, int i, PacketType t, float clock, float cost) {
    events[i].packet = Packet{.t=t};
    events[i].clock = clock;
    events[i].cost = cost;
}

void fill(Event *events, int length, float server_rate, Flow *f) {
    Status state = INIT_SOURCE_STATE;

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
            Event *events = (Event *) malloc(100000000 * sizeof(Event));
            memset(events, 0, 100000000 * sizeof(Event));
            if (!events)
                return NULL;

            // register to lines table
            lines[m] = events;

            // fill events
            fill(events, 100000000, config->server.rate, &(config->source.flows[i]));

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

    Event *merge_line = (Event *) malloc(100000000 * sizeof(Event));
    memset(merge_line, 0, 100000000 * sizeof(merge_line));
    if (!merge_line)
        return NULL;

    // merge all lines
    int line_index[line_num];
    for (int i = 0; i < line_num; i++) {
        line_index[i] = 0;
    }
    int n = 0;
    while (n < 100000000) {
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


void init_source_state(Config *config, State *state) {
    state->clock = 0.0;

    for (int i = 0; i < config->source.flows.size(); i++) {
        for (int n = 0; n < config->source.flows[i].streams; n++) {
            StreamState *stream_state = (StreamState *) malloc(sizeof(StreamState));
            memset(stream_state, 0, sizeof(StreamState));

            // init configuration
            stream_state->index = n;
            stream_state->flow = config->source.flows[i];
            int peak_packets_per_sec = (int) (stream_state->flow.peak_bit_rate * 1000 / 8 /
                                              stream_state->flow.packet_size);
            stream_state->packet_interval = 1.0 / peak_packets_per_sec;
            stream_state->cost = stream_state->flow.packet_size / config->server.rate;

            // runtime states
            stream_state->status = INIT_SOURCE_STATE;
            if (stream_state->status == OFF) {
                stream_state->next_on = state->clock + exponential_random(stream_state->flow.mean_on_time);
                stream_state->next_off = 1.0e+30;
            } else {
                stream_state->next_off = state->clock + exponential_random(stream_state->flow.mean_off_time);
                stream_state->next_on = 1.0e+30;
            }
            stream_state->next_arrival = nullptr;

            state->stream_states.push_back(stream_state);
        }
    }
}


Event *next_arrival(Config *config, State *state) {
    bool desired = false;
    float desired_next_clock = state->clock;

    for (int i = 0; i < state->stream_states.size(); i++) {
        StreamState *stream = state->stream_states[i];

        if (stream->status == OFF) {
            if (state->clock < stream->next_on) {
                // calculate desired clock
                if (!desired) {
                    desired_next_clock = stream->next_on;
                } else {
                    if (stream->next_on < desired_next_clock) {
                        desired_next_clock = stream->next_on;
                    }
                }
            } else {
                // time for switching status
                stream->status = ON;

                // calculate next_off
                stream->next_off = state->clock + exponential_random(stream->flow.mean_off_time);
                stream->next_on = 1.0e+30;

                // event for return
                Event *e = (Event *) malloc(sizeof(Event));
                if (!e)
                    exit(EXIT_FAILURE);
                memset(e, 0, sizeof(Event));
                e->packet = Packet{.t=stream->flow.t};
                e->clock = state->clock;
                e->cost = stream->cost;

                // event for next
                if (state->clock + stream->packet_interval < stream->next_off) {
                    Event *next = (Event *) malloc(sizeof(Event));
                    if (!next)
                        exit(EXIT_FAILURE);
                    memset(next, 0, sizeof(Event));
                    next->packet = Packet{.t=stream->flow.t};
                    next->clock = state->clock + stream->packet_interval;
                    next->cost = stream->cost;

                    stream->next_arrival = next;
                } else {
                    stream->next_arrival = nullptr;
                }

                // don't need to move clock
                return e;
            }
        } else {
            if (state->clock >= stream->next_off) {
                // time for switching status
                stream->status = OFF;
                stream->next_on = state->clock + exponential_random(stream->flow.mean_on_time);
                stream->next_off = 1.0e+30;
                stream->next_arrival = nullptr;

                if (!desired) {
                    desired_next_clock = stream->next_on;
                } else {
                    if (stream->next_on < desired_next_clock) {
                        desired_next_clock = stream->next_on;
                    }
                }
            } else {
                if (stream->next_arrival && state->clock >= stream->next_arrival->clock) {
                    Event *e = stream->next_arrival;
                    // event for next
                    if (state->clock + stream->packet_interval < stream->next_off) {
                        Event *next = (Event *) malloc(sizeof(Event));
                        if (!next)
                            exit(EXIT_FAILURE);
                        memset(next, 0, sizeof(Event));
                        next->packet = Packet{.t=stream->flow.t};
                        next->clock = state->clock + stream->packet_interval;
                        next->cost = stream->cost;

                        stream->next_arrival = next;
                    } else {
                        stream->next_arrival = nullptr;
                    }

                    return e;
                } else {
                    if (stream->next_arrival) {
                        if (!desired) {
                            desired_next_clock = stream->next_arrival->clock;
                        } else {
                            if (stream->next_arrival->clock < desired_next_clock) {
                                desired_next_clock = stream->next_arrival->clock;
                            }
                        }
                    }
                }
            }
        }
    }

    state->clock = desired_next_clock;

    return next_arrival(config, state);
}