#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <sys/time.h>

#include "src/source.h"
#include "src/config.h"
#include "src/utils.h"
#include "src/queue.h"
#include "src/runtime.h"


volatile sig_atomic_t stop;

void signal(int signum) {
    printf("\nterminating program with signal: %d\n", signum);
    stop = 1;
}

// snapshot of the system at clock 0
void initialize(Runtime *runtime) {
    runtime->clock_next_departure = 1.0e+30;

    runtime->arrival_event = next_arrival(runtime->config, runtime->state);

    runtime->num_generated = 0;

    /* system state */
    // simulator clock
    runtime->clock_system = 0.0;
    // time of last event
    runtime->clock_last_event = 0.0;

    // statistical counters
    runtime->num_delayed = 0;
    runtime->total_of_delays = 0.0;
    runtime->area_num_in_q = 0.0;
    runtime->area_server_status = 0.0;
}


void update_time_avg_stats(Runtime *runtime) {
    float clock_between_last_event = runtime->clock_system - runtime->clock_last_event;
    // TODO
    runtime->area_num_in_q += clock_between_last_event * (float) (runtime->fifo.size());

    runtime->area_server_status += float(runtime->server_status) * clock_between_last_event;
}

void arrive(Runtime *runtime) {
    if (IDLE == runtime->server_status) {
        // since no Event in Q, this Event can be handled immediately
        // and departure clock can be calculated
        runtime->clock_next_departure = cal_departure_clock(runtime->clock_system, runtime->arrival_event->cost);
        // since server is IDLE, this event will be served without pushing in fifo, update num delayed
        runtime->num_delayed += 1;
        // update server status
        runtime->server_status = BUSY;
    } else {
        push(runtime, runtime->arrival_event);
    }
}

void depart(Runtime *runtime) {
    Event *arrival_head = pop(runtime);
    if (arrival_head != nullptr) {
        runtime->num_delayed += 1;
        float event_delay = runtime->clock_system - arrival_head->clock;
        runtime->total_of_delays += event_delay;
        runtime->clock_next_departure = cal_departure_clock(runtime->clock_system, arrival_head->cost);
    } else {
        runtime->clock_next_departure = 1.0e+30;
        runtime->server_status = IDLE;
    }
}

void report(Runtime *runtime) {
    float drop_rate = (float) (runtime->num_generated - runtime->num_delayed) / (float) runtime->num_generated;
    printf("num_generated: %d, num_delayed: %d, drop rate: %f, total_of_delays: %f, area_num_in_q: %f, area_server_status: %f \n",
           runtime->num_generated, runtime->num_delayed, drop_rate,
           runtime->total_of_delays, runtime->area_num_in_q, runtime->area_server_status);
}


int main(int argc, char *argv[]) {
    signal(SIGINT, signal);

    Runtime runtime;

    // get command line arguments
    char *config_file = nullptr;
    int ch;
    while ((ch = getopt(argc, argv, "c:")) != -1) {
        if (ch == 'c') {
            config_file = optarg;
            break;
        }
    }
    if (nullptr == config_file || strlen(config_file) <= 0) {
        printf("please specify config file with '-c' command\n");
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

    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    srand(tv.tv_sec ^ tv.tv_usec ^ getpid());

    // TODO init source state
    init_source_state(runtime.config, runtime.state);

    init_queue(&runtime);

    // TODO
    initialize(&runtime);

    while (!stop) {
        if (runtime.arrival_event->clock < runtime.clock_next_departure) {
            runtime.next_event_type = ARRIVAL;
            runtime.clock_system = runtime.arrival_event->clock;
            runtime.num_generated++;
        } else {
            runtime.next_event_type = DEPARTURE;
            runtime.clock_system = runtime.clock_next_departure;
        }

        update_time_avg_stats(&runtime);

        switch (runtime.next_event_type) {
            case ARRIVAL:
                arrive(&runtime);
                runtime.arrival_event = next_arrival(runtime.config, runtime.state);
                break;
            case DEPARTURE:
                depart(&runtime);
                break;
        }

        runtime.clock_last_event = runtime.clock_system;
    }

    report(&runtime);

    return EXIT_SUCCESS;
}