#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "args.h"
#include "echo.h"
#include "exec.h"
#include "misc.h"
#include "path.h"
#include "parse.h"
#include "prompt.h"

extern char* home_dir;
extern long long home_dir_len;

void Pwd(ArgList* arglist)
{
    if (arglist->size > 1)
    {
        Print_Out(RED, "DotCom: Too Many Arguments\n");
    }
    char* pwd = getcwd(NULL, 0);
    Print_Out(GREEN, pwd);
    fprintf(stdout, "\n");
    free(pwd);
}

char* prev_dir = NULL;

void Cd(ArgList* arglist)
{
    char* curr_dir = getcwd(NULL, 0);
    if (arglist->size > 2)
    {
        Print_Out(RED, "DotCom: Too Many Arguments\n");
        return;
    }
    else if (arglist->size == 1)
    {
        if (chdir(home_dir) == -1)
        {
            char* err_str = strerror(errno);
            Print_Out(RED, err_str);
            fprintf(stdout, "\n");
            return;
        }
    }
    else
    {
        char* abs_next_path = Rel_to_Abs(arglist->args[1]);
        if (strcmp(arglist->args[1], "-") == 0)
        {
            if (prev_dir == NULL)
            {
                prev_dir = home_dir;
            }
            abs_next_path = prev_dir;
            /* Print_Out(YELLOW, Abs_to_Rel(abs_next_path)); */
            Print_Out(YELLOW, abs_next_path);
            fprintf(stdout, "\n");
        }
        if (chdir(abs_next_path) == -1)
        {
            char* err_str = strerror(errno);
            Print_Out(RED, err_str);
            fprintf(stdout, "\n");
            return;
        }
    }
    prev_dir = curr_dir;
}
