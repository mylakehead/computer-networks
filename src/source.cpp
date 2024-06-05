#include <cstdlib>
#include <cstring>
#include <vector>

#include "source.h"
#include "utils.h"


void init_source_state(Config *config, State *state) {
    state->clock = 0.0;

    for (int i = 0; i < config->source.flows.size(); i++) {
        for (int n = 0; n < config->source.flows[i].streams; n++) {
            StreamState *stream_state = (StreamState *) malloc(sizeof(StreamState));
            memset(stream_state, 0, sizeof(StreamState));

            // init configuration
            stream_state->index = n;
            stream_state->flow = config->source.flows[i];
            int peak_packets_per_sec = (int) (stream_state->flow.peak_bit_rate * 1000 / stream_state->flow.packet_size);
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
                e->packet = Packet{.t=stream->flow.t, .size=stream->flow.packet_size};
                e->clock = state->clock;
                e->cost = stream->cost;

                // event for next
                if (state->clock + stream->packet_interval < stream->next_off) {
                    Event *next = (Event *) malloc(sizeof(Event));
                    if (!next)
                        exit(EXIT_FAILURE);
                    memset(next, 0, sizeof(Event));
                    next->packet = Packet{.t=stream->flow.t, .size=stream->flow.packet_size};
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
                        next->packet = Packet{.t=stream->flow.t, .size=stream->flow.packet_size};
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