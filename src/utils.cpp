#include <random>

#include "utils.h"

float exponential_random(float lambda) {
    float u;
    u = rand() / (RAND_MAX + 1.0);

    return -log(1 - u) / lambda;
}

