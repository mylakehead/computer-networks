#include <stdio.h>

#include "../include/tomlcpp.hpp"

#include "config.h"


int parse_config_file(char *config_file, Config *config) {
    auto res = toml::parseFile(config_file);
    if (!res.table) {
        printf("parsing file error: %s\n", res.errmsg.c_str());
        return 1;
    }

    auto queue = res.table->getTable("queue");
    if (!queue) {
        printf("parsing file error: %s\n", "missing [queue]");
        return 1;
    }

    auto [ok, activate] = queue->getString("activate");
    if (!ok) {
        printf("parsing file error: %s\n", "missing or bad [queue.activate] entry");
        return 1;
    }
    if (activate.compare("FIFO") == 0) {
        config->queue_type = FIFO;
    } else if (activate.compare("SPQ") == 0) {
        config->queue_type = SPQ;
    } else if (activate.compare("WFQ") == 0) {
        config->queue_type = WFQ;
    } else {
        printf("parsing file error: %s\n", "missing or bad [queue.activate] entry");
        return 1;
    }

    auto queueFIFO = queue->getTable("FIFO");
    if (!queueFIFO) {
        printf("parsing file error: %s\n", "missing [queue.FIFO]");
        return 1;
    }

    std::pair p = queueFIFO->getInt("size");
    if (!p.first) {
        printf("parsing file error: %s\n", "missing or bad [queue.FIFO.size] entry");
        return 1;
    }
    config->fifo.size = p.second;

    // server
    auto server = res.table->getTable("server");
    if (!server) {
        printf("parsing file error: %s\n", "missing [server]");
        return 1;
    }

    std::pair rate = server->getDouble("rate");
    if (!rate.first) {
        printf("parsing file error: %s\n", "missing or bad [server.rate] entry");
        return 1;
    }
    config->server.rate = (float) (rate.second) * 1000;

    // source
    auto source = res.table->getTable("source");
    if (!source) {
        printf("parsing file error: %s\n", "missing [source]");
        return 1;
    }

    std::pair event_count = source->getInt("event_count");
    if (!event_count.first) {
        printf("parsing file error: %s\n", "missing or bad [source.event_count] entry");
        return 1;
    }
    config->source.event_count = event_count.second;

    auto flows = source->getArray("flows");
    if (!flows) {
        printf("parsing file error: %s\n", "missing or bad [source.flows] entry");
        return 1;
    }

    // Little's formula
    float lambda = 0.0;

    for (int i = 0;; i++) {
        auto flow_table = flows->getTable(i);
        if (!flow_table) {
            break;
        }

        Flow flow;
        std::pair flow_type = flow_table->getString("type");
        if (!flow_type.first) {
            printf("parsing file error: %s\n", "missing or bad [source.flows.type] entry");
            return 1;
        }
        if (flow_type.second == "AUDIO") {
            flow.t = AUDIO;
        } else if (flow_type.second == "VIDEO") {
            flow.t = VIDEO;
        } else if (flow_type.second == "DATA") {
            flow.t = DATA;
        } else {
            printf("parsing file error: %s\n", "missing or bad [source.flows.type] entry");
            return 1;
        }

        std::pair flow_streams = flow_table->getInt("streams");
        if (!flow_streams.first) {
            printf("parsing file error: %s\n", "missing or bad [source.flows.streams] entry");
            return 1;
        }
        flow.streams = flow_streams.second;

        std::pair flow_mean_on_time = flow_table->getDouble("mean_on_time");
        if (!flow_mean_on_time.first) {
            printf("parsing file error: %s\n", "missing or bad [source.flows.mean_on_time] entry");
            return 1;
        }
        flow.mean_on_time = (float) flow_mean_on_time.second;

        std::pair flow_mean_off_time = flow_table->getDouble("mean_off_time");
        if (!flow_mean_off_time.first) {
            printf("parsing file error: %s\n", "missing or bad [source.flows.mean_off_time] entry");
            return 1;
        }
        flow.mean_off_time = (float) flow_mean_off_time.second;

        std::pair flow_peak_bit_rate = flow_table->getInt("peak_bit_rate");
        if (!flow_peak_bit_rate.first) {
            printf("parsing file error: %s\n", "missing or bad [source.flows.peak_bit_rate] entry");
            return 1;
        }
        flow.peak_bit_rate = flow_peak_bit_rate.second;

        std::pair flow_packet_size = flow_table->getInt("packet_size");
        if (!flow_packet_size.first) {
            printf("parsing file error: %s\n", "missing or bad [source.flows.packet_size] entry");
            return 1;
        }
        flow.packet_size = flow_packet_size.second;

        lambda += (flow.mean_on_time / (flow.mean_on_time + flow.mean_off_time)) * flow.streams * flow.peak_bit_rate *
                  1000;

        config->source.flows.push_back(flow);
    }

    printf("Little's formula, λ = %f\n", lambda);

    return 0;
}
