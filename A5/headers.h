#ifndef __HEADERS_H
#define __HEADERS_H

#include <iostream>
#include <fstream>
<<<<<<< HEAD
#include <algorithm>
#include <queue>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string>
#include <cstring>
#include <sstream>
=======
#include <sstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <queue>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
>>>>>>> origin/pontu
#include <climits>
#include <chrono>
#include <random>

using namespace std;

<<<<<<< HEAD

=======
extern sem_t sem_guest;
typedef struct _Room
{
    int guest;
    int occupancy;
} Room;

void* guest_routine(void*);
void* cleaner_routine(void*);
void parse_input(int argc, char** argv, long &X, long &N, long &Y);
>>>>>>> origin/pontu

#endif