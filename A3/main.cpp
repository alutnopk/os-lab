#include "headers.h"

int main()
{   

    struct sigaction act;
    act.sa_handler = SIG_IGN;
    sigaction(SIGINT, (const struct sigaction *)&act, NULL);
    
    int shmid;
    Graph *gptr;
    pid_t prodpid;
    pid_t conpid[CONSUMER_COUNT];
    // create System V shared memory segment
    shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | 0666);
    if(shmid == -1){ cerr<<"ERROR: Failure in shared memory allocation."<<endl; return 1; }

    // attach shared memory segment to address space of main process
    gptr = (Graph*)shmat(shmid, NULL, 0);
    if(!gptr){ cerr<<"ERROR: Failure in attachment of shared memory to virtual address space."<<endl; return 1; }

    cout<<"Shared memory segment successfully created, shmid: "<<shmid<<endl;
    if(gptr->init("facebook_combined.txt") == -1) { cerr<<"ERROR: Unable to load graph from file."<<endl; return 1; }
    cout<<"Graph successfully stored at address "<<gptr<<endl;
    cout<<"Initial node count: "<<gptr->nodeCount<<endl;
    // gptr->print_graph("maingraph.txt");
    // gptr->print_path("mainpath.txt");

    char *temp = (char*)malloc(10*sizeof(char)); // to store shmid
    char *temp2 = (char*)malloc(2*sizeof(char)); // to store consumer index
    snprintf(temp, 10, "%d", shmid);
    if((prodpid = fork()) == 0) // producer process
    {
            execlp("./producer", "./producer", temp, NULL);
            cerr<<"ERROR: Failure in forking producer."<<endl;
            exit(1);
    }
    // sleep(1);
    for(int i=0; i<CONSUMER_COUNT; i++)
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
    for(int i=0; i<CONSUMER_COUNT; i++) waitpid(conpid[i], NULL, WUNTRACED);
    cout<<endl<<"Back to main process.\nDetaching and deleting shared memory segment..."<<endl;
    // Detach shared memory segment
    shmdt(gptr);
    // Mark the segment to be destroyed
    shmctl(shmid, IPC_RMID, NULL);
    cout<<"Program successfully terminated."<<endl;
    return 0;
}
