#include "headers.h"

int main(int argc, char** argv) // Legal argument range: 1 <= X < N < Y
{
    long X, N, Y;
    try{ parse_input(argc, argv, X, N, Y); } // reads and validates command line inputs
    catch(runtime_error& e) { cerr<<e.what()<<endl; return EXIT_FAILURE; }
    
    vector<pthread_t> guests(Y);
    vector<pthread_t> cleaners(X);
    
    // thread creation
    for(int i=0; i<Y; i++)
    {
        int ret = pthread_create(&guests[i], NULL, guest_routine, (void*)&i);
        if(ret) { cerr<<"Failure in guest thread creation"<<endl; return EXIT_FAILURE; }
    }
    for(int i=0; i<X; i++)
    {
        int ret = pthread_create(&cleaners[i], NULL, cleaner_routine, (void*)&i);
        if(ret) { cerr<<"Failure in cleaner thread creation"<<endl; return EXIT_FAILURE; }
    }

    // initialize semaphores here

    if(sem_init(&sem_guest, 0, N) == -1)
    { cerr<<"Failure in semaphore initialization"<<endl; return EXIT_FAILURE; }


    // thread cleanup
    for(int i=0; i<Y; i++)
    {
        int ret = pthread_join(guests[i], NULL);
        if(ret) { cerr<<"Failure in guest thread join"<<endl; return EXIT_FAILURE; }
    }
    for(int i=0; i<X; i++)
    {
        int ret = pthread_join(guests[i], NULL);
        if(ret) { cerr<<"Failure in cleaner thread join"<<endl; return EXIT_FAILURE; }
    }

    return 0;
}