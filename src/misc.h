#ifndef MISC_H
#define MISC_H

#define BLACK "\033[0;30m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define PURPLE "\033[0;35m"
#define CYAN "\033[0;36m"
#define WHITE "\033[0;37m"
#define DEFAULT "\033[0m"

#define CLEAR "\e[1;1H\e[2J"

#define UP(x) fprintf(stdout, "\033[%dA", (int) (x))
#define DOWN(x) fprintf(stdout, "\033[%dB", (int) (x))
#define RIGHT(x) fprintf(stdout, "\033[%dC", (int) (x))
#define LEFT(x) fprintf(stdout, "\033[%dD", (int) (x))

char* Low_Str(char* s);
void Print_Out(char* color, char* str);
void Fatal_Error(int err);
char* Abs_to_Rel(char* abs);
char* Rel_to_Abs(char* rel);
long long Min(long long a, long long b);
long long Max(long long a, long long b);

#endif
