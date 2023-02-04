#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

void exit_error(string s);
void debug_print(void* ptr, size_t n)
{
    unsigned char* hehu = (unsigned char*) ptr;
    cout<<"Data stored:"<<endl;
    for(int i=0; i<(int)n; i++)
    {
        printf("%02x ", *(hehu+i));
    }
    cout<<endl;
}
int main()
{
    int keep_running = 1;
    string input;
    vector<char*> args;

    cout<<"\033[H\033[J";
    for(;keep_running;)
    {
        cout << "cppshell> ";
        input = ""; args.clear();
        getline(cin, input);
        for(int i=0; i< input.length(); i++){cout<<input[i]<<endl;}
        char* token = strtok(&input[0], " ");
        while (token != NULL)
        {
            args.push_back(token);
            token = strtok(NULL, " ");
        }
        args.push_back(NULL);
        // for(auto x: args) cout<<x<<endl;
        if (strcmp(args[0], "exit") == 0) break;
        if(args[0])
        {
            pid_t ret = fork();
            if (ret == 0)
            {
                execvp(args[0], &args[0]);
                exit_error("ERROR: execvp failed\n");
            }
            else if (ret < 0)
            {
                exit_error("ERROR: fork failure\n");
            }
            else
            {
                int status;
                do
                {
                    waitpid(ret, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }
        }
    }
    return 0;
}

void exit_error(string s)
{
    cerr<<s<<endl;
    exit(EXIT_FAILURE);
}