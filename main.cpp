#include <iostream>
#include <cstdlib>

#include "src/event.h"


// TODO transfer below parameters to config and inputs
#define NUM_AUDIO_SOURCE 1
#define NUM_VIDEO_SOURCE 1
#define NUM_DATA_SOURCE 1


#define SOURCE_EVENTS_LENGTH 1000


int main() {
    srand(time(nullptr));

    // prepare events
    printf("preparing events...\n");
    SourceConfig c[] = {
            SourceConfig{.num=NUM_AUDIO_SOURCE, .t = AUDIO, .mean_on_time = 0.36, .mean_off_time=0.64, .peak_bit_rate = 64, .size=120},
            SourceConfig{.num=NUM_VIDEO_SOURCE, .t = VIDEO, .mean_on_time = 0.33, .mean_off_time=0.73, .peak_bit_rate = 384, .size=1000},
            SourceConfig{.num=NUM_DATA_SOURCE, .t = DATA, .mean_on_time = 0.35, .mean_off_time=0.65, .peak_bit_rate = 256, .size=583}
    };
    Event *events = prepare_events(c, sizeof(c) / sizeof(c[0]), SOURCE_EVENTS_LENGTH);
    if (nullptr == events) {
        return 1;
    }
    /*
    for (int j = 0; j < SOURCE_EVENTS_LENGTH; j++) {
        printf("merge line, event: %d, clock: %f, type: %i\n", j + 1, events[j].clock, events[j].packet.t);
    }
     */
    printf("%d events prepared.\n", SOURCE_EVENTS_LENGTH);

    return EXIT_SUCCESS;
}