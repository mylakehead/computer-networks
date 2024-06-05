#include <random>

#include "utils.h"

double exponential_random(double lambda) {
    double u;
    u = rand() / (RAND_MAX + 1.0);

    return -log(1 - u) / lambda;
}

// departure clock can only be calculated during Sever sending Event
// it's not a precalculated value, because the delay of Event in Q can not be predicted
double cal_departure_clock(double clock_system, double server_cost) {
    return clock_system + server_cost;
}

