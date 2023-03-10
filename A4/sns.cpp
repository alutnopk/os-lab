
#include "headers.h"

#define G_FEEDSIZE 694200

Graph gptr;

queue<Action> globalFeed;

pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t globalFeedMutex = PTHREAD_MUTEX_INITIALIZER, snslogMutex = PTHREAD_MUTEX_INITIALIZER;


void* userSimulator(void*)
{
    cout<<endl<<"User simulator thread active...\n"<<endl;
    // create file named "sns.log" and write in it using fstream
    fstream snslog;
    // Implement userSimulator thread here

    random_device rd; // non-deterministic random number generator (RNG)
    mt19937 gen(rd()); // Mersenne twister seeded with the generator
    uniform_int_distribution<> distNode(0,MAX_NODES-1); // URNG of required range
    uniform_int_distribution<> distAction(0,2);
    // vector<int> choices(100, -1);
    // for(int i=0; i<100; i++){ choices[i] = dist(gen); } // optionally could store them as an array
    int currentNode = -1;
    Action currentAction;
    vector<Action> allNodeActions;
    int currentActionCount = 0;

    while(1)
    {
        allNodeActions.clear();
        for (int i=0;i<100;i++)
        {
            currentNode = distNode(gen); // assumption: random choices are distinct
            currentActionCount = 10*(1 + log2(gptr.nodelist[currentNode].degree));
            for(int j=0;j<currentActionCount;j++)
            {
                currentAction.userId = currentNode;
                int actNo = distAction(gen);
                currentAction.actionType = actNo;
                currentAction.actionId = (gptr.nodelist[currentNode].actionCount[actNo])++;
                currentAction.userDegree = gptr.nodelist[currentNode].degree;
                currentAction.actionTime = time(NULL); // TODO: Consider using a high-res clock
                // push this action to the wall of the current node and to allNodeActions
                gptr.nodelist[currentNode].wall.push(currentAction);
                allNodeActions.push_back(currentAction);
            }
        }
        // TODO: Decide if pushing all the actions to global queue is optimal
        pthread_mutex_lock(&globalFeedMutex);
        for(int i=0; i<allNodeActions.size(); i++)
        {
            globalFeed.push(allNodeActions[i]);
        }
        pthread_mutex_unlock(&globalFeedMutex);
        
        // print to sns.log and terminal
        pthread_mutex_lock(&snslogMutex);
        snslog.open("sns.log", ios::out);
        for(int i=0; i<allNodeActions.size(); i++)
        {
            Action a = allNodeActions[i];
            snslog<<"User "<<a.userId<<" of degree "<<a.userDegree<<" has performed "<<action_types[a.actionType]<<" no. "<<a.actionId<<" at timestamp "<<a.actionTime<<"\n";
            cout<<"User "<<a.userId<<" of degree "<<a.userDegree<<" has performed "<<action_types[a.actionType]<<" no. "<<a.actionId<<" at timestamp "<<a.actionTime<<"\n";
        }
        snslog<<endl;
        snslog.close();
        pthread_mutex_unlock(&snslogMutex);
        
        cout<<endl<<"User simulator going to sleep...\n"<<endl;
        sleep(120);
    }
    pthread_exit(0);
}

void* readPost(void*)
{
    // Implement readPost thread here
    cout<<"readpost wheee"<<endl;
    pthread_exit(0);
}

void* pushUpdate(void*)
{
    cout<<"pushupdate wheee"<<endl;
    // Implement pushUpdate thread here
    pthread_exit(0);
}

int main()
{
    pthread_t userThread;
    vector<pthread_t> pushThreads(25);
    std::vector<pthread_t> readThreads(10);

    // load the graph from the musae_git_edges.csv file
    gptr.init("musae_git_edges.csv");
    // gptr.print_graph();

    // gptr.populate_wall();
    // gptr.print_wall();

    // Create userSimulator thread
    pthread_create(&userThread, NULL, userSimulator, NULL);
    
    // Create pool of 25 pushUpdate threads
    // for (int i = 0; i < 25; i++)
    // {
    //     pthread_create(&pushThreads[i], NULL, pushUpdate, NULL);
    // }

    // Create pool of 10 readPost threads
    // for (int i = 0; i < 10; i++)
    // {
    //     pthread_create(&readThreads[i], NULL, readPost, NULL);
    // }

    // Wait for all threads to finish
    pthread_join(userThread, NULL);
    // for (int i = 0; i < 25; i++)
    // {
    //     pthread_join(pushThreads[i], NULL);
    // }
    
    // for (int i = 0; i < 10; i++)
    // {
    //     pthread_join(readThreads[i], NULL);
    // }
    cout << "Main thread ends."<<endl;
    return 0;
}