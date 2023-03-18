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
#include <signal.h>
#include <assert.h>

using namespace std;

typedef struct _Room
{
    pthread_t current_guest;
    int priority;
    int occupancy;
} Room;

typedef struct _Hotel
{
    vector<Room> rooms;
    int tot_occupancy;
} Hotel;

extern long X, N, Y;

extern vector<pthread_t> cleaners;
extern Hotel hotel;
extern vector<pthread_t> guests;

extern sem_t sem_guest;
extern sem_t sem_cleaner;
extern pthread_mutex_t mutex_hotel;
extern pthread_cond_t cond_occupancy;

void init(Hotel &h, int n);
void book(Hotel &h, int n, pthread_t g, int pr);
void vacate(Hotel &h, int n, pthread_t g);
void evict(Hotel &h, int n, pthread_t g, int pr);

void guest_sighandler(int signum);
void* guest_routine(void*);
void* cleaner_routine(void*);

void parse_input(int argc, char** argv, long &X, long &N, long &Y);

#endif