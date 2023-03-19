#include "headers.h"
// runner function for cleaner threads
void* cleaner_routine(void* arg)
{
    int idx = *(int*)arg;
    pthread_mutex_lock(&mutex_hotel);
    while(hotel.tot_occupancy < 2*N)
        pthread_cond_wait(&cond_occupancy, &mutex_hotel);
    pthread_mutex_unlock(&mutex_hotel);
    // Cleaner apparently kicks out the guests at this point LOL
    cout<<"Hotel cleanup begins. Evicting current guests"<<endl;
    // TODO

    pthread_exit(0);
}