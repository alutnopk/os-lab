#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <sstream>
#include <random>
#include <climits>
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
    int shortest_path[MAXCOUNT][MAXCOUNT];
    Graph()
    {
        nodeCount = 0;
        for(int i=0; i<MAXCOUNT; i++) { this->nodelist[i].current = -1; this->nodelist[i].neighborCount = 0; }
        for(int i=0; i<MAXCOUNT; i++)
            for(int j=0; j<MAXCOUNT; j++)
                shortest_path[i][j] = INT_MAX;
    }
    int addEdge(int x, int y);
    void print_graph(string filepath);
    void print_path(string filepath);
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

void Graph::print_graph(string filepath)
{
    // cout<<"Total Nodes: "<<nodeCount<<endl;
    ios_base::sync_with_stdio(false);
    ofstream outfile(filepath);
    if(!outfile) { cerr<<"ERROR: Cannot open file."<<endl; return; }
    for(int i=0; i<nodeCount;)
    {
        if(nodelist[i].current == -1) continue;
        outfile << nodelist[i].current<<"\t:\t";
        // cout<<nodelist[i].current<<":";
        for(int j=0; j<nodelist[i].neighborCount; j++) 
        {
            outfile<<nodelist[i].neighborlist[j]<< " ";
            // cout<<nodelist[i].neighborlist[j]<< " ";
        }
        outfile<<"\n";
        // cout<<endl;
        i++;
    }
    outfile<<"----------------------------------------------"<<endl;
    outfile.close();
    ios_base::sync_with_stdio(true);
}
void Graph::print_path(string filepath)
{
    ios_base::sync_with_stdio(false);
    ofstream outfile(filepath);
    if(!outfile) { cerr<<"ERROR: Cannot open file."<<endl; return; }
    for(int i=0; i<nodeCount; i++)
        for(int j=0; j<nodeCount; j++)
        {
            if(shortest_path[i][j] == INT_MAX) outfile<<i<<"->"<<j<<" : INF\n";
            else outfile<<i<<"->"<<j<<" : "<<shortest_path[i][j]<<"\n";
        }
    outfile<<"----------------------------------------------"<<endl;
    outfile.close();
    ios_base::sync_with_stdio(true);
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
    // gptr->print_graph();
    random_device rd;
    mt19937 gen(rd());
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
        cout<<endl<<m<<" new nodes added ["<<oldCount<<" - "<<oldCount+m-1<<"] with "<<k<<" neighbors each."<<endl;
        cout<<"New node count: "<<gptr->nodeCount<<endl<<"Producer going to sleep now...";
        uncolor();
        // gptr->print_graph("producer_graph");
        sleep(TIMEOUT);
    }
    return 0;
}
