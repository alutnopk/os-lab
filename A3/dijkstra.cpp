#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <queue>
#include <sys/wait.h>
#include <climits>
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
    
    int nodeCount;
    AdjList nodelist[MAXCOUNT];
    // constructor, destructor
    public:
    Graph()
    {
        this->nodeCount = 0;
        for(int i=0; i<MAXCOUNT; i++) { this->nodelist[i].current = -1; this->nodelist[i].neighborCount = 0; }
    }
    int init(string filepath);
    void show();
    int dijkstra(int source);
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
                // cout<<endl;
                int abc=0;
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
    cout<<"Node Count: "<<nodeCount<<endl;
    for(int i=0; i<nodeCount; i++)
    {
        cout<<nodelist[i].current<<": ";
        for(int j=0; j<nodelist[i].neighborCount; j++)
            cout<<nodelist[i].neighborlist[j]<<" ";
        cout<<endl;
    }
}

// select closest vertex
// check if any of its neighbors are to be relaxed
// add relaxed neighbors if necessary
int Graph::dijkstra(int source)
{
    // Create a priority queue to hold the nodes to be visited, in min-heap priority of their distance
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    // Create a vector to hold the distances from the source to each node
    vector<int> distance(nodeCount, INT_MAX);

    // Initialize single source
    pq.push(make_pair(0, source));
    int idxinit = -1;
    for(idxinit=0; idxinit<nodeCount; idxinit++) if(source == nodelist[idxinit].current) break;
    if(idxinit >= nodeCount) return -1; // source not in graph
    distance[idxinit] = 0;

    // Loop until the priority queue is empty
    while (!pq.empty()) {
        // Dequeue the node with the smallest distance (call it u)
        int u = pq.top().second;
        pq.pop();
        
        // find index of node u
        int idxu = -1;
        for(idxu=0; idxu<nodeCount; idxu++) if(u == nodelist[idxu].current) break;
        if(idxu >= nodeCount) return -1; // node not in graph

        // Loop over the neighbors of the dequeued node
        for (int i = 0; i < nodelist[idxu].neighborCount; i++) {
            int v = nodelist[idxu].neighborlist[i];
            // find index of node v
            int idxv = -1;
            for(idxv=0; idxv<nodeCount; idxv++) if(v == nodelist[idxv].current) break;

            int weight = 1; // assuming all edge weights are 1
            // Update the distance if a shorter path is found
            if ((distance[idxv] == INT_MAX) || (distance[idxv] > distance[idxu] + weight)) {
                distance[idxv] = distance[idxu] + weight;
                pq.push(make_pair(distance[idxv], v));
            }
        }
    }
    ofstream MyFile("lol.txt");
    // Print the distances from the source to each node
    for (int i = 0; i < nodeCount; i++) {
        // cout << "Shortest path from " << nodelist[idxinit].current << " to " << nodelist[i].current << " is " << distance[i] << endl;
        MyFile << "Shortest path from " << nodelist[idxinit].current << " to " << nodelist[i].current << " is " << distance[i] << endl;
    }
    cout<< INT_MAX << endl;
    return 0;
}


int main(int argc, char* argv[])
{
    int shmid;
    Graph *gptr;

    // create System V shared memory segment
    shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | 0666);
    if(shmid == -1){ cerr<<"ERROR: Failure in shared memory allocation."<<endl; return 1; }
    cout<<shmid<<endl;
    // attach shared memory segment to address space of main process
    gptr = (Graph*)shmat(shmid, NULL, 0);
    if(!gptr){ cerr<<"ERROR: Failure in attachment of shared memory to virtual address space."<<endl; return 1; }

    cout<<"Shared memory segment successfully created."<<endl;
    // cout<<sizeof(Graph)<<endl;
    if(gptr->init("facebook_combined.txt") == -1) { cerr<<"ERROR: Unable to load graph from file."<<endl; return 1; }
    cout<<"Graph successfully stored."<<endl;
    // gptr->show();
    if(gptr->dijkstra(400) == -1) { cerr<<"ERROR: Invalid source node."<<endl; return 1; }
    return 0;
}