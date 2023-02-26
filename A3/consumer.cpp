#include <iostream>
#include <fstream>
#include <algorithm>
#include <queue>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <cmath>
#include <sstream>
#include <climits>
#include <chrono>
using namespace std;

#define SHMSIZE 4294967296
#define SHMKEY 0
#define MAXCOUNT 8192
#define TIMEOUT 30

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
    void print_graph(string filepath);
    void print_path(string filepath);
    int dijkstra_init(int source, char *filename);
};

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
        outfile<<endl;
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
            if(shortest_path[i][j] == INT_MAX) outfile<<i<<"->"<<j<<" : "<<"INF"<<endl;
            else outfile<<i<<"->"<<j<<" : "<<shortest_path[i][j]<<endl;
        }
    outfile<<"----------------------------------------------"<<endl;
    outfile.close();
    ios_base::sync_with_stdio(true);
}
int Graph::dijkstra_init(int source, char *filename)
{
    // Create a priority queue to hold the nodes to be visited, in min-heap priority of their distance
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    // // Create a vector to hold the distances from the source to each node
    // vector<int> distance(nodeCount, INT_MAX);

    // distance[i] is replaced by shortest_path[source][i]

    // Initialize single source
    if(nodelist[source].current == -1) return -1; // source not in graph
    pq.push(make_pair(0, source));
    shortest_path[source][source] = 0;

    // Loop until the priority queue is empty
    while (!pq.empty())
    {
        // Dequeue the node with the smallest distance (call it u)
        int u = pq.top().second;
        pq.pop();
        if(nodelist[u].current == -1) return -1; // u not in graph

        // Loop over the neighbors of the dequeued node
        for (int i = 0; i < nodelist[u].neighborCount; i++)
        {

            int v = nodelist[u].neighborlist[i];

            int weight = 1; // assuming all edge weights are 1
            // Update the distance if a shorter path is found
            if ((shortest_path[source][v] == INT_MAX) || (shortest_path[source][v] > shortest_path[source][u] + weight))
            {
                shortest_path[source][v] = shortest_path[source][u] + weight;
                pq.push(make_pair(shortest_path[source][v], v));
            }
        }
        // auto start = chrono::high_resolution_clock::now();
        // auto stop = chrono::high_resolution_clock::now();
        // auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
        // cout<<"time is:"<<duration.count()<<endl;
    }
    // ofstream MyFile;
    // MyFile.open(filename, fstream::app);
    // // Print the distances from the source to each node
    // for (int i = 0; i < nodeCount;)
    // {
    //     if(nodelist[i].current == -1) continue;
    //     MyFile <<source << "->" << i << " : " << shortest_path[source][i] << endl;
    //     i++;
    // }
    // MyFile.close();
    return 0;
}

void color()
{
    cout<<"\x1b[33;1m";
}
void color2()
{
    cout<<"\x1b[32;1m";
}
void uncolor()
{
    cout<<"\x1b[0m"<<endl;
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
    cout<<"shmid: "<<shmid<<endl<<"idx: "<<idx;
    uncolor();
    // attach shared memory segment to address space of main process
    global_gptr = shmat(shmid, NULL, 0);
    gptr = (Graph*)global_gptr;
    if(!gptr){ cerr<<"ERROR: Failure in attachment of shared memory to virtual address space."<<endl; return 1; }

    double k = (gptr->nodeCount)/10.0;
    int startidx = intceil(idx*k), endidx = intceil((idx+1)*k) - 1;

    color();
    cout<<"Consumer "<< idx+1 <<" is allotted nodes ["<<startidx<<"-"<<endidx<<"]";
    uncolor();
    // create filename buffer, snprintf
    char *filename;
    // ofstream MyFile;
    // MyFile.open("debug.txt", fstream::app);
    filename = (char*)malloc(50*sizeof(char));
    for(;1;)
    {
        snprintf(filename, 50, "consumer%d.txt", idx+1);
        remove(filename); // delete file if it already exists
        for(int i=startidx; i<=endidx; i++)
        {
            color();
            cout<<"Consumer "<<idx+1<<" running Dijkstra with source node "<<i;
            uncolor();
            if(gptr->dijkstra_init(i, filename) == -1) { cerr<<"ERROR: Invalid source node."<<endl; return 1; }
        }

        color2();
        cout<<"Consumer "<<idx+1<<" Dijkstra complete. Writing output to "<<filename<<endl;
        uncolor();
        gptr->print_path(filename);
        sleep(TIMEOUT);
    }
    free(filename);
    return 0;
}