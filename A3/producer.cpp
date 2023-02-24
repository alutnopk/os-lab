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
using namespace std;

#define SHMSIZE 4294967296
#define SHMKEY 0
#define MAXCOUNT 8192

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
    int init(string filepath);
    void show();
};

int Graph::init(string filepath)
{
    this->nodeCount=0;
    fstream fs;
    fs.open(filepath, ios::in); 
    if(!fs) return -1;
    if(fs.is_open())
    {
        string lin;
        int x, y;
        while(getline(fs, lin)) // store graph edges as pairs of vertices starting from index 2
        {
            stringstream slin(lin);
            slin>>x>>y;
            // if(idx >= SHMSIZE-2){ cerr<<"ERROR: Insufficient shared memory to store graph."<<endl; return 1; }
            int ix = -1, iy = -1;
            for(int i=0; i<nodeCount; i++)
            {
                if(nodelist[i].current == x) ix = i;
                if(nodelist[i].current == y) iy = i;
                if(ix>=0 && iy>=0) break;
            }
            if(ix>=0 && iy>=0) // if both x,y are old nodes
            {
                // add y into x list only if y not found
                for(int j=0; j<nodelist[ix].neighborCount; j++) // iterate through neighbors of x
                    if(nodelist[ix].neighborlist[j] == y) goto y_repeated; // if y already found, skip addition step
                // else add y as neighbor
                nodelist[ix].neighborlist[nodelist[ix].neighborCount] = y;
                nodelist[ix].neighborCount += 1;
                if(nodelist[ix].neighborCount >= MAXCOUNT+1) return -1; // check if limit reached
                y_repeated:

                // add x into y list only if x not found
                for(int j=0; j<nodelist[iy].neighborCount; j++) // iterate through neighbors of y
                    if(nodelist[iy].neighborlist[j] == x) goto x_repeated; // if x already found, skip addition step
                // else add x as neighbor
                nodelist[iy].neighborlist[nodelist[iy].neighborCount] = x;
                nodelist[iy].neighborCount += 1;
                if(nodelist[iy].neighborCount >= MAXCOUNT+1) return -1; // check if limit reached
                x_repeated:
                cout<<endl;
            }
            else if(ix>=0 && iy==-1) // x found but not y
            {
                // add y to x list
                nodelist[ix].neighborlist[nodelist[ix].neighborCount] = y;
                nodelist[ix].neighborCount += 1;
                if(nodelist[ix].neighborCount >= MAXCOUNT+1) return -1; // check if limit reached
                // init y list
                nodelist[nodeCount].current = y;
                nodelist[nodeCount].neighborlist[0] = x;
                nodelist[nodeCount].neighborCount = 1;
                nodeCount++;
            }
            else if(ix==-1 && iy>=0) // x is new node, not y
            {
                // add x to y list
                nodelist[iy].neighborlist[nodelist[iy].neighborCount] = x;
                nodelist[iy].neighborCount += 1;
                if(nodelist[iy].neighborCount >= MAXCOUNT+1) return -1; // check if limit reached
                // init x list
                nodelist[nodeCount].current = x;
                nodelist[nodeCount].neighborlist[0] = y;
                nodelist[nodeCount].neighborCount = 1;
                nodeCount++;
            }
            else // both new nodes
            {
                // init x list
                nodelist[nodeCount].current = x;
                nodelist[nodeCount].neighborlist[0] = y;
                nodelist[nodeCount].neighborCount = 1;
                nodeCount++;
                // init y list
                nodelist[nodeCount].current = y;
                nodelist[nodeCount].neighborlist[0] = x;
                nodelist[nodeCount].neighborCount = 1;
                nodeCount++;
            }
        }
    }
    return 0;
}

void Graph::show()
{
    cout<<"Total Nodes: "<<nodeCount<<endl;
    // ofstream MyFile("output.txt");
    for(int i=0; i<nodeCount; i++)
    {
        // MyFile << nodelist[i].current<<":";
        cout<<nodelist[i].current<<":";
        for(int j=0; j<nodelist[i].neighborCount; j++) 
        {
            // MyFile<<nodelist[i].neighborlist[j]<< " ";
            cout<<nodelist[i].neighborlist[j]<< " ";
        }
        // MyFile<<endl;
        cout<<endl;
    }
    // MyFile.close();
}
int main(int argc, char** argv)
{
    int shmid = atoi(argv[1]);
    Graph *gptr;
    cout<<"Producer begins."<<endl;
    // get System V shared memory segment
    // shmid = shmget(SHMKEY, SHMSIZE, IPC_CREAT | 0666);
    // if(shmid == -1){ cerr<<"ERROR: Failure in shared memory allocation."<<endl; return 1; }
    cout<<shmid<<endl;
    // attach shared memory segment to address space of main process
    gptr = (Graph*)shmat(shmid, NULL, 0);
    if(!gptr){ cerr<<"ERROR: Failure in attachment of shared memory to virtual address space."<<endl; return 1; }
    cout<<gptr->nodeCount<<endl;
    // gptr->show();
    shmdt(gptr);
    return 0;
}