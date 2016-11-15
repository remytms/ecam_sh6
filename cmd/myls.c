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

#include <getopt.h>

#include "mylslib.h"

int main(int argc, char *argv[])
{
    int c, cur_arg_index, j, dir, nread, bpos;
    int all_flag = 0;
    int ignore_backup_flag = 0;
    char buf[BUF_SIZE];
    char **filenames;
    int filenames_len;
    char *cur_arg;
    struct linux_dirent *dir_entries;
    struct option options[] = 
    {
        {"all", no_argument, NULL, 'a'},
        {"ignore-backups", no_argument, NULL, 'B'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv,
                ":aB",
                options, NULL)) != -1) {
        switch (c) {
            case 0:
                printf("Case 0.\n");
                break;
            case 'a':
                all_flag = 1;
                break;
            case 'B':
                ignore_backup_flag = 1;
                break;
            case '?':
                printf("Case ?.\n");
            default:
                abort();
        }
    }

    cur_arg_index = optind;
    do {
        if (argc - optind == 0)
            cur_arg = ".";
        else
            cur_arg = argv[cur_arg_index];

        if (argc - optind > 1)
            printf("%s:\n", cur_arg);

        myls_list_file_in_dir(cur_arg, &filenames, &filenames_len);

        for(j = 0; j < filenames_len; j++) {
            printf("%s\n", filenames[j]);
            free(filenames[j]);
        }
        free(filenames);

        cur_arg_index++;
    } while (cur_arg_index < argc);

    return EXIT_SUCCESS;
}

