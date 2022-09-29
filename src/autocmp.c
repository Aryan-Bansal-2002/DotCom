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

extern char* home_dir;

char* Autocomplete(char* line)
{
    char word[1024];
    Get_Last_Token(strdup(line), word);
    char* abs_word = Rel_to_Abs(word);
    char* left_qry;
    char* qry = strrchr(abs_word, '/');
    if (qry == NULL)
    {
        qry = strdup(abs_word);
        left_qry = NULL;
    }
    else
    {
        qry = strdup(qry);
        qry++;
        left_qry = strdup(abs_word);
        char* temp = strrchr(left_qry, '/');
        temp[1] = '\0';
    }
    long long qry_len = strlen(qry);

    char dir[1024];
    if (left_qry == NULL)
    {
        // Directory to open should be .
        strcpy(dir, ".");
    }
    else
    {
        // Directory to open should be left_qry
        strcpy(dir, left_qry);
    }
    DIR* fd = opendir(dir);
    if (fd == NULL)
    {
        // No such path exists
        return NULL;
    }
    struct dirent* child = readdir(fd);
    errno = 0;
    long long child_no = 0;
    while (child != NULL)
    {
        child_no++;
        child = readdir(fd);
    }
    if (errno != 0)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return NULL;
    }
    char* child_arr[child_no];
    rewinddir(fd);
    long long j = 0;
    long long smallest_len = 1024;
    char child_path[1024];
    char child_entry[1024];
    for (long long i = 0; i < child_no; i++)
    {
        child = readdir(fd);

        sprintf(child_path, "%s/%s", dir, child->d_name);

        struct stat sb;
        if (lstat(child_path, &sb) == -1)
        {
            char* err_str = strerror(errno);
            Print_Out(RED, err_str);
            fprintf(stdout, "\n");
            return NULL;
        }
        if (S_ISDIR(sb.st_mode))
        {
            // Child is a directory
            sprintf(child_entry, "%s/", child->d_name);
        }
        else
        {
            sprintf(child_entry, "%s ", child->d_name);
        }

        if (strncmp(child->d_name, qry, qry_len) == 0)
        {
            child_arr[j] = malloc(sizeof(char) * strlen(child_entry) + 1);
            strcpy(child_arr[j], child_entry);
            smallest_len = Min(smallest_len, strlen(child_arr[j]));
            j++;
        }
    }
    child_no = j;
    if (child_no <= 1)
    {
        if (child_no == 0)
        {
            return NULL;
        }
        else
        {
            // Autocomplete the 1 word found
            if (qry_len != 0)
            {
                LEFT(qry_len);
            }
            Print_Out(DEFAULT, child_arr[0]);
            return Shit_Func(qry, child_arr[0]);
        }
    }
    char cmplt_to[1024];
    memset(cmplt_to, '\0', 1024);
    strcpy(cmplt_to, qry);
    long long cmplt_to_len = strlen(cmplt_to);
    for (long long char_index = cmplt_to_len; char_index < smallest_len; char_index++)
    {
        int flag = 1;
        char c = child_arr[0][cmplt_to_len];
        for (long long child_index = 0; child_index < child_no; child_index++)
        {
            if (child_arr[child_index][cmplt_to_len] != c)
            {
                flag = 0;
                break;
            }
        }
        if (flag == 1)
        {
            cmplt_to[cmplt_to_len++] = c;
        }
    }

    Print_Out(DEFAULT, "\n");

    for (long long i = 0; i < child_no; i++)
    {
        Print_Out(YELLOW, child_arr[i]);
        Print_Out(DEFAULT, "\n");
    }
    Print_Out(DEFAULT, "\n");
    Print_Prompt(0, home_dir, 0);
    Print_Out(DEFAULT, line);

    if (qry_len != 0)
    {
        LEFT(qry_len);
    }
    Print_Out(DEFAULT, cmplt_to);
    return Shit_Func(qry, cmplt_to);
}

void Get_Last_Token(char* line, char* word)
{
    if (line[strlen(line) - 1] == ' ')
    {
        word[0] = '.';
        word[1] = '/';
        word[2] = '\0';
    }
    if (line == NULL)
    {
        word = NULL;
        return;
    }
    ArgList* arglist = InitArgList();
    char* saveptr;
    char* token = strtok_r(line, " \t\n;&", &saveptr);
    while (token != NULL)
    {
        AddArg(arglist, token);
        token = strtok_r(NULL, " \t\n;&", &saveptr);
    }
    if (arglist->size != 0)
    {
        strcpy(word, arglist->args[arglist->size - 1]);
    }
    else
    {
        word[0] = '\0';
    }
}

char* Shit_Func(char* old, char* cmplted)
{
    char ret_buf[1024];
    long long old_len = strlen(old);
    long long cmplted_len = strlen(cmplted);
    memset(ret_buf, '\0', 1024);
    for (long long i = old_len; i < cmplted_len; i++)
    {
        ret_buf[i - old_len] = cmplted[i];
    }

    return strdup(ret_buf);
}
