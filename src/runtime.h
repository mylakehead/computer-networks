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
    double weight;
    double last_finish_time;
    std::queue<Event *> q;
};

struct WFQScheduler {
    double virtual_time;
    std::vector<SubWFQ> subs{};
};


struct Runtime {
    Config *config{};

    // source state
    State *state{};

    // system state
    double clock_system{};
    double clock_last_event{};
    EventType next_event_type;
    Event *arrival_event{};
    double clock_next_departure{};
    ServerStatus server_status = IDLE;

    // FIFO
    std::queue<Event *> fifo{};

    // SPQ
    std::vector<SubSPQ> spq{};

    // WFQ
    WFQScheduler wfq{};

    // statistics
    std::vector<int> num_arrived_in_sub_spq{};
    std::vector<int> num_dropped_in_sub_spq{};
    std::vector<int> num_pushed_in_sub_spq{};
    std::vector<double> area_num_in_sub_spq{};

    std::vector<int> num_arrived_in_sub_wfq{};
    std::vector<int> num_dropped_in_sub_wfq{};
    std::vector<int> num_pushed_in_sub_wfq{};
    std::vector<double> area_num_in_sub_wfq{};

    int total_num_arrived_in_system{};
    int total_num_arrived_in_q{};
    int total_num_without_in_q{};

    int total_num_dropped_in_q{};
    int total_num_pushed_in_q{};
    double total_area_num_in_q{};

    int total_num_delayed_in_server{};
    double total_response_delays_in_server{};
    double area_server_status{};
};

#endif //COMPUTER_NETWORKS_RUNTIME_H
