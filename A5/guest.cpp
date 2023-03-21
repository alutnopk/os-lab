#include "headers.h"

// runner function for guest threads
void* guest_routine(void* arg)
{
    int pr = *(int*)arg;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dsleep(10, 20);
    uniform_int_distribution<> dstay(10, 30);
    // uniform_int_distribution<> dpr(1, Y);

    // int pr = dpr(gen);
    try
    {
        for(;1;)
        {
            int evict_target_idx = -1;
            int current_room_idx = -1;
            // guest sleeps before inquiring the hotel
            sleep(dsleep(gen));
            // guest is kept waiting outside when hotel is being cleaned
            pthread_mutex_lock(&total_occupancy);
            while(hotel.tot_occupancy == 2*N) // might have to be changed, considering that cleaners gradually change tot_occupancy
                pthread_cond_wait(&cond_guest_wait, &total_occupancy); // could there be a problem because of the mutex?
            pthread_mutex_unlock(&total_occupancy);
            // guest is let in, they inquire for a room
            if(sem_trywait(&sem_guest) == -1) // if hotel is fully occupied
            {
                cout<<"Hotel full. Looking for victims..."<<endl;
                pthread_mutex_lock(&mutex_hotel);
                evict_target_idx = find_lowerpr_guest(hotel, N, pr); // guest locates a victim to replace
                // how about we maintain the index of the lowest priority dude and just target them everytime
                // in which case the new visitor needs to only check if that target pr is lower than its current pr
                if(evict_target_idx == -1) // no guest is kickable
                {
                    // guest has no choice but to wait on the semaphore
                    cout<<"No lower priority guest exists"<<endl;
                    if(sem_wait(&sem_guest) == -1) { cerr<<"sem_wait failed in guest"<<endl; }
                    // guest has acquired the semaphore, pick a room and book it
                    pthread_mutex_lock(&mutex_hotel);
                    current_room_idx = book(hotel, N, pthread_self(), pr); // find first empty room by index and occupy it
                    pthread_mutex_unlock(&mutex_hotel);
                    pthread_cond_signal(&cond_occupancy); // what if 2N is reached here itself? cleaners MUST kick the guests out in that case
                    cout<<endl<<"Guest "<<pr<<" has occupied Room "<<current_room_idx<<endl;
                }
                else
                {
                    cout<<"Eviction target found at Room "<<evict_target_idx<<". Sending signal..."<<endl;
                    
                    pthread_mutex_lock(&mutex_evict[evict_target_idx]);
                    // guest replaces room credentials with self's
                    pthread_t target_tid = evict(hotel, N, pthread_self(), pr, evict_target_idx);
                    pthread_mutex_unlock(&mutex_evict[evict_target_idx]);
                    pthread_cond_signal(&cond_evict[evict_target_idx]);
                    // old guest's semaphore is triggered (DISAPPOINTMENT)
                    // if(sem_post(&sem_evict[evict_target_idx]) == -1) { cerr<<"sem_post on sem_evict failed"<<endl; }
                    current_room_idx = evict_target_idx;
                    cout<<"Guest "<<pr<<" has forcefully occupied Room "<<current_room_idx<<endl; // but has it though? there ought to be some sync
                }
                pthread_mutex_unlock(&mutex_hotel);
            }
            else // guest acquired the semaphore normally, just need to book a room now
            {
                cout<<"Hotel not full"<<endl;
                pthread_mutex_lock(&mutex_hotel);
                current_room_idx = book(hotel, N, pthread_self(), pr); // find first empty room by index and occupy it
                pthread_mutex_unlock(&mutex_hotel);
                pthread_cond_signal(&cond_occupancy); // what if 2N is reached here itself? cleaners MUST kick the guests out in that case
                cout<<endl<<"Guest of priority "<<pr<<" has occupied Room "<<current_room_idx<<endl;
            }

            struct timespec t;
            if(clock_gettime(CLOCK_REALTIME, &t) == -1) { cerr<<"Failure in clock_gettime() inside guest"<<endl; }
            t.tv_sec += dstay(gen);
            
            int evict_status=0;
            // cout<<"Guest "<<pr<<" executing sem_timedwait() while in Room "<<current_room_idx<<"..."<<endl;
            pthread_mutex_lock(&mutex_evict[current_room_idx]);
            while(pthread_equal(pthread_self(), hotel.rooms[current_room_idx].current_guest) && evict_status == 0)
                evict_status = pthread_cond_timedwait(&cond_evict[current_room_idx], &mutex_evict[current_room_idx], &t);
            pthread_mutex_unlock(&mutex_evict[current_room_idx]);

            // while ((evict_status = sem_timedwait(&sem_evict[current_room_idx], &t)) == -1 && errno == EINTR) (DISAPPOINTMENT)
            //    continue;

            // cout<<"Guest "<<pr<<" leaves sem_timedwait()"<<endl;

            if(evict_status == -1) // bruh what if it gets an evict message after it leaves sem_timedwait? lmao wow
            {
                if(errno == ETIMEDOUT) cout<<"Stay complete of Guest "<<pr<<" at Room "<<current_room_idx<<endl; else throw runtime_error("guest.cpp: sem_timedwait() did not time out correctly");
                // vacate normally
                pthread_mutex_lock(&mutex_hotel);
                vacate(hotel, N, pthread_self(), current_room_idx); // use current_room_idx to vacate
                pthread_mutex_unlock(&mutex_hotel);
                // release the semaphores
                if(sem_post(&sem_guest) == -1) { cerr<<"sem_post failed in guest"<<endl; }
                cout<<endl<<"Guest "<<pr<<" has left their room"<<endl;
            }
            else
            {
                cout<<"Evict signal received at Room "<<current_room_idx<<" where "<<pr<<" is residing"<<endl;
                // EVICTION PROCEDURE
                // ...do nothing?
                cout<<"Guest "<<pr<<" has received the boot from Room "<<current_room_idx<<endl;
            }   
        }
    }
    catch(exception &e) { cerr<<e.what()<<endl; exit(EXIT_FAILURE); }
    pthread_exit(0);
}