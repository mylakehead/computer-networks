#include <pthread.h>
#include <cstdio>

#include "source.h"


int create_sources(int num_audio_source, source_parameters *a, int num_video_source, source_parameters *v,
                   int num_data_source, source_parameters *d, void *f(void *arg)) {
    int total_num_source = num_audio_source + num_video_source + num_data_source;
    pthread_t threads[total_num_source];
    int n = 0;

    for (int j = 0; j < num_audio_source; j++) {
        int status = pthread_create(&threads[n], NULL, f, a);
        if (status != 0) {
            return status;
        }
        n++;
    }

    for (int j = 0; j < num_video_source; j++) {
        int status = pthread_create(&threads[n], NULL, f, v);
        if (status != 0) {
            return status;
        }
        n++;
    }

    for (int j = 0; j < num_data_source; j++) {
        int status = pthread_create(&threads[n], NULL, f, d);
        if (status != 0) {
            return status;
        }
        n++;
    }

    for (int i = 0; i < total_num_source; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

