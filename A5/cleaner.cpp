#include "headers.h"

<<<<<<< HEAD
int main()
{
    return 0;
=======
void* cleaner_routine(void* arg)
{
    int idx = *((int*)arg);
    cout<<"Cleaner "<<idx<<" begins"<<endl;
    
    pthread_exit(0);
>>>>>>> origin/pontu
}