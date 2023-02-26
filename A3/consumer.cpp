#include "headers.h"

void color() // yellow with bold style
{
    cout<<"\x1b[33;1m";
}
void color2() // blue with bold style
{
    cout<<"\x1b[34;1m";
}
void color3() // green with bold style
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
    cout<<"Consumer process terminated."<<endl;
    shmdt(global_gptr);
    exit(0);
}
int floorval(double x)
{
    return (int)x;
}
int ceilval(double x)
{
    return (int)x + 1;
}

int main(int argc, char** argv)
{
    struct sigaction act;
    act.sa_handler = &ctrlc_handler;
    sigaction(SIGINT, (const struct sigaction *)&act, NULL);

    int shmid = atoi(argv[1]), idx = atoi(argv[2]);
    Graph *gptr;
    color();
    cout<<"Consumer "<< idx+1 <<" begins.";
    uncolor();
    // attach shared memory segment to address space of main process
    global_gptr = shmat(shmid, NULL, 0);
    gptr = (Graph*)global_gptr;
    if(!gptr){ cerr<<"ERROR: Failure in attachment of shared memory to virtual address space."<<endl; return 1; }

    double k = (gptr->nodeCount)/10.0;
    int startidx = ceilval(idx*k), endidx = floorval((idx+1)*k);
    int oldstartidx = startidx, oldendidx = endidx, oldCount = gptr->nodeCount;
    char *filename = (char*)malloc(50*sizeof(char));
    snprintf(filename, 50, "consumer%d.txt", idx+1);
    remove(filename); // delete file if it already exists

    // initialize shortest paths table
    // append output to file
    color();
    cout<<"Consumer "<<idx+1<<" running first-time Dijkstra with source nodes ["<<startidx<<"-"<<endidx<<"]...";
    uncolor();

    for(int i=startidx; i<=endidx; i++)
        if(gptr->dijkstra_init(i, filename) == -1) { cerr<<"ERROR: Invalid source node."<<endl; return 1; }

    color2();
    cout<<"Consumer "<<idx+1<<" first-time Dijkstra complete. Writing output to "<<filename;
    uncolor();

    gptr->print_path(filename, startidx, endidx);

    color3();
    cout<<"Consumer "<<idx+1<<" output written successfully to "<<filename<<". Going to sleep now...";
    uncolor();

    for(;1;)
    {
        sleep(CONSUMER_TIMEOUT);
        // startidx, endidx, nodeCount are updated by producer
        k = (gptr->nodeCount)/10.0;
        startidx = ceilval(idx*k);
        endidx = floorval((idx+1)*k);


        color();
        cout<<"Consumer "<<idx+1<<" running optimized Dijkstra with source nodes ["<<startidx<<"-"<<endidx<<"]...";
        uncolor();

        // normal dijkstra
        for(int i=startidx; i<=endidx; i++)
        if(gptr->dijkstra_init(i, filename) == -1) { cerr<<"ERROR: Invalid source node."<<endl; return 1; }

        // // alternative optimization
        // for(int i=oldendidx+1; i<=endidx; i++) // for each new source node
        // {
        //     // run usual dijkstra, populate the shortest paths for new rows
        //     if(gptr->dijkstra_init(i, filename) == -1) { cerr<<"ERROR: Invalid source node."<<endl; return 1; } // dijkstra to be changed
        // }
        // // now we deal with old rows
        // for(int i=startidx; i<=oldendidx; i++)
        // {
        //     // iterate through each new node
        //     // dijkstra_update(i, newnode)
        // }

        color2();
        cout<<"Consumer "<<idx+1<<" optimized Dijkstra complete. Writing output to "<<filename;
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