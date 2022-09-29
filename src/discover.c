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
#include "prompt.h"

char* discover_home = NULL;
long long disc_home_len = 0;
char* first_parent = NULL;

void Discover(ArgList* arglist)
{
    char* cwd = getcwd(NULL, 0);
    disc_home_len = strlen(cwd);
    discover_home = (char*) malloc(sizeof(char) * disc_home_len + 1);
    strcpy(discover_home, cwd);
    free(cwd);
    
    char* parent = (char*) calloc(1024, sizeof(char));
    char* query = (char*) calloc(1024, sizeof(char));
    bool d, f;
    if (Pre_Discover(arglist, &d, &f, &parent, &query) == -1)
    {
        chdir(discover_home);
        return;
    }
    first_parent = (char*) malloc(sizeof(char) * 1024);
    strcpy(first_parent, parent);
    if (chdir(Rel_to_Abs(parent)) == -1)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        chdir(discover_home);
        return;
    }
    if (query == NULL)
    {
        Print_Out(BLUE, first_parent);
        fprintf(stdout, "\n");
    }
    if (Post_Discover(".", query, d, f) == -1)
    {
        chdir(discover_home);
        return;
    }
    chdir(discover_home);
}

int Pre_Discover(ArgList* arglist, bool* d, bool* f, char** parent, char** query)
{
    *d = false;
    *f = false;
    long long bitmask = 0;
    long long bit_d = 1;
    long long bit_f = 2;
    long long temp = 0;

    bool done_flag = false;
    bool done_parent = false;
    bool done_query = false;

    for (long long a = 1; a < arglist->size; a++)
    {
        long long arg_len = strlen(arglist->args[a]);
        if ((arglist->args[a][0] == '-') && (arg_len > 1))
        {
            done_flag = true;
            temp++;
            for (long long i = 1; i < arg_len; i++)
            {
                if (arglist->args[a][i] == 'd')
                {
                    bitmask = bitmask | bit_d;
                }
                else if (arglist->args[a][i] == 'f')
                {
                    bitmask = bitmask | bit_f;
                }
                else
                {
                    char errbuf[1024];
                    sprintf(errbuf, "DotCom: Invalid Option => %c\n", arglist->args[a][i]);
                    Print_Out(RED, errbuf);
                }
            }
        }
    }
    if ((bitmask & bit_d) != 0)
    {
        *d = true;
    }
    if ((bitmask & bit_f) != 0)
    {
        *f = true;
    }
    if ((*d == false) && (*f == false))
    {
        *d = true;
        *f = true;
    }

    for (long long a = 1; a < arglist->size; a++)
    {
        if ((arglist->args[a][0] != '-') && !((arglist->args[a][0] == '\"') && (arglist->args[a][strlen(arglist->args[a]) - 1] == '\"')))
        {
            if (done_parent)
            {
                Print_Out(RED, "DotCom: Too Many Arguments\n");
                return -1;
            }
            done_parent = true;
            strcpy(*parent, arglist->args[a]);
        }
    }
    if (!done_parent)
    {
        strcpy(*parent, ".");
    }

    for (long long a = 1; a < arglist->size; a++)
    {
        if ((arglist->args[a][0] == '\"') && (arglist->args[a][strlen(arglist->args[a]) - 1] == '\"'))
        {
            if (done_query)
            {
                Print_Out(RED, "DotCOm: Too Many Arguments\n");
                return -1;
            }
            done_query = true;
            // Modify string to remove "
            arglist->args[a]++;
            arglist->args[a][strlen(arglist->args[a]) - 1] = '\0';
            strcpy(*query, arglist->args[a]);
        }
    }
    if (!done_query)
    {
        *query = NULL;
    }

    return 0;
}

int Post_Discover(char* parent, char* query, bool d, bool f)
{
    /* DIR* dir = opendir(parent); */
    DIR* dir = opendir(".");
    if (dir == NULL)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return -1;
    }
    struct dirent* child = readdir(dir);
    errno = 0;
    while (child != NULL)
    {
        char* ch_name = child->d_name;
        if ((strcmp(ch_name, "..") == 0) || (strcmp(ch_name, ".") == 0))
        {
            child = readdir(dir);
            continue;
        }
        // Check if directory or file
        struct stat sb;
        if (lstat(ch_name, &sb) == -1)
        {
            char* err_str = strerror(errno);
            Print_Out(RED, err_str);
            fprintf(stdout, "\n");
            errno = 0;
            return -1;
        }
        if ((S_ISDIR(sb.st_mode)) && (strcmp(ch_name, ".") != 0))
        {
            if (d)
            {
                if ((query == NULL) || (strcmp(query, ch_name) == 0))
                {
                    {
                        Print_Discover(ch_name, BLUE);
                    }
                }
            }
            
            if (chdir(ch_name) == -1)
            {
                char* err_str = strerror(errno);
                Print_Out(RED, err_str);
                fprintf(stdout, "\n");
                return -1;
            }
            Post_Discover(ch_name, query, d, f);
        }
        else if (f)
        {
            if ((query == NULL) || (strcmp(query, ch_name) == 0))
            {
                {
                    Print_Discover(ch_name, WHITE);
                }
            }
        }
        child = readdir(dir);
    }
    if (errno != 0)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return -1;
    }
    closedir(dir);
    chdir("..");

    return 0;
}

void Print_Discover(char* name, char* color)
{
    char* cwd = getcwd(NULL, 0);
    char* buf = (char*) malloc(1024);
    sprintf(buf, "%s%s/%s\n", first_parent, cwd + disc_home_len, name);
    Print_Out(color, buf);
    free(cwd);
    free(buf);
}
