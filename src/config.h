#ifndef COMPUTER_NETWORKS_CONFIG_H
#define COMPUTER_NETWORKS_CONFIG_H

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

struct Server {
    float rate; // TODO precision
};

struct Flow {
    PacketType t;
    int streams;
    float mean_on_time; // TODO precision
    float mean_off_time; // TODO precision
    int peak_bit_rate;
    int packet_size;
};

struct Source {
    int event_count;
    std::vector<Flow> flows;
    float lambda; // λ, for Little's Law, the average arrival rate into the queue
};

struct Config {
    QueueType queue_type;
    FIFOConfig fifo;
    Server server;
    Source source;
};

int parse_config_file(char *config_file, Config *config);

#endif //COMPUTER_NETWORKS_CONFIG_H
