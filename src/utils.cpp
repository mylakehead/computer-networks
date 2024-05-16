#include <random>

#include "utils.h"

float exponential_random(float lambda) {
    float u;
    u = rand() / (RAND_MAX + 1.0);

    return -log(1 - u) / lambda;
}

// departure clock can only be calculated during Sever sending Event
// it's not a precalculated value, because the delay of Event in Q can not be predicted
float cal_departure_clock(float clock_system, float server_cost) {
    return clock_system + server_cost;
}

