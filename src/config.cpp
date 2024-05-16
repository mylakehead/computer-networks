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
    config->server.rate = (float) (rate.second) * 1000 * 1000;


    return 0;
}
