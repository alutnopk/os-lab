#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_BUF_SIZE 256
///////////////// plag code
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}
////////////// plag code end
void exit_error(char* s)
{
    perror(s); exit(1);
}
int main(int argc, char** argv)
{
    char buf[MAX_BUF_SIZE];
    char* cmdline=NULL;
    size_t n=0;
    char **toks;
    int keep_running = 1;
    // int i;
    printf("\033[H\033[J"); // clear everything from the screen, move cursor to top left
    for(;keep_running;)
    {
        printf("noobshell> ");
        // read the line
        getline(&cmdline, &n, stdin);
        // separate into tokens
        // strtok(cmdline, " \t\n\r\a"); 
        // handle NULL case, no tokens present
        toks = lsh_split_line(cmdline);
        // args is an array of pointers containing each token
        // execute commands

        // first check if cd or pwd is used
        if(strcmp(toks[0], "cd") == 0)
        {
            if(toks[1] == NULL) exit_error("No directory entered.\n");
            if(chdir(toks[1]) == -1) exit_error("cd operation failed.\n");
            goto next_iter;
        }
        if(strcmp(toks[0], "pwd") == 0)
        {
            memset(buf, 0, MAX_BUF_SIZE);
            if(getcwd(buf, MAX_BUF_SIZE) == NULL) exit_error("pwd failed.\n");
            printf("%s\n",buf);
            continue;
        }
        if (toks[0] != NULL)
        {
            pid_t ret;
            ret = fork();
            if(ret == 0) // child process
            {
                execvp(toks[0], toks);
                // if this point is reached, it means execvp failed
                exit_error("Child execution failure.\n");
            }
            else if (ret < 0)
                exit_error("Fork failure.\n");
            else // parent process
            {
                int status;
                do 
                {
                    waitpid(ret, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }
        }

        // free(cmdline);
        free(toks);
        next_iter:
        // sleep(3);
    }
    return 0;
}