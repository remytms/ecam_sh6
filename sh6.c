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

#include <stdio.h>

#include "sh6lib.h"

int main(int argc, char *argv[])
{
    int i;
    char *path_to_pgr;
    char *pgr_name;
    char *cmd_line = NULL;
    size_t cmd_line_len = 0;

    pgr_name = argv[0];

    if ((path_to_pgr = sh6_path_to_custom_programs(argv[0])) == NULL) {
        perror(pgr_name);
        return EXIT_FAILURE;
    }

    if (sh6_modify_path(path_to_pgr)) {
        perror(pgr_name);
        return EXIT_FAILURE;
    }
    free(path_to_pgr);

    if (argc >= 2) {
        for(i = 1; i < argc; i++)
            sh6_exec_bash(argv[i]);
    } else {
        while (1) {
            // If hit Ctrl-D then exit
            if (feof(stdin)) {
                printf("\n");
                break;
            }
            // Print the prompt and ask user an input
            printf("sh6 > ");
            if (getline(&cmd_line, &cmd_line_len, stdin) > -1) {
                if (sh6_is_exit(cmd_line))
                    break;
                if(mysh6_system(cmd_line) < 0)
                    printf("Error when executing '%s'\n", cmd_line);
            }
        }
    }
    free(cmd_line);
    printf("Bye!\n");
    return EXIT_SUCCESS;
}

