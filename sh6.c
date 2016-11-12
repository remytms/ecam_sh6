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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sh6lib.h"

int main(int argc, char *argv[])
{
    int i, err;
    char cmd_line[256];

    if (argc >= 2) {
        for(i = 1; i < argc; i++) {
            sh6_exec_bash(argv[i]);
        }
    } else {
        do {
            printf("sh6 > ");
            if (fgets(cmd_line, 256, stdin) != NULL) {
                if(system(cmd_line) < 0)
                    printf("Error when executing '%s'\n", cmd_line);
            }
        } while (!sh6_is_exit(cmd_line));
    }
    printf("Bye!\n");
}

