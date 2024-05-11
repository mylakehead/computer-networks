#include <iostream>
#include <cstdlib>

#include "src/event.h"

#define NUM_AUDIO_SOURCE 1
#define NUM_VIDEO_SOURCE 1
#define NUM_DATA_SOURCE 1

#define SOURCE_EVENTS_LENGTH 1000

#define Q_SIZE 100

enum ServerStatus {
    IDLE = 0,
    BUSY = 1
};

enum EventType {
    ARRIVAL = 1,
    DEPARTURE = 2
};

float sim_clock, clock_last_event;
ServerStatus server_status;
float q[Q_SIZE + 1];
int num_in_q;

int event_index;
Event *arrival_event;
float clock_departure_event;

int num_delayed;
float total_of_delays, area_num_in_q, area_server_status;

EventType next_event_type;


void initialize(Event *events) {
    /* system state */

    // simulator clock
    sim_clock = 0.0;
    // server status
    server_status = IDLE;
    // q
    num_in_q = 0;
    // time of last event
    clock_last_event = 0.0;

    // event source
    event_index = 0;
    arrival_event = &events[event_index];
    clock_departure_event = 1.0e+30;
    // index move to next event
    event_index++;

    // statistical counters
    num_delayed = 0;
    total_of_delays = 0.0;
    area_num_in_q = 0.0;
    area_server_status = 0.0;
}

void step(Event *events) {
    // update system clock
    if (arrival_event->clock < clock_departure_event) {
        next_event_type = ARRIVAL;
        sim_clock = arrival_event->clock;
    } else {
        next_event_type = DEPARTURE;
        sim_clock = clock_departure_event;
    }
    // TODO update server status
    server_status = BUSY;
    // TODO update number in q
    num_in_q = 0;
    // update time of last event
    clock_last_event = sim_clock;
    // update arrival event
    arrival_event = &events[event_index];
    event_index++;
    // TODO update departure clock
    clock_departure_event = 0.0;

    printf("%d\n", event_index);
}


void update_time_avg_stats(void) {
    float time_since_last_event;

    /* Compute time since last event, and update last-event-time marker. */

    time_since_last_event = sim_clock - clock_last_event;
    clock_last_event = sim_clock;

    /* Update area under number-in-queue function. */

    area_num_in_q += num_in_q * time_since_last_event;

    /* Update area under server-busy indicator function. */

    area_server_status += server_status * time_since_last_event;
}

void arrive(void)  /* Arrival event function. */
{
    // to be completed by students
}


void depart(void)  /* Departure event function. */
{
    // to be completed by studnts
}


void report(void)  /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */

    // to be completed by students
}

int main() {
    // TODO another source
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

    initialize(events);

    /* Run the simulation while more delays are still needed. */

    while (event_index < SOURCE_EVENTS_LENGTH) {
        step(events);

        /* Update time-average statistical accumulators. */

        update_time_avg_stats();

        /* Invoke the appropriate event function. */

        switch (next_event_type) {
            case ARRIVAL:
                arrive();
                break;
            case DEPARTURE:
                depart();
                break;
        }
    }

    /* Invoke the report generator and end the simulation. */

    report();

    return EXIT_SUCCESS;
}