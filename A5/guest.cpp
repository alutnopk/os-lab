#include "headers.h"

void* guest_routine(void* arg)
{
    int idx = *(int*)arg; // TODO: make this the priority

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dsleep(10, 20);
    uniform_int_distribution<> dstay(10, 30);
    for(;1;)
    {
        sleep(dsleep(gen));

        // acquire room (occupancy must NOT exceed 1)
        if(sem_wait(&sem_guest) == -1) { cerr<<"sem_wait failed in guest"<<endl; exit(EXIT_FAILURE); }


        cout<<"Guest "<<idx<<" has occupied a room\n";

        sleep(dstay(gen));

        if(sem_post(&sem_guest) == -1) { cerr<<"sem_post failed in guest"<<endl; exit(EXIT_FAILURE); }

        cout<<"Guest "<<idx<<" has left their room\n";

    }
    
    pthread_exit(0);
}