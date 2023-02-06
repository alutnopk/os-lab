#include <iostream>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

vector<string> split_string(const string &input);
vector<string,vector<string>> handle_pipes(const string &line, int *noOfCMDS);
int executeCMD(vector<string> tokens);
int executeExit();
int executeCD(vector<string> tokens);


int main()
{
    while (true)
    {
        cout << "\n$ ";
        string line;
        getline(cin, line);
        if (line.empty())
        {
            continue;
        }
        int noCMDs;
        vector<string, vector<string>> cmds = handle_pipes(line,&noCMDs);
        printf("%d",noCMDs);
        /*vector<string> tokens = split_string(line);
        if (tokens[0] == "cd")
        {
            executeCD(tokens);
        }
        else if(tokens[0]=="exit")
        {
            executeExit();
        }
        else
        {

            executeCMD(tokens);
        }*/
    }

    return 0;
}

vector<string> split_string(const string &input)
{
    vector<string> tokens;
    string current_token;
    for (char c : input)
    {
        if (c == ' ')
        {
            tokens.push_back(current_token);
            current_token.clear();
        }
        else
        {
            current_token += c;
        }
    }
    tokens.push_back(current_token);
    return tokens;
}

vector<string,vector<string>> handle_pipes(const string &line, int *noOfCMDS)
{
    vector<string,vector<string>> tokens;
    string current_token;
    int CMDno=0;

    for (size_t i;i<line.size();)
    {
        if(line[i]=='|')
        {
            if(tokens[CMDno].empty())
            {
                fprintf(stderr, "Error: Syntax ERROR\n");
                CMDno = 0;
                return tokens;
            }
            CMDno++;
            i++;
        }
        else if (line[i] == '\"')
        {
            tokens[CMDno].push_back(line[i]);
            i++;
            while(line[i] != '\"' && i < line.size())
            {
                tokens[CMDno].push_back(line[i]); 
                i++;
            }
            tokens[CMDno].push_back(line[i]);
            i++;
        }
        else if(line[i]=='\'')
        {
            tokens[CMDno].push_back(line[i]);
            i++;
            while(line[i] != '\'' && i < line.size())
            {
                tokens[CMDno].push_back(line[i]);
                i++;
            }
            tokens[CMDno].push_back(line[i]);
            i++;
        }
        else
        {
            tokens[CMDno].push_back(line[i]);
        }
    }
    *noOfCMDS = CMDno+1;
    tokens[CMDno+1].push_back('\0');
    
    return tokens;
}

int executeCMD(vector<string> tokens)
{
    vector<char *> argv(tokens.size() + 1);
    for (size_t i = 0; i < tokens.size(); i++)
    {
        argv[i] = &tokens[i][0];
    }
    argv[tokens.size()] = nullptr;

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
    }
    else if (pid == 0)
    {
        execvp(argv[0], argv.data());
        perror("execvp");
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
    }
    return 0;
}

int executeExit()
{
        exit(0);
}

int executeCD(vector<string> tokens)
{
        if (tokens.size() != 2)
        {
            cout << "usage: cd <dir>" << endl;
        }
        else
        {
            if (chdir(tokens[1].c_str()) == -1)
            {
                perror("chdir");
            }
        }
        return 0;
}
