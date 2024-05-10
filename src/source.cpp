#include <pthread.h>
#include <cstdio>

#include "source.h"


int create_threads(int num, void *f(void *arg), source_parameters *p) {
    for (int i = 0; i < num; i++) {
        pthread_t t;
        int status = pthread_create(&t, NULL, f, p);
        if (status != 0) {
            return status;
        }
        pthread_join(t, NULL);
    }
    return 0;
}

int create_sources(int num_audio_source, source_parameters *a, int num_video_source, source_parameters *v,
                   int num_data_source, source_parameters *d, void *f(void *arg)) {
    int status = 0;
    status = create_threads(num_audio_source, f, a);
    if (status != 0) {
        return status;
    }
    status = create_threads(num_video_source, f, v);
    if (status != 0) {
        return status;
    }
    status = create_threads(num_data_source, f, d);
    if (status != 0) {
        return status;
    }
    return 0;
}


source::source() {

}

source::~source() {

}

void source::on() {
    return;
}

void source::off() {
    return;
}