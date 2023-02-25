#include <iostream>
#include <fstream>
#include <algorithm>
#include <queue>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <cmath>
#include <sstream>
#include <sys/wait.h>
using namespace std;

#define SHMSIZE 4294967296
#define SHMKEY 0
#define MAXCOUNT 8192
#define TIMEOUT 2

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
    int init(string filepath);
    void show();
    void dijkstra(int source, char *filename);
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
                ;
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

void Graph::dijkstra(int source, char *filename)
{
    // Create a priority queue to hold the nodes to be visited
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    // Create a vector to hold the distances from the source to each node
    vector<int> distance(nodeCount, 9999);
    distance[source] = 0;

    // Enqueue the source node
    pq.push(make_pair(0, source));

    // Loop until the priority queue is empty
    while (!pq.empty()) {
        // Dequeue the node with the smallest distance
        int u = pq.top().second;
        pq.pop();

        // Loop over the neighbors of the dequeued node
        for (int i = 0; i < nodelist[u].neighborCount; i++) {
            int v = nodelist[u].neighborlist[i];
            int weight = 1; // assuming all edge weights are 1

            // Update the distance if a shorter path is found
            if (distance[v] > distance[u] + weight) {
                distance[v] = distance[u] + weight;
                pq.push(make_pair(distance[v], v));
            }
        }
    }
    ofstream MyFile;
    MyFile.open(filename, fstream::app);
    // Print the distances from the source to each node
    for (int i = 0; i < nodeCount; i++) {
        // cout << "Distance from " << source << " to " << i << " is " << distance[i] << endl;
        MyFile << "Distance from " << source << " to " << i << " is " << distance[i] << endl;
    }
}

void color()
{
    cout<<"\x1b[33;1m";
}
void uncolor()
{
    cout<<"\x1b[0m";
}
void ctrlc_handler(int signum)
{
    uncolor();
    cout<<endl<<"Consumer process terminated."<<endl;
    shmdt(global_gptr);
    exit(0);
}
int intceil(double x)
{
    int y = (int)x;
    if(x-y > 0) return y+1;
    else return y;
}
int main(int argc, char** argv)
{
    struct sigaction act;
    act.sa_handler = &ctrlc_handler;
    sigaction(SIGINT, (const struct sigaction *)&act, NULL);

    int shmid = atoi(argv[1]), idx = atoi(argv[2]);
    Graph *gptr;
    color();
    cout<<"Consumer "<< idx+1 <<" begins."<<endl;

    // cout<<"shmid: "<<shmid<<endl;
    uncolor();
    // attach shared memory segment to address space of main process
    global_gptr = shmat(shmid, NULL, 0);
    gptr = (Graph*)global_gptr;
    if(!gptr){ cerr<<"ERROR: Failure in attachment of shared memory to virtual address space."<<endl; return 1; }

    // color();
    // cout<<"Node count: "<<gptr->nodeCount<<endl;
    // uncolor();
    // gptr->show();
    int n = gptr->nodeCount;
    double k = n/10.0;
    int startidx = intceil(idx*k), endidx = intceil((idx+1)*k) - 1;
    // create filename buffer, snprintf
    char *filename;
    filename = (char*)malloc(20*sizeof(char));
    for(;1;)
    {
        // this is where dijkstra is run usi g [startidx, endidx] as source

        // open file, write to it, close it
        snprintf(filename, 20, "consumer%d.txt", idx+1);
        
        for(int i=startidx; i<=endidx; i++)
        {
            // cout<<"Consumer "<<idx+1<<" running Dijkstra from "<<gptr->nodelist[i].current<<endl;
            gptr->dijkstra(i, filename);
        }

        color();
        // cout<<"Consumer "<<idx+1<<" running Dijkstra."<<endl;
        uncolor();
        sleep(TIMEOUT);
    }

    return 0;
}