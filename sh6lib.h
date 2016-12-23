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

/*
 * Return true if the string given in args means 'exit'
 */
int sh6_is_exit(char str[]);

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
int mysh6_system(const char *cmd);

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
int sh6_exec_bash(char *filename);

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
char* sh6_path_to_custom_programs(char *pgr_name);

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
int sh6_modify_path(char *path);

#endif
