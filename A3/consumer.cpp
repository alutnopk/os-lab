#include "headers.h"

void color() // bold yellow font
{
    cout<<"\x1b[33;1m";
}
void color2() // bold blue font
{
    cout<<"\x1b[34;1m";
}
void color3() // bold green font
{
    cout<<"\x1b[32;1m";
}
void uncolor()
{
    cout<<"\x1b[0m"<<endl;
}
void ctrlc_handler(int signum) // signal handler to be executed upon Ctrl+C
{
    uncolor();
    cout<<"Consumer process terminated."<<endl;
    shmdt(global_gptr); // detach shared memory
    exit(0); // terminate process
}
int ceilval(double x)
{
    int y = (int)x;
    if(x-y > 0) return y+1;
    else return y;
}

int main(int argc, char** argv)
{
    // mapping the signal handler
    struct sigaction act;
    act.sa_handler = &ctrlc_handler;
    sigaction(SIGINT, (const struct sigaction *)&act, NULL);

    int shmid = atoi(argv[1]), idx = atoi(argv[2]);
    Graph *gptr;

    color();
    cout<<"Consumer "<< idx+1 <<" begins.";
    uncolor();
    // attach shared memory segment to address space of consumer process
    global_gptr = shmat(shmid, NULL, 0);
    gptr = (Graph*)global_gptr;
    if(!gptr){ cerr<<"ERROR: Failure in attachment of shared memory to virtual address space."<<endl; return 1; }

    double k = (gptr->nodeCount)/10.0;
    int startidx = ceilval(idx*k), endidx = ceilval((idx+1)*k)-1; // range of nodes allotted to this process
    int oldstartidx = startidx, oldendidx = endidx, oldCount = gptr->nodeCount;

    char *filename = (char*)malloc(50*sizeof(char));
    snprintf(filename, 50, "consumer%d.txt", idx+1); // output filename
    remove(filename); // delete file if it already exists

    color();
    cout<<"Consumer "<<idx+1<<" running Dijkstra with source nodes ["<<startidx<<"-"<<endidx<<"]...";
    uncolor();

    for(int i=startidx; i<=endidx; i++) // run single-source Dijkstra for each source node
        if(gptr->dijkstra(i, filename) == -1) { cerr<<"ERROR: Invalid source node."<<endl; return 1; }

    color2();
    cout<<"Consumer "<<idx+1<<" Dijkstra complete. Writing output to "<<filename;
    uncolor();

    gptr->print_path(filename, startidx, endidx); // print shortest paths from each source

    color3();
    cout<<"Consumer "<<idx+1<<" output written successfully to "<<filename<<". Going to sleep now...";
    uncolor();

    for(;1;)
    {
        sleep(CONSUMER_TIMEOUT);
        // startidx, endidx, nodeCount are updated by producer
        k = (gptr->nodeCount)/10.0;
        startidx = ceilval(idx*k);
        endidx = ceilval((idx+1)*k)-1;

        color();
        cout<<"Consumer "<<idx+1<<" running Dijkstra with source nodes ["<<startidx<<"-"<<endidx<<"]...";
        uncolor();

        for(int i=startidx; i<=endidx; i++)
        if(gptr->dijkstra(i, filename) == -1) { cerr<<"ERROR: Invalid source node."<<endl; return 1; }

        color2();
        cout<<"Consumer "<<idx+1<<" Dijkstra complete. Writing output to "<<filename;
        uncolor();

        gptr->print_path(filename, startidx, endidx);

        color3();
        cout<<"Consumer "<<idx+1<<" output appended successfully to "<<filename<<". Going to sleep now...";
        uncolor();

        // update old values to current
        oldstartidx = startidx;
        oldendidx = endidx;
        oldCount = gptr->nodeCount;
    }
    free(filename);
    return 0;
}