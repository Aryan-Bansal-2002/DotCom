#ifndef ARGS_H
#define ARGS_H

typedef struct ArgList
{
    long long size;
    long long max;
    char** args;
} ArgList;

ArgList* InitArgList();
void AddArg(ArgList* arglist, char* arg_str);
void DeleteArgList(ArgList* arglist);

#endif
