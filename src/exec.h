#ifndef EXEC_H
#define EXEC_H

#include <stdbool.h>
#include "args.h"

void Reset_IO();
void Reset_I();
void Reset_O();
void Pre_Exec(ArgList* arglist);
void Exec_Cmd(ArgList* arglist, bool use_pipe);
void Exec_Ext(ArgList* arglist);

#endif
