#include <iostream>
#include <fstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <sstream>
using namespace std;

#define SHMSIZE 1048576
#define SHMKEY 17

int main()
{   
    int shmid;
    int *mem;
    // create System V shared memory segment
    shmid = shmget(SHMKEY, SHMSIZE, IPC_CREAT | 0666);
    if(shmid == -1){ cerr<<"ERROR: Failure in shared memory allocation."<<endl; return 1; }
    // attach shared memory segment to address space of main process
    mem = (int*)shmat(shmid, NULL, 0);
    if(!mem){ cerr<<"ERROR: Failure in attachment of shared memory to virtual address space."<<endl; return 1; }

    cout<<"Shared memory segment successfuly created."<<endl;
    // open initial graph file and store into mem
    /*
        Representation of the graph:-
        mem[0]: 2 * number of edges
        mem[1]: -1 (to indicate that edges begin from next entry )
        for i > 1, (mem[2*i], mem[2*i+1]) represents an edge (u, v) in the graph
        finally, the two entries after the last edge are -1 and -1 (to indicate end of representation)
    */
    fstream fs;
    fs.open("facebook_combined.txt", ios::in); 
    if(!fs){ cerr<<"ERROR: Cannot open file."<<endl; return 1; }
    if(fs.is_open())
    {
        string lin;
        int x, y, idx=2;
        while(getline(fs, lin)) // store graph edges as pairs of vertices starting from index 2
        {
            stringstream slin(lin);
            slin>>x>>y;
            if(idx >= SHMSIZE-2){ cerr<<"ERROR: Insufficient shared memory to store graph."<<endl; return 1; }
            mem[idx] = x; mem[idx+1] = y;
            idx += 2;
        }
        // assigning mem[0], mem[1], and final two -1 entries
        mem[idx] = -1; mem[idx+1] = -1;
        mem[0] = idx+2; mem[1] = -1;
    }
    cout<<"Graph successfully stored."<<endl;

    // test printing the edges of the graph
    // int i = 2;
    // do
    // {
    //     cout<<mem[i]<<" "<<mem[i+1]<<endl;
    //     i += 2;
    // } while(mem[i] != -1 && mem[i+1] != -1);

    // This is where child processes shall be spawned which inherit the shared memory

    // Detach shared memory segment
    shmdt(mem);
    // Mark the segment to be destroyed
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}