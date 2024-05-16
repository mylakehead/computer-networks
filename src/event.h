#ifndef COMPUTER_NETWORKS_EVENT_H
#define COMPUTER_NETWORKS_EVENT_H

enum PacketType {
    AUDIO = 1,
    VIDEO,
    DATA
};

struct Packet {
    PacketType t;
};

struct Event {
    float clock;
    float cost;
    Packet packet;
};

struct SourceConfig {
    int num; // number of this kind of source
    PacketType t;
    float mean_on_time;
    float mean_off_time;
    float peak_bit_rate;
    int size;
};

Event *prepare_events(SourceConfig c[], int size, float server_rate, int total);

#endif //COMPUTER_NETWORKS_EVENT_H
