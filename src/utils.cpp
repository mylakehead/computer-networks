#include <random>

#include "utils.h"

double exponential_random(double lambda) {
    double u;
    u = rand() / (RAND_MAX + 1.0);

    return -log(1 - u) / lambda;
}

