#ifndef COMPUTER_NETWORKS_CONFIG_H
#define COMPUTER_NETWORKS_CONFIG_H


enum QueueType {
    FIFO,
    SPQ,
    WFQ
};

struct FIFOConfig {
    long size;
};

struct Server {
    float rate;
};

struct Config {
    QueueType queue_type;
    FIFOConfig fifo;
    Server server;
};

int parse_config_file(char *config_file, Config *config);

#endif //COMPUTER_NETWORKS_CONFIG_H
