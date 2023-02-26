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
using namespace std;

#define SHMSIZE 4294967296
#define SHMKEY 0
#define MAXCOUNT 8192
#define CONSUMERCOUNT 10

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
    public:
    Graph()
    {
        nodeCount = 0;
        for(int i=0; i<MAXCOUNT; i++) { this->nodelist[i].current = -1; this->nodelist[i].neighborCount = 0; }
    }
    int init(string filepath);
    void show();
};

int Graph::init(string filepath)
{
    // initialize structure
    nodeCount = 0;
    for(int i=0; i<MAXCOUNT; i++) { this->nodelist[i].current = -1; this->nodelist[i].neighborCount = 0; }
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

            // for(int i=0; i<nodeCount; i++)
            // shall be replaced by
            // for(int i=0; i<nodeCount;)
            // {
            //    if(nodelist[i].current == -1) continue;
            //    code here
            //    i++;  
            // }
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
        }
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
int main()
{   
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    sigaction(SIGINT, (const struct sigaction *)&act, NULL);
    
    int shmid;
    Graph *gptr;
    pid_t prodpid;
    pid_t conpid[CONSUMERCOUNT];
    // create System V shared memory segment
    shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | 0666);
    if(shmid == -1){ cerr<<"ERROR: Failure in shared memory allocation."<<endl; return 1; }
    cout<<"shmid: "<<shmid<<endl;
    // attach shared memory segment to address space of main process
    gptr = (Graph*)shmat(shmid, NULL, 0);
    if(!gptr){ cerr<<"ERROR: Failure in attachment of shared memory to virtual address space."<<endl; return 1; }

    cout<<"Shared memory segment successfully created."<<endl;
    // cout<<sizeof(Graph)<<endl;
    if(gptr->init("facebook_combined.txt") == -1) { cerr<<"ERROR: Unable to load graph from file."<<endl; return 1; }
    cout<<"Graph successfully stored at address "<<gptr<<endl;
    cout<<"Node count: "<<gptr->nodeCount<<endl;
    // gptr->show();

    char *temp = (char*)malloc(10*sizeof(char)); // to store shmid
    char *temp2 = (char*)malloc(2*sizeof(char)); // to store consumer index
    snprintf(temp, 10, "%d", shmid);
    if((prodpid = fork()) == 0) // producer process
    {
            cout<<"Producer forked."<<endl;
            execlp("./producer", "./producer", temp, NULL);
            cerr<<"ERROR: Failure in forking producer."<<endl;
            exit(1);
    }
    sleep(1);
    for(int i=0; i<CONSUMERCOUNT; i++)
    {
        snprintf(temp2, 2, "%d", i);
        if((conpid[i] = fork()) == 0) // Consumer process
        {
            // sleep(2);
            execlp("./consumer", "./consumer", temp, temp2, NULL);
            cerr<<"ERROR: Failure in forking consumer."<<endl;
            exit(1);
        }
        // wait(NULL);
        memset(temp2, 0, sizeof(temp2));
    }

    free(temp);
    free(temp2);

    waitpid(prodpid, NULL, WUNTRACED);
    for(int i=0; i<CONSUMERCOUNT; i++) waitpid(conpid[i], NULL, WUNTRACED);
    cout<<"Back to main. Detaching and deleting shared memory segment..."<<endl;
    // Detach shared memory segment
    shmdt(gptr);
    // Mark the segment to be destroyed
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
