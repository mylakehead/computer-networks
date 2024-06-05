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

    /* system state */
    // simulator clock
    runtime->clock_system = 0.0;
    // time of last event
    runtime->clock_last_event = 0.0;

    // statistical counters
    runtime->total_num_arrived_in_system = 0;
    runtime->total_num_arrived_in_q = 0;
    runtime->total_num_without_in_q = 0;

    runtime->total_num_dropped_in_q = 0;
    runtime->total_num_pushed_in_q = 0;
    runtime->total_area_num_in_q = 0.0;

    runtime->total_num_delayed_in_server = 0;
    runtime->total_response_delays_in_server = 0.0;
    runtime->area_server_status = 0.0;
}

void arrive(Runtime *runtime) {
    if (IDLE == runtime->server_status) {
        // since no Event in Q, this Event can be handled immediately
        // and departure clock can be calculated
        runtime->clock_next_departure = cal_departure_clock(runtime->clock_system, runtime->arrival_event->cost);
        // since server is IDLE, this event will be served without pushing in q, update num delayed
        runtime->total_num_without_in_q++;
        runtime->total_num_delayed_in_server++;
        runtime->total_response_delays_in_server += runtime->arrival_event->cost;
        // update server status
        runtime->server_status = BUSY;
    } else {
        push(runtime, runtime->arrival_event);
    }
}

void depart(Runtime *runtime) {
    Event *arrival_head = pop(runtime);
    if (arrival_head != nullptr) {
        runtime->total_num_delayed_in_server++;
        runtime->total_response_delays_in_server += runtime->clock_system - arrival_head->clock;

        runtime->clock_next_departure = cal_departure_clock(runtime->clock_system, arrival_head->cost);
        free(arrival_head);
    } else {
        runtime->clock_next_departure = 1.0e+30;
        runtime->server_status = IDLE;
    }
}

void update_time_avg_stats(Runtime *runtime) {
    double clock_between_last_event = runtime->clock_system - runtime->clock_last_event;
    switch (runtime->config->queue_type) {
        case FIFO: {
            runtime->total_area_num_in_q += clock_between_last_event * (double) (runtime->fifo.size());
            break;
        }
        case SPQ: {
            for (int i = 0; i < runtime->area_num_in_sub_spq.size(); i++) {
                runtime->area_num_in_sub_spq[i] += clock_between_last_event * (double) (runtime->spq[i].q.size());
                runtime->total_area_num_in_q += clock_between_last_event * (double) (runtime->spq[i].q.size());
            }
            break;
        }
        case WFQ: {
            for (int i = 0; i < runtime->area_num_in_sub_wfq.size(); i++) {
                runtime->area_num_in_sub_wfq[i] += clock_between_last_event * (double) (runtime->wfq.subs[i].q.size());
                runtime->total_area_num_in_q += clock_between_last_event * (double) (runtime->wfq.subs[i].q.size());
            }
            break;
        }
    }

    runtime->area_server_status += double(runtime->server_status) * clock_between_last_event;
}

void report(Runtime *runtime) {
    switch (runtime->config->queue_type) {
        case FIFO: {
            printf("FIFO statistics\n");
            break;
        }
        case SPQ: {
            printf("SPQ statistics\n");
            for (int i = 0; i < runtime->num_arrived_in_sub_spq.size(); i++) {
                printf("SPQ %d, arrived %d, dropped: %d, pushed: %d, area: %f\n", i + 1,
                       runtime->num_arrived_in_sub_spq[i],
                       runtime->num_dropped_in_sub_spq[i],
                       runtime->num_pushed_in_sub_spq[i],
                       runtime->area_num_in_sub_spq[i]);
            }
            break;
        }
        case WFQ: {
            printf("WFQ statistics\n");
            for (int i = 0; i < runtime->num_arrived_in_sub_wfq.size(); i++) {
                printf("WFQ %d, arrived %d, dropped: %d, pushed: %d, area: %f\n", i + 1,
                       runtime->num_arrived_in_sub_wfq[i],
                       runtime->num_dropped_in_sub_wfq[i],
                       runtime->num_pushed_in_sub_wfq[i],
                       runtime->area_num_in_sub_wfq[i]);
            }
            break;
        }
    }
    printf("\n");
    printf("total customers arrived in system: %d\n", runtime->total_num_arrived_in_system);
    printf("total customers without in q: %d\n", runtime->total_num_without_in_q);
    printf("total customers arrived in q: %d\n", runtime->total_num_arrived_in_q);
    printf("total customers dropped in q: %d\n", runtime->total_num_dropped_in_q);
    printf("total customers pushed in q: %d\n", runtime->total_num_pushed_in_q);
    printf("total customers delayed in server: %d\n", runtime->total_num_delayed_in_server);
    printf("total customers response delays in server: %f\n", runtime->total_response_delays_in_server);
    printf("total area of customers in q: %f\n", runtime->total_area_num_in_q);
    printf("\n");
    printf("average queuing delay of customers popped from q: %f\n",
           runtime->total_area_num_in_q / (double) runtime->total_num_pushed_in_q);
    printf("average queuing delay of all sent: %f\n",
           runtime->total_area_num_in_q / (double) runtime->total_num_delayed_in_server);
    printf("average response time: %f\n",
           runtime->total_response_delays_in_server / (double) runtime->total_num_delayed_in_server);
    printf("average packet blocking ratio: %f\n",
           runtime->total_num_dropped_in_q / (double) runtime->total_num_arrived_in_system);
    printf("average packet backlogged: %f\n",
           runtime->total_area_num_in_q / (double) runtime->clock_system); // ?
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

    init_source_state(runtime.config, runtime.state);

    init_queue(&runtime);

    initialize(&runtime);

    while (!stop || !empty(&runtime)) {
        if (stop) {
            runtime.next_event_type = DEPARTURE;
            runtime.clock_system = runtime.clock_next_departure;
        } else {
            if (runtime.arrival_event->clock < runtime.clock_next_departure) {
                runtime.next_event_type = ARRIVAL;
                runtime.clock_system = runtime.arrival_event->clock;
                runtime.total_num_arrived_in_system++;
            } else {
                runtime.next_event_type = DEPARTURE;
                runtime.clock_system = runtime.clock_next_departure;
            }
        }

        update_time_avg_stats(&runtime);

        switch (runtime.next_event_type) {
            case ARRIVAL:
                arrive(&runtime);
                runtime.arrival_event = next_arrival(runtime.config, runtime.state);
                // TODO remove
                switch (runtime.arrival_event->packet.t) {
                    case AUDIO: {
                        int i = 0;
                        break;
                    }
                    case VIDEO: {
                        int j = 0;
                        break;
                    }
                    case DATA: {
                        int t = 0;
                        break;
                    }
                }
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