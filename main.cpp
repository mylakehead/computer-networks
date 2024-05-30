#include <iostream>
#include <cstdlib>
#include <queue>
#include <unistd.h>
#include <stdio.h>

#include "src/source.h"
#include "src/config.h"
#include "src/utils.h"

struct Runtime {
    Config *config;

    State *state;

    Event *arrival_event;
    float clock_next_departure;

    std::queue<Event *> q_arrival;
};

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

int num_delayed;
float total_of_delays, area_num_in_q, area_server_status;

EventType next_event_type;

// snapshot of the system at clock 0
void initialize(Runtime *runtime) {
    runtime->clock_next_departure = 1.0e+30;

    runtime->arrival_event = next_arrival(runtime->config, runtime->state);

    /* system state */
    // simulator clock
    clock_system = 0.0;
    // server status
    server_status = IDLE;
    // time of last event
    clock_last_event = 0.0;

    // statistical counters
    num_delayed = 0;
    total_of_delays = 0.0;
    area_num_in_q = 0.0;
    area_server_status = 0.0;
}

void arrive(Runtime *runtime) {
    clock_system = runtime->arrival_event->clock;

    if (IDLE == server_status) {
        // since no Event in Q, this Event can be handled immediately
        // and departure clock can be calculated
        runtime->clock_next_departure = cal_departure_clock(clock_system, runtime->arrival_event->cost);
        // since server is IDLE, this event will be served without pushing in q_arrival, update num delayed
        num_delayed += 1;
        // update server status
        server_status = BUSY;
    } else {
        // server keep busy
        // update q_arrival
        if (runtime->q_arrival.size() >= runtime->config->fifo.size) {
            printf("event dropped\n");
        } else {
            runtime->q_arrival.push(runtime->arrival_event);
        }
    }

    // calculate area_num_in_q
    float clock_between_last_event = clock_system - clock_last_event;
    area_num_in_q += clock_between_last_event * runtime->q_arrival.size();
    // calculate area server status
    area_server_status += int(server_status) * clock_between_last_event;
    // update last event
    clock_last_event = clock_system;
}

void depart(Runtime *runtime) {
    clock_system = runtime->clock_next_departure;

    // calculate area_num_in_q
    float clock_between_last_event = clock_system - clock_last_event;
    area_num_in_q += clock_between_last_event * runtime->q_arrival.size();
    // calculate area server status
    area_server_status += int(server_status) * clock_between_last_event;

    // update last event
    clock_last_event = clock_system;

    // delay in q_arrival
    if (runtime->q_arrival.size() > 0) {
        // move out event from q_arrival
        Event *arrival_head = runtime->q_arrival.front();
        num_delayed += 1;
        float event_delay = clock_system - arrival_head->clock;
        total_of_delays += event_delay;
        runtime->clock_next_departure = cal_departure_clock(clock_system, arrival_head->cost);
        runtime->q_arrival.pop();
    } else {
        runtime->clock_next_departure = 1.0e+30;
        server_status = IDLE;
    }
}


void report(void) {
    printf("num_delayed: %d, total_of_delays: %f, area_num_in_q: %f, area_server_status: %f \n", num_delayed,
           total_of_delays, area_num_in_q, area_server_status);
}


int main(int argc, char *argv[]) {
    Runtime runtime;

    // get command line arguments
    char *config_file = nullptr;
    int ch;
    while ((ch = getopt(argc, argv, "c:")) != -1) {
        switch (ch) {
            case 'c':
                config_file = optarg;
                break;
        }
    }
    if (nullptr == config_file || strlen(config_file) <= 0) {
        return EXIT_FAILURE;
    }

    // parse config file
    printf("parse config file: <%s>\n", config_file);
    Config config;
    int r = parse_config_file(config_file, &config);
    if (0 != r) {
        printf("parse config file failed: <%s>\n", config_file);
        return EXIT_FAILURE;
    }
    printf("parse config file successfully\n");
    runtime.config = &config;

    State state;
    runtime.state = &state;

    srand(time(nullptr));

    // init source state
    init_source_state(runtime.config, runtime.state);

    // prepare arrival q
    switch (config.queue_type) {
        case FIFO:
            if (config.fifo.size >= 9223372036854775807) {
                printf("integrating FIFO with infinite size\n");
            } else {
                printf("integrating FIFO with size: %ld\n", config.fifo.size);
            }
            break;
        case SPQ:
            printf("unimplemented\n");
            return EXIT_FAILURE;
            break;
        case WFQ:
            printf("unimplemented\n");
            return EXIT_FAILURE;
            break;
    }

    initialize(&runtime);

    while (true) {
        // decide next event type
        if (runtime.arrival_event->clock < runtime.clock_next_departure) {
            next_event_type = ARRIVAL;
        } else {
            next_event_type = DEPARTURE;
        }

        switch (next_event_type) {
            case ARRIVAL:
                arrive(&runtime);
                runtime.arrival_event = next_arrival(runtime.config, runtime.state);
                if (runtime.arrival_event->packet.t == AUDIO) {
                    printf("next event: AUDIO, clock: %f\n", runtime.arrival_event->clock);
                } else if (runtime.arrival_event->packet.t == VIDEO) {
                    printf("next event: VIDEO, clock: %f\n", runtime.arrival_event->clock);
                } else {
                    printf("next event: DATA, clock: %f\n", runtime.arrival_event->clock);
                }

                break;
            case DEPARTURE:
                depart(&runtime);
                break;
        }
        report();
    }

    report();

    return EXIT_SUCCESS;
}