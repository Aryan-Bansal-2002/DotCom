#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "args.h"
#include "discover.h"
#include "echo.h"
#include "exec.h"
#include "history.h"
#include "ls.h"
#include "misc.h"
#include "path.h"
#include "parse.h"
#include "pinfo.h"
#include "prompt.h"

void Pinfo(ArgList* arglist)
{
    pid_t pid;
    if (arglist->size == 1)
    {
        pid = getpid();
    }
    else if (arglist->size == 2)
    {
        pid = atoi(arglist->args[1]);
    }
    else
    {
        Print_Out(RED, "DotCom: Too Many Arguments\n");
        return;
    }

    char procpath[1024];
    sprintf(procpath, "/proc/%d/stat", pid);
    FILE* procfile = fopen(procpath, "r");
    if (procfile == NULL)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return;
    }
    char pid_buf[1024];
    sprintf(pid_buf, "pid : %d\n", pid);
    Print_Out(GREEN, pid_buf);

    char comm[1024];
    char state;
    int ppid, pgrp, session, tty_nr, tpgid;
    fscanf(procfile, "%d %s %c %d %d %d %d %d", &pid, comm, &state, &ppid, &pgrp, &session, &tty_nr, &tpgid);
    char flag;
    if (pgrp == tpgid)
    {
        flag = '+';
    }
    else
    {
        flag = ' ';
    }
    char state_buf[1024];
    sprintf(state_buf, "Process Status : %c%c\n", state, flag);
    Print_Out(GREEN, state_buf);
    fclose(procfile);

    char statmpath[1024];
    sprintf(statmpath, "/proc/%d/statm", pid);
    FILE* statmfile = fopen(statmpath, "r");
    if (statmfile == NULL)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return;
    }
    long long size;
    fscanf(statmfile, "%lld", &size);
    char statm_buf[1024];
    sprintf(statm_buf, "Memory : %lld\n", size);
    Print_Out(GREEN, statm_buf);
    fclose(statmfile);

    char exepath[1024];
    sprintf(exepath, "/proc/%d/exe", pid);
    FILE* exefile = fopen(exepath, "r");
    if (exefile == NULL)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return;
    }
    char exec[1024];
    long long read = readlink(exepath, exec, 1024);
    exec[read] = '\0';
    if (read == -1)
    {
        Print_Out(RED, "Executable Path : NULL\n");
        return;
    }
    char* final = Abs_to_Rel(exec);
    char exebuf[1024];
    sprintf(exebuf, "Executable Path : %s\n", final);
    Print_Out(GREEN, exebuf);
    return;
}
