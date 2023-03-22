#include "headers.h"
double k = 0.5;
// runner function for cleaner threads
void* cleaner_routine(void* arg)
{
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
            sem_wait(&sem_stdcout); cout<<"Cleaners released"<<endl; sem_post(&sem_stdcout);
            for(;1;)
            {
                // pthread_mutex_lock(&mutex_cleaner[idx]);
                // while (hotel.tot_occupancy < 2*N)
                //     pthread_cond_wait(&cond_cleaner, &mutex_cleaner[idx]);
                // pthread_mutex_unlock(&mutex_cleaner[idx]);                
                
                // cleaner is assigned a random room
                sem_wait(&sem_hotel);
                room_idx = clean_assign(hotel, N);
                sem_post(&sem_hotel);

                if(room_idx >= 0) // room allotted
                {
                    // cleaner locks the room and cleans it
                    sem_wait(&sem_stdcout); cout<<"Cleaner "<<idx<<" allotted Room "<<room_idx<<endl; sem_post(&sem_stdcout);

                    pthread_mutex_lock(&mutex_evict[room_idx]);
                    hotel.rooms[room_idx].occupancy = -1;
                    clean_time = (int)(k*(hotel.rooms[room_idx].time));
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
            sem_wait(&sem_stdcout); cout<<"Cleaner "<<idx<<" waiting for all cleaners to be finished..."<<endl; sem_post(&sem_stdcout);
            pthread_barrier_wait(&barr_cleaner);

            if(hotel.tot_occupancy != 0) throw runtime_error("Cleanup was not fully done");
            // ensure the semaphore is N
            int semval;
            sem_getvalue(&sem_guest, &semval);
            for(;semval<N;)
            {
                sem_post(&sem_guest);
                sem_getvalue(&sem_guest, &semval);
            }
            pthread_cond_broadcast(&cond_guest);
            sem_wait(&sem_stdcout); cout<<"Cleanup finished. Opening the hotel..."<<endl; sem_post(&sem_stdcout);
        }
    }
    catch(exception &e) { cerr<<e.what()<<endl; exit(EXIT_FAILURE); }
    pthread_exit(0);
}