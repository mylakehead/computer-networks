#ifndef COMPUTER_NETWORKS_SOURCE_H
#define COMPUTER_NETWORKS_SOURCE_H

struct source_parameters {
    char *name;
    double mean_on_time;
    double mean_off_time;
    double peak_bit_rate;
    int packet_size;
};

enum source_state {
    ON,
    OFF
};


int create_sources(int num_audio_source, source_parameters *a, int num_video_source, source_parameters *v,
                   int num_data_source, source_parameters *d, void *f(void *arg));

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
