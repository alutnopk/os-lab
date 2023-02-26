#include "headers.h"

void color()
{
    cout<<"\x1b[35;1m";
}
void uncolor()
{
    cout<<"\x1b[0m"<<endl;
}
void ctrlc_handler(int signum)
{
    uncolor();
    cout<<"Producer process terminated."<<endl;
    shmdt(global_gptr);
    exit(0);
}

int main(int argc, char** argv)
{
    struct sigaction act;
    act.sa_handler = &ctrlc_handler;
    sigaction(SIGINT, (const struct sigaction *)&act, NULL);
    
    int shmid = atoi(argv[1]);
    Graph *gptr;

    color();
    cout<<"Producer begins.";
    uncolor();

    // attach shared memory segment to address space of main process
    global_gptr = shmat(shmid, NULL, 0);
    gptr = (Graph*)global_gptr;
    if(!gptr){ cerr<<"ERROR: Failure in attachment of shared memory to virtual address space."<<endl; return 1; }

    // gptr->print_graph("producer_graph.txt");
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distm(10,30);
    uniform_int_distribution<> distk(1,20);
    vector<int> weights;
    for(;1;)
    {
        sleep(PRODUCER_TIMEOUT);
        // this is where new nodes are added
        int m = distm(gen), k = distk(gen);
        // create m new nodes
        int oldCount = gptr->nodeCount;
        weights.clear();
        for(int i=0; i<oldCount; i++)
        {
            weights.push_back(gptr->nodelist[i].neighborCount);
        }
        for(int i=oldCount; i<oldCount+m; i++) // we assume that nodes are sequentially numbered
        {
            int x = i, y = -1;
            discrete_distribution<> disty(weights.begin(), weights.end());
            for(int j=0; j<k; j++)
            {
                y = disty(gen);
                gptr->addEdge(x, y);
            }
        }
        color();
        cout<<endl<<"Producer has added "<<m<<" new nodes ["<<oldCount<<" - "<<oldCount+m-1<<"] with "<<k<<" neighbors each.\n";
        cout<<"New node count: "<<gptr->nodeCount<<"\nProducer going to sleep now..."<<endl;
        uncolor();
        // gptr->print_graph("producer_graph");
    }
    return 0;
}
