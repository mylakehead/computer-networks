#ifndef COMPUTER_NETWORKS_SOURCE_H
#define COMPUTER_NETWORKS_SOURCE_H

#include "config.h"

struct Packet {
    PacketType t;
};

struct Event {
    float clock;
    float cost;
    Packet packet;
};

#define INIT_SOURCE_STATE OFF

enum Status {
    ON,
    OFF
};

struct StreamState {
    int index;
    Flow flow;
    float packet_interval;
    float cost;

    Status status;
    float next_on;
    float next_off;
    Event *next_arrival;
};

struct State {
    float clock;

    std::vector<StreamState *> stream_states;
};

void init_source_state(Config *config, State *state);

Event *next_arrival(Config *config, State *state);

#endif //COMPUTER_NETWORKS_SOURCE_H
