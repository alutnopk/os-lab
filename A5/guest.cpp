#include "headers.h"

void *guest_routine(void *arg)
{
    int idx = *((int *)arg);
    cout << "Guest " << idx << " begins" << endl;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dsleep(10, 20);
    uniform_int_distribution<> dstay(10, 30);
    for (; 1;)
    {
        sleep(dsleep(gen));

        // acquire room
        if (sem_wait(&sem_guest) == -1)
        {
            cerr << "sem_wait failed in guest" << endl;
            exit(EXIT_FAILURE);
        }
        cout << idx << " has occupied a room" << endl;
        sleep(dstay(gen));
        if (sem_post(&sem_guest) == -1)
        {
            cerr << "sem_post failed in guest" << endl;
            exit(EXIT_FAILURE);
        }
    }

    pthread_exit(0);
}