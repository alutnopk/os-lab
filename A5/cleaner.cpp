#include "headers.h"

void* cleaner_routine(void* arg)
{
    int idx = *((int*)arg);
    cout<<"Cleaner "<<idx<<" begins"<<endl;
    
    pthread_exit(0);
}