//
// Created by Jayden Hong on 2024-06-03.
//

#include "queue.h"


void init_queue(Runtime *runtime) {
    switch (runtime->config->queue_type) {
        case FIFO:
            break;
        case SPQ:
            for (auto &i: runtime->config->spq.sizes) {
                SubSPQ sub = {
                        .size = i
                };
                runtime->spq.push_back(sub);

                // init statistics
                runtime->num_arrived_in_sub_spq.push_back(0);
                runtime->num_dropped_in_sub_spq.push_back(0);
                runtime->num_pushed_in_sub_spq.push_back(0);
                runtime->area_num_in_sub_spq.push_back(0.0);
            }
            break;
        case WFQ:
            runtime->wfq.virtual_time = 0.0;
            for (int i = 0; i < runtime->config->wfq.weights.size(); i++) {
                SubWFQ sub = {
                        .size = runtime->config->wfq.sizes[i],
                        .weight = runtime->config->wfq.weights[i],
                        .last_finish_time=0.0
                };
                runtime->wfq.subs.push_back(sub);
                // init statistics
                runtime->num_arrived_in_sub_wfq.push_back(0);
                runtime->num_dropped_in_sub_wfq.push_back(0);
                runtime->num_pushed_in_sub_wfq.push_back(0);
                runtime->area_num_in_sub_wfq.push_back(0.0);
            }
            break;
    }
}

void push(Runtime *runtime, Event *e) {
    runtime->total_num_arrived_in_q += 1;

    switch (runtime->config->queue_type) {
        case FIFO: {
            if (runtime->fifo.size() >= runtime->config->fifo.size) {
                runtime->total_num_dropped_in_q++;
            } else {
                runtime->fifo.push(e);
                runtime->total_num_pushed_in_q++;
            }
            break;
        }
        case SPQ: {
            unsigned long priority = e->packet.t;

            runtime->num_arrived_in_sub_spq[priority - 1]++;

            unsigned long s = runtime->spq[priority - 1].size;
            unsigned long l = runtime->spq[priority - 1].q.size();
            if (l >= s) {
                runtime->num_dropped_in_sub_spq[priority - 1]++;
                runtime->total_num_dropped_in_q++;
            } else {
                runtime->spq[priority - 1].q.push(e);
                runtime->num_pushed_in_sub_spq[priority - 1]++;
                runtime->total_num_pushed_in_q++;
            }
            break;
        }
        case WFQ: {
            unsigned long flow_type = e->packet.t;

            runtime->num_arrived_in_sub_wfq[flow_type - 1]++;

            unsigned long s = runtime->wfq.subs[flow_type - 1].size;
            unsigned long l = runtime->wfq.subs[flow_type - 1].q.size();
            if (l >= s) {
                runtime->num_dropped_in_sub_wfq[flow_type - 1]++;
                runtime->total_num_dropped_in_q++;
            } else {
                runtime->num_pushed_in_sub_wfq[flow_type - 1]++;
                runtime->total_num_pushed_in_q++;
                runtime->wfq.subs[flow_type - 1].q.push(e);
            }
            break;
        }
    }
}

bool empty(Runtime *runtime) {
    switch (runtime->config->queue_type) {
        case FIFO: {
            return runtime->fifo.empty();
        }
        case SPQ: {
            bool empty = true;
            for (auto &i: runtime->spq) {
                if (!i.q.empty()) {
                    empty = false;
                    break;
                }
            }
            return empty;
        }
        case WFQ: {
            bool empty = true;
            for (auto &i: runtime->wfq.subs) {
                if (!i.q.empty()) {
                    empty = false;
                    break;
                }
            }
            return empty;
        }
    }
}

Event *pop(Runtime *runtime) {
    switch (runtime->config->queue_type) {
        case FIFO: {
            if (runtime->fifo.empty()) {
                return nullptr;
            } else {
                Event *e = runtime->fifo.front();
                runtime->fifo.pop();
                return e;
            }
        }
        case SPQ: {
            Event *e = nullptr;
            for (auto &i: runtime->spq) {
                if (i.q.empty()) {
                    continue;
                } else {
                    e = i.q.front();
                    i.q.pop();
                    return e;
                }
            }
            return e;
        }
        case WFQ: {
            Event *res = nullptr;
            double min_finish_time = 1.0e+30;
            int flow_to_serve = -1;

            for (int i = 0; i < runtime->wfq.subs.size(); i++) {
                if (!runtime->wfq.subs[i].q.empty()) {
                    Event *e = runtime->wfq.subs[i].q.front();
                    double start_time = std::max(runtime->wfq.virtual_time, runtime->wfq.subs[i].last_finish_time);
                    double finish_time = start_time + e->packet.size / runtime->wfq.subs[i].weight;
                    if (finish_time < min_finish_time) {
                        min_finish_time = finish_time;
                        flow_to_serve = i;
                    }
                }
            }

            if (flow_to_serve != -1) {
                res = runtime->wfq.subs[flow_to_serve].q.front();
                runtime->wfq.subs[flow_to_serve].q.pop();
                runtime->wfq.virtual_time = min_finish_time;
                runtime->wfq.subs[flow_to_serve].last_finish_time = runtime->wfq.virtual_time;
            }

            return res;
        }
    }
}
