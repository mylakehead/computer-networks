#ifndef COMPUTER_NETWORKS_SOURCE_H
#define COMPUTER_NETWORKS_SOURCE_H


class source {
private:
    unsigned int peak_bit_rate; // kbps
    unsigned int average_on_time; // sec
    unsigned int average_off_time; // sec
    unsigned int packet_size; // bytes
    bool state; // on or off
public:
    source();

    ~source();

    void on();

    void off();
};


#endif //COMPUTER_NETWORKS_SOURCE_H
