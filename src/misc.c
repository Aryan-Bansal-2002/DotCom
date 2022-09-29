#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "misc.h"
#include "path.h"
#include "prompt.h"

extern char* home_dir;
extern long long home_dir_len;

char* Low_Str(char* s)
{
    long long slen = strlen(s);
    for (long long i = 0; i < slen; i++)
    {
        s[i] = tolower(s[i]);
    }
    return s;
}

void Print_Out(char* color, char* str)
{
    fprintf(stdout, "%s", color);
    fprintf(stdout, "%s", str);
    fprintf(stdout, DEFAULT);
    fflush(stdout);
}

void Fatal_Error(int err)
{
    char* s = strerror(err);
    Print_Out(RED, s);
    fprintf(stdout, "\n");
    exit(1);
}

char* Abs_to_Rel(char* abs)
{
    if (memcmp(abs, home_dir, home_dir_len) != 0)
    {
        return abs;
    }
    char* buf = (char*) malloc(sizeof(char) * 1024);
    sprintf(buf, "~%s", abs + home_dir_len);

    return buf;
}

char* Rel_to_Abs(char* rel)
{
    if (rel[0] != '~')
    {
        return rel;
    }
    char* buf = (char*) malloc(sizeof(char) * 1024);
    sprintf(buf, "%s%s", home_dir, rel + 1);

    return buf;
}

long long Min(long long a, long long b)
{
    return (a >= b ? b : a);
}
long long Max(long long a, long long b)
{
    return (a >= b ? a : b);
}
