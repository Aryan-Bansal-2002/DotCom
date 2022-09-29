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
#include "prompt.h"

job* joblist;
long long job_count = 0;
extern char* home_dir;

void Init_Job_List()
{
    joblist = (job*) malloc(sizeof(job));
    joblist->no = 0;
    joblist->name = NULL;
    joblist->pid = 0;
    joblist->next = NULL;
}

long long Add_Job(pid_t pid, char* name)
{
    job* new_job = (job*) malloc(sizeof(job));
    new_job->no = ++job_count;
    new_job->pid = pid;
    new_job->name = strdup(name);
    new_job->next = NULL;
    job* temp = joblist;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = new_job;
    return new_job->no;
}

void Remove_Job(pid_t pid)
{
    if (joblist->next == NULL)
    {
        return;
    }
    job* temp = joblist->next;
    job* prev = joblist;
    job* del = NULL;
    while (temp != NULL)
    {
        if (temp->pid == pid)
        {
            del = temp;
            prev->next = temp->next;
        }
        prev = temp;
        temp = temp->next;
    }
    if (del != NULL)
    {
        free(del);
    }
}

char* Get_Job_Name(pid_t pid)
{
    job* temp = joblist;
    while (temp != NULL)
    {
        if (temp->pid == pid)
        {
            return temp->name;
        }
        temp = temp->next;
    }
    return NULL;
}

pid_t Get_Job(int index)
{
    job* walk = joblist->next;
    for (int i = 0; i < job_count; i++)
    {
        if (walk == NULL)
        {
            return -1;
        }
        else if (index == walk->no)
        {
            return walk->pid;
        }
        else
        {
            walk = walk->next;
        }
    }
    return -1;
}

void Init_Child_Handler()
{
    struct sigaction sa;
    /* sa.sa_handler = Child_Handler; */
    sa.sa_sigaction = Child_Handler;
    sigemptyset(&sa.sa_mask);
    /* sa.sa_flags = SA_RESTART; */
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGCHLD, &sa, NULL);
}

void Child_Handler(int sig, siginfo_t* sig_info, void* puch_hi_mat)
{
    pid_t pid = sig_info->si_pid;
    char* name = Get_Job_Name(pid);
    if (name == NULL)
    {
        return;
    }
    int status;
    if (waitpid(pid, &status, WNOHANG | WCONTINUED | WUNTRACED) == -1)
    {
        return;
    }

    int sig_code = sig_info->si_code;
    char state[1024];
    switch (sig_code)
    {
        case CLD_EXITED:
            strcpy(state, "Exited Normally");
            Remove_Job(pid);
            break;
        case CLD_KILLED:
            strcpy(state, "Killed Legally");
            Remove_Job(pid);
            break;
        case CLD_DUMPED:
            strcpy(state, "Terminated Abnormally");
            break;
        case CLD_TRAPPED:
            strcpy(state, "Child Trapped");
            break;
        case CLD_STOPPED:
            strcpy(state, "Stopped Successfully");
            break;
        case CLD_CONTINUED:
            strcpy(state, "Child Execution Continued");
            break;
        default:
            strcpy(state, "Exited Abnormally");
            Remove_Job(pid);
            break;
    }
    char buf[1024];
    sprintf(buf, "\nProcess %s with pid = %d %s\n", name, pid, state);
    Print_Out(PURPLE, buf);
    Print_Prompt(0, home_dir, -1);
    fflush(stdout);
}

int Job_Sortcmp(const void* a, const void* b)
{
    jobsort* job1 = (jobsort*) a;
    jobsort* job2 = (jobsort*) b;
    return (strcmp(job1->name, job2->name) == 0 ? (job1->no > job2->no) : strcmp(job1->name, job2->name));
}

void Jobs(ArgList* arglist)
{
    bool s, r;
    long long bitmask = 0;
    long long bit_s = 1;
    long long bit_r = 2;
    for (long long a = 1; a < arglist->size; a++)
    {
        long long arg_len = strlen(arglist->args[a]);
        if ((arglist->args[a][0] == '-') && (arg_len > 1))
        {
            for (long long i = 1; i < arg_len; i++)
            {
                if (arglist->args[a][i] == 's')
                {
                    bitmask = bitmask | bit_s;
                }
                else if (arglist->args[a][i] == 'r')
                {
                    bitmask = bitmask | bit_r;
                }
                else
                {
                    char errbuf[1024];
                    sprintf(errbuf, "DotCom: Invalid Option => %c\n", arglist->args[a][i]);
                    Print_Out(RED, errbuf);
                }
            }
        }
        else
        {
            Print_Out(RED, "DotCom: Invalid Arguments to function jobs\n");
        }
    }
    if ((bitmask & bit_s) != 0)
    {
        s = true;
    }
    if ((bitmask & bit_r) != 0)
    {
        r = true;
    }
    if (!s && !r)
    {
        s = true;
        r = true;
    }

    job* walk = joblist->next;
    int temp_count = 0;
    while (walk != NULL)
    {
        temp_count++;
        walk = walk->next;
    }

    jobsort sortarr[temp_count];
    walk = joblist->next;
    for (int i = 0; i < temp_count; i++)
    {
        sortarr[i].no = walk->no;
        sortarr[i].pid = walk->pid;
        sortarr[i].name = walk->name;
        walk = walk->next;
    }
    qsort(sortarr, temp_count, sizeof(jobsort), Job_Sortcmp);

    for (int i = 0; i < temp_count; i++)
    {
        char procpath[1024];
        sprintf(procpath, "/proc/%d/stat", sortarr[i].pid);
        FILE* procfile = fopen(procpath, "r");
        if (procfile == NULL)
        {
            Print_Out(RED, "DotCom: Did not find process\n");
            return;
        }
        int pid;
        char comm[1024];
        char state;
        char status[1024];
        fscanf(procfile, "%d %s %c", &pid, comm, &state);
        if (state == 'R')
        {
            strcpy(status, "Running");
        }
        else if (state == 'S')
        {
            strcpy(status, "Sleeping");
        }
        else if (state == 'Z')
        {
            strcpy(status, "Zombie");
        }
        else if (state == 'T')
        {
            strcpy(status, "Stopped");
        }
        else
        {
            strcpy(status, "Unknown");
        }
        if (s && (state == 'T'))
        {
            char buf[1024];
            sprintf(buf, "[%lld] %s %s [%d]\n", sortarr[i].no, status, sortarr[i].name, sortarr[i].pid);
            Print_Out(GREEN, buf);
        }
        if (r && ((state == 'R') || state == 'S'))
        {
            char buf[1024];
            sprintf(buf, "[%lld] %s %s [%d]\n", sortarr[i].no, "Running", sortarr[i].name, sortarr[i].pid);
            Print_Out(GREEN, buf);
        }
        fclose(procfile);
    }
}
