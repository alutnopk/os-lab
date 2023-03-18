
#include "headers.h"
<<<<<<< HEAD
#include <cmath>
=======
>>>>>>> origin/pontu

#define G_FEEDSIZE 694200

Graph gptr;

queue<Action> globalFeed;
<<<<<<< HEAD

pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexlock = PTHREAD_MUTEX_INITIALIZER;

void *userSimulator(void *)
{

    // create file named "sns.log" and write in it using fstream
    fstream file;
    file.open("sns.log", ios::out);
    
    // Implement userSimulator thread here
    // choose 100 random nodes from the graph (between node ids 1 and 37,700). Then for each node, generate n actions, n needs to be proportional to the log 2 (degree of the node)
    while (1)
    {
        for(int j = 0; j< 4 ;j++)
        {
            vector<Action> temp;
            // cout<<"userSimulator thread running in loop"<<endl;
            for (int i = 0; i < 25; i++)
            {

                int randomNo = (rand() % 37700) + 1;
                
                auto it = find_if(gptr.nodelist.begin(), gptr.nodelist.end(), [randomNo](const AdjList &obj)
                                { return obj.current == randomNo; });
                if (it != gptr.nodelist.end())
                {
                    // file << "Random Node generated: " << randomNo << endl;
                    int index = distance(gptr.nodelist.begin(), it);
                    int n = 10 * (1 + log2(gptr.nodelist[index].nbrs.size()));
                    // file << "Number of actions generated for node:"<<randomNo<<"is"<< n << endl;
                    // file << "Degree of node: " << gptr.nodelist[index].nbrs.size() << endl;
                    for (int j = 0; j < n; j++)
                    {
                        int action = rand() % 3;

                        if (action == 0)
                        {
                            // generate post
                            Action newAct;
                            newAct.actionId = gptr.nodelist[index].postNo;
                            gptr.nodelist[index].postNo++;
                            newAct.userId = gptr.nodelist[index].current;
                            newAct.actionType = 0;
                            newAct.senderDegree = gptr.nodelist[index].nbrs.size();
                            newAct.actionTime = time(NULL);
                            gptr.nodelist[index].wall.push(newAct);
                            temp.push_back(newAct);
                            // pthread_mutex_lock(&mutexlock);
                            // while(globalFeed.size() >= G_FEEDSIZE)
                            //     pthread_cond_wait(&cv, &mutexlock);
                            // globalFeed.push(newAct);
                            // pthread_mutex_unlock(&mutexlock);
                            // file << "Post generated by " << gptr.nodelist[index].current << endl;
                            cout << "Post generated by " << gptr.nodelist[index].current << endl;
                        }
                        else if (action == 1)
                        {
                            // generate comment
                            Action newAct;
                            newAct.actionId = gptr.nodelist[index].commentsNo;
                            gptr.nodelist[index].commentsNo++;
                            newAct.userId = gptr.nodelist[index].current;
                            newAct.actionType = 1;
                            newAct.senderDegree = gptr.nodelist[index].nbrs.size();
                            newAct.actionTime = time(NULL);
                            gptr.nodelist[index].wall.push(newAct);
                            temp.push_back(newAct);
                            // pthread_mutex_lock(&mutexlock);
                            // while(globalFeed.size() == 0)
                            //     pthread_cond_wait(&cv, &mutexlock);
                            // globalFeed.push(newAct);
                            // pthread_mutex_unlock(&mutexlock);
                            // file << "Comment generated by " << gptr.nodelist[index].current << endl;
                            cout << "Comment generated by " << gptr.nodelist[index].current << endl;
                        }
                        else
                        {
                            // generate like
                            Action newAct;
                            newAct.actionId = gptr.nodelist[index].likesNo;
                            gptr.nodelist[index].likesNo++;
                            newAct.userId = gptr.nodelist[index].current;
                            newAct.actionType = 2;
                            newAct.senderDegree = gptr.nodelist[index].nbrs.size();
                            newAct.actionTime = time(NULL);
                            gptr.nodelist[index].wall.push(newAct);
                            temp.push_back(newAct);
                            // pthread_mutex_lock(&mutexlock);
                            // while(globalFeed.size() == 0)
                            //     pthread_cond_wait(&cv, &mutexlock);
                            // globalFeed.push(newAct);
                            // pthread_mutex_unlock(&mutexlock);
                            // file << "Like generated by " << gptr.nodelist[index].current << endl;
                            cout << "Like generated by " << gptr.nodelist[index].current << endl;
                        }
                    }
                }
=======
unordered_set<int> unreadNodes;
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
        sleep(5);
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
>>>>>>> origin/pontu
            }
            pthread_mutex_lock(&mutexlock);
            while (globalFeed.size() >= G_FEEDSIZE)
                pthread_cond_wait(&cv, &mutexlock);
            for (int i = 0; i < temp.size(); i++)
            {
                globalFeed.push(temp[i]);
            }
            pthread_mutex_unlock(&mutexlock);
        }
<<<<<<< HEAD
        sleep(5);
=======
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
        snslog<<endl;
        snslog.close();
        pthread_mutex_unlock(&mutexSnslog);
        
        cout<<endl<<"User simulator going to sleep...\n"<<endl;
        // sleep(120);
>>>>>>> origin/pontu
    }
    pthread_exit(0);
}

