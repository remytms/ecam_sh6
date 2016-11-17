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
    int all_flag = 0;
    int ignore_backup_flag = 0;
    int details_flag = 0;
    int recursive_flag = 0;

    int c, cur_arg_index, j, dir, nread, bpos;
    char buf[BUF_SIZE];
    char **filenames;
    int filenames_len;
    char *cur_arg;
    char *permission;
    char *username;
    char *groupname;
    char *mtime;
    struct linux_dirent *dir_entries;
    struct option options[] = 
    {
        {"all", no_argument, NULL, 'a'},
        {"ignore-backups", no_argument, NULL, 'B'},
        {"recursive", no_argument, NULL, 'R'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv,
                "aBlR",
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
            case 'l':
                details_flag = 1;
                break;
            case 'R':
                recursive_flag = 1;
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

        if (myls_is_reg_file(cur_arg)) {
            filenames = calloc(1, sizeof(char*));
            filenames[0] = calloc(strlen(cur_arg)+1, sizeof(char));
            strcpy(filenames[0], cur_arg);
            filenames_len = 1;
        } else if (myls_list_file_in_dir(cur_arg, &filenames, &filenames_len)) {
            return EXIT_FAILURE;
        }

        if ((argc - optind) > 1 && myls_is_dir(cur_arg))
            printf("%s:\n", cur_arg);

        qsort(filenames, filenames_len, sizeof(char*), myls_str_alphanum_cmp);

        for(j = 0; j < filenames_len; j++) {
            // Filter hidden file if needed
            if (!(!all_flag && filenames[j][0] == '.')) {
                // Show details if needed
                if (details_flag) {
                    // Permission 
                    permission = calloc(11, sizeof(char));
                    if (myls_get_permission(filenames[j], permission, 11))
                        strcpy(permission, "err");
                    printf("%s\t", permission);
                    free(permission);

                    // Hard link
                    printf("%d\t", myls_get_nlink(filenames[j]));

                    // Username
                    username = calloc(100, sizeof(char));
                    if (myls_get_username(filenames[j], username, 100))
                        strcpy(username, "error");
                    printf("%s\t", username);
                    free(username);

                    // Groupname 
                    groupname = calloc(100, sizeof(char));
                    if (myls_get_username(filenames[j], groupname, 100))
                        strcpy(groupname, "error");
                    printf("%s\t", groupname);
                    free(groupname);

                    // size
                    printf("%d\t", myls_get_size(filenames[j]));

                    // Modification date
                    mtime = calloc(20, sizeof(char));
                    myls_get_mtime(filenames[j], mtime, 20);
                    printf("%s\t", mtime);
                    free(mtime);
                }
                printf("%s\n", filenames[j]);
            }
            free(filenames[j]);
        }
        free(filenames);

        cur_arg_index++;
    } while (cur_arg_index < argc);

    return EXIT_SUCCESS;
}

