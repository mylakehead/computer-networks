//
// Created by Jayden Hong on 2024-06-04.
//

#ifndef COMPUTER_NETWORKS_RUNTIME_H
#define COMPUTER_NETWORKS_RUNTIME_H

#include <queue>

#include "config.h"
#include "source.h"


enum EventType {
    ARRIVAL = 1,
    DEPARTURE = 2
};

enum ServerStatus {
    IDLE = 0,
    BUSY = 1
};

struct SubSPQ {
    long size;
    std::queue<Event *> q;
};

struct SubWFQ {
    long size;
    float weight;
    float last_finish_time;
    std::queue<Event *> q;
};

struct WFQScheduler {
    float virtual_time;
    std::vector<SubWFQ> subs{};
};


struct Runtime {
    Config *config{};

    State *state{};

    float clock_system{};
    float clock_last_event{};

    int num_generated{};
    int num_delayed{};
    float total_of_delays{};
    float area_num_in_q{};
    float area_server_status{};

    EventType next_event_type;

    Event *arrival_event{};
    float clock_next_departure{};

    // system state
    ServerStatus server_status = IDLE;

    // FIFO
    std::queue<Event *> fifo{};

    // SPQ
    std::vector<SubSPQ> spq{};

    // WFQ
    WFQScheduler wfq{};
};

#endif //COMPUTER_NETWORKS_RUNTIME_H
