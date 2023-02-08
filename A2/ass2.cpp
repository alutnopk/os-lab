#include <iostream>
#include <vector>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

using namespace std;

#define MAX_BUF_SIZE 1000

int runCMD(char **toks, int noofToks);
int runExternalCMD(char **toks, int noofToks, int in_fd, int out_fd);
char **Hpipes(char *line, int *numCMD);
char **Hcmds(char *cmd, int *tokenNos);
vector<string> glob(const string &pat);
void exit_error(char *s);

bool cntrlC = 0, cntrlZ = 0;
int currentPG = 0;
int wildcard = 0;

void sigint_handler(int signo)
{
    if(signo == SIGINT)
    {
        //do nothing
    }
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigint_handler);
    // signal(SIGTSTP, sigtstp_handler);
    char *cmdline = NULL;
    size_t n = 0;
    char **cmds;
    char **toks;
    int noOfCMD = 0;
    int nofToks = 0;
    int status;
    // int i;
    // printf("\033[H\033[J"); // clear everything from the screen, move cursor to top left
    do
    {
        wildcard=0;
        cmdline = (char *)malloc(sizeof(char) * MAX_BUF_SIZE);
        printf("\nshell> ");
        getline(&cmdline, &n, stdin);
        cmdline[strlen(cmdline) - 1] = '\0';
        printf("%slol", cmdline);
        cmds = Hpipes(cmdline, &noOfCMD);
        // printf("%d\n",noOfCMD);

        if (noOfCMD == 1)
        {
            toks = Hcmds(cmdline, &nofToks);
            // printf("%s\n",*toks);
            if (toks != NULL && nofToks > 0)
                status = runCMD(toks, nofToks);
        }
        else if (noOfCMD > 1)
        {
            int fd[2];
            int in_fd = 0;
            int pipeerror = 0;
            for (int i = 0; i < noOfCMD - 1; i++)
            {
                if (pipe(fd) == -1)
                {
                    pipeerror = 1;
                    break;
                }
                else
                {
                    toks = Hcmds(cmds[i], &nofToks);
                    if (nofToks > 0)
                    {
                        status = runExternalCMD(toks, nofToks, in_fd, fd[1]);
                    }
                    else
                    {
                        pipeerror = 1;
                        break;
                    }
                    close(fd[1]);
                    in_fd = fd[0];
                }
            }
            if (!pipeerror)
            {
                toks = Hcmds(cmds[noOfCMD - 1], &nofToks);
                if (nofToks > 0)
                {
                    status = runExternalCMD(toks, nofToks, in_fd, 1);
                }
                else
                {
                    pipeerror = 1;
                    break;
                }
            }
        }

        free(toks);
        free(cmdline);
        fflush(stdin);
        fflush(stdout);
    } while (status == EXIT_SUCCESS);
    return 0;
}

