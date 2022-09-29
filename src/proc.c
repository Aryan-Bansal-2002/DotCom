#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
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

int shell_terminal;
int shell_is_interactive;
pid_t shell_pgid;
struct termios shell_tmodes;

void Init_Shell()
{
    shell_terminal = STDIN_FILENO;
    // isatty() tests whether the argument is an open file descriptor to a terminal
    // it returns 1 if arg is terminal else it returns 0
    // errno is set to indicate error
    shell_is_interactive = isatty(shell_terminal);
    // Check if shell is running interactively
    if (shell_is_interactive)
    {
        // Makes sure shell is running in foreground
        shell_pgid = getpgrp();
        while (tcgetpgrp(shell_terminal) != shell_pgid)
        {
            // If this shell is not foreground
            // stop this shell by sending SIGTTIN
            // to every process in process group of this shell

            // SIGTTIN is a signal sent to background processes
            // which tells processes that they won't be getting input
            // because they are not in the foreground
            
            // This causes this shell to suspend in the parent shell session
            kill(-shell_pgid, SIGTTIN);
        }

        // Ignore interactive and job-control signals

        /* Ctrl + C */
        signal(SIGINT, SIG_IGN);
        /* Ctrl + \ */
        signal(SIGQUIT, SIG_IGN);
        /* Ctrl + Z */
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);
        
        // getpid() returns the PID of this shell
        // we use this to generate unique filenames
        shell_pgid = getpid();
        // Putting this shell in its own process group
        if (setpgid(shell_pgid, shell_pgid) == -1)
        {
            char buf[1024];
            char* err_str = strerror(errno);
            sprintf(buf, "Couldn't put the DotCom in its own process group: %s\n", err_str);
            Print_Out(RED, buf);
            exit(1);
        }
        // Making this shell grab control of terminal
        tcsetpgrp(shell_terminal, shell_pgid);
        // Save default terminal attributes for shell
        tcgetattr(shell_terminal, &shell_tmodes);
    }
}

void Prepare_Proc(int is_bg)
{
    // Put process into process group
    // give the group the terminal
    pid_t pid = getpid();
    setpgid(pid, pid);
    if (!is_bg)
    {
        tcsetpgrp(shell_terminal, pid);
    }
    // Set handling for job control signals
    // back to default
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);

/* TODO: Set standard I/O channels of process */
}

void Fg(ArgList* arglist)
{
    if (arglist->size != 2)
    {
        Print_Out(RED, "DotCom: Invalid Arguments\n");
        return;
    }
    pid_t pid;
    int pid_int;
    char* save;
    pid_int = strtol(arglist->args[1], &save, 10);
    if (save[0] != '\0')
    {
        Print_Out(RED, "DotCom: Invalid Arguments\n");
        return;
    }
    pid = Get_Job(pid_int);
    if (pid == -1)
    {
        Print_Out(RED, "DotCom: Process not found\n");
        return;
    }
    int status;
    if (kill(pid, SIGCONT) == -1)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return;
    }
    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(shell_terminal, pid);
    waitpid(pid, &status, WUNTRACED);
    if (!WIFSTOPPED(status))
    {
        Remove_Job(pid);
    }
    tcsetpgrp(shell_terminal, getpgrp());
    signal(SIGTTOU, SIG_DFL);
}

void Bg(ArgList* arglist)
{
    if (arglist->size != 2)
    {
        Print_Out(RED, "DotCom: Invalid Arguments\n");
        return;
    }
    pid_t pid;
    int pid_int;
    char* save;
    pid_int = strtol(arglist->args[1], &save, 10);
    if (save[0] != '\0')
    {
        Print_Out(RED, "DotCom: Invalid Arguments\n");
        return;
    }
    pid = Get_Job(pid_int);
    if (pid == -1)
    {
        Print_Out(RED, "DotCom: Process not found\n");
        return;
    }
    int status;
    if (kill(pid, SIGCONT) == -1)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return;
    }
}
