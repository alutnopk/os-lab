#include <iostream>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_BUF_SIZE 128
using namespace std;

vector<string> split_string(const string &input);
void handle_pipes(const string &line, int &noOfCMDS, vector<string> &tokens);
void handle_cmds(string line, int &tokenNos, vector<string> &toks);
int executeCMD(vector<string> input);
int executeExit();
int executeCD(string cmd);
int runExternalCMD(vector<string> toks, int noofToks, int in_fd, int out_fd);
string trim_string(string s)
{
    int i = 0;
    while (i < s.size() && s[i] == ' ')
    {
        i++;
    }
    s = s.substr(i);
    i = s.size() - 1;
    while (i >= 0 && s[i] == ' ')
    {
        i--;
    }
    s = s.substr(0, i + 1);
    return s;
}

int main()
{
    while (true)
    {
        cout << "\n$ ";
        string line;
        int nofToks;

        getline(cin, line);
        if (line.empty())
        {
            continue;
        }
        int noCMDs;
        vector<string> cmds, toks(MAX_BUF_SIZE);

        handle_pipes(line, noCMDs, cmds);
        // printf("%d",noCMDs);
        // vector<string> tokens = split_string(line);
        if (noCMDs == 1)
        {
            if (cmds[0][0] == 'c' && cmds[0][1] == 'd' && cmds[0][2] == ' ')
            {
                executeCD(cmds[0]);
            }
            else if (cmds[0][0] == 'e' && cmds[0][1] == 'x' && cmds[0][2] == 'i' && cmds[0][3] == 't')
            {
                executeExit();
            }
            else 
            {
                runExternalCMD(cmds, cmds.size(), 0, 1);
            }
            handle_cmds(line, nofToks, toks);

        }
        else if(noCMDs>1)
        {

            
            runExternalCMD(cmds, cmds.size(), 0, 1);
        }
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

void handle_pipes(const string &line, int &noOfCMDS, vector<string> &tokens)
{
    // vector<vector<string>> tokens;
    string current_token;
    int CMDno = 0;
    // cout<<line<<endl;
    tokens.push_back("");
    for (size_t i = 0; i < line.size();)
    {
        if (line[i] == '|')
        {
            if (tokens[CMDno].empty())
            {
                fprintf(stderr, "Error: Syntax ERROR\n");
                CMDno = 0;
                // return tokens;
            }
            tokens.push_back("");
            CMDno++;
            i++;
        }
        else if (line[i] == '\"')
        {
            tokens[CMDno] += line[i];
            i++;
            while (line[i] != '\"' && i < line.size())
            {
                tokens[CMDno] += line[i];
                i++;
            }
            tokens[CMDno] += line[i];
            i++;
        }
        else if (line[i] == '\'')
        {
            tokens[CMDno] += line[i];
            i++;
            while (line[i] != '\'' && i < line.size())
            {
                tokens[CMDno] += line[i];
                i++;
            }
            tokens[CMDno] += line[i];

            i++;
        }
        else
        {
            tokens[CMDno] += line[i];
            i++;
        }
    }
    // noOfCMDS = CMDno+1;
    // tokens[CMDno+1] = string(1,'\0');
    for (int i = 0; i < tokens.size(); i++)
    {
        tokens[i] = trim_string(tokens[i]);
    }

    for (auto x : tokens)
    {
        cout << x << endl;
    }
    // return tokens;
}
int runExternalCMD(vector<string> toks, int noofToks, int in_fd, int out_fd)
{
    pid_t ret, wret;
    // ret = fork();
    // printf("%s\n",*toks);
    int status;
    ret = fork();
    char *token;
    if (ret == 0)
    {
        if (in_fd != 0)
        {
            dup2(in_fd, 0);
            close(in_fd);
        }
        if (out_fd != 1)
        {
            dup2(out_fd, 1);
            close(out_fd);
        }
        int cmmnd = 0;
        int op = 0;

        for (int i = 0; i < noofToks; i++)
        {
            token = (char *)toks[i].c_str();
            if (!op)
            {
                if (strcmp(token, "&") == 0 || strcmp(token, ">") == 0 || strcmp(token, "<") == 0)
                {
                    op = 1;
                    cmmnd = i;
                }
                if (strcmp(token, ">") == 0)
                {
                    int redirect_out_fd = open(toks[i + 1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    dup2(redirect_out_fd, STDOUT_FILENO);
                }
                if (strcmp(token, "<") == 0)
                {
                    int redirect_in_fd = open(toks[i + 1].c_str(), O_RDONLY);
                    dup2(redirect_in_fd, STDIN_FILENO);
                }
            }
            if (!op)
                cmmnd = noofToks;

            // char **token2 = malloc(cmmnd * sizeof(char *));
            vector<char *> token2(cmmnd);

            // if (token2 == NULL)
            // {
            //     fprintf(stderr, "Error: allocation error\n");
            //     exit(EXIT_FAILURE);
            // }

            int j;
            for (j = 0; j < cmmnd; j++)
            {
                token2[j] = (char *)toks[j].c_str();
            }
            token2.push_back(NULL);

            if (execvp(token2[0], token2.data()) == -1)
            {
                fprintf(stderr, "Error: exec failed\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    else if (ret < 0)
    {
        fprintf(stderr, "Error: fork failed\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        if (toks[noofToks - 1] != "&")
        {
            do
            {
                wret = waitpid(ret, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }

    return EXIT_SUCCESS;
}

void handle_cmds(string line, int &tokenNos, vector<string> &toks)
{
    tokenNos = 0;
    int bufsize = MAX_BUF_SIZE;
    int cmdLen = line.length();
    int i, j, cmdNo = 0;
    char space = ' ';
    char nwl = '\n';
    char sc = '\'';

    // char **tokens = malloc(bufsize * sizeof(char *));


    // if (tokens == NULL)
    // {
    //     fprintf(stderr, "Error: allocation error\n");
    //     exit(EXIT_FAILURE);
    // }
    // for (int i = 0; i < bufsize; i++)
    // {
    //     toks[i] = malloc(cmdLen * sizeof(char));
    //     if (tokens[i] == NULL)
    //     {
    //         fprintf(stderr, "Error: allocation error\n");
    //         exit(EXIT_FAILURE);
    //     }
    // }

    for (i = 0; i < cmdLen;)
    {
        if (line[i] == space || line[i] == '\t' || line[i] == '\a' || line[i] == '\r' || line[i] == nwl)
        {
            if (toks[(tokenNos)].length() > 0)
            {
                (tokenNos)++;
            }
            cmdNo = 0;
            i++;
        }
        else if (line[i] == '\"')
        {
            if (toks[(tokenNos) - 1][0] == 'a' && toks[(tokenNos) - 1][1] == 'w' && toks[(tokenNos) - 1][2] == 'k')
            {
                fprintf(stderr, "Error: Syntax ERROR\n");
                return;
            }
            cmdNo = 0;
            j = i + 1;
            while (j < cmdLen && line[j] != '\"')
            {
                toks[(tokenNos)][cmdNo] = line[j];
                cmdNo++;
                j++;
            }
            if (toks[(tokenNos)].length() > 0)
            {
                (tokenNos)++;
            }
            cmdNo = 0;
            i = j + 1;
        }

        else if (line[i] == sc)
        {
            cmdNo = 0;
            j = i + 1;
            while (j < cmdLen && line[j] != sc)
            {
                toks[(tokenNos)][cmdNo] = line[j];
                cmdNo++;
                j++;
            }
            if (toks[(tokenNos)].length() > 0)
            {
                (tokenNos)++;
            }
            cmdNo = 0;
            i = j + 1;
        }
        else
        {
            toks[(tokenNos)][cmdNo] = line[i];
            cmdNo++;
            if (i == cmdLen - 1)
            {
                if (toks[(tokenNos)].length() > 0)
                {
                    (tokenNos)++;
                }
            }
            i++;
        }
        // if ((tokenNos) >= bufsize)
        // {
        //     bufsize += MAX_BUF_SIZE;
        //     toks = realloc(toks, bufsize * sizeof(char *));
        //     if (!toks)
        //     {
        //         fprintf(stderr, "ERROR: allocation error\n");
        //         exit(EXIT_FAILURE);
        //     }
        // }
    }

    return;
}

int executeCMD(string input)
{
    vector<char *> argv;
    char *token = strtok(&input[0], " ");
    while (token != NULL)
    {
        argv.push_back(token);
        token = strtok(NULL, " ");
    }
    argv.push_back(NULL);

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

int executeCD(string cmd)
{
    char *line = (char *)cmd.c_str();
    strtok(line, " "); // eat up cd
    char *dir = strtok(NULL, "\"");
    cout << dir << endl;
    if (dir == NULL)
    {
        cout << "usage: cd <dir>" << endl;
    }
    else
    {
        if (chdir(dir) == -1)
        {
            perror("chdir");
        }
    }
    return 0;
}
