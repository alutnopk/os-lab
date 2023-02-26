#include "headers.h"

void color() // bold purple text
{
    cout<<"\x1b[35;1m";
}
void uncolor() // remove text color and flush output stream
{
    cout<<"\x1b[0m"<<endl;
}
void ctrlc_handler(int signum) // signal handler to be executed upon Ctrl+C
{
    uncolor();
    cout<<"Producer process terminated."<<endl;
    shmdt(global_gptr); // detach shared memory from address space
    exit(0); // terminate process
}

int main(int argc, char** argv)
{
    // mapping the signal handler
    struct sigaction act;
    act.sa_handler = &ctrlc_handler;
    sigaction(SIGINT, (const struct sigaction *)&act, NULL);
    
    int shmid = atoi(argv[1]);
    Graph *gptr;

    color();
    cout<<"Producer begins.";
    uncolor();

    // attach shared memory segment to address space of producer process
    global_gptr = shmat(shmid, NULL, 0); // global_gptr is a global copy of the pointer
    gptr = (Graph*)global_gptr;
    if(!gptr){ cerr<<"ERROR: Failure in attachment of shared memory to virtual address space."<<endl; return 1; }

    random_device rd; // non-deterministic random number generator (RNG)
    mt19937 gen(rd()); // Mersenne twister seeded with the generator
    uniform_int_distribution<> distm(10,30); // URNG of range [10-30]
    uniform_int_distribution<> distk(1,20); // URNG of range [1-20]
    vector<int> degree;
    for(;1;)
    {
        sleep(PRODUCER_TIMEOUT);

        // generate m and k
        int m = distm(gen), k = distk(gen);

        // populate vector with degree of each node
        int oldCount = gptr->nodeCount;
        degree.clear();
        for(int i=0; i<oldCount; i++)
        {
            degree.push_back(gptr->nodelist[i].neighborCount);
        }

        // create m new nodes
        for(int i=oldCount; i<oldCount+m; i++) // we add m nodes sequentially starting from nodeCount
        {
            int x = i, y = -1;
            discrete_distribution<> disty(degree.begin(), degree.end()); // RNG that selects nodes with probability proportional to degree
            for(int j=0; j<k; j++) // attach k random old nodes to current node
            {
                y = disty(gen);
                gptr->addEdge(x, y);
            }
        }
        color();
        cout<<endl<<"Producer has added "<<m<<" new nodes ["<<oldCount<<" - "<<oldCount+m-1<<"] with "<<k<<" neighbors each.\n";
        cout<<"New node count: "<<gptr->nodeCount<<"\nProducer going to sleep now..."<<endl;
        uncolor();
        // gptr->print_graph("producer_graph.txt", oldCount, oldCount+m-1);
    }
    return 0;
}
