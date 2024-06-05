#include <cstdio>

#include "../include/tomlcpp.hpp"

#include "config.h"


int parse_config_file(const char *config_file, Config *config) {
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
    if (activate == "FIFO") {
        config->queue_type = FIFO;
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

        if (config->fifo.size >= 9223372036854775807) {
            printf("integrating FIFO with infinite size\n");
        } else {
            printf("integrating FIFO with size: %ld\n", config->fifo.size);
        }
    } else if (activate == "SPQ") {
        config->queue_type = SPQ;
        auto queueSPQ = queue->getTable("SPQ");
        if (!queueSPQ) {
            printf("parsing file error: %s\n", "missing [queue.SPQ]");
            return 1;
        }

        auto Qs = queueSPQ->getArray("Q");
        if (!Qs) {
            printf("parsing file error: %s\n", "missing or bad [queue.SPQ.Q] entry");
            return 1;
        }

        for (int i = 0;; i++) {
            auto q_table = Qs->getTable(i);
            if (!q_table) {
                break;
            }

            std::pair q_size = q_table->getInt("size");
            if (!q_size.first) {
                printf("parsing file error: %s\n", "missing or bad [queue.SPQ.Q.size] entry");
                return 1;
            }

            if (q_size.second >= 9223372036854775807) {
                printf("integrating SPQ priority:%d with infinite size\n", i + 1);
            } else {
                printf("integrating SPQ priority:%d with size: %lld\n", i + 1, q_size.second);
            }

            config->spq.sizes.push_back(q_size.second);
        }
    } else if (activate == "WFQ") {
        config->queue_type = WFQ;
        auto queueWFQ = queue->getTable("WFQ");
        if (!queueWFQ) {
            printf("parsing file error: %s\n", "missing [queue.WFQ]");
            return 1;
        }
        auto Qs = queueWFQ->getArray("Q");
        if (!Qs) {
            printf("parsing file error: %s\n", "missing or bad [queue.WFQ.Q] entry");
            return 1;
        }

        for (int i = 0;; i++) {
            auto q_table = Qs->getTable(i);
            if (!q_table) {
                break;
            }

            std::pair q_weight = q_table->getDouble("weights");
            if (!q_weight.first) {
                printf("parsing file error: %s\n", "missing or bad [queue.WFQ.Q.weights] entry");
                return 1;
            }
            config->wfq.weights.push_back((float) q_weight.second);

            std::pair q_size = q_table->getInt("size");
            if (!q_size.first) {
                printf("parsing file error: %s\n", "missing or bad [queue.WFQ.Q.size] entry");
                return 1;
            }

            if (q_size.second >= 9223372036854775807) {
                printf("integrating WFQ priority:%d with infinite size\n", i + 1);
            } else {
                printf("integrating WFQ priority:%d with size: %lld\n", i + 1, q_size.second);
            }

            config->wfq.sizes.push_back(q_size.second);
        }
    } else {
        printf("parsing file error: %s\n", "missing or bad [queue.activate] entry");
        return 1;
    }

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

        Flow flow{};
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
        flow.streams = (int) flow_streams.second;

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
        flow.peak_bit_rate = (int) flow_peak_bit_rate.second;

        std::pair flow_packet_size = flow_table->getInt("packet_size");
        if (!flow_packet_size.first) {
            printf("parsing file error: %s\n", "missing or bad [source.flows.packet_size] entry");
            return 1;
        }
        flow.packet_size = (int) flow_packet_size.second * 8;

        lambda += (flow.mean_on_time / (flow.mean_on_time + flow.mean_off_time)) * (float) flow.streams *
                  (float) flow.peak_bit_rate *
                  1000;

        config->source.flows.push_back(flow);
    }

    printf("Little's formula, λ = %f\n", lambda);

    return 0;
}
