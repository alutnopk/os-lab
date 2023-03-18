#include "headers.h"

void* cleaner_routine(void* arg)
{
    int idx = *(int*)arg;
    
    pthread_exit(0);
}