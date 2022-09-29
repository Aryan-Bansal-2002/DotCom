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

void Parse(char* line)
{
    char* saveptr;
    char* token = strtok_r(line, ";\n", &saveptr);
    while (token != NULL)
    {
        Parse_Cmd(token);
        token = strtok_r(NULL, ";\n", &saveptr);
    }
}

void Parse_Cmd(char* cmd)
{
    ArgList* arglist = InitArgList();

    char* saveptr;
    char* token = strtok_r(cmd, " \t", &saveptr);
    while (token != NULL)
    {
        char* walk = token;
        long long tok_len = strlen(token);
        for (long long i = 0; i < tok_len; i++)
        {
            if (token[i] == '&')
            {
                token[i] = '\0';
                if ((walk != NULL) && (strcmp(walk, "") != 0))
                {
                    AddArg(arglist, walk);
                }
                walk = &token[++i];
                AddArg(arglist, "&");
            }
            else if (token[i] == '<')
            {
                token[i] = '\0';
                if ((walk != NULL) && (strcmp(walk, "") != 0))
                {
                    AddArg(arglist, walk);
                }
                walk = &token[++i];
                AddArg(arglist, "<");
            }
            else if (token[i] == '|')
            {
                token[i] = '\0';
                if ((walk != NULL) && (strcmp(walk, "") != 0))
                {
                    AddArg(arglist, walk);
                }
                walk = &token[++i];
                AddArg(arglist, "|");
            }
            else if (token[i] == '>')
            {
                token[i] = '\0';
                if ((walk != NULL) && (strcmp(walk, "") != 0))
                {
                    AddArg(arglist, walk);
                }
                if (token[i + 1] == '>')
                {
                    token[i + 1] = '\0';
                    walk = &token[i + 2];
                    i += 2;
                    AddArg(arglist, ">>");
                }
                else
                {
                    walk = &token[++i];
                    AddArg(arglist, ">");
                }
            }
        }
        if ((walk != NULL) && (strcmp(walk, "") != 0))
        {
            AddArg(arglist, walk);
        }
        token = strtok_r(NULL, " \t", &saveptr);
    }
    Pre_Exec(arglist);
}
