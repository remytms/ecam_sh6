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

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int number_flag = 0;
    int show_ends_flag = 0;

    int c, i, nread, arg_ind;
    int file;
    int line_nbr;
    int do_print_line_nbr = 0;
    char *pgr_name;
    char *filename;
    int buf_len = 4;
    char buf[buf_len];
    char buf_c;
    struct stat file_stat;
    struct option options[] = 
    {
        {"show-ends", no_argument, NULL, 'E'},
        {"number", no_argument, NULL, 'n'},
        {0, 0, 0, 0}
    };

    pgr_name = argv[0];

    while ((c = getopt_long(argc, argv,
                "En",
                options, NULL)) != -1) {
        switch (c) {
            case 'E':
                show_ends_flag = 1;
                break;
            case 'n':
                number_flag = 1;
                break;
            default:
                abort();
        }
    }

    // If no args are specified it will open stdin
    if (argc == optind) {
        file = STDIN_FILENO;
        argc++;
    }

    line_nbr = 0;
    do_print_line_nbr = number_flag;
    for (arg_ind = optind; arg_ind < argc; arg_ind++) {
        // We don't need to open the args if file equals to stdin
        if (file != STDIN_FILENO) {
            filename = argv[arg_ind];

            if (access(filename, R_OK)) {
                fprintf(stderr, "%s: %s: %s\n",
                        pgr_name, filename, strerror(errno));
                return EXIT_FAILURE;
            }

            if (stat(filename, &file_stat)) {
                fprintf(stderr, "%s: %s: %s\n",
                        pgr_name, filename, strerror(errno));
                return EXIT_FAILURE;
            }

            if (S_ISDIR(file_stat.st_mode)) {
                errno = EISDIR;
                fprintf(stderr, "%s: %s: %s\n",
                        pgr_name, filename, strerror(errno));
                return EXIT_FAILURE;
            }

            file = open(filename, 0);
            if (file == -1) {
                fprintf(stderr, "%s: %s: %s\n",
                        pgr_name, filename, strerror(errno));
                return EXIT_FAILURE;
            }
        }

        while ((nread = read(file, buf, buf_len))) {
            if (nread == -1) {
                fprintf(stderr, "%s: %s: %s\n",
                        pgr_name, filename, strerror(errno));
                return EXIT_FAILURE;
            }

            for (i = 0; i < nread; i++) {
                // Show line number
                if (number_flag && do_print_line_nbr) {
                    fprintf(stdout, "%6d  ", ++line_nbr);
                    fflush(stdout);
                }

                // Set to 1 if we need to print the line number on the next
                // iteration
                do_print_line_nbr = buf[i] == '\n';

                // Show ends of line
                if (show_ends_flag && buf[i] == '\n') {
                    buf_c = '$';
                    if (write(STDOUT_FILENO, &buf_c, sizeof(char)) == -1) {
                        fprintf(stderr, "%s: %s: %s\n",
                                pgr_name, filename, strerror(errno));
                        return EXIT_FAILURE;
                    }
                }

                // Print regular current char
                if (write(STDOUT_FILENO, &buf[i], sizeof(char)) == -1) {
                    fprintf(stderr, "%s: %s: %s\n",
                            pgr_name, filename, strerror(errno));
                    return EXIT_FAILURE;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

