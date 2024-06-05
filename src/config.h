#ifndef COMPUTER_NETWORKS_CONFIG_H
#define COMPUTER_NETWORKS_CONFIG_H

#include <vector>

enum PacketType {
    AUDIO = 1,
    VIDEO,
    DATA
};

enum QueueType {
    FIFO,
    SPQ,
    WFQ
};

struct FIFOConfig {
    long size;
};

struct SPQConfig {
    std::vector<long> sizes;
};

struct WFQConfig {
    std::vector<double> weights;
    std::vector<long> sizes;
};

struct Server {
    double rate;
};

struct Flow {
    PacketType t;
    int streams;
    double mean_on_time;
    double mean_off_time;
    int peak_bit_rate;
    int packet_size;
};

struct Source {
    std::vector<Flow> flows;
};

struct Config {
    QueueType queue_type{};
    FIFOConfig fifo{};
    SPQConfig spq{};
    WFQConfig wfq{};
    Server server{};
    Source source{};
};

int parse_config_file(const char *config_file, Config *config);

#endif //COMPUTER_NETWORKS_CONFIG_H
