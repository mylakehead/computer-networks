#ifndef COMPUTER_NETWORKS_EVENT_H
#define COMPUTER_NETWORKS_EVENT_H

enum PacketType {
    AUDIO,
    VIDEO,
    DATA
};

struct Packet {
    PacketType t;
};

const Packet AudioPacket = {
        .t = AUDIO,
};

const Packet VideoPacket = {
        .t = VIDEO,
};

const Packet DataPacket = {
        .t = DATA,
};

struct Event {
    double clock;
    Packet *packet;
};

struct SourceConfig {
    int num; // number of this kind of source
    PacketType t;
    double mean_on_time;
    double mean_off_time;
    double peak_bit_rate;
    int size;
};

struct EventsConfig {
    int sourceNum;
    SourceConfig *sc;
};

Event *prepare_events(SourceConfig c[], int size, int total);

#endif //COMPUTER_NETWORKS_EVENT_H
