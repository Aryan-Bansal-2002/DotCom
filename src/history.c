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
#include "echo.h"
#include "exec.h"
#include "history.h"
#include "ls.h"
#include "misc.h"
#include "path.h"
#include "parse.h"
#include "prompt.h"

#define HIS_MAX 20
#define HIS_PRN 10
#define HIS_PATH "/tmp/DotCom_History"

void History(ArgList* arglist)
{
    char* his_arr[HIS_MAX];
    for (long long i = 0; i < HIS_MAX; i++)
    {
        his_arr[i] = (char*) calloc(1024, sizeof(char));
    }
    long long his_count = Read_History(his_arr);
    if (his_count >= HIS_PRN)
    {
        his_count = HIS_PRN;
    }
    for (long long i = his_count - 1; i >= 0; i--)
    {
        Print_Out(GREEN, his_arr[i]);
        fprintf(stdout, "\n");
    }
}

void Write_History(char* line)
{
    char* his_arr[HIS_MAX];
    for (long long i = 0; i < HIS_MAX; i++)
    {
        his_arr[i] = (char*) calloc(1024, sizeof(char));
    }
    long long his_count = Read_History(his_arr);
    // If new line is same as previous one
    if (strcmp(line, his_arr[0]) == 0)
    {
        return;
    }
    for (long long i = his_count; i >= 1; i--)
    {
        strcpy(his_arr[i], his_arr[i - 1]);
    }
    strcpy(his_arr[0], line);

    FILE* hd = fopen(HIS_PATH, "w");
    if (hd == NULL)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return;
    }
    his_count++;
    for (long long i = 0; i < his_count; i++)
    {
        fprintf(hd, "%s\n", his_arr[i]);
    }
    fclose(hd);
}

long long Read_History(char** his_arr)
{
    /*
     * Check if .history exists
     * if not then create it
    */
    FILE* hd = fopen(HIS_PATH, "a+");
    if (hd == NULL)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return -1;
    }
    char* line = (char*) malloc(sizeof(char) * 1024);
    size_t len = 0;
    ssize_t nread;
    errno = 0;
    long long his_count = 0;
    while (fgets(line, 1024, hd) != NULL)
    {
        his_count++;
        if (his_count > HIS_MAX)
        {
            his_count = HIS_MAX;
            break;
        }
        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }
        strcpy(his_arr[his_count - 1], line);
    }
    if (errno != 0)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return -1;
    }
    free(line);
    fclose(hd);
    return his_count;
}
