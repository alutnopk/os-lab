#ifndef __HEADERS_H
#define __HEADERS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <utility>
#include <queue>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <climits>
#include <chrono>
#include <random>
#include <signal.h>
#include <assert.h>
#include <errno.h>

using namespace std;

typedef struct _Room
{
    pthread_t current_guest;
    int priority;
    int occupancy;
    int time;
} Room;

typedef struct _Hotel
{
    vector<Room> rooms;
    int tot_occupancy;
} Hotel;

extern long X, N, Y;

extern vector<pair<pthread_t, int>> cleaners;
extern Hotel hotel;
// extern vector<pthread_t> guests;
extern vector<pair<pthread_t, pair<int, int>>> guests;

extern sem_t sem_guest; // tracks the number of vacant rooms
extern sem_t sem_stdcout; // to keep the output clean
extern pthread_mutex_t mutex_hotel; // for mutual exclusion while accessing the hotel structure

extern vector<pthread_mutex_t> mutex_evict;
extern vector<pthread_cond_t> cond_evict; // these two are because semaphores turned out to be a huge disappointment
extern vector<pthread_mutex_t> mutex_guest;
extern pthread_cond_t cond_guest; // for guests to wait until the hotel is fully cleaned
extern vector<pthread_mutex_t> mutex_cleaner;
extern pthread_cond_t cond_cleaner; // for cleaners to wait until the hotel is fully dirty
extern pthread_barrier_t barr_guest;
extern pthread_barrier_t barr_cleaner;

void init(Hotel &h, int n);
int book(Hotel &h, int n, pthread_t g, int pr);
void vacate(Hotel &h, int n, pthread_t g, int idx, int t);
int find_lowerpr_guest(Hotel &h, int n, int pr);
pthread_t evict(Hotel &h, int n, pthread_t g, int pr, int idx);
int clean_assign(Hotel &h, int n);

void* guest_routine(void* arg);
void* cleaner_routine(void* arg);

void parse_input(int argc, char** argv, long &X, long &N, long &Y);

#endif