vector<string> glob(const string &pat)
{
    using namespace std;
    glob_t glob_result;
    glob(pat.c_str(), GLOB_TILDE, NULL, &glob_result);
    vector<string> ret;
    for (unsigned int i = 0; i < glob_result.gl_pathc; ++i)
    {
        ret.push_back(string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return ret;
}

int runCMD(char **toks, int noofToks)
{
    char buf[MAX_BUF_SIZE];

    if (strcmp(toks[0], "cd") == 0)
    {
        // printf("CD: %s",*toks);
        if (toks[1] == NULL)
        {
            exit_error("No directory entered.\n");
            return 0;
        }
        else if (chdir(toks[1]) == -1)
        {
            exit_error("cd operation failed.\n");
            return 0;
        }
    }

    else if (toks[0] != NULL)
    {
        // printf("OTHER: %s",*toks);
        return runExternalCMD(toks, noofToks, 0, 1);
    }
}

int runExternalCMD(char **toks, int noofToks, int in_fd, int out_fd)
{
    pid_t ret, wret;
    vector<string> files;
    int status;
    int i = 0;

    
    if (wildcard == 1)
    {
        i=0;
        printf("wildcard is 1\n");
        files = glob(toks[1]);
        for(i=0;i<files.size();i++)
        {
            toks[i+1] = (char*)files[i].c_str();
        }
        noofToks = files.size()+1;    
    }
    
    
    
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
            token = toks[i];
            if (!op)
            {
                if (strcmp(token, "&") == 0 || strcmp(token, ">") == 0 || strcmp(token, "<") == 0)
                {
                    op = 1;
                    cmmnd = i;
                }
            }
            if (strcmp(token, ">") == 0)
            {
                int redirect_out_fd = open(toks[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
                dup2(redirect_out_fd, STDOUT_FILENO);
            }
            if (strcmp(token, "<") == 0)
            {
                int redirect_in_fd = open(toks[i + 1], O_RDONLY);
                dup2(redirect_in_fd, STDIN_FILENO);
            }
        }
        if (!op)
            cmmnd = noofToks;

        char **token2 = (char **)malloc(cmmnd * sizeof(char *));

        if (token2 == NULL)
        {
            fprintf(stderr, "Error: allocation error\n");
            exit(EXIT_FAILURE);
        }

        int j;
        for (j = 0; j < cmmnd; j++)
        {
            token2[j] = toks[j];
        }
            token2[j] = NULL;
        if (execvp(token2[0], token2) == -1)
        {
            fprintf(stderr, "Error: exec failed\n");
            exit(EXIT_FAILURE);
        }
    
    }
    else if (ret < 0)
    {
        fprintf(stderr, "Error: fork failed\n");
        exit(EXIT_FAILURE);
    }
    
    if (strcmp(toks[noofToks - 1], "&") != 0)
    {
        do
        {
            wret = waitpid(ret, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return EXIT_SUCCESS;
}

char **Hpipes(char *line, int *numCMD)
{
    int bufsize = MAX_BUF_SIZE;
    char **cmds = (char **)malloc(bufsize * sizeof(char *));
    int lenofcmd = strlen(line);
    int k = 0;
    int cmdNo = 0;
    char p = '|';
    char dc = '\"';
    char sc = '\'';

    if (cmds == NULL)
    {
        fprintf(stderr, "Error: allocation error\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < bufsize; i++)
    {
        cmds[i] = (char *)malloc(lenofcmd * sizeof(char));
        if (!cmds[i])
        {
            fprintf(stderr, "Error: allocation error\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < lenofcmd;)
    {

        if (line[i] == p)
        {
            if (strlen(cmds[cmdNo]) <= 0)
            {
                fprintf(stderr, "Error: Syntax ERROR\n");

                *numCMD = 0;
                return NULL;
            }
            cmdNo++;
            if (cmdNo >= bufsize)
            {
                bufsize += MAX_BUF_SIZE;
                cmds = (char **)realloc(cmds, bufsize * sizeof(char *));
                if (cmds == NULL)
                {
                    fprintf(stderr, "Error: allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }
            k = 0;
            i++;
        }

        else if (line[i] == dc)
        {
            cmds[cmdNo][k] = line[i];
            i++;
            k++;
            while (line[i] != dc && i < lenofcmd)
            {
                cmds[cmdNo][k] = line[i];
                i++;
                k++;
            }
            cmds[cmdNo][k] = line[i];
            i++;
            k++;
        }

        else if (line[i] == sc)
        {
            cmds[cmdNo][k] = line[i];
            i++;
            k++;
            while (line[i] != sc && i < lenofcmd)
            {
                cmds[cmdNo][k] = line[i];
                i++;
                k++;
            }
            cmds[cmdNo][k] = line[i];
            i++;
            k++;
        }

        else
        {
            cmds[cmdNo][k] = line[i];
            i++;
            k++;
        }
    }
    *numCMD = cmdNo + 1;
    cmds[*numCMD] = NULL;
    return cmds;
}

char **Hcmds(char *cmd, int *tokenNos)
{
    *tokenNos = 0;
    int bufsize = MAX_BUF_SIZE;
    int cmdLen = strlen(cmd);
    int i, j, cmdNo = 0;
    char space = ' ';
    char nwl = '\n';
    char sc = '\'';

    char **tokens = (char **)malloc(bufsize * sizeof(char *));

    if (tokens == NULL)
    {
        fprintf(stderr, "Error: allocation error\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < bufsize; i++)
    {
        tokens[i] = (char *)malloc(cmdLen * sizeof(char));
        if (tokens[i] == NULL)
        {
            fprintf(stderr, "Error: allocation error\n");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < cmdLen;)
    {
        if (cmd[i] == space || cmd[i] == '\t' || cmd[i] == '\a' || cmd[i] == '\r' || cmd[i] == nwl)
        {
            if (strlen(tokens[(*tokenNos)]) > 0)
            {
                (*tokenNos)++;
            }
            cmdNo = 0;
            i++;
        }
        else if (cmd[i] == '\"')
        {
            if (tokens[(*tokenNos) - 1][0] == 'a' && tokens[(*tokenNos) - 1][1] == 'w' && tokens[(*tokenNos) - 1][2] == 'k')
            {
                fprintf(stderr, "Error: Syntax ERROR\n");
                return NULL;
            }
            if (cmd[i] == '?' || cmd[i] == '*')
                wildcard = 1;
            cmdNo = 0;
            j = i + 1;
            while (j < cmdLen && cmd[j] != '\"')
            {
                tokens[(*tokenNos)][cmdNo] = cmd[j];
                cmdNo++;
                j++;
            }
            if (strlen(tokens[(*tokenNos)]) > 0)
            {
                (*tokenNos)++;
            }
            cmdNo = 0;
            i = j + 1;
        }

        else if (cmd[i] == sc)
        {
            cmdNo = 0;
            j = i + 1;
            while (j < cmdLen && cmd[j] != sc)
            {
                tokens[(*tokenNos)][cmdNo] = cmd[j];
                cmdNo++;
                j++;
            }
            if (strlen(tokens[(*tokenNos)]) > 0)
            {
                (*tokenNos)++;
            }
            cmdNo = 0;
            i = j + 1;
        }
        else
        {
            if (cmd[i] == '?' || cmd[i] == '*') wildcard = 1;
            tokens[(*tokenNos)][cmdNo] = cmd[i];
            cmdNo++;
            if (i == cmdLen - 1)
            {
                if (strlen(tokens[(*tokenNos)]) > 0)
                {
                    (*tokenNos)++;
                }
            }
            i++;
        }
        if ((*tokenNos) >= bufsize)
        {
            bufsize += MAX_BUF_SIZE;
            tokens = (char **)realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                fprintf(stderr, "ERROR: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    return tokens;
}

void exit_error(char *s)
{
    perror(s);
    // exit(1);
}
