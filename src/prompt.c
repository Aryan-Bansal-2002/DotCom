#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include "misc.h"
#include "path.h"
#include "prompt.h"

char user[1024];
char sys_name[1024];

void Print_Prompt(int type, char* home_dir, int time_taken)
{
    char prompt_buffer[1024];
    getlogin_r(user, 1024);
    gethostname(sys_name, 1024);
    
    char cwd[1024];
    long long home_dir_len = strlen(home_dir);
    char* pwd = getcwd(NULL, 0);
    long long pwd_len = strlen(pwd);
    if ((pwd_len < home_dir_len) || (memcmp(pwd, home_dir, home_dir_len * sizeof(char)) != 0))
    {
        strcpy(cwd, pwd);
    }
    else
    {
        sprintf(cwd, "~%s", pwd + home_dir_len);
    }

    if (type == 1)
    {
        strcpy(prompt_buffer, "> ");
    }
    else if (type == 0)
    {
        if (time_taken >= 1)
        {
            sprintf(prompt_buffer, "<%s@%s: %s "YELLOW"%d Seconds"DEFAULT" > ", user, sys_name, cwd, time_taken);
        }
        else
        {
            sprintf(prompt_buffer, "<%s@%s: %s > ", user, sys_name, cwd);
        }
    }
    Print_Out(CYAN, prompt_buffer);
}

char* Read_Cmd()
{
    char* buf1 = (char*) malloc(1024);
    char* buf2 = NULL;
    while (fgets(buf1, 1024, stdin) != NULL)
    {

        long long buflen = strlen(buf1);
        if (buflen >= 1023)
        {
            Print_Out(RED, "Length of Input Exceeds Limit!\n");
        }
        else if ((buflen >= 2) && (buf1[buflen - 2] == '\\'))
        {
            buf1[buflen - 2] = '\0';
            buflen -= 2;
            if (buf2 == NULL)
            {
                buf2 = (char*) malloc(buflen + 1);
                strcpy(buf2, buf1);
            }
            else
            {
                char* temp = (char*) realloc(buf2, strlen(buf2) + buflen + 1);
                strcat(temp, buf1);
                strcpy(buf2, temp);
            }
            Print_Prompt(1, "", -1);
            fflush(stdout);
        }
        else if (buf1[buflen - 1] == '\n')
        {
            if (buf2 == NULL)
            {
                buf2 = (char*) malloc(strlen(buf1) + 1);
                strcpy(buf2, buf1);
            }
            else
            {
                char* temp = (char*) realloc(buf2, strlen(buf2) + strlen(buf1) + 1);
                strcat(temp, buf1);
                strcpy(buf2, temp);
            }
            buf2[strlen(buf2) - 1] = '\0';
            return buf2;
        }
    }
    if (buf2 != NULL)
    {
        buf2[strlen(buf2) - 1] = '\0';
    }
    return buf2;
}
