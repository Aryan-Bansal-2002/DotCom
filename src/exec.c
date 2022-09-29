#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "args.h"
#include "discover.h"
#include "echo.h"
#include "exec.h"
#include "history.h"
#include "jobs.h"
#include "ls.h"
#include "misc.h"
#include "path.h"
#include "parse.h"
#include "pinfo.h"
#include "proc.h"
#include "prompt.h"
#include "sig.h"

extern job* joblist;
extern long long job_count;
extern int shell_terminal;

int STDIN_FD;
int STDOUT_FD;

void Reset_IO()
{
    Reset_I();
    Reset_O();
    close(STDIN_FD);
    close(STDOUT_FD);
}

void Reset_I()
{
    if (dup2(STDIN_FD, STDIN_FILENO) == -1)
    {
        Fatal_Error(errno);
    }
}

void Reset_O()
{
    if (dup2(STDOUT_FD, STDOUT_FILENO) == -1)
    {
        Fatal_Error(errno);
    }
}

void Pre_Exec(ArgList* arglist)
{
    ArgList* carglist = InitArgList();

    // Save default I/O Streams
    STDIN_FD = dup(STDIN_FILENO);
    if (STDIN_FD == -1)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return;
    }
    STDOUT_FD = dup(STDOUT_FILENO);
    if (STDOUT_FD == -1)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return;
    }

    for (long long i = 0; i < arglist->size; i++)
    {
        if (strcmp(arglist->args[i], "|") == 0)
        {
            Exec_Cmd(carglist, true);
            DeleteArgList(carglist);
            carglist = InitArgList();
            // Reset STDOUT changed by Exec_Cmd when piping
            Reset_O();
        }
        else if (strcmp(arglist->args[i], "<") == 0)
        {
            if (arglist->size == i + 1)
            {
                Print_Out(RED, "DotCom: Missing Filename after '<'\n");
                return;
            }
            int fd = open(arglist->args[++i], O_RDONLY);
            if (fd == -1)
            {
                char* err_str = strerror(errno);
                Print_Out(RED, err_str);
                fprintf(stdout, "\n");
                return;
            }
            dup2(fd, STDIN_FILENO);
        }
        else if (strcmp(arglist->args[i], ">") == 0)
        {
            if (arglist->size == i + 1)
            {
                Print_Out(RED, "DotCom: Missing Filename after '>'\n");
                return;
            }
            int fd = open(arglist->args[++i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1)
            {
                char* err_str = strerror(errno);
                Print_Out(RED, err_str);
                fprintf(stdout, "\n");
                return;
            }
            dup2(fd, STDOUT_FILENO);
        }
        else if (strcmp(arglist->args[i], ">>") == 0)
        {
            if (arglist->size == i + 1)
            {
                Print_Out(RED, "DotCom: Missing Filename after '>>'\n");
                return;
            }
            int fd = open(arglist->args[++i], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1)
            {
                char* err_str = strerror(errno);
                Print_Out(RED, err_str);
                fprintf(stdout, "\n");
                return;
            }
            dup2(fd, STDOUT_FILENO);
        }
        else
        {
            AddArg(carglist, arglist->args[i]);
        }
    }
    Exec_Cmd(carglist, false);
    Reset_IO();
}

