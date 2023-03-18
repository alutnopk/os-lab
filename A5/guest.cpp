#include "headers.h"
// runner function for guest threads
void* guest_routine(void* arg)
{
    // stringstream sstr;
    signal(SIGUSR1, guest_sighandler);
    int pr = *(int*)arg; // TODO: make this the priority

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dsleep(10, 20);
    uniform_int_distribution<> dstay(10, 30);
    for(;1;)
    {
        sleep(dsleep(gen));
        int semval = -1;
        // TODO:
        // if(sem_getvalue(&sem_guest, &semval) == -1) { cerr<<"sem_getvalue failed in guest"<<endl; }
        // if(semval == 0) // now look for lower priority guests
        // {
        //     pthread_mutex_lock(&mutex_hotel);
        //     evict(hotel, N, pthread_self(), pr);
        //     pthread_kill(target, SIGUSR1);
        //     pthread_mutex_unlock(&mutex_hotel);
        // }

        // acquire room (occupancy must NOT exceed 1)
        try
        {
            if(sem_wait(&sem_guest) == -1) { cerr<<"sem_wait failed in guest"<<endl; }

            pthread_mutex_lock(&mutex_hotel);
            book(hotel, N, pthread_self(), pr);
            pthread_mutex_unlock(&mutex_hotel);
            pthread_cond_signal(&cond_occupancy);
            cout<<endl<<"Guest of priority "+to_string(pr)+" has occupied a room"<<endl;

            sleep(dstay(gen));

            pthread_mutex_lock(&mutex_hotel);
            vacate(hotel, N, pthread_self());
            pthread_mutex_unlock(&mutex_hotel);

            if(sem_post(&sem_guest) == -1) { cerr<<"sem_post failed in guest"<<endl; }
            cout<<endl<<"Guest of priority "+to_string(pr)+" has left their room"<<endl;
        }
        catch(runtime_error &e) { cerr<<e.what()<<endl; exit(EXIT_FAILURE); }
    }
    pthread_exit(0);
}
// Signal handler used for eviction
void guest_sighandler(int signum)
{
    assert(signum == SIGUSR1);
    cout<<"A guest has been evicted"<<endl;
}