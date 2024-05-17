#ifndef COMPUTER_NETWORKS_EVENT_H
#define COMPUTER_NETWORKS_EVENT_H

#include "config.h"

struct Packet {
    PacketType t;
};

struct Event {
    float clock;
    float cost;
    Packet packet;
};

Event *prepare_events(Config *config);

#endif //COMPUTER_NETWORKS_EVENT_H
