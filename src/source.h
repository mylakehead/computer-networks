#ifndef COMPUTER_NETWORKS_SOURCE_H
#define COMPUTER_NETWORKS_SOURCE_H

#include "config.h"

struct Packet {
    PacketType t;
    int size;
};

struct Event {
    double clock;
    double cost;
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
    double packet_interval;
    double cost;

    Status status;
    double next_on;
    double next_off;
    Event *next_arrival;
    double desired_clock;
};

struct State {
    double clock;

    std::vector<StreamState *> stream_states;
};

void init_source_state(Config *config, State *state);

Event *next_arrival(Config *config, State *state);

#endif //COMPUTER_NETWORKS_SOURCE_H
