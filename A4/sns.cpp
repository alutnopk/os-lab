
#include "headers.h"

#define G_FEEDSIZE 694200

Graph gptr;

queue<Action> globalFeed;
set<int> unreadNodes;
// TODO: Check if static initializers work correctly
pthread_mutex_t mutexGlobalFeed = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condGlobalFeed = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexUnreadNodes = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condUnreadNodes = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexSnslog = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexNodeFeed[MAX_NODES];

void* userSimulator(void*)
{
    cout<<endl<<"User simulator thread active...\n"<<endl;
    // create file named "sns.log" and write in it using fstream
    fstream snslog;

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
        pthread_mutex_lock(&mutexGlobalFeed);
        for(int i=0; i<allNodeActions.size(); i++)
        {
            globalFeed.push(allNodeActions[i]);
        }
        pthread_mutex_unlock(&mutexGlobalFeed);
        pthread_cond_broadcast(&condGlobalFeed);
        
        // print to sns.log and terminal
        pthread_mutex_lock(&mutexSnslog);
        snslog.open("sns.log", ios::out | ios::app);
        for(int i=0; i<allNodeActions.size(); i++)
        {
            Action a = allNodeActions[i];
            snslog<<"User "<<a.userId<<" of degree "<<a.userDegree<<" has performed "<<action_types[a.actionType]<<" no. "<<a.actionId<<" at timestamp "<<a.actionTime<<"\n";
            // cout<<"User "<<a.userId<<" of degree "<<a.userDegree<<" has performed "<<action_types[a.actionType]<<" no. "<<a.actionId<<" at timestamp "<<a.actionTime<<"\n";
        }
        snslog<<endl; cout<<endl;
        snslog.close();
        pthread_mutex_unlock(&mutexSnslog);
        
        cout<<endl<<"User simulator going to sleep...\n"<<endl;
        sleep(120);
    }
    pthread_exit(0);
}

void* pushUpdate(void* arg)
{
    int idx = *((int*)arg);
    vector<Action> currentActions; // action list to be currently sent by this thread
    while(1)
    {
        // cout<<idx<<" is waiting..."<<endl;
        currentActions.clear();
        Action act;
        int currentNode;
        fstream snslog;
        // pop action from globalFeed queue
        pthread_mutex_lock(&mutexGlobalFeed);
        while(globalFeed.empty())
            pthread_cond_wait(&condGlobalFeed, &mutexGlobalFeed);
        while(!globalFeed.empty())
        {
            act = globalFeed.front(); 
            globalFeed.pop();
            currentActions.push_back(act);
            // Keep popping actions from the same sender node
            currentNode = act.userId; if(currentNode != globalFeed.front().userId) break;
        }
        pthread_mutex_unlock(&mutexGlobalFeed);
        pthread_cond_broadcast(&condGlobalFeed);

        for(auto &nbr: gptr.nodelist[currentNode].nbrs) // for each neighbor
        {
            // push act into each neighbor
            pthread_mutex_lock(&mutexNodeFeed[nbr]);
            for(Action act: currentActions)
            {
                if(gptr.nodelist[nbr].orderType == 0) // priority based feed order
                {
                    gptr.nodelist[nbr].feed.push(make_pair(act.actionTime, act)); // TODO: add priority
                }
                else // chronological
                {
                    gptr.nodelist[nbr].feed.push(make_pair(act.actionTime, act));
                }
            }
            pthread_mutex_unlock(&mutexNodeFeed[nbr]);

            // Add node to unread feed nodes set
            if(!gptr.nodelist[nbr].feed.empty())
            {
                pthread_mutex_lock(&mutexUnreadNodes);
                    unreadNodes.insert(nbr);
                pthread_mutex_unlock(&mutexUnreadNodes);
                pthread_cond_broadcast(&condUnreadNodes);
            }
            // write to sns.log
            pthread_mutex_lock(&mutexSnslog);
            snslog.open("sns.log", ios::out | ios::app);
            for(Action act: currentActions)
            {
                snslog<<"Pushed action "<<action_types[act.actionType]<<"no. "<<act.actionId<<" from node "<<act.userId<<" to its neighbor "<<nbr<<"\n";
                // cout<<"Pushed action "<<action_types[act.actionType]<<" from node "<<act.userId<<" to its neighbor "<<nbr<<"\n";
            }
            snslog<<endl; cout<<endl;
            snslog.close();
            pthread_mutex_unlock(&mutexSnslog);
        }
    }
    pthread_exit(0);
}

void* readPost(void* arg)
{
    int idx = *((int*)arg);
    int currentNode;
    stringstream sstr;
    fstream snslog;
    while(1)
    {
        // acquire element whose feed queue is to be read
        currentNode = -1;
        sstr.str("");
        pthread_mutex_lock(&mutexUnreadNodes);
        while(unreadNodes.empty())
            pthread_cond_wait(&condUnreadNodes, &mutexUnreadNodes);
        currentNode = *(unreadNodes.begin()); unreadNodes.erase(unreadNodes.begin());
        pthread_mutex_unlock(&mutexUnreadNodes);
        pthread_cond_broadcast(&condUnreadNodes);

        // read the feed and print
        pthread_mutex_lock(&mutexNodeFeed[currentNode]);
        if(!gptr.nodelist[currentNode].feed.empty())
        {
            // dequeue as per priority
            Action act = gptr.nodelist[currentNode].feed.top().second;
            gptr.nodelist[currentNode].feed.pop();
            sstr<<currentNode<<" has read "<<action_types[act.actionType]<<" no. "<<act.actionId<<"posted by "<<act.userId<<" at timestamp"<<act.actionTime<<"\n";
        }
        pthread_mutex_unlock(&mutexNodeFeed[currentNode]);
        string outs = sstr.str();
        // write to sns.log
        pthread_mutex_lock(&mutexSnslog);
        snslog.open("sns.log", ios::out | ios::app);
        snslog<<outs; cout<<outs;
        snslog<<endl; cout<<endl;
        snslog.close();
        pthread_mutex_unlock(&mutexSnslog);
    }
    pthread_exit(0);
}


int main()
{
    pthread_t userThread;
    vector<pthread_t> pushThreads(25);
    std::vector<pthread_t> readThreads(10);

    // load the graph from the musae_git_edges.csv file
    gptr.init("musae_git_edges.csv");
    gptr.print_graph();

    // gptr.populate_wall();
    // gptr.print_wall();

    remove("sns.log");
    // initialize mutexes and condition variables
    for (int i = 0; i < MAX_NODES; ++i)
    {
        pthread_mutex_init(&mutexNodeFeed[i], NULL);
    }

    queue<Action> empty; swap(globalFeed, empty); // initializing empty queue
    unreadNodes.clear();

    // Create userSimulator thread
    pthread_create(&userThread, NULL, userSimulator, NULL);
    
    // Create pool of 25 pushUpdate threads
    for (int i = 0; i < 25; i++)
    {
        pthread_create(&pushThreads[i], NULL, pushUpdate, (void*)&i);
    }

    // Create pool of 10 readPost threads
    for (int i = 0; i < 10; i++)
    {
        pthread_create(&readThreads[i], NULL, readPost, NULL);
    }

    // Wait for all threads to finish
    pthread_join(userThread, NULL);
    for (int i = 0; i < 25; i++)
    {
        pthread_join(pushThreads[i], NULL);
    }
    
    for (int i = 0; i < 10; i++)
    {
        pthread_join(readThreads[i], NULL);
    }
    cout << "Main thread ends."<<endl;
    return 0;
}