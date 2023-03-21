#include "headers.h"

long X, N, Y;

vector<pthread_t> cleaners;
Hotel hotel;
vector<pair<pthread_t, int>> guests;

sem_t sem_guest;
vector<sem_t> sem_evict;
sem_t stdcout;
pthread_mutex_t mutex_hotel = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t total_occupancy = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_occupancy = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_guest_wait = PTHREAD_COND_INITIALIZER;

vector<pthread_mutex_t> mutex_evict; // these two are because semaphores turned out to be a huge disappointment
vector<pthread_cond_t> cond_evict;

int main(int argc, char** argv) // Legal argument range: 1 <= X < N < Y
{
    try{ parse_input(argc, argv, X, N, Y); } // reads and validates command line inputs
    catch(exception& e) { cerr<<e.what()<<endl; return EXIT_FAILURE; }
    // initialization
    cleaners = vector<pthread_t>(X);
    init(hotel, N);
    guests = vector<pair<pthread_t, int>>(Y); // can we use this somewhere else?
    sem_evict = vector<sem_t>(N); // disappointment
    
    mutex_evict = vector<pthread_mutex_t>(N, PTHREAD_MUTEX_INITIALIZER);
    cond_evict = vector<pthread_cond_t>(N, PTHREAD_COND_INITIALIZER);

    // assign non-repeating guest priorities
    random_device rd; mt19937 gen(rd());
    vector<int> fisher_yates(Y);
    for(int i=0; i<Y; i++) fisher_yates[i] = i+1;

    for (int i=Y-1; i>=1; i--)
    {
        uniform_int_distribution<int> dpr(0, i);
        int j = dpr(gen);
        int temp = fisher_yates[i];
        fisher_yates[i] = fisher_yates[j];
        fisher_yates[j] = temp;
    }

    for(int i=0; i<Y; i++) { guests[i].second = fisher_yates[i]; }

    cout<<"Guest priorities: "<<endl;
    for(int i=0; i<Y; i++) { cout<<guests[i].second<<endl; }

    // signal handler and unmasking
    // signal(SIGUSR1, guest_sighandler);
    // sigset_t signal_mask;
    // sigemptyset(&signal_mask);  // Initialize the signal mask to empty
    // sigaddset(&signal_mask, SIGUSR1);  // Add SIGUSR1 to the signal mask
    // if(pthread_sigmask(SIG_UNBLOCK, &signal_mask, NULL) == -1) { cerr<<"Failure in removal of SIGUSR1 from sigmask"<<endl; }  // Unblock SIGUSR1 for all threads

    // struct sigaction sa;
    // sa.sa_handler = guest_sighandler;
    // sigemptyset(&sa.sa_mask);
    // sa.sa_flags = 0;
    // if (sigaction(SIGUSR1, &sa, NULL) == -1)
    //     { cerr<<"Signal handler setup failure"<<endl; }
    
    // thread creation
    for(int i=0; i<Y; i++)
    {
        int ret = pthread_create(&(guests[i].first), NULL, guest_routine, (void*)&(guests[i].second));
        if(ret) { cerr<<"Failure in guest thread creation"<<endl; }
        else cout<<"Guest thread "<<i<<" created"<<endl;
    }
    // for(int i=0; i<X; i++)
    // {
    //     int ret = pthread_create(&cleaners[i], NULL, cleaner_routine, (void*)NULL);
    //     if(ret) { cerr<<"Failure in cleaner thread creation"<<endl; }
    //     else cout<<"Cleaner thread "<<i<<" created"<<endl;
    // }

    // initialize semaphores here
    sem_init(&sem_guest, 0, N);
    for(int i=0; i<N; i++)
    {
        if(sem_init(&sem_evict[i], 0, 0) == -1) { cerr<<"Failure in semaphore initialization"<<endl; }
    }
    sem_init(&stdcout, 0, 1);
    cout<<"Semaphores created"<<endl;

    // thread cleanup
    for(int i=0; i<Y; i++)
    {
        int ret = pthread_join(guests[i].first, NULL);
        if(ret) { cerr<<"Failure in guest thread join"<<endl; }
        else cout<<"Guest thread "<<i<<" terminated"<<endl;
    }
    // for(int i=0; i<X; i++)
    // {
    //     int ret = pthread_join(guests[i].first, NULL);
    //     if(ret) { cerr<<"Failure in cleaner thread join"<<endl; }
    //     else cout<<"Cleaner thread "<<i<<" terminated"<<endl;
    // }

    // destroy semaphores and mutexes
    // TODO: signal handler for clean termination
    if(sem_destroy(&sem_guest) == -1) { cerr<<"Failed to destroy semaphore"<<endl; }
    for(int i=0; i<N; i++)
    {
        if(sem_destroy(&sem_evict[i]) == -1) { cerr<<"Failed to destroy semaphore"<<endl; }
    }
    if(sem_destroy(&stdcout) == -1) { cerr<<"Failed to destroy semaphore"<<endl; }

    if(pthread_mutex_destroy(&mutex_hotel) == -1) { cerr<<"Failed to destroy mutex"<<endl; }
    if(pthread_mutex_destroy(&total_occupancy) == -1) { cerr<<"Failed to destroy mutex"<<endl; }
    if(pthread_cond_destroy(&cond_occupancy) == -1) { cerr<<"Failed to destroy condition variable"<<endl; }
    if(pthread_cond_destroy(&cond_guest_wait) == -1) { cerr<<"Failed to destroy condition variable"<<endl; }

    for(int i=0; i<N; i++)
    {
        pthread_mutex_destroy(&mutex_evict[i]);
        pthread_cond_destroy(&cond_evict[i]);
    }
    
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
    if(n < 2) throw runtime_error("init: Cannot make hotel with n < 2");
    h.rooms = vector<Room>(n);
    h.tot_occupancy = 0;

    for(int i=0; i<n; i++)
    {
        hotel.rooms[i].current_guest = NULL;
        h.rooms[i].priority = -1;
        h.rooms[i].occupancy = 0;
        h.rooms[i].time = 0;
    }
    // for(auto x: h.rooms)
    //     cout<<x.current_guest<<" "<<x.occupancy<<" "<<x.priority<<endl;
}
// locate empty room and allot to guest. Only to be used when hotel isn't full
int book(Hotel &h, int n, pthread_t g, int pr)
{
    if(h.tot_occupancy >= 2*n) throw runtime_error("book: All rooms have occupancy 2 (or more)");
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

    return i;
}
// self-explanatory
void vacate(Hotel &h, int n, pthread_t g, int idx)
{
    // int i;
    // for(i=0; i<n; i++)
    // {
    //     if(pthread_equal(g, h.rooms[i].current_guest))
    //         break;
    // }
    // if(i == n) throw runtime_error("vacate: Could not locate guest in hotel");
    int i = idx;
    if(pthread_equal(h.rooms[i].current_guest, g))
    {
        h.rooms[i].current_guest = NULL;
        h.rooms[i].priority = -1;
    }
    else throw runtime_error("vacate: Guest doesn't match");

}
// find the room with the lower priority guest. Only to be used when hotel is fully occupied
int find_lowerpr_guest(Hotel &h, int n, int pr)
{
    int i, flag=-1;
    for(i=0; i<n; i++)
    {
        if(h.rooms[i].priority == -1) throw runtime_error("find_lowerpr_guest: Function called despite presence of empty room(s)");
        if(h.rooms[i].priority < pr && h.rooms[i].occupancy <= 1 )
        {
            flag = i;
            break;
        }
    }
    return flag;
}
// update room credentials with new guest. Only to be used on an occupied room
pthread_t evict(Hotel &h, int n, pthread_t g, int pr, int idx)
{
    if(h.rooms[idx].priority == -1) throw runtime_error("evict: Room is already empty");
    pthread_t prev_guest = h.rooms[idx].current_guest;
    h.rooms[idx].current_guest = g;
    h.rooms[idx].priority = pr;
    h.rooms[idx].occupancy += 1;
    h.tot_occupancy += 1;
    return prev_guest;
}