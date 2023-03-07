
#include "headers.h"

Graph gptr;

//pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


void* userSimulator(void*) {
    // Implement userSimulator thread here
    return NULL;
}

void* readPost(void*) {
    // Implement readPost thread here
    return NULL;
}

void* pushUpdate(void*) {
    // Implement pushUpdate thread here
    return NULL;
}


void* mainThread(void*) {

    //load the graph from the musae_git_edges.csv file



    // Create userSimulator thread
    pthread_t userThread;
    pthread_create(&userThread, NULL, userSimulator, NULL);

    // Create pool of 10 readPost threads
    std::vector<pthread_t> readThreads(10);
    for (int i = 0; i < 10; i++) {
        pthread_create(&readThreads[i], NULL, readPost, NULL);
    }

    // Create pool of 25 pushUpdate threads
    std::vector<pthread_t> pushThreads(25);
    for (int i = 0; i < 25; i++) {
        pthread_create(&pushThreads[i], NULL, pushUpdate, NULL);
    }

    // Wait for all threads to finish
    pthread_join(userThread, NULL);
    for (int i = 0; i < 10; i++) {
        pthread_join(readThreads[i], NULL);
    }
    for (int i = 0; i < 25; i++) {
        pthread_join(pushThreads[i], NULL);
    }

    return NULL;
}




int main() {
    
    // Create Main thread
    // pthread_t mainThread;
    // pthread_create(&mainThread, NULL, NULL, NULL);

    // Wait for main thread to finish
    // pthread_join(mainThread, NULL);

    Graph g;
    g.init("musae_git_edges.csv");
    g.print_graph();

    // gptr.init("musae_git_edges.csv");
    // gptr.populate_wall();
    // gptr->init("musae_git_edges.csv");
    // gptr->print_wall();
    // cout<<gptr->nodeCount<<endl;
    // cout<<gptr->nodelist[0].current;

    return 0;
}