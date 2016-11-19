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

char *pgr_name;

#include "mylslib.h"

int main(int argc, char *argv[])
{
    int all_flag = 0;
    int ignore_backup_flag = 0;
    int details_flag = 0;
    int recursive_flag = 0;

    int c, cur_arg_index, j, dir, res;
    char buf[BUF_SIZE];
    char **filenames;
    int filenames_len;
    char *cur_dirname;
    char *permission;
    char *username;
    char *groupname;
    char *mtime;
    struct linux_dirent *dir_entries;
    struct stat cur_dir_stat;
    struct stat *file_stat;
    struct option options[] = 
    {
        {"all", no_argument, NULL, 'a'},
        {"ignore-backups", no_argument, NULL, 'B'},
        {"recursive", no_argument, NULL, 'R'},
        {0, 0, 0, 0}
    };

    pgr_name = argv[0];

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
            cur_dirname = ".";
        else
            cur_dirname = argv[cur_arg_index];

        if ((dir = open(cur_dirname, O_RDONLY)) < 0) {
            fprintf(stderr, 
                    "%s: cannot open %s: %s\n", 
                    pgr_name,
                    cur_dirname,
                    strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (lstat(cur_dirname, &cur_dir_stat)) {
            fprintf(stderr, 
                    "%s: cannot open %s: %s\n", 
                    pgr_name,
                    cur_dirname,
                    strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (!S_ISDIR(cur_dir_stat.st_mode)) {
            filenames_len = 1;
            filenames = calloc(filenames_len, sizeof(char*));
            filenames[0] = strdup(cur_dirname);
            close(dir);
            dir = -1;
        } else {
            if (myls_list_file_in_dir(dir, &filenames, &filenames_len)) {
                fprintf(stderr, 
                        "%s: cannot open %s: %s\n", 
                        pgr_name,
                        cur_dirname,
                        strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

        if (cur_arg_index != optind)
            printf("\n");
        if ((argc - optind) > 1 && S_ISDIR(cur_dir_stat.st_mode))
            printf("%s:\n", cur_dirname);

        qsort(filenames, filenames_len, sizeof(char*), myls_str_alphanum_cmp);

        for(j = 0; j < filenames_len; j++) {
            // Filter hidden file if needed
            if (!(!all_flag && filenames[j][0] == '.')) {
                // Show details if needed
                if (details_flag) {
                    // Get file stat
                    file_stat = malloc(sizeof(struct stat));
                    if (myls_get_file_stat(dir, filenames[j], &file_stat)) {
                        fprintf(stderr, 
                                "%s: cannot access to details of %s: %s\n", 
                                pgr_name,
                                filenames[j],
                                strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    // Permission 
                    if (myls_get_permission(file_stat, &permission)) {
                        fprintf(stderr, 
                                "%s: cannot access permissions of %s\n", 
                                pgr_name,
                                filenames[j]);
                        exit(EXIT_FAILURE);
                    }
                    printf("%s ", permission);
                    free(permission);

                    // Hard link
                    printf("%d\t", myls_get_nlink(file_stat));

                    // Username
                    if (myls_get_username(file_stat, &username)) {
                        fprintf(stderr, 
                                "%s: cannot access user owner of %s\n", 
                                pgr_name,
                                filenames[j]);
                        exit(EXIT_FAILURE);
                    }
                    printf("%s\t", username);
                    free(username);

                    // Groupname 
                    if (myls_get_groupname(file_stat, &groupname)) {
                        fprintf(stderr, 
                                "%s: cannot access group owner of %s\n", 
                                pgr_name,
                                filenames[j]);
                        exit(EXIT_FAILURE);
                    }
                    printf("%s\t", groupname);
                    free(groupname);

                    // size
                    printf("%d\t", myls_get_size(file_stat));

                    // Modification date
                    if ((res = myls_get_mtime(file_stat, &mtime)) < 0) {
                        fprintf(stderr, 
                                "%s: cannot access modification time of %s\n", 
                                pgr_name,
                                filenames[j]);
                        exit(EXIT_FAILURE);
                    }
                    printf("%s\t", mtime);
                    free(mtime);

                    free(file_stat);
                }
                printf("%s\n", filenames[j]);
            }
            free(filenames[j]);
        }
        free(filenames);
        if (dir > -1)
            close(dir);

        cur_arg_index++;
    } while (cur_arg_index < argc);

    return EXIT_SUCCESS;
}

