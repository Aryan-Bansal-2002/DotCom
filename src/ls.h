#ifndef LS_H
#define LS_H

#include <stdbool.h>
#include <sys/stat.h>
#include "args.h"

void Ls(ArgList* arglist);
ArgList* Ls_Pre(ArgList* arglist, bool* l, bool* a);
void Ls_Post(char* path, bool l, bool a);
void Print_Shit(char* path);
char File_Type(mode_t mode);

#endif
