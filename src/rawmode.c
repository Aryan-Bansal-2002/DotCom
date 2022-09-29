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
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "args.h"
#include "autocmp.h"
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
#include "proc.h"
#include "prompt.h"
#include "rawmode.h"
#include "sig.h"

struct termios orig_termios;

void Enable_Raw()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    {
        Print_Out(RED, "DotCom: Raw Mode Unsuccessful\n");
        exit(1);
    }
    atexit(Disable_Raw);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        Print_Out(RED, "DotCom: Raw Mode Unsuccessful\n");
        exit(1);
    }
}

void Disable_Raw()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    {
        Print_Out(RED, "DotCom: Raw Mode Unsuccessful\n");
        exit(1);
    }
}

char* inp;
char* Raw_Inp()
{
    char c;
    inp = malloc(sizeof(char) * 1024);
    int pt = 0;
    setbuf(stdout, NULL);
    Enable_Raw();
    memset(inp, '\0', 1024);
    while (read(STDIN_FILENO, &c, 1) == 1)
    {
        if (iscntrl(c))
        {
            if (c == 10)
            {
                // Newline Character
                printf("%c", c);
                fflush(stdout);
                break;
            }
            else if (c == 27)
            {
                // Esc (Arrow Keys)
                char buf[3];
                buf[2] = 0;
                if (read(STDIN_FILENO, buf, 2) == 2)
                {
                    printf("\nArrow Key: %s", buf);
                    fflush(stdout);
                }
            }
            else if (c == 127)
            {
                // Backspace
                if (pt > 0)
                {
                    if (inp[pt - 1] == 9)
                    {
                        // Tab Present
                        for (int i = 0; i < 7; i++)
                        {
                            printf("\b");
                            fflush(stdout);
                        }
                    }
                    inp[--pt] = '\0';
                    printf("\b \b");
                    fflush(stdout);
                }
            }
            else if (c == 9)
            {
                // Tab
                char* old = strdup(inp);
                char* s = Autocomplete(inp);
                if (s != NULL)
                {
                    sprintf(inp, "%s%s", old, s);
                    pt += strlen(s);
                }
            }
            else if (c == 4)
            {
                // EOF
                printf("\n");
                fflush(stdout);
                exit(0);
            }
            else if (c == 12)
            {
                // Ctrl + L
                inp[0] = '\n';
                inp[1] = '\0';
                printf(CLEAR);
                fflush(stdout);
                break;
            }
            else
            {
                printf("%d\n", c);
                fflush(stdout);
            }
        }
        else
        {
            inp[pt++] = c;
            printf("%c", c);
            fflush(stdout);
        }
    }
    Disable_Raw();
    return inp;
}
