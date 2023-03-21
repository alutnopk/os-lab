#include "headers.h"
// runner function for cleaner threads
void* cleaner_routine(void* arg)
{
    // // int idx = *(int*)arg;
    // pthread_mutex_lock(&mutex_hotel);
    // while(hotel.tot_occupancy < 2*N)
    //     pthread_cond_wait(&cond_occupancy, &mutex_hotel);
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
    int idx = *(int *)arg;
    while (1)
    {
        int i=0;
        int cleantime = 0;
        pthread_mutex_lock(&total_occupancy);
        while (hotel.tot_occupancy < 2 * N)
            pthread_cond_wait(&cond_occupancy, &total_occupancy);
        pthread_mutex_unlock(&total_occupancy);
        // Cleaner apparently kicks out the guests at this point LOL
        cout << "Hotel cleanup begins. Evicting current guests\n"<< endl;

        // TODO
        // evict all guests
        for (i = 0; i < N; i++)
        {
            pthread_mutex_lock(&mutex_hotel);
            if (hotel.rooms[i].occupancy == 2)
            {
                cleantime = hotel.rooms[i].time;
                hotel.rooms[i].time = 0;
                hotel.rooms[i].priority = 0;
                hotel.rooms[i].occupancy = 0;
                hotel.rooms[i].current_guest = NULL;
            }
            pthread_mutex_unlock(&mutex_hotel);
            sleep(cleantime);
        }
        pthread_mutex_lock(&total_occupancy);
        if(i == N && hotel.tot_occupancy != 0)
        {
            hotel.tot_occupancy = 0;
        }
        pthread_mutex_unlock(&total_occupancy);
    }
    pthread_exit(0);
}