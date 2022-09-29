#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "args.h"
#include "echo.h"
#include "misc.h"
#include "path.h"
#include "parse.h"
#include "prompt.h"

ArgList* InitArgList()
{
    ArgList* arglist = (ArgList*) malloc(sizeof(ArgList));
    arglist->size = 0;
    arglist->max = 2;
    arglist->args = (char**) malloc(sizeof(char*) * arglist->max);
    if (arglist->args == NULL)
    {
        Fatal_Error(errno);
    }
    arglist->args[0] = NULL;

    return arglist;
}

void AddArg(ArgList* arglist, char* arg_str)
{
    if (arglist->size == arglist->max)
    {
        arglist->max *= 2;
        arglist->args = (char**) realloc(arglist->args, sizeof(char*) * arglist->max);
        if (arglist->args == NULL)
        {
            Fatal_Error(errno);
        }
    }
    if (arg_str == NULL)
    {
        arglist->args[(arglist->size)++] = NULL;
        return;
    }
    else
    {
        char* temp = (char*) malloc(sizeof(char) * (strlen(arg_str) + 1));
        if (temp == NULL)
        {
            Fatal_Error(errno);
        }
        strcpy(temp, arg_str);
        arglist->args[arglist->size] = temp;
        arglist->size++;
    }
}

void DeleteArgList(ArgList* arglist)
{
    for (long long i = 0; i < arglist->size; i++)
    {
        free(arglist->args[i]);
    }
    free(arglist->args);
    free(arglist);
}
