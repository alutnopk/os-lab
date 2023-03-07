#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

typedef struct AdjList
{
    int current;      // current node
    vector<int> nbrs; // array of neighbors of current node
} AdjList;

class Graph
{
public:
    vector<AdjList> nodelist; // adjacency list

    int init(string filepath);
    void print_graph();
};

int Graph::init(string filepath)
{
    fstream fs;
    fs.open(filepath, ios::in);
    if (!fs)
    {
        return -1; // error opening file
    }
    int k = 0;
    if (fs.is_open())
    {
        string line;
        int x, y;
        getline(fs, line); // skip first line (header)
        while(getline(fs, line))
        {
            // k++;
            // if(k==1) continue; // skip first line (header)
            // read x,y from input file
            stringstream slin(line);
            string token;
            getline(slin, token, ',');
            x = stoi(token);
            getline(slin, token, ',');
            y = stoi(token);
            
            // cout << x << " " << y << endl;
            
            bool found = false;
            for (auto &node : nodelist)
            {
                if (node.current == x)
                {
                    node.nbrs.push_back(y);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                AdjList node;
                node.current = x;
                node.nbrs.push_back(y);
                nodelist.push_back(node);
            }
            
        }
    }
   
    return 0; // success
}

void Graph::print_graph()
{
    for (auto &node : nodelist)
    {
        cout << node.current << ": ";
        for (auto &nbr : node.nbrs)
        {
            cout << nbr << " ";
        }
        cout << endl;
    }
}

int main()
{
    Graph g;
    g.init("musae_git_edges.csv");
    // g.print_graph();
    // cout << g.nodelist.size() << endl;
    return 0;
}