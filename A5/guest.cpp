#include "headers.h"
// runner function for guest threads
void* guest_routine(void* arg)
{
    pair<int, int>* p = reinterpret_cast<pair<int, int>*>(arg);
    int idx = p->first;
    int pr = p->second;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dsleep(10, 20);
    uniform_int_distribution<> dstay(10, 30);
    try
    {
        for(;1;)
        {
            int evict_target_idx = -1;
            int current_room_idx = -1;
            int stay_time = 0;
            // guest is kept waiting outside when hotel is being cleaned
            if(hotel.tot_occupancy == 2*N)
            {
                cout<<"Guest "<<pr<<" waiting on the barrier barr_guest"<<endl;
                pthread_barrier_wait(&barr_guest);
            }

            pthread_mutex_lock(&mutex_guest[idx]);
            while(hotel.tot_occupancy == 2*N) // might have to be changed, considering that cleaners gradually change tot_occupancy
                pthread_cond_wait(&cond_guest, &mutex_guest[idx]); // could there be a problem because of the mutex?
            pthread_mutex_unlock(&mutex_guest[idx]);
            // guest sleeps before inquiring the hotel
            sleep(dsleep(gen));
            // guest is let in, they inquire for a room
            if(sem_trywait(&sem_guest) == -1) // if hotel is fully occupied
            {
                cout<<"Hotel full. Guest "<<pr<<" looking for potential victims..."<<endl;
                pthread_mutex_lock(&mutex_hotel);
                evict_target_idx = find_lowerpr_guest(hotel, N, pr); // guest locates a victim to replace
                pthread_mutex_unlock(&mutex_hotel);
                // how about we maintain the index of the lowest priority dude and just target them everytime
                // in which case the new visitor needs to only check if that target pr is lower than its current pr
                if(evict_target_idx == -1) // no guest is kickable
                {
                    // guest has no choice but to wait on the semaphore
                    cout<<"No lower priority guest exists. Guest "<<pr<<" waiting for someone to vacate..."<<endl;
                    if(sem_wait(&sem_guest) == -1) { cerr<<"sem_wait failed in guest"<<endl; }
                    cout<<"Guest "<<pr<<" has entered the hotel now. Proceeding to book..."<<endl;
                    // guest has acquired the semaphore, pick a room and book it
                    pthread_mutex_lock(&mutex_hotel);
                    current_room_idx = book(hotel, N, pthread_self(), pr); // find first empty room by index and occupy it
                    pthread_mutex_unlock(&mutex_hotel);
                    if(current_room_idx == -1) continue;
                    pthread_cond_broadcast(&cond_cleaner);
                    cout<<endl<<"Guest "<<pr<<" has occupied Room "<<current_room_idx<<endl;
                }
                else
                {
                    cout<<"Eviction target found at Room "<<evict_target_idx<<". Sending signal..."<<endl;
                    
                    // guest replaces room credentials with self's
                    pthread_mutex_lock(&mutex_evict[evict_target_idx]);
                    pthread_t target_tid = evict(hotel, N, pthread_self(), pr, evict_target_idx); // TODO
                    pthread_mutex_unlock(&mutex_evict[evict_target_idx]);
                    pthread_cond_broadcast(&cond_evict[evict_target_idx]);
                    // if(sem_post(&sem_evict[evict_target_idx]) == -1) { cerr<<"sem_post on sem_evict failed"<<endl; }
                    current_room_idx = evict_target_idx;
                    cout<<"Guest "<<pr<<" has forcefully occupied Room "<<current_room_idx<<endl; // but has it though? there ought to be some sync
                }
            }
            else // guest acquired the semaphore normally, just need to book a room now
            {
                cout<<"Hotel not full. Guest "<<pr<<" proceeding to book..."<<endl;
                pthread_mutex_lock(&mutex_hotel);
                current_room_idx = book(hotel, N, pthread_self(), pr); // find first empty room by index and occupy it
                pthread_mutex_unlock(&mutex_hotel);
                if(current_room_idx == -1) continue;
                pthread_cond_broadcast(&cond_cleaner); // what if 2N is reached here itself? cleaners MUST kick the guests out in that case
                cout<<"Guest "<<pr<<" has occupied Room "<<current_room_idx<<endl;
            }

            struct timespec t;
            if(clock_gettime(CLOCK_REALTIME, &t) == -1) { cerr<<"Failure in clock_gettime() inside guest"<<endl; }
            t.tv_sec += dstay(gen);
            
            int evict_status=0;
            // cout<<"Guest "<<pr<<" executing sem_timedwait() while in Room "<<current_room_idx<<"..."<<endl;
            pthread_mutex_lock(&mutex_evict[current_room_idx]);
            auto start_time = chrono::high_resolution_clock::now();
            while(pthread_equal(pthread_self(), hotel.rooms[current_room_idx].current_guest) && evict_status != 110)
                evict_status = pthread_cond_timedwait(&cond_evict[current_room_idx], &mutex_evict[current_room_idx], &t);
            auto end_time = chrono::high_resolution_clock::now();
            pthread_mutex_unlock(&mutex_evict[current_room_idx]);

            // while ((evict_status = sem_timedwait(&sem_evict[current_room_idx], &t)) == -1 && errno == EINTR)
            //    continue;

            // cout<<"Guest "<<pr<<" leaves sem_timedwait()"<<endl;
            stay_time = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();

            if(evict_status == 110) // Time-out event
            {
                // if(errno == ETIMEDOUT) cout<<"Stay complete of Guest "<<pr<<" at Room "<<current_room_idx<<endl; else throw runtime_error("guest.cpp: sem_timedwait() did not time out correctly");
                cout<<"Stay complete of Guest "<<pr<<" at Room "<<current_room_idx<<endl;
                // vacate normally
                pthread_mutex_lock(&mutex_evict[current_room_idx]);
                vacate(hotel, N, pthread_self(), current_room_idx, stay_time); // TODO
                pthread_mutex_unlock(&mutex_evict[current_room_idx]);
                // release the semaphores
                if(sem_post(&sem_guest) == -1) { cerr<<"sem_post failed in guest"<<endl; }
                cout<<"Guest "<<pr<<" has vacated their room"<<endl;
            }
            else if (evict_status == 0) // Eviction signal
            {
                cout<<"Evict signal received at Room "<<current_room_idx<<" where Guest "<<pr<<" is residing"<<endl;
                // update hotel.rooms[i].time with actual stay time
                pthread_mutex_lock(&mutex_evict[current_room_idx]);
                hotel.rooms[current_room_idx].time += stay_time;
                pthread_mutex_unlock(&mutex_evict[current_room_idx]);

                cout<<"Guest "<<pr<<" has received the yeet from Room "<<current_room_idx<<endl;
            }   
            else throw runtime_error("Unforeseen error in pthread_cond_timedwait()");
        }
    }
    catch(exception &e) { cerr<<e.what()<<endl; exit(EXIT_FAILURE); }
    pthread_exit(0);
}