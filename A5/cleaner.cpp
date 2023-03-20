#include "headers.h"
// runner function for cleaner threads
void *cleaner_routine(void *arg)
{
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
                hotel.rooms[i].occupancy = 0;
                hotel.rooms[i].current_guest = 0;
            }
            pthread_mutex_unlock(&mutex_hotel);
            sleep(cleantime);
        }
        pthread_mutex_lock(&total_occupancy);
        if(i==N && hotel.tot_occupancy != 0)
        {
            hotel.tot_occupancy = 0;
        }
        pthread_mutex_unlock(&total_occupancy);
    }

    pthread_exit(0);
}