<<<<<<< HEAD
void *readPost(void *)
=======
void* pushUpdate(void* arg)
>>>>>>> origin/pontu
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
            // TODO: add priority condition here
            // push act into each neighbor
            pthread_mutex_lock(&mutexNodeFeed[nbr]);
            for(Action act: currentActions)
            {
                gptr.nodelist[nbr].feed.push(act);
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
            snslog<<endl;
            snslog.close();
            pthread_mutex_unlock(&mutexSnslog);
        }
    }
    pthread_exit(0);
}

<<<<<<< HEAD
void *pushUpdate(void *)
=======
void* readPost(void* arg)
>>>>>>> origin/pontu
{
    int idx = *((int*)arg);

    pthread_exit(0);
}

<<<<<<< HEAD
void *mainThread(void *)
{

    // load the graph from the musae_git_edges.csv file
    gptr.init("musae_git_edges.csv");
    // gptr.print_graph();
    // gptr.populate_wall();
    // gptr.print_wall();

    // Create userSimulator thread
    pthread_t userThread;
    pthread_create(&userThread, NULL, userSimulator, NULL);
    // cout<<"userSimulator thread created"<<endl;

    // // Create pool of 25 pushUpdate threads
    // std::vector<pthread_t> pushThreads(25);
    // for (int i = 0; i < 25; i++) {
    //     pthread_create(&pushThreads[i], NULL, pushUpdate, NULL);
    // }
=======

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
>>>>>>> origin/pontu

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
    // for (int i = 0; i < 10; i++)
    // {
    //     pthread_create(&readThreads[i], NULL, readPost, NULL);
    // }

<<<<<<< HEAD
    // // Wait for all threads to finish
    pthread_join(userThread, NULL);
    // for (int i = 0; i < 25; i++) {
    //     pthread_join(pushThreads[i], NULL);
    // }

    // for (int i = 0; i < 10; i++) {
    //     pthread_join(readThreads[i], NULL);
    // }

    return NULL;
}

int main()
{

    // Create Main thread
    pthread_t mThread;
    pthread_create(&mThread, NULL, mainThread, NULL);

    // Wait for main thread to finish
    pthread_join(mThread, NULL);

    // gptr.init("musae_git_edges.csv");
    // gptr.populate_wall();
    // gptr->init("musae_git_edges.csv");
    // gptr->print_wall();
    // cout<<gptr->nodeCount<<endl;
    // cout<<gptr->nodelist[0].current;

=======
    // Wait for all threads to finish
    pthread_join(userThread, NULL);
    for (int i = 0; i < 25; i++)
    {
        pthread_join(pushThreads[i], NULL);
    }
    
    // for (int i = 0; i < 10; i++)
    // {
    //     pthread_join(readThreads[i], NULL);
    // }
    cout << "Main thread ends."<<endl;
>>>>>>> origin/pontu
    return 0;
}