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

/*
 * Solve: Value too large for defined data type
 * See:
 * http://stackoverflow.com/questions/13893580/calling-stat-from-sys-stat-h-faills-with-value-too-large-for-defined-data-type
 */
//#define _FILE_OFFSET_BITS 64

#include "mylslib.h"

int main(int argc, char *argv[])
{
    int all_flag = 0;
    int ignore_backup_flag = 0;
    int details_flag = 0;
    int recursive_flag = 0;
    int do_not_sort = 0;

    int c, cur_arg_index, j, dir, res, d_ind, insert_pos, nread;
    int is_hidden, is_backup;
    char **filenames;
    int filenames_len;
    char **dirnames;
    int dirnames_len;
    char *pgr_name;
    char *full_dirname;
    char *permission;
    char *username;
    char *groupname;
    char *mtime;
    char *symlink;
    int symlink_len = 4096;
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
                "aBflRU",
                options, NULL)) != -1) {
        switch (c) {
            case 'a':
                all_flag = 1;
                break;
            case 'B':
                ignore_backup_flag = 1;
                break;
            case 'f':
                all_flag = 1;
                do_not_sort = 1;
                break;
            case 'l':
                details_flag = 1;
                break;
            case 'R':
                recursive_flag = 1;
                break;
            case 'U':
                do_not_sort = 1;
                break;
            case '?':
            default:
                abort();
        }
    }

    if (argc - optind == 0) {
        dirnames_len = 1;
        if ((dirnames = calloc(dirnames_len, sizeof(char*))) == NULL) {
            perror(pgr_name);
            return EXIT_FAILURE;
        }
        dirnames[0] = strdup(".");
    } else {
        dirnames_len = argc - optind;
        if ((dirnames = calloc(dirnames_len, sizeof(char*))) == NULL) {
            perror(pgr_name);
            return EXIT_FAILURE;
        }
        for (cur_arg_index = optind; cur_arg_index < argc; cur_arg_index++) {
            dirnames[cur_arg_index-optind] = strdup(argv[cur_arg_index]);
        }
    }

    for(d_ind = 0; d_ind < dirnames_len; d_ind++) {
        if ((dir = open(dirnames[d_ind], O_RDONLY)) < 0) {
            fprintf(stderr, 
                    "%s: cannot open %s: %s\n", 
                    pgr_name,
                    dirnames[d_ind],
                    strerror(errno));
            return EXIT_FAILURE;
        }

        if (details_flag)
            // No dereference link
            res = lstat(dirnames[d_ind], &cur_dir_stat);
        else
            // Dereference link
            res = stat(dirnames[d_ind], &cur_dir_stat);
        if (res) {
            fprintf(stderr, 
                    "%s: cannot open %s: %s\n", 
                    pgr_name,
                    dirnames[d_ind],
                    strerror(errno));
            return EXIT_FAILURE;
        }

        if (!S_ISDIR(cur_dir_stat.st_mode)) {
            filenames_len = 1;
            filenames = calloc(filenames_len, sizeof(char*));
            filenames[0] = strdup(dirnames[d_ind]);
            close(dir);
            dir = AT_FDCWD;
        } else {
            if (myls_list_file_in_dir(dir, &filenames, &filenames_len)) {
                fprintf(stderr, 
                        "%s: cannot open %s: %s\n", 
                        pgr_name,
                        dirnames[d_ind],
                        strerror(errno));
                return EXIT_FAILURE;
            }
        }

        if (d_ind > 0)
            printf("\n");
        if ((dirnames_len > 1 && S_ISDIR(cur_dir_stat.st_mode)) ||
                recursive_flag)
            printf("%s:\n", dirnames[d_ind]);

        if (!do_not_sort)
            qsort(filenames, filenames_len, sizeof(char*), myls_str_alphanum_cmp);

        insert_pos = d_ind + 1;
        for(j = 0; j < filenames_len; j++) {
            is_hidden = filenames[j][0] == '.';
            is_backup = filenames[j][strlen(filenames[j])-1] == '~';
            // Filter hidden and backup file if needed
            if ((all_flag && !ignore_backup_flag) || 
                    (all_flag && !is_backup) ||
                    (!is_hidden && !ignore_backup_flag) ||
                    (!is_hidden && !is_backup)) {
                // Get file stat
                if ((file_stat = malloc(sizeof(struct stat))) == NULL) {
                    perror(pgr_name);
                    return EXIT_FAILURE;
                }
                if (myls_get_file_stat(dir, filenames[j], &file_stat)) {
                    fprintf(stderr, 
                            "%s: cannot access to details of %s: %s\n", 
                            pgr_name,
                            filenames[j],
                            strerror(errno));
                    return EXIT_FAILURE;
                }

                if (recursive_flag && S_ISDIR(file_stat->st_mode) &&
                        strcmp(filenames[j], ".") && 
                        strcmp(filenames[j], "..")) {
                    full_dirname = myls_path_concat(dirnames[d_ind], filenames[j]);
                    if (full_dirname == NULL) {
                        perror(pgr_name);
                        return EXIT_FAILURE;
                    }
                    if (myls_array_insert(
                            insert_pos++,
                            full_dirname,
                            &dirnames,
                            &dirnames_len)) {
                        perror(pgr_name);
                        return EXIT_FAILURE;
                    }
                    free(full_dirname);
                }

                // Show details if needed
                if (details_flag) {
                    // Permission 
                    if (myls_get_permission(file_stat, &permission)) {
                        fprintf(stderr, 
                                "%s: cannot access permissions of %s: %s\n", 
                                pgr_name,
                                filenames[j],
                                strerror(errno));
                        return EXIT_FAILURE;
                    }
                    printf("%s ", permission);
                    free(permission);

                    // Hard link
                    printf("%d\t", myls_get_nlink(file_stat));

                    // Username
                    if (myls_get_username(file_stat, &username)) {
                        fprintf(stderr, 
                                "%s: cannot access user owner of %s: %s\n", 
                                pgr_name,
                                filenames[j],
                                strerror(errno));
                        return EXIT_FAILURE;
                    }
                    printf("%s\t", username);
                    free(username);

                    // Groupname 
                    if (myls_get_groupname(file_stat, &groupname)) {
                        fprintf(stderr, 
                                "%s: cannot access group owner of %s: %s\n", 
                                pgr_name,
                                filenames[j],
                                strerror(errno));
                        return EXIT_FAILURE;
                    }
                    printf("%s\t", groupname);
                    free(groupname);

                    // size
                    printf("%d\t", myls_get_size(file_stat));

                    // Modification date
                    if ((res = myls_get_mtime(file_stat, &mtime)) < 0) {
                        fprintf(stderr, 
                                "%s: cannot access modification time of %s: %s\n", 
                                pgr_name,
                                filenames[j],
                                strerror(errno));
                        return EXIT_FAILURE;
                    }
                    printf("%s\t", mtime);
                    free(mtime);

                    // Print link
                    if (S_ISLNK(file_stat->st_mode)) {
                        symlink = calloc(symlink_len, sizeof(char));
                        if (symlink == NULL) {
                            perror(pgr_name);
                            return EXIT_FAILURE;
                        }

                        if ((nread = readlinkat(dir, filenames[j], 
                                    symlink, symlink_len)) == -1) {
                            fprintf(stderr,
                                    "%s: cannot read link '%s': %s\n",
                                    pgr_name,
                                    filenames[j],
                                    strerror(errno));
                            return EXIT_FAILURE;
                        }
                        // Add terminal null point
                        if (nread < symlink_len)
                            symlink[nread] = '\0';
                        else
                            symlink[nread-1] = '\0';

                        printf("%s -> %s\n", 
                                filenames[j],
                                symlink);
                        free(symlink);
                    } else {
                        printf("%s\n", filenames[j]);
                    }
                } else { // if we don't print details
                    printf("%s\n", filenames[j]);
                }
                free(file_stat);
            }
            free(filenames[j]);
        }
        free(filenames);
        close(dir);
    }

    for(d_ind = 0; d_ind < dirnames_len; d_ind++)
        free(dirnames[d_ind]);
    free(dirnames);

    return EXIT_SUCCESS;
}

