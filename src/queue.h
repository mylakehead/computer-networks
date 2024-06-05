//
// Created by Jayden Hong on 2024-06-03.
//

#ifndef COMPUTER_NETWORKS_QUEUE_H
#define COMPUTER_NETWORKS_QUEUE_H

#include "config.h"
#include "source.h"
#include "runtime.h"


void init_queue(Runtime *runtime);

void push(Runtime *runtime, Event *e);

Event *pop(Runtime *runtime);

#endif //COMPUTER_NETWORKS_QUEUE_H
