#include "headers.h"

void *guest_routine(void *arg)
{
    int idx = *((int *)arg);
    // cout << "Guest " << idx << " begins" << "\n";

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dsleep(10, 20);
    uniform_int_distribution<> dstay(10, 30);
    uniform_int_distribution<> dpriority(1, Y);

    int priority = dpriority(gen);
    // pid_t tid = pthread_self();
    // pid_t tid1 = pthread_self();

    // cout<<"THREAD ID:"<<tid<<endl;
    // cout<<idx<<" has priority "<<priority<<"\n";
    int i;
    int semval;
    for (; 1;)
    {
        sleep(dsleep(gen));

        // acquire room
        sem_getvalue(&sem_guest, &semval);

        if (semval == 0)
        {
            for (i = 0; i < N; i++)
            {
                if (hotel[i].priority < priority)
                {

                }
            }
        }

        else
        {
            if (sem_wait(&sem_guest) == -1)
            {
                cerr << "sem_wait failed in guest" << endl;
                exit(EXIT_FAILURE);
            }
            for (i = 0; i < N; i++)
            {
                if (hotel[i].occupancy < 2)
                {
                    hotel[i].guest_tid = pthread_self();
                    hotel[i].priority = priority;
                    hotel[i].occupancy++;
                    break;
                }
            }
            // if(i==N)
            // {
            //     for(i=0; i<N; i++)
            //     {
            //         if(hotel[i].priority < priority)
            //         {
            //             hotel[i].guest_tid = pthread_self();
            //             hotel[i].priority = priority;
            //             hotel[i].occupancy++;
            //             break;
            //         }
            //     }
            // }
            cout << idx << " has occupied a room" << endl;
            
            sleep(dstay(gen));
            
            hotel[i].guest_tid = -1;
            hotel[i].priority = -1;

            if (sem_post(&sem_guest) == -1)
            {
                cerr << "sem_post failed in guest" << endl;
                exit(EXIT_FAILURE);
            }
        }
    }

    pthread_exit(0);
}