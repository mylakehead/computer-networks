#include <iostream>
#include <cstdlib>
#include <queue>

#include "src/event.h"


#define NUM_AUDIO_SOURCE 1
#define NUM_VIDEO_SOURCE 1
#define NUM_DATA_SOURCE 1

#define SOURCE_EVENTS_LENGTH 1000000

#define Q_SIZE 100

enum ServerStatus {
    IDLE = 0,
    BUSY = 1
};

enum EventType {
    ARRIVAL = 1,
    DEPARTURE = 2
};

float clock_system, clock_last_event;
ServerStatus server_status;
std::queue<Event *> q_arrival;
std::queue<Event *> q_departure;

int event_index;
Event *arrival_event;

int num_delayed;
float total_of_delays, area_num_in_q, area_server_status;

EventType next_event_type;

// snapshot of the system at clock 0
void initialize(Event *events) {
    /* system state */
    // simulator clock
    clock_system = 0.0;
    // server status
    server_status = IDLE;
    // time of last event
    clock_last_event = 0.0;

    // event source
    event_index = 0;
    arrival_event = &events[event_index];
    // index move to next event
    event_index++;

    // statistical counters
    num_delayed = 0;
    total_of_delays = 0.0;
    area_num_in_q = 0.0;
    area_server_status = 0.0;
}

void step(Event *events) {
    // decide next event type
    if (0 == q_departure.size()) {
        next_event_type = ARRIVAL;
    } else {
        Event *departure_head = q_departure.front();
        if (arrival_event->clock < departure_head->clock) {
            next_event_type = ARRIVAL;
        } else {
            next_event_type = DEPARTURE;
        }
    }

    printf("%d\n", event_index);
}


void update_time_avg_stats(void) {
}

void arrive(Event *events) {
    if (IDLE == server_status) {
        clock_system = arrival_event->clock;
        // update arrival event
        arrival_event = &events[event_index];
        event_index++;

        // create a departure event
        Event *departure_event = (Event *) malloc(sizeof(Event));
        memset(events, 0, sizeof(Event));
        // TODO simulate depart interval according to packet type
        departure_event->clock = arrival_event->clock + 0.2;
        departure_event->packet = arrival_event->packet;
        q_departure.push(departure_event);

        // since server is IDLE, this event will be served without pushing in q_arrival, update num delayed
        num_delayed += 1;
        total_of_delays += 0.0;
        // calculate area_num_in_q
        float clock_between_last_event = clock_system - clock_last_event;
        area_num_in_q += clock_between_last_event * q_arrival.size();
        // calculate area server status
        area_server_status += int(server_status) * clock_between_last_event;

        // update last event
        clock_last_event = clock_system;
        // update server status
        server_status = BUSY;
        // number in q_arrival no changes
        // q_arrival no changes
    } else {
        clock_system = arrival_event->clock;
        // update arrival event
        arrival_event = &events[event_index];
        event_index++;

        // no event served, num delayed no change
        num_delayed += 0;
        // total delays no change
        total_of_delays += 0.0;
        // calculate area_num_in_q
        float clock_between_last_event = clock_system - clock_last_event;
        area_num_in_q += clock_between_last_event * q_arrival.size();
        // calculate area server status
        area_server_status += int(server_status) * clock_between_last_event;

        // update last event
        clock_last_event = clock_system;
        // server keep busy
        // update q_arrival
        if (q_arrival.size() >= Q_SIZE) {
            // TODO drop event
        } else {
            // create a departure event
            Event *departure_event = (Event *) malloc(sizeof(Event));
            memset(events, 0, sizeof(Event));
            // TODO simulate depart interval according to packet type
            departure_event->clock = arrival_event->clock + 0.2;
            departure_event->packet = arrival_event->packet;
            q_departure.push(departure_event);

            q_arrival.push(arrival_event);
        }
    }
}


void depart(void) {
    Event *departure_head = q_departure.front();

    clock_system = departure_head->clock;

    // move out event from q_arrival
    Event *arrival_head = q_arrival.front();

    // calculate area_num_in_q
    float clock_between_last_event = clock_system - clock_last_event;
    area_num_in_q += clock_between_last_event * q_arrival.size();
    // calculate area server status
    area_server_status += int(server_status) * clock_between_last_event;

    // update last event
    clock_last_event = clock_system;

    q_departure.pop();

    // delay in q_arrival
    if (q_arrival.size() > 0) {
        num_delayed += 1;
        float event_delay = clock_system - arrival_head->clock;
        total_of_delays += event_delay;
        q_arrival.pop();
    } else {
        if (0 == q_departure.size()) {
            server_status = IDLE;
        }
    }
}


void report(void) {
    printf("num_delayed: %d, total_of_delays: %f, area_num_in_q: %f, area_server_status: %f ", num_delayed,
           total_of_delays, area_num_in_q, area_server_status);
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
                arrive(events);
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