#include "headers.h"

long X, N, Y;

vector<pthread_t> cleaners;
Hotel hotel;
vector<pthread_t> guests;

sem_t sem_guest;
sem_t sem_cleaner;
pthread_mutex_t mutex_hotel = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_occupancy = PTHREAD_COND_INITIALIZER;

int main(int argc, char** argv) // Legal argument range: 1 <= X < N < Y
{
    try{ parse_input(argc, argv, X, N, Y); } // reads and validates command line inputs
    catch(runtime_error& e) { cerr<<e.what()<<endl; return EXIT_FAILURE; }

    // initialization
    cleaners = vector<pthread_t>(X);
    init(hotel, N);
    guests = vector<pthread_t>(Y);

    // thread creation
    for(int i=0; i<Y; i++)
    {
        // TODO: figure out how to assign random distinct priorities (perhaps shuffle a list of numbers 1 to Y)
        int ret = pthread_create(&guests[i], NULL, guest_routine, (void*)&i);
        if(ret) { cerr<<"Failure in guest thread creation"<<endl; }
        else cout<<"Guest thread "<<i<<" created"<<endl;
    }
    for(int i=0; i<X; i++)
    {
        int ret = pthread_create(&cleaners[i], NULL, cleaner_routine, (void*)&i);
        if(ret) { cerr<<"Failure in cleaner thread creation"<<endl; }
        else cout<<"Cleaner thread "<<i<<" created"<<endl;
    }

    // initialize semaphores here

    if(sem_init(&sem_guest, 0, N) == -1) { cerr<<"Failure in semaphore initialization"<<endl; return EXIT_FAILURE; }
    // if(sem_init(&sem_cleaner, 0, X) == -1) { cerr<<"Failure in semaphore initialization"<<endl; return EXIT_FAILURE; }

    cout<<"Semaphore created"<<endl;

    // thread cleanup
    for(int i=0; i<Y; i++)
    {
        int ret = pthread_join(guests[i], NULL);
        if(ret) { cerr<<"Failure in guest thread join"<<endl; }
        else cout<<"Guest thread "<<i<<" terminated"<<endl;
    }
    for(int i=0; i<X; i++)
    {
        int ret = pthread_join(guests[i], NULL);
        if(ret) { cerr<<"Failure in cleaner thread join"<<endl; }
        else cout<<"Cleaner thread "<<i<<" terminated"<<endl;
    }

    // destroy semaphores and mutexes
    if(sem_destroy(&sem_guest) == -1) { cerr<<"Failed to destroy semaphore"<<endl; }
    if(pthread_mutex_destroy(&mutex_hotel) == -1) { cerr<<"Failed to destroy mutex"<<endl; }
    if(pthread_cond_destroy(&cond_occupancy) == -1) { cerr<<"Failed to destroy condition variable"<<endl; }
    
    return 0;
}
// read X, N, Y and validate them
void parse_input(int argc, char** argv, long &X, long &N, long &Y)
{
    if(argc != 4) throw runtime_error("Expected usage: ./a.out <X> <N> <Y>");
    X = strtol(argv[1], NULL, 10);
    N = strtol(argv[2], NULL, 10);
    Y = strtol(argv[3], NULL, 10);
    // Error detection in input
    if(!X || !Y || !N )
        throw runtime_error("parse_input: Inputs must be decimal integers");
    if(X == LONG_MIN || X == LONG_MAX || Y == LONG_MIN || Y == LONG_MAX || N == LONG_MIN || N == LONG_MAX)
        throw runtime_error("parse_input: Input integer(s) out of range");
    if(!((X>=1) && (N>X) && (Y>N)))
        throw runtime_error("parse_input: Inputs must satisfy 1 <= X < N < Y\nExpected usage: ./<exec> <X> <N> <Y>");
}

// self-explanatory
void init(Hotel &h, int n)
{
    if(n <= 0) throw runtime_error("init: Cannot make hotel with n < 1");
    h.rooms = vector<Room>(n);
    h.tot_occupancy = 0;

    for(int i=0; i<n; i++)
    {
        hotel.rooms[i].current_guest = NULL;
        h.rooms[i].priority = -1;
        h.rooms[i].occupancy = 0;
    }
    // for(auto x: h.rooms)
    //     cout<<x.current_guest<<" "<<x.occupancy<<" "<<x.priority<<endl;
}
// locate empty room and allot to guest (only used when hotel isn't full)
void book(Hotel &h, int n, pthread_t g, int pr)
{
    if(h.tot_occupancy == 2*n) throw runtime_error("book: All rooms have occupancy 2");
    int i;
    for(i=0; i<n; i++)
    {
        if( h.rooms[i].priority == -1 && h.rooms[i].occupancy < 2 )
        break;
    }
    if(i == n) throw runtime_error("book: Cannot find an empty room");

    h.rooms[i].current_guest = g;
    h.rooms[i].priority = pr;
    h.rooms[i].occupancy += 1;
    h.tot_occupancy += 1;
}
// self-explanatory
void vacate(Hotel &h, int n, pthread_t g)
{
    int i;
    for(i=0; i<n; i++)
    {
        if(pthread_equal(g, h.rooms[i].current_guest))
            break;
    }
    if(i == n) throw runtime_error("vacate: Could not locate guest in hotel");

    h.rooms[i].current_guest = NULL;
    h.rooms[i].priority = -1;
}
// locate lower priority guest and yeet them out
void evict(Hotel &h, int n, pthread_t g, int pr)
{
    // TODO
}