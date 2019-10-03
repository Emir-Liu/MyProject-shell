#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define sh_rl_bufsize 1024
#define sh_tok_bufsize 64
#define sh_tok_delim " "

void sh_loop(void);
char *sh_read_line(void);
char **sh_split_line(char *);
int sh_execute(char **);
int sh_launch(char **);
int sh_num_builtins(void);
int sh_cd(char **);
int sh_help(char **);
int sh_exit(char **);
int sh_num_builtins(void);
char *builtin_str[] =
{
    "cd",
    "help",
    "exit"
};

int (*builtin_func[])(char **) =
{
    &sh_cd,
    &sh_help,
    &sh_exit
};

int main()
{
    sh_loop();
    return EXIT_SUCCESS;
}

void sh_loop(void)
{
    char *line;
    char **args;
    int status;

    do{
        printf("> ");
        line = sh_read_line();
        args = sh_split_line(line);
        status = sh_execute(args);

        free(line);
        free(args);
    }while(status);
}

char *sh_read_line(void)
{
    int bufsize = sh_rl_bufsize;
    int position = 0;
    char *buffer = malloc(sizeof(char)*bufsize);
    int c;

    if(!buffer)
    {
        printf("sh : allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        c = getchar();

        if(c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;
        if(position >= bufsize)
        {
            bufsize += sh_rl_bufsize;
            buffer = realloc(buffer,bufsize);
            if(!buffer)
            {
                printf("lsh :allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **sh_split_line(char *line)
{
    int bufsize = sh_tok_bufsize,position = 0;
    char **tokens = malloc(bufsize*sizeof(char*));
    char *token;

    if(!tokens)
    {
        printf("sh : allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line,sh_tok_delim);
    while(token != NULL)
    {
        tokens[position] = token;
        position++;

        if(position >= bufsize)
        {
            bufsize += sh_tok_bufsize;
            tokens = realloc(tokens,bufsize*sizeof(char*));
            if(!tokens)
            {
                printf("sh : allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL,sh_tok_delim);
    }
    tokens[position] = NULL;
    return tokens;
}

int sh_execute(char **args)
{
    if(args[0] == NULL)
    {
        return 1;
    }

    for(int i = 0;i < sh_num_builtins();i++)
    {
        if(strcmp(args[0],builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }

    return sh_launch(args);
}

int sh_num_builtins(void)
{
    return sizeof(builtin_str)/sizeof(char *);
}

int sh_cd(char **args)
{
  if (args[1] == NULL) {
    printf("sh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}
int sh_help(char **args)
{
    int i;
    printf("Emir liu's SH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < sh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int sh_exit(char **args)
{
    return 0;
}

int sh_launch(char **args)
{
    int pid,wpid;
    int status;

    pid = fork();

    if(pid == 0)
    {
        if(execvp(args[0],args) == -1)
        {
            perror("sh ");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0)
    {
        perror("sh ");
    }
    else
    {
        do
        {
            wpid = waitpid(pid,&status,WUNTRACED);
        }
        while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}
