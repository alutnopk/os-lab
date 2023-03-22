#include "headers.h"
// runner function for guest threads
void* guest_routine(void* arg) // TODO: improve prints by specifying priorities and times stayed
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
                sem_wait(&sem_stdcout); cout<<"Guest "<<idx <<" with priority "<<pr<<" waiting on the barrier barr_guest"<<endl; sem_post(&sem_stdcout);
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
                sem_wait(&sem_stdcout); cout<<"Hotel full. Guest "<<idx <<" with priority "<<pr<<" looking for potential victims..."<<endl; sem_post(&sem_stdcout);
                pthread_mutex_lock(&mutex_hotel);
                evict_target_idx = find_lowerpr_guest(hotel, N, pr); // guest locates a victim to replace
                pthread_mutex_unlock(&mutex_hotel);
                // how about we maintain the index of the lowest priority dude and just target them everytime
                // in which case the new visitor needs to only check if that target pr is lower than its current pr
                if(evict_target_idx == -1) // no guest is kickable
                {
                    // guest has no choice but to wait on the semaphore
                    sem_wait(&sem_stdcout); cout<<"No lower priority guest exists. Guest "<<idx <<" with priority "<<pr<<" waiting for someone to vacate..."<<endl; sem_post(&sem_stdcout);
                    sem_wait(&sem_guest);
                    sem_wait(&sem_stdcout); cout<<"Guest "<<idx <<" with priority "<<pr<<" has entered the hotel now. Proceeding to book..."<<endl; sem_post(&sem_stdcout);
                    // guest has acquired the semaphore, pick a room and book it
                    pthread_mutex_lock(&mutex_hotel);
                    current_room_idx = book(hotel, N, pthread_self(), pr); // find first empty room by index and occupy it
                    pthread_mutex_unlock(&mutex_hotel);
                    if(current_room_idx == -1) { sem_post(&sem_guest); continue; }
                    // pthread_cond_broadcast(&cond_cleaner);
                    sem_wait(&sem_stdcout); cout<<endl<<"Guest "<<idx <<" with priority "<<pr<<" has occupied Room "<<current_room_idx<<endl; sem_post(&sem_stdcout);
                }
                else
                {
                    sem_wait(&sem_stdcout); cout<<"Eviction target found at Room "<<evict_target_idx<<". Sending signal..."<<endl; sem_post(&sem_stdcout);
                    
                    // guest replaces room credentials with self's
                    pthread_mutex_lock(&mutex_evict[evict_target_idx]);
                    evict(hotel, N, pthread_self(), pr, evict_target_idx);
                    pthread_mutex_unlock(&mutex_evict[evict_target_idx]);
                    pthread_cond_broadcast(&cond_evict[evict_target_idx]);
                    // pthread_cond_broadcast(&cond_cleaner);
                    // if(sem_post(&sem_evict[evict_target_idx]) == -1) { cerr<<"sem_post on sem_evict failed"<<endl; }
                    current_room_idx = evict_target_idx;
                    sem_wait(&sem_stdcout); cout<<"Guest "<<idx <<" with priority "<<pr<<" has forcefully occupied Room "<<current_room_idx<<endl; sem_post(&sem_stdcout);
                }
            }
            else // guest acquired the semaphore normally, just need to book a room now
            {
                
                pthread_mutex_lock(&mutex_hotel);
                current_room_idx = book(hotel, N, pthread_self(), pr); // find first empty room by index and occupy it
                pthread_mutex_unlock(&mutex_hotel);
                if(current_room_idx == -1) { sem_post(&sem_guest); continue; }
                sem_wait(&sem_stdcout); cout<<"Hotel not full. Guest "<<idx <<" with priority "<<pr<<" proceeding to book..."<<endl; sem_post(&sem_stdcout);
                // pthread_cond_broadcast(&cond_cleaner); // what if 2N is reached here itself? cleaners MUST kick the guests out in that case
                sem_wait(&sem_stdcout); cout<<"Guest "<<idx <<" with priority "<<pr<<" has occupied Room "<<current_room_idx<<endl; sem_post(&sem_stdcout);
            }

            struct timespec t;
            if(clock_gettime(CLOCK_REALTIME, &t) == -1) { cerr<<"Failure in clock_gettime() inside guest"<<endl; }
            t.tv_sec += dstay(gen);
            
            int evict_status=0;
            // cout<<"Guest "<<pr<<" executing sem_timedwait() while in Room "<<current_room_idx<<"..."<<endl;
            pthread_mutex_lock(&mutex_evict[current_room_idx]);
            auto start_time = chrono::high_resolution_clock::now();
            while(pthread_equal(pthread_self(), hotel.rooms[current_room_idx].current_guest) && evict_status != ETIMEDOUT)
                evict_status = pthread_cond_timedwait(&cond_evict[current_room_idx], &mutex_evict[current_room_idx], &t);
            auto end_time = chrono::high_resolution_clock::now();
            pthread_mutex_unlock(&mutex_evict[current_room_idx]);
            
            // while ((evict_status = sem_timedwait(&sem_evict[current_room_idx], &t)) == -1 && errno == EINTR)
            //    continue;

            // cout<<"Guest "<<pr<<" leaves sem_timedwait()"<<endl;
            stay_time = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();

            if(evict_status == ETIMEDOUT) // Time-out event
            {
                // if(errno == ETIMEDOUT) cout<<"Stay complete of Guest "<<pr<<" at Room "<<current_room_idx<<endl; else throw runtime_error("guest.cpp: sem_timedwait() did not time out correctly");
                sem_wait(&sem_stdcout); cout<<"Stay complete of Guest "<<idx <<" with priority "<<pr<<" at Room "<<current_room_idx<<endl; sem_post(&sem_stdcout);
                // vacate normally
                pthread_mutex_lock(&mutex_evict[current_room_idx]);
                vacate(hotel, N, pthread_self(), current_room_idx, stay_time);
                pthread_mutex_unlock(&mutex_evict[current_room_idx]);
                // release the semaphores
                sem_post(&sem_guest);
                sem_wait(&sem_stdcout); cout<<"Guest "<<idx <<" with priority "<<pr<<" has vacated their room"<<endl; sem_post(&sem_stdcout);
            }
            else if (evict_status == 0) // Eviction signal
            {
                sem_wait(&sem_stdcout); cout<<"Evict signal received at Room "<<current_room_idx<<" where Guest "<<idx <<" with priority "<<pr<<" is residing"<<endl; sem_post(&sem_stdcout);
                // update hotel.rooms[i].time with actual stay time
                pthread_mutex_lock(&mutex_evict[current_room_idx]);
                hotel.rooms[current_room_idx].time += stay_time;
                pthread_mutex_unlock(&mutex_evict[current_room_idx]);

                sem_wait(&sem_stdcout); cout<<"Guest "<<idx <<" with priority "<<pr<<" has received the yeet from Room "<<current_room_idx<<endl; sem_post(&sem_stdcout);
            }   
            else throw runtime_error("Unforeseen error in pthread_cond_timedwait()");
        }
    }
    catch(exception &e) { cerr<<e.what()<<endl; exit(EXIT_FAILURE); }
    pthread_exit(0);
}