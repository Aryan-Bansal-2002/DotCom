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

void Echo(ArgList* arglist)
{
    if (arglist->size == 1)
    {
        fprintf(stdout, "\n");
        return;
    }
    char temp[1024];
    for (long long i = 1; i < arglist->size; i++)
    {
        sprintf(temp, "%s ", arglist->args[i]);
        Print_Out(GREEN, temp);
    }
    fprintf(stdout, "\n");
}
