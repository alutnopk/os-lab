#include "headers.h"
int k = 0.5;
// runner function for cleaner threads
void* cleaner_routine(void* arg)
{
    // // int idx = *(int*)arg;
    // pthread_mutex_lock(&mutex_hotel);
    // while(hotel.tot_occupancy < 2*N)
    //     pthread_cond_wait(&cond_cleaner, &mutex_hotel);
    // pthread_mutex_unlock(&mutex_hotel);
    // // Cleaner apparently kicks out the guests at this point LOL
    // cout<<"Hotel cleanup begins. Evicting current guests"<<endl;
    // // clean all the rooms
    // // TODO
    // for(int i=0; i<N; i++)
    // {
    //     if(hotel.rooms[i].occupancy == 2)
    //     {
    //         pthread_t target = evict(hotel, N, pthread_self(), 0, i);
    //         pthread_kill(target, SIGUSR1);
    //     }
    // }
    // pthread_exit(0);
    int* p = reinterpret_cast<int*>(arg);
    int idx = *p;
    try
    {
        for(;1;)
        {
            int room_idx = -1;
            int clean_time = 0;
            // cleaners accumulate at the barrier, waiting for all the guests to join
            pthread_barrier_wait(&barr_guest);
            cout<<"Cleaners unleashed"<<endl;
            for(;1;)
            {
                // pthread_mutex_lock(&mutex_cleaner[idx]);
                // while (hotel.tot_occupancy < 2*N)
                //     pthread_cond_wait(&cond_cleaner, &mutex_cleaner[idx]);
                // pthread_mutex_unlock(&mutex_cleaner[idx]);                
                
                // cleaner is assigned a random room
                room_idx = clean_assign(hotel, N); // what if all clean?

                if(room_idx >= 0) // room allotted
                {
                    // cleaner locks the room and cleans it
                    sem_wait(&sem_stdcout); cout<<"Cleaner "<<idx<<" allotted Room "<<room_idx<<endl; sem_post(&sem_stdcout);

                    pthread_mutex_lock(&mutex_evict[room_idx]);
                    hotel.rooms[room_idx].occupancy = -1;
                    clean_time = k*(hotel.rooms[room_idx].time);
                    // cleaner cleans
                    sem_wait(&sem_stdcout); cout<<"Cleaner "<<idx<<" busy cleaning..."<<endl; sem_post(&sem_stdcout);
                    sleep(clean_time);
                    hotel.rooms[room_idx].occupancy = 0;
                    hotel.rooms[room_idx].time = 0;
                    hotel.tot_occupancy -= 2;
                    pthread_mutex_unlock(&mutex_evict[room_idx]);
                    sem_wait(&sem_stdcout);cout<<"Cleaner "<<idx<<" finished cleaning"<<endl; sem_post(&sem_stdcout);
                }
                else
                    break;
            }
            sem_wait(&sem_stdcout); cout<<"Cleaner "<<idx<<" entering the second barrier..."<<endl; sem_post(&sem_stdcout);
            pthread_barrier_wait(&barr_cleaner);

            if(hotel.tot_occupancy != 0) throw runtime_error("Cleanup was not fully done");
            // set semaphore to N
            int semval;
            sem_getvalue(&sem_guest, &semval);
            for(;semval<N;)
            {
                sem_post(&sem_guest);
                sem_getvalue(&sem_guest, &semval);
            }
            pthread_cond_broadcast(&cond_guest);
            cout<<"Cleanup fin."<<endl;
        }
    }
    catch(exception &e) { cerr<<e.what()<<endl; exit(EXIT_FAILURE); }
    pthread_exit(0);
}