void Exec_Cmd(ArgList* arglist, bool use_pipe)
{
    if (arglist->size == 0)
    {
        return;
    }

    int pipefd[2];
    if (use_pipe)
    {
        if (pipe(pipefd) == -1)
        {
            char* err_str = strerror(errno);
            Print_Out(RED, err_str);
            fprintf(stdout, "\n");
            return;
        }
        dup2(pipefd[1], STDOUT_FILENO);
    }

    if (strcmp(arglist->args[0], "clear") == 0)
    {
        fprintf(stdout, CLEAR);
    }
    else if (strcmp(arglist->args[0], "pwd") == 0)
    {
        Pwd(arglist);
    }
    else if (strcmp(arglist->args[0], "echo") == 0)
    {
        Echo(arglist);
    }
    else if (strcmp(arglist->args[0], "cd") == 0)
    {
        Cd(arglist);
    }
    else if (strcmp(arglist->args[0], "ls") == 0)
    {
        Ls(arglist);
    }
    else if (strcmp(arglist->args[0], "history") == 0)
    {
        History(arglist);
    }
    else if (strcmp(arglist->args[0], "discover") == 0)
    {
        Discover(arglist);
    }
    else if (strcmp(arglist->args[0], "pinfo") == 0)
    {
        Pinfo(arglist);
    }
    else if (strcmp(arglist->args[0], "jobs") == 0)
    {
        Jobs(arglist);
    }
    else if (strcmp(arglist->args[0], "sig") == 0)
    {
        Sig(arglist);
    }
    else if (strcmp(arglist->args[0], "fg") == 0)
    {
        Fg(arglist);
    }
    else if (strcmp(arglist->args[0], "bg") == 0)
    {
        Bg(arglist);
    }
    else
    {
        Exec_Ext(arglist);
    }
    
    if (use_pipe)
    {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
    }
}

void Exec_Ext(ArgList* arglist)
{
    bool is_bg = false;
    if (strcmp(arglist->args[arglist->size - 1], "&") == 0)
    {
        is_bg = true;
        arglist->size--;
    }

    if (arglist->size == 0)
    {
        return;
    }
    int success;
    pid_t pid = fork();
    if (pid == -1)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return;
    }
    else if (pid == 0)
    {
        // Child process detected

        Prepare_Proc(is_bg);
        for (long long i = 0; i < arglist->size; i++)
        {
            arglist->args[i] = Rel_to_Abs(arglist->args[i]);
        }

        AddArg(arglist, NULL);
        success = execvp(arglist->args[0], arglist->args);
        if (success == -1)
        {
            char* err_str = strerror(errno);
            Print_Out(RED, err_str);
            fprintf(stdout, "\n");
            /* return; */
            exit(1);
        }
    }
    else
    {
        // Parent process detected
        int status;
        char* tmp;
        if (!is_bg)
        {
            setpgid(pid, pid);
            signal(SIGTTOU, SIG_IGN);
            // Give child control of terminal
            tcsetpgrp(shell_terminal, pid);
            waitpid(pid, &status, WUNTRACED);
            /* waitpid(pid, &status, 0); */
            /* tcsetpgrp(shell_terminal, getpgrp()); */
            // Give shell back control of terminal
            tcsetpgrp(shell_terminal, getpid());
            if (WIFSTOPPED(status))
            {
                char buf[1024];
                if ((tmp = strrchr(arglist->args[0], '/')) == NULL)
                {
                    long long job_no = Add_Job(pid, arglist->args[0]);
                    sprintf(buf, "[%lld] %d suspended %s\n", job_no, pid, arglist->args[0]);
                    Print_Out(PURPLE, buf);
                }
                else
                {
                    long long job_no = Add_Job(pid, tmp + 1);
                    sprintf(buf, "[%lld] %d suspended %s\n", job_no, pid, tmp + 1);
                    Print_Out(PURPLE, buf);
                }
            }
            signal(SIGTTOU, SIG_DFL);
        }
        else
        {
            char buf[1024];
            if ((tmp = strrchr(arglist->args[0], '/')) == NULL)
            {
                long long job_no = Add_Job(pid, arglist->args[0]);
                sprintf(buf, "[%lld] %d suspended %s\n", job_no, pid, arglist->args[0]);
                Print_Out(PURPLE, buf);
            }
            else
            {
                long long job_no = Add_Job(pid, tmp + 1);
                sprintf(buf, "[%lld] %d suspended %s\n", job_no, pid, tmp + 1);
                Print_Out(PURPLE, buf);
            }
        }
    }
    fprintf(stdout, "\n");
    fflush(stdout);
}
