#ifndef PROC_H
#define PROC_H

#include "args.h"

void Init_Shell();
void Prepare_Proc(int is_bg);
void Fg(ArgList* arglist);
void Bg(ArgList* arglist);

#endif
