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
#include <time.h>
#include <unistd.h>

#include "args.h"
#include "echo.h"
#include "exec.h"
#include "ls.h"
#include "misc.h"
#include "path.h"
#include "parse.h"
#include "prompt.h"

int cmp(const void* a, const void* b)
{
    // Don't consider '.' in the start while sorting
    char* A = *(char**) a;
    char* B = *(char**) b;
    int flag;
    if (A[0] == '.')
    {
        A += 1;
    }
    if (B[0] == '.')
    {
        B += 1;
    }
    flag = strcasecmp(A, B);
    if (flag >= 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void Ls(ArgList* arglist)
{
    bool l, a;
    ArgList* dir_file = Ls_Pre(arglist, &l, &a);

    // If ls has only 1 argument
    if (dir_file->size == 1)
    {
        Ls_Post(dir_file->args[0], l, a);
        return;
    }

    /*
     * If there are multuple file/directories
     * we need to print the name of them first
     * before giving their ls output
    */
    char buf[1024];
    for (long long i = 0; i < dir_file->size; i++)
    {
        // Checking if dir_file->args[i] is a file instead of directory
        struct stat sb;
        /*
         * The stat() function returns 0 on success
         * and returns -1 on error
        */
        if (stat(dir_file->args[i], &sb) == -1)
        {
            char* err_str = strerror(errno);
            Print_Out(RED, err_str);
            fprintf(stdout, "\n");
            return;
        }
        if (S_ISREG(sb.st_mode))
        {
            // file confirmed
            Print_Out(WHITE, dir_file->args[i]);
            fprintf(stdout, "\n\n");
            continue;
        }

        // directory confirmed
        sprintf(buf, "%s :\n", dir_file->args[i]);
        Print_Out(WHITE, buf);
        Ls_Post(dir_file->args[i], l, a);
        // Extra newline for clarity
        /* fprintf(stdout, "\n"); */
    }
}

ArgList* Ls_Pre(ArgList* arglist, bool* l, bool* a)
{
    ArgList* dir_file = InitArgList();
    *l = false;
    *a = false;
    long long bitmask = 0;
    long long bit_l = 1;
    long long bit_a = 2;
    for (long long a = 1; a < arglist->size; a++)
    {
        long long arg_len = strlen(arglist->args[a]);
        if ((arglist->args[a][0] == '-') && (arg_len > 1))
        {
            for (long long i = 1; i < arg_len; i++)
            {
                if (arglist->args[a][i] == 'l')
                {
                    bitmask = bitmask | bit_l;
                }
                else if (arglist->args[a][i] == 'a')
                {
                    bitmask = bitmask | bit_a;
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
            /*
             * Convert all possible input to ls into absolute
             * except if input is of the form:
             * ls folder
             * since we don't need to use absolute path here
            */
            AddArg(dir_file, Rel_to_Abs(arglist->args[a]));
        }
    }
    if ((bitmask & bit_l) != 0)
    {
        *l = true;
    }
    if ((bitmask & bit_a) != 0)
    {
        *a = true;
    }

    /*
     * If there is no file or directory argument
     * then argument will be current directory
     * opendir accepts argument "."
    */
    if (dir_file->size == 0)
    {
        AddArg(dir_file, ".");
    }

    return dir_file;
}

void Ls_Post(char* path, bool l, bool a)
{
    DIR* dir = opendir(path);
    if (dir == NULL)
    {
        // Checking if dir is a file instead of directory
        struct stat stat_buf;
        /*
         * The stat() function returns 0 on success
         * and returns -1 on error
        */
        if (lstat(path, &stat_buf) == -1)
        {
            char* err_str = strerror(errno);
            Print_Out(RED, err_str);
            fprintf(stdout, "\n");
            return;
        }
        if (S_ISREG(stat_buf.st_mode))
        {
            // file confirmed
            if (l == 1)
            {
                Print_Shit(path);
            }
            // Print only name of file
            char* file_name = strrchr(path, '/');
            // if '/' is not found, NULL is returned
            if (file_name == NULL)
            {
                file_name = path;
            }
            char buf[1024];
            sprintf(buf, "%s\n", file_name);
            Print_Out(WHITE, buf);
            return;
        }
    }
    else
    {
        // dir successfully opened as directory
        struct dirent* child = readdir(dir);
        /* readdir() returns NULL when it reaches end or error is thrown
         * to distinguish between error and end of stream
         * we make errno = 0 initially, and if errno changes
         * this will mean there was an error
        */
        errno = 0;
        long long child_no = 0;
        while (child != NULL)
        {
            child_no++;
            child = readdir(dir);
        }
        if (errno != 0)
        {
            char* err_str = strerror(errno);
            Print_Out(RED, err_str);
            fprintf(stdout, "\n");
            return;
        }
        // Store all children in an array
        char* child_arr[child_no];
        rewinddir(dir);
        for (long long i = 0; i < child_no; i++)
        {
            child = readdir(dir);
            child_arr[i] = (char*) malloc(sizeof(char) * strlen(child->d_name) + 1);
            strcpy(child_arr[i], child->d_name);
        }
        // sort the array now
        qsort(child_arr, child_no, sizeof(char*), cmp);
        // print the "total" parameter
        if (l)
        {
            long long count = 0;
            for (long long i = 0; i < child_no; i++)
            {
                if ((!a) && (child_arr[i][0] == '.'))
                {
                    continue;
                }
                char child_path[1024];
                sprintf(child_path, "%s/%s", path, child_arr[i]);
                struct stat sb;
                if (lstat(child_path, &sb) == -1)
                {
                    char* err_str = strerror(errno);
                    Print_Out(RED, err_str);
                    fprintf(stdout, "\n");
                    continue;
                }
                count += sb.st_blocks;
            }
            char buf[1024];
            sprintf(buf, "Total = %lld\n", count / 2);
            Print_Out(BLUE, buf);
        }
        // print rest of the information
        for (long long i = 0; i < child_no; i++)
        {
            if ((!a) && (child_arr[i][0] == '.'))
            {
                continue;
            }
            if (l)
            {
                /*
                 * Form the absolute path to each children
                 * given the path to parent directory
                */
                char child_path[1024];
                sprintf(child_path, "%s/%s", path, child_arr[i]);
                Print_Shit(child_path);
            }
            // Check if child_arr[i] is file or directory
            // to add custom color
            char child_path[1024];
            sprintf(child_path, "%s/%s", path, child_arr[i]);
            struct stat temp_buf;
            if (lstat(child_path, &temp_buf) == -1)
            {
                char* err_str = strerror(errno);
                Print_Out(RED, err_str);
                fprintf(stdout, "\n");
                return;
            }
            if (S_ISREG(temp_buf.st_mode))
            {
                if ((S_IXUSR & temp_buf.st_mode) || (S_IXGRP & temp_buf.st_mode) || (S_IXOTH & temp_buf.st_mode))
                {
                    Print_Out(GREEN, child_arr[i]);
                    fprintf(stdout, "\n");
                }
                else
                {
                    Print_Out(WHITE, child_arr[i]);
                    fprintf(stdout, "\n");
                }
            }
            else if (S_ISDIR(temp_buf.st_mode))
            {
                Print_Out(BLUE, child_arr[i]);
                fprintf(stdout, "\n");
            }
            else
            {
                Print_Out(WHITE, child_arr[i]);
                fprintf(stdout, "\n");
            }
        }
    }
    // For neatness
    /* fprintf(stdout, "\n"); */
    closedir(dir);
}

void Print_Shit(char* path)
{
    struct stat sb;
    // Using lstat() for symbolic link support
    if (lstat(path, &sb) == -1)
    {
        char* err_str = strerror(errno);
        Print_Out(RED, err_str);
        fprintf(stdout, "\n");
        return;
    }
    char* rwx[8] = {"---", "--x", "-w-", "-wx","r--", "r-x", "rw-", "rwx"};
    char perms[11];
    perms[10] = '\0';
    perms[0] = File_Type((sb.st_mode));
    strcpy(&perms[1], rwx[(sb.st_mode >> 6) & 7]);
    strcpy(&perms[4], rwx[(sb.st_mode >> 3) & 7]);
    strcpy(&perms[7], rwx[(sb.st_mode & 7)]);
    // Special File Mode Handling
    if (sb.st_mode & S_ISUID)
    {
        perms[3] = (sb.st_mode & S_IXUSR) ? 's' : 'S';
    }
    if (sb.st_mode & S_ISGID)
    {
        perms[3] = (sb.st_mode & S_IXGRP) ? 's' : 'l';
    }
    if (sb.st_mode & S_ISVTX)
    {
        perms[3] = (sb.st_mode & S_IXOTH) ? 't' : 'T';
    }
    Print_Out(YELLOW, perms);
    fprintf(stdout, " ");
    char sym_link[1024];
    sprintf(sym_link, "%3.ld ", sb.st_nlink);
    Print_Out(DEFAULT, sym_link);
    char usrid[1024], grpid[1024];
    sprintf(usrid, "%10s ", getpwuid(sb.st_uid)->pw_name);
    sprintf(grpid, "%10s ", getgrgid(sb.st_gid)->gr_name);
    Print_Out(PURPLE, usrid);
    Print_Out(GREEN, grpid);
    char size[1024];
    sprintf(size, "%10.ld ", sb.st_size);
    Print_Out(RED, size);
    char date[21];
    if (time(0) - sb.st_mtime < 15780000)
        strftime(date, 20, "%b %d %H:%M", localtime(&(sb.st_mtime)));
    else
        strftime(date, 20, "%b %d  %Y", localtime(&(sb.st_mtime)));
    Print_Out(CYAN, date);
    fprintf(stdout, " ");
    return;
}

char File_Type(mode_t mode)
{
    char c;
    if (S_ISREG(mode))
    {
        c = '-';
    }
    else if (S_ISDIR(mode))
    {
        c = 'd';
    }
    else if (S_ISBLK(mode))
    {
        c = 'b';
    }
    else if (S_ISCHR(mode))
    {
        c = 'c';
    }
    else if (S_ISFIFO(mode))
    {
        c = 'p';
    }
    else if (S_ISLNK(mode))
    {
        c = 'l';
    }
    else if (S_ISSOCK(mode))
    {
        c = 's';
    }
    else
    {
        c = '?';
    }
    return c;
}
