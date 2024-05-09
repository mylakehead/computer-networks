#include <iostream>

#include "src/cdf.h"

// Driver Code
int main() {
    srand((unsigned) time(NULL));

    std::cout << "on:" << std::endl;

    generateArrival();

    std::cout << "off:" << std::endl;

    generateArrivalOff();

    return 1;
}