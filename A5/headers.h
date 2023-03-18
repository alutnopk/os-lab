#ifndef __HEADERS_H
#define __HEADERS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <queue>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <climits>
#include <chrono>
#include <random>

using namespace std;

extern sem_t sem_guest;
extern pthread_mutex_t mutex_hotel;
extern vector<Room> hotel;
typedef struct _Room
{
    int guest_tid;
    int priority;
    int occupancy;
} Room;

void* guest_routine(void*);
void* cleaner_routine(void*);
void parse_input(int argc, char** argv, long &X, long &N, long &Y);

#endif