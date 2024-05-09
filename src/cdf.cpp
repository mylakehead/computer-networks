#include <cmath>
#include <cstdlib>
#include <time.h>
#include <iostream>

#include "cdf.h"


#define N 1000

/*
 * Given average time of "on" and "off" state in 1 sec:
 * "on"  — average_on_time,
 * "off" — average_off_time
 * and
 * average_on_time + average_off_time = 1
 *
 * that is,
 * every average_off_time, there is an "on" signal, and
 * every average_on_time, there is an "off" signal.
 *
 * For the formula of Poisson Distribution:
 * λ_on = 1 / average_off_time (the average number of "on" signal per sec)
 * λ_off = 1 / average_on_time (the average number of "off" signal per sec)
 *
 * For Exponential distribution, if the initial state is "on",
 * to calculate the probability of the next 1 sec is all "on",
 * that is in next 1 sec, no "off" happens.
 * P(X > t) = P(N(t) = 0) = (λt)^0 * e^(-λt)/0! = e^(-λt)
 * and λ = λ_off = 1 / average_on_time
 * that is
 * e^(-λ_off*t) = e^(-(1 / average_on_time)*t)
 *
 * For example, if average_on_time is 0.9sec,
 * the probability of the next 1 sec is all "on" is
 * e^(-(1/0.9)*1) = 0.3291929878
 * the probability of the next 0.5 sec is all "on" is
 * e^(-(1/0.9)*0.5) = 0.5737534208
 * the probability of the next 0.1 sec is all "on" is
 * e^(-(1/0.9)*0.1) = 0.8948393168
 *
 * To generate random "on" and "off" signals,
 */

int nextPoisson(double lambda, double t) {
    double L = exp(-lambda * t);
    double p = 1.0;
    int k = 0;

    do {
        k++;
        p *= (rand() % N) / (float) (N);
    } while (p > L);

    return k - 1;
}

// suppose average_on_time = 0.9 sec, average_off_time = 0.1 sec
// so λ_on = 1 / average_off_time = 1 / 0.1 = 10
// time unit: sec
// generate random "on" signal arrival time
void generateArrival() {
    int arrival_time[100] = {[0 ... 99] = -1};

    arrival_time[0] = nextPoisson(10, 1);
    for (int i = 1; i < 100; i++) {
        arrival_time[i] = arrival_time[i - 1] + nextPoisson(10, 1);
    }

    for (int i = 1; i < 100; i++) {
        std::cout << arrival_time[i] << std::endl;
    }
}

void generateArrivalOff() {
    int arrival_time[100] = {[0 ... 99] = -1};

    arrival_time[0] = nextPoisson((float) (1.0 / 0.9), 1);
    for (int i = 1; i < 100; i++) {
        arrival_time[i] = arrival_time[i - 1] + nextPoisson((float) (1.0 / 0.9), 1);
    }

    for (int i = 1; i < 100; i++) {
        std::cout << arrival_time[i] << std::endl;
    }
}