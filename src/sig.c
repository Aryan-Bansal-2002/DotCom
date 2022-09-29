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
#include "sig.h"

void Sig(ArgList* arglist)
{
    if (arglist->size != 3)
    {
        Print_Out(RED, "DotCom: Invalid Arguments\n");
        return;
    }

    pid_t pid;
    int pid_int;
    int sig;
    char* save;
    pid_int = strtol(arglist->args[1], &save, 10);
    if (save[0] != '\0')
    {
        Print_Out(RED, "DotCom: Invalid Arguments\n");
        return;
    }
    sig = strtol(arglist->args[2], &save, 10);
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
    if (kill(pid, sig) == -1)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return;
    }
}
