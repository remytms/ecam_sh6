/*
 * Copyright 2016 Rémy Taymans <14291@student.ecam.be>
 *                Louis Hugues Randriamora <12413@student.ecam.be>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SH6LIB_H
#define SH6LIB_H

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Return true if the string given in args means 'exit'
 */
int sh6_is_exit(char str[])
{
    return (strcmp(str, "exit") == 0 || strcmp(str, "exit\n") == 0 ||
            strcmp(str, "quit") == 0 || strcmp(str, "quit\n") == 0);
}

/*
 * Execute a shell command.
 *
 * Args:
 *     char *cmd: a command
 * 
 * Return value:
 *    If a child process could not be created, or its status could not
 *    be retrieved, the return value is -1.
 *    If  all  system  calls succeed, then the return value is the
 *    termination status of the child shell used to execute command.
 *    (The termination status of a shell is the termination status of
 *    the last command it executes.)
 *
 * See also:
 *     http://man7.org/tlpi/code/online/dist/procexec/system.c.html
 */
int mysh6_system(const char *cmd)
{
    int pid, status;

    pid = fork();

    switch (pid) {
        case -1:
            return EXIT_FAILURE;
            break;
        case 0:
            // We are the child
            if (execlp("/bin/sh", "sh", "-c", cmd, NULL)) {
                exit(127);
            }
        default:
            // We are the parent
            while(waitpid(pid, &status, 0) == -1) {
                if (errno != EINTR) {
                    status = -1;
                }
            }
            break;
    }

    return status;
}

/* 
 * Execute bash file. It read the file (filename) and execute each
 * line of this file. It print an error if a line of the file cannot be
 * executed.
 *
 * Args:
 *     char *filename: the name of the bash file that will be read
 *
 * Return value:
 *     EXIT_SUCCESS if no errors occured
 *     EXIT_FAILURE if an error occured
 */
int sh6_exec_bash(char *filename)
{
    FILE *file;
    char *line = NULL;
    size_t len = 0;

    file = fopen(filename, "r");
    if (file == NULL)
        return 1;

    while (getline(&line, &len, file) > -1) {
        if (strlen(line) > 1) {
            printf("sh6 > %s", line);
            if (mysh6_system(line) < 0) {
                printf("Error when executing '%s'\n", line);
                return EXIT_FAILURE;
            }
        }
    }

    fclose(file);
    if (line)
        free(line);
    return EXIT_SUCCESS;
}

/*
 * Return the absolute path to directory containing the custom programs.
 *
 * Args:
 *     char *pgr_name: the value of argv[0]
 *
 * Return value:
 *     A string containing the required path. If an error occured NULL
 *     is return.
 *
 * See also:
 *     man 3 strrchr
 *     man 3 getcwd
 *     http://stackoverflow.com/questions/9449241/where-is-path-max-defined-in-linux
 */
char* sh6_path_to_custom_programs(char *pgr_name)
{
    char cwd[PATH_MAX+1];
    char *dir_to_exec;
    char *last_slash;
    char *complete_path;

    getcwd(cwd, PATH_MAX+1);
    dir_to_exec = strdup(pgr_name);
    last_slash = strrchr(dir_to_exec, '/');
    *last_slash = '\0';

    // The path looks like: %s/%s/cmd
    if ((complete_path = calloc(
                    strlen(cwd)+strlen(dir_to_exec)+6, 
                    sizeof(char))) == NULL)
        return NULL;
    complete_path[0] = '\0';

    strcat(complete_path, cwd);
    strcat(complete_path, "/");
    strcat(complete_path, dir_to_exec);
    strcat(complete_path, "/cmd");

    free(dir_to_exec);

    return complete_path;
}

/*
 * Add path to the PATH environemen= list.
 *
 * Args:
 *     char *path: a path where executable can be find
 *
 * Return value:
 *     EXIT_FAILURE if an error occured
 *     EXIT_SUCCESS if no errors occured
 *
 * See also:
 *     man 3 getenv
 *     man 3 setenv
 */
int sh6_modify_path(char *path)
{
    const char *path_old;
    char *path_new;

    path_old = getenv("PATH");

    if ((path_new = calloc(
                    strlen(path_old)+strlen(path)+2, 
                    sizeof(char))) == NULL)
        return EXIT_FAILURE;

    strcat(path_new, path);
    strcat(path_new, ":");
    strcat(path_new, path_old);

    setenv("PATH", path_new, 1);

    free(path_new);

    return EXIT_SUCCESS;
}

/*
 * Read a line.
 */
/*
char* getline(void)
{
    size_t len_max = 100;
    size_t len;
    char *line = malloc(len * sizeof(char));
    int c;

    for (len = 0; len < len_max; len++) {
        c = fgetc(stdin);
    }
}
*/

#endif
