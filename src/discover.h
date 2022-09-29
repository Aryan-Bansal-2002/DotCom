#ifndef DISC_H
#define DISC_H

#include <stdbool.h>
#include "args.h"

void Discover(ArgList* arglist);
int Pre_Discover(ArgList* arglist, bool *d, bool *f, char** parent, char** query);
int Post_Discover(char* parent, char* query, bool d, bool f);
void Print_Discover(char* name, char* color);

#endif
