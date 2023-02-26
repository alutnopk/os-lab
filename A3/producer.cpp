#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <sstream>
#include <sys/wait.h>
#include <random>
using namespace std;

#define SHMSIZE 4294967296
#define SHMKEY 0
#define MAXCOUNT 8192
#define TIMEOUT 1000

void* global_gptr;
typedef struct AdjList
{
    public:
    int current;
    int neighborCount;
    int neighborlist[MAXCOUNT];
} AdjList;

class Graph
{
    
    public:
    int nodeCount;
    AdjList nodelist[MAXCOUNT];
    // constructor, destructor
    Graph()
    {
        this->nodeCount = 0;
        for(int i=0; i<MAXCOUNT; i++) { this->nodelist[i].current = -1; this->nodelist[i].neighborCount = 0; }
    }
    int addEdge(int x, int y);
    void show();
};

int Graph::addEdge(int x, int y)
{
    int ix = nodelist[x].current, iy = nodelist[y].current;

    if(ix>=0 && iy>=0) // if both x,y are old nodes
    {
        // add y into x list only if y not found
        for(int j=0; j<nodelist[x].neighborCount; j++) // iterate through neighbors of x
            if(nodelist[x].neighborlist[j] == y) goto y_repeated; // if y already found, skip addition step
        // else add y as neighbor
        nodelist[x].neighborlist[nodelist[x].neighborCount] = y;
        nodelist[x].neighborCount += 1;
        if(nodelist[x].neighborCount >= MAXCOUNT+1) return -1; // check if limit reached
        y_repeated:

        // add x into y list only if x not found
        for(int j=0; j<nodelist[y].neighborCount; j++) // iterate through neighbors of y
            if(nodelist[y].neighborlist[j] == x) goto x_repeated; // if x already found, skip addition step
        // else add x as neighbor
        nodelist[y].neighborlist[nodelist[iy].neighborCount] = x;
        nodelist[y].neighborCount += 1;
        if(nodelist[y].neighborCount >= MAXCOUNT+1) return -1; // check if limit reached
        x_repeated:
        ;
    }
    else if(ix>=0 && iy==-1) // x found but not y
    {
        // add y to x list
        nodelist[x].neighborlist[nodelist[x].neighborCount] = y;
        nodelist[x].neighborCount += 1;
        if(nodelist[x].neighborCount >= MAXCOUNT+1) return -1; // check if limit reached
        // init y list
        nodelist[y].current = y;
        nodelist[y].neighborlist[0] = x;
        nodelist[y].neighborCount = 1;
        nodeCount++;
    }
    else if(ix==-1 && iy>=0) // x is new node, not y
    {
        // add x to y list
        nodelist[y].neighborlist[nodelist[y].neighborCount] = x;
        nodelist[y].neighborCount += 1;
        if(nodelist[y].neighborCount >= MAXCOUNT+1) return -1; // check if limit reached
        // init x list
        nodelist[x].current = x;
        nodelist[x].neighborlist[0] = y;
        nodelist[x].neighborCount = 1;
        nodeCount++;
    }
    else // both new nodes
    {
        // init x list
        nodelist[x].current = x;
        nodelist[x].neighborlist[0] = y;
        nodelist[x].neighborCount = 1;
        nodeCount++;
        // init y list
        nodelist[y].current = y;
        nodelist[y].neighborlist[0] = x;
        nodelist[y].neighborCount = 1;
        nodeCount++;
    }
    return 0;
}

void Graph::show()
{
    cout<<"Total Nodes: "<<nodeCount<<endl;
    // ofstream MyFile("output.txt");
    for(int i=0; i<nodeCount;)
    {
        if(nodelist[i].current == -1) continue;
        // MyFile << nodelist[i].current<<":";
        cout<<nodelist[i].current<<":";
        for(int j=0; j<nodelist[i].neighborCount; j++) 
        {
            // MyFile<<nodelist[i].neighborlist[j]<< " ";
            cout<<nodelist[i].neighborlist[j]<< " ";
        }
        // MyFile<<endl;
        cout<<endl;
        i++;
    }
    // MyFile.close();
}

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
    cout<<endl<<"Producer process terminated."<<endl;
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
    cout<<"Producer begins."<<endl;
    // cout<<shmid<<endl;
    uncolor();

    // attach shared memory segment to address space of main process
    global_gptr = shmat(shmid, NULL, 0);
    gptr = (Graph*)global_gptr;
    if(!gptr){ cerr<<"ERROR: Failure in attachment of shared memory to virtual address space."<<endl; return 1; }
    color();
    // cout<<gptr->nodeCount<<endl;
    uncolor();
    // gptr->show();
    random_device dre;
    mt19937 gen(dre());
    uniform_int_distribution<> distm(10,30);
    uniform_int_distribution<> distk(1,20);
    vector<int> weights;
    for(;1;)
    {
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
        cout<<endl<<m<<" new nodes added ["<<gptr->nodelist[oldCount].current<<" - "<<gptr->nodelist[oldCount+m-1].current<<"] with "<<k<<" neighbors each."<<endl;
        cout<<"New node count: "<<gptr->nodeCount<<endl;
        uncolor();
        // gptr->show();
        sleep(TIMEOUT);
    }
    return 0;
}
