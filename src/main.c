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
#include "autocmp.h"
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
#include "rawmode.h"
#include "sig.h"

char* home_dir = NULL;
long long home_dir_len = 0;

int main ()
{
    Init_Shell();
    fprintf(stdout, CLEAR);
    char* cwd = getcwd(NULL, 0);
    if (home_dir == NULL)
    {
        home_dir_len = strlen(cwd);
        home_dir = (char*) malloc(home_dir_len * sizeof(char) + 1);
        strcpy(home_dir, cwd);
        free(cwd);
    }

    // Ignore Ctrl+C & Ctrl+Z
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    Init_Job_List();
    Init_Child_Handler();

    time_t t;
    int time_taken = -1;

    char* inp;
    while (1)
    {
        Print_Prompt(0, home_dir, time_taken);
        fflush(stdout);
        inp = Raw_Inp();
        if (inp == NULL)
        {
            fprintf(stdout, "\n");
            fflush(stdout);
            exit(0);
        }

        // Check if inp contains anything other than
        // whitespaces and tabs
        // this is to check if we want to push it in history or not
        char* inp_dup = strdup(inp);
        char* saveptr;
        char* token = strtok_r(inp_dup, " \t", &saveptr);
        if (token != NULL)
        {
            Write_History(inp);
        }

        if ((strcmp("Quit", inp) == 0) || (strcmp("Exit", inp) == 0))
        {
            exit(0);
        }
        t = time(NULL);
        Parse(inp);
        time_t temp = time(NULL);
        t = temp - t;
        time_taken = (int) t;
    }

    free(home_dir);
    free(inp);
    
    return 0;
}
