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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "sh6lib.h"

int main(int argc, char *argv[])
{
    int i;
    char cmd_line[256];

    if (argc >= 2) {
        printf("This will read the file(s) : ");
        for(i = 1; i < argc; i++) {
            printf(argv[i]);
            printf(" ");
        }
        printf("\n");
    } else {
        do {
            printf("sh6 > ");
            scanf("%[^\n]%*c", cmd_line);
            printf("Entry : %s\n", cmd_line);
        } while (strcmp(cmd_line, "exit"));
        printf("Exiting\nBye!\n");
    }
}

