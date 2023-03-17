#ifndef __HEADERS_H
#define __HEADERS_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <queue>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string>
#include <cstring>
#include <sstream>
#include <climits>
#include <chrono>
#include <random>

using namespace std;


typedef struct _Room
{
    int guest;
    int occupancy;
} Room;

sem_t sem_guest;

void* guest_routine(void*);
void* cleaner_routine(void*);

void parse_input(int argc, char** argv, long &X, long &N, long &Y)
{
    if(argc != 4) throw runtime_error("Expected usage: ./<exec> <X> <N> <Y>");
    X = strtol(argv[1], NULL, 10);
    N = strtol(argv[2], NULL, 10);
    Y = strtol(argv[3], NULL, 10);
    // Error detection in input
    if(!X || !Y || !N )
        throw runtime_error("Inputs must be decimal integers");
    if(X == LONG_MIN || X == LONG_MAX || Y == LONG_MIN || Y == LONG_MAX || N == LONG_MIN || N == LONG_MAX)
        throw runtime_error("Input integer(s) out of range");
    if(!((X>=1) && (N>X) && (Y>N)))
        throw runtime_error("Inputs must satisfy 1 <= X < N < Y\nExpected usage: ./<exec> <X> <N> <Y>");
}

#endif