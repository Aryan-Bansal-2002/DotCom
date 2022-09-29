#ifndef JOBS_H
#define JOBS_H

#include <unistd.h>
#include <stdbool.h>
#include "args.h"

typedef struct job
{
    long long no;
    pid_t pid;
    char* name;
    struct job* next;
} job;

typedef struct jobsort
{
    long long no;
    pid_t pid;
    char* name;
} jobsort;

void Init_Job_List();
long long Add_Job(pid_t pid, char* name);
void Remove_Job(pid_t pid);
char* Get_Job_Name(pid_t pid);
pid_t Get_Job(int index);
void Init_Child_Handler();
void Child_Handler();
void Jobs(ArgList* arglist);
int Job_Sortcmp(const void* a, const void* b);

#endif
