#include "headers.h"
sem_t sem_guest;
vector<Room> hotel;
pthread_mutex_t mutex_hotel = PTHREAD_MUTEX_INITIALIZER;

long X, N, Y;

int main(int argc, char** argv) // Legal argument range: 1 <= X < N < Y
{
    
    try{ parse_input(argc, argv, X, N, Y); } // reads and validates command line inputs
    catch(runtime_error& e) { cerr<<e.what()<<endl; return EXIT_FAILURE; }

    // initialization
    vector<pthread_t> cleaners(X);
    vector<Room> hotel(N);
    vector<pthread_t> guests(Y);
    
    for(int i=0; i<N; i++)
    {
        hotel[i].guest_tid = -1;
        hotel[i].priority = -1;
        hotel[i].occupancy = 0;
    }
    // thread creation
    
    for (int i = 0; i < Y; i++)
    {
        // TODO: figure out how to assign random distinct priorities (perhaps shuffle a list of numbers 1 to Y)
        int ret = pthread_create(&guests[i], NULL, guest_routine, (void*)&i);
        if(ret) { cerr<<"Failure in guest thread creation"<<endl; return EXIT_FAILURE; }
        cout<<"Guest thread "<<i<<" created"<<endl;
    }
    for (int i = 0; i < X; i++)
    {
        int ret = pthread_create(&cleaners[i], NULL, cleaner_routine, (void*)&i);
        if(ret) { cerr<<"Failure in cleaner thread creation"<<endl; return EXIT_FAILURE; }
        cout<<"Cleaner thread "<<i<<" created"<<endl;
    }

    // initialize semaphores here

    if(sem_init(&sem_guest, 0, N) == -1)
    { cerr<<"Failure in semaphore initialization"<<endl; return EXIT_FAILURE; }

    cout<<"Semaphore created"<<endl;

    // thread cleanup
    for (int i = 0; i < Y; i++)
    {
        int ret = pthread_join(guests[i], NULL);
        if(ret) { cerr<<"Failure in guest thread join"<<endl; return EXIT_FAILURE; }
        cout<<"Guest thread "<<i<<" exited"<<endl;
    }
    for (int i = 0; i < X; i++)
    {
        int ret = pthread_join(guests[i], NULL);
        if(ret) { cerr<<"Failure in cleaner thread join"<<endl; return EXIT_FAILURE; }
        cout<<"Cleaner thread "<<i<<" exited"<<endl;
    }

    return 0;
}

void parse_input(int argc, char **argv, long &X, long &N, long &Y)
{
    if (argc != 4)
        throw runtime_error("Expected usage: ./a.out <X> <N> <Y>");
    X = strtol(argv[1], NULL, 10);
    N = strtol(argv[2], NULL, 10);
    Y = strtol(argv[3], NULL, 10);
    // Error detection in input
    if (!X || !Y || !N)
        throw runtime_error("Inputs must be decimal integers");
    if (X == LONG_MIN || X == LONG_MAX || Y == LONG_MIN || Y == LONG_MAX || N == LONG_MIN || N == LONG_MAX)
        throw runtime_error("Input integer(s) out of range");
    if (!((X >= 1) && (N > X) && (Y > N)))
        throw runtime_error("Inputs must satisfy 1 <= X < N < Y\nExpected usage: ./<exec> <X> <N> <Y>");
}