#include "headers.h"

long X, N, Y;

vector<pair<pthread_t, int>> cleaners;
Hotel hotel;
vector<pair<pthread_t, pair<int, int>>> guests;

sem_t sem_guest;
sem_t sem_stdcout;
sem_t sem_hotel;
pthread_cond_t cond_guest = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_cleaner = PTHREAD_COND_INITIALIZER;

vector<pthread_mutex_t> mutex_evict;
vector<pthread_cond_t> cond_evict;
vector<pthread_mutex_t> mutex_guest;
vector<pthread_mutex_t> mutex_cleaner;

pthread_barrier_t barr_guest;
pthread_barrier_t barr_cleaner;

int main(int argc, char** argv) // Legal argument range: 1 <= X < N < Y
{
    try{ parse_input(argc, argv, X, N, Y); } // reads and validates command line inputs
    catch(exception& e) { cerr<<e.what()<<endl; return EXIT_FAILURE; }
    // initialization
    cleaners = vector<pair<pthread_t, int>>(X);
    init(hotel, N);
    guests = vector<pair<pthread_t, pair<int, int>>>(Y);
    
    mutex_evict = vector<pthread_mutex_t>(N, PTHREAD_MUTEX_INITIALIZER);
    cond_evict = vector<pthread_cond_t>(N, PTHREAD_COND_INITIALIZER);
    mutex_guest = vector<pthread_mutex_t>(Y, PTHREAD_MUTEX_INITIALIZER);
    mutex_cleaner = vector<pthread_mutex_t>(X, PTHREAD_MUTEX_INITIALIZER);

    pthread_barrier_init(&barr_guest, NULL, static_cast<unsigned int>(X+Y));
    pthread_barrier_init(&barr_cleaner, NULL, static_cast<unsigned int>(X));

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

    for(int i=0; i<X; i++)
    {
        cleaners[i].second = i;
    }
    for(int i=0; i<Y; i++)
    {
        guests[i].second.first = i;
        guests[i].second.second = fisher_yates[i]; 
    }

    cout<<"Guest priorities: "<<endl;
    for(int i=0; i<Y; i++) { cout<<guests[i].second.second<<endl; }
    
    // thread creation
    for(int i=0; i<Y; i++)
    {
        int ret = pthread_create(&(guests[i].first), NULL, guest_routine, (void*)&(guests[i].second));
        if(ret) { cerr<<"Failure in guest thread creation"<<endl; }
        else cout<<"Guest thread "<<i<<" created"<<endl;
    }
    for(int i=0; i<X; i++)
    {
        int ret = pthread_create(&(cleaners[i].first), NULL, cleaner_routine, (void*)&(cleaners[i].second));
        if(ret) { cerr<<"Failure in cleaner thread creation"<<endl; }
        else cout<<"Cleaner thread "<<i<<" created"<<endl;
    }

    // initialize semaphores here
    sem_init(&sem_guest, 0, N);
    sem_init(&sem_stdcout, 0, 1);
    sem_init(&sem_hotel, 0, 1);
    cout<<"Semaphores created"<<endl;

    // thread cleanup
    for(int i=0; i<Y; i++)
    {
        int ret = pthread_join(guests[i].first, NULL);
        if(ret) { cerr<<"Failure in guest thread join"<<endl; }
        else cout<<"Guest thread "<<i<<" terminated"<<endl;
    }
    for(int i=0; i<X; i++)
    {
        int ret = pthread_join(guests[i].first, NULL);
        if(ret) { cerr<<"Failure in cleaner thread join"<<endl; }
        else cout<<"Cleaner thread "<<i<<" terminated"<<endl;
    }

    // destroy semaphores and mutexes
    sem_destroy(&sem_guest);
    sem_destroy(&sem_stdcout);
    sem_destroy(&sem_hotel);

    pthread_cond_destroy(&cond_guest);
    pthread_cond_destroy(&cond_cleaner);

    for(int i=0; i<N; i++)
    {
        pthread_mutex_destroy(&mutex_evict[i]);
        pthread_cond_destroy(&cond_evict[i]);
    }
    for(int i=0; i<Y; i++)
    {
        pthread_mutex_destroy(&mutex_guest[i]);
    }
    for(int i=0; i<X; i++)
    {
        pthread_mutex_destroy(&mutex_cleaner[i]);
    }
    pthread_barrier_destroy(&barr_guest);
    pthread_barrier_destroy(&barr_cleaner);
    
    return 0;
}
// Read X, N, Y and validate them
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
// Self-explanatory
void init(Hotel &h, int n)
{
    if(n < 2) throw runtime_error("init: Cannot make hotel with n < 2");
    h.rooms = vector<Room>(n);
    h.tot_occupancy = 0;

    for(int i=0; i<n; i++)
    {
        hotel.rooms[i].current_guest = (pthread_t)NULL;
        h.rooms[i].priority = -1;
        h.rooms[i].occupancy = 0;
        h.rooms[i].time = 0;
    }
    // for(auto x: h.rooms)
    //     cout<<x.current_guest<<" "<<x.occupancy<<" "<<x.priority<<endl;
}
// Locate empty room and allot to guest. Only to be used when hotel isn't full
int book(Hotel &h, int n, pthread_t g, int pr)
{
    if(h.tot_occupancy >= 2*n) return -1; // throw runtime_error("book: All rooms have occupancy 2 (or more)");
    int i;
    for(i=0; i<n; i++)
    {
        if( h.rooms[i].priority == -1 && h.rooms[i].occupancy < 2 )
        break;
    }
    if(i == n) return -1; // throw runtime_error("book: Cannot find an empty room");

    h.rooms[i].current_guest = g;
    h.rooms[i].priority = pr;
    h.rooms[i].occupancy += 1;
    h.tot_occupancy += 1;

    return i;
}
// Self-explanatory
void vacate(Hotel &h, int n, pthread_t g, int idx, int t)
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
        h.rooms[i].current_guest = (pthread_t)NULL;
        h.rooms[i].priority = -1;
        h.rooms[i].time += t;
    }
    else throw runtime_error("vacate: Guest doesn't match");

}
// Find the room with the lower priority guest. Only to be used when hotel is fully occupied
int find_lowerpr_guest(Hotel &h, int n, int pr)
{
    int i, flag=-1;
    for(i=0; i<n; i++)
    {
        if(h.rooms[i].priority == -1) return -1; // throw runtime_error("find_lowerpr_guest: Function called despite presence of empty room(s)");
        if(h.rooms[i].priority < pr && h.rooms[i].occupancy < 2 )
        {
            flag = i;
            break;
        }
    }
    return flag;
}
// Update room credentials with new guest. Only to be used on an occupied room
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
// Assign a room to a cleaner
int clean_assign(Hotel &h, int n)
{
    if(h.tot_occupancy == 0) return -1;

    random_device rd; mt19937 gen(rd());
    vector<int> indexes(N);
    for(int i=0; i<N; i++) indexes[i] = i;

    for (int i=N-1; i>=1; i--)
    {
        uniform_int_distribution<int> did(0, i);
        int j = did(gen);
        int temp = indexes[i];
        indexes[i] = indexes[j];
        indexes[j] = temp;
    }
    int i;
    for(i=0; i<n; i++)
    {
        if( h.rooms[indexes[i]].priority == -1 && h.rooms[indexes[i]].occupancy == 2 )
        break;
    }
    if(i == n) return -1;

    return indexes[i];
}