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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mycplib.h"

int main(int argc, char *argv[])
{
    int dereference_flag = 1;
    int interactive_flag = 0;
    int no_clobber_flag = 0;
    int no_target_directory_flag = 0;
    int target_directory_flag = 0;
    int update_flag = 0;
    int verbose_flag = 0;

    int c, arg_ind, src_ind;
    int nbr_args;
    char *pgr_name;
    int err_msg_len = 2048;
    char *err_msg;
    char **tmp;
    char **sources;
    int sources_len = 0;
    char *dest = NULL;
    int dest_is_dir;
    int dest_must_be_a_dir;
    int source_error;
    int src_exist;
    struct stat dest_stat;
    struct stat src_stat;
    struct option options[] = 
    {
        {"dereference", no_argument, NULL, 'L'},
        {"interactive", no_argument, NULL, 'i'},
        {"no-clobber", no_argument, NULL, 'n'},
        {"no-dereference", no_argument, NULL, 'P'},
        {"no-target-directory", no_argument, NULL, 'T'},
        {"target-directory", required_argument, NULL, 't'},
        {"update", no_argument, NULL, 'u'},
        {"verbose", no_argument, NULL, 'v'},
        {0, 0, 0, 0}
    };

    pgr_name = argv[0];

    while ((c = getopt_long(argc, argv,
                "iLnPt:Tuv",
                options, NULL)) != -1) {
        switch (c) {
            case 'i':
                /*
                 * Interactive prompt before overwrite.
                 */
                interactive_flag = 1;
                no_clobber_flag = 0;
                break;
            case 'L':
                /*
                 * Always follow symbolic links in source.
                 */
                dereference_flag = 1;
                break;
            case 'n':
                /*
                 * do not overwrite an existing file.
                 */
                no_clobber_flag = 1;
                interactive_flag = 0;
                break;
            case 'P':
                /*
                 * Never follow symbolic links in source.
                 */
                dereference_flag = 0;
                break;
            case 't':
                /*
                 * copy all source arguments into the directory argument
                 * of this option.
                 */
                target_directory_flag = 1;
                if (optarg == NULL) {
                    perror(pgr_name);
                    return EXIT_FAILURE;
                }
                dest = strdup(optarg);
                if (dest == NULL) {
                    perror(pgr_name);
                    return EXIT_FAILURE;
                }
                break;
            case 'T':
                /*
                 * treat destination as a normal file.
                 */
                no_target_directory_flag = 1;
                break;
            case 'u':
                /*
                 * Copy only when the source file is newer than the
                 * destination file or when the destination file is missing.
                 */
                update_flag = 1;
                break;
            case 'v':
                /*
                 * Explain what's being done.
                 */
                verbose_flag = 1;
                break;
            default:
                abort();
        }
    }

    /*
     * Check errors in the number of arguments
     */
    nbr_args = argc - optind;
    dest_must_be_a_dir = target_directory_flag || (nbr_args > 2);

    if (nbr_args == 0) {
        fprintf(stderr, 
                "%s: missing file operand\n",
                pgr_name);
        return EXIT_FAILURE;
    }

    if (nbr_args == 1 && !target_directory_flag) {
        fprintf(stderr, 
                "%s: missing destination file after '%s'\n",
                pgr_name,
                argv[1]);
        return EXIT_FAILURE;
    }

    if (no_target_directory_flag && dest_must_be_a_dir) {
        fprintf(stderr,
                "%s: extra operand '%s'\n",
                pgr_name,
                dest);
        free(dest);
        return EXIT_FAILURE;
    }

    /*
     * Catch the destination file or directory
     */
    if (!target_directory_flag) {
        dest = strdup(argv[argc-1]);
        if (dest == NULL) {
            perror(pgr_name);
            return EXIT_FAILURE;
        }
    }

    /*
     * Check errors for the destination
     */
    dest_is_dir = 0;
    if (!stat(dest, &dest_stat)) {
        dest_is_dir = S_ISDIR(dest_stat.st_mode);
    }

    if (!dest_is_dir && dest_must_be_a_dir) {
        fprintf(stderr,
                "%s: target '%s' is not a directory\n",
                pgr_name,
                dest);
        free(dest);
        return EXIT_FAILURE;
    }

    if (no_target_directory_flag && dest_is_dir) {
        fprintf(stderr,
                "%s: cannot overwrite directory '%s' with non-directory\n",
                pgr_name,
                dest);
        free(dest);
        return EXIT_FAILURE;
    }

    /*
     * Catch source files
     */
    if (target_directory_flag) {
        sources_len = nbr_args;
    } else {
        sources_len = nbr_args - 1;
    }

    sources = malloc(sources_len * sizeof(char*));
    if (sources == NULL) {
        perror(pgr_name);
        free(dest);
        return EXIT_FAILURE;
    }

    src_ind = 0;
    source_error = 0;
    src_exist = 0;
    for (arg_ind = optind; arg_ind < optind + sources_len; arg_ind++) {
        src_exist = !access(argv[arg_ind], R_OK);
        if (!src_exist) {
            fprintf(stderr, 
                    "%s: cannot stat '%s': %s\n", 
                    pgr_name,
                    argv[arg_ind],
                    strerror(errno));
            source_error = 1;
        } else {
            if (stat(argv[arg_ind], &src_stat)) {
                perror(pgr_name);
                free(dest);
                mycp_free_sources(sources, src_ind);
                return EXIT_FAILURE;
            }

            if (S_ISDIR(src_stat.st_mode)) {
                fprintf(stderr, 
                        "%s: omitting directory '%s'\n", 
                        pgr_name,
                        argv[arg_ind]);
            } else {
                sources[src_ind] = strdup(argv[arg_ind]);
                if (sources[src_ind] == NULL) {
                    perror(pgr_name);
                    free(dest);
                    mycp_free_sources(sources, src_ind);
                    return EXIT_FAILURE;
                }
                src_ind++;
            }
        }
    }
    // src_ind is the size of sources

    if (source_error) {
        // Error messages have already been printed
        free(dest);
        mycp_free_sources(sources, src_ind);
        return EXIT_FAILURE;
    }

    tmp = realloc(sources, src_ind * sizeof(char*));
    if (tmp == NULL) {
        perror(pgr_name);
        free(dest);
        mycp_free_sources(sources, sources_len);
        return EXIT_FAILURE;
    }
    sources_len = src_ind;
    sources = tmp;

    /*
     * Copy the source(s) in destination
     */
    err_msg = calloc(err_msg_len, sizeof(char));
    if (err_msg == NULL) {
        perror(pgr_name);
        free(dest);
        mycp_free_sources(sources, sources_len);
        return EXIT_FAILURE;
    }

    if (mycp_do_copy(sources, sources_len, dest, dest_is_dir, 
                &err_msg, err_msg_len,
                verbose_flag, update_flag, dereference_flag,
                no_clobber_flag, interactive_flag)) {
        if (strlen(err_msg) > 0) {
            fprintf(stderr,
                    "%s: %s: %s\n",
                    pgr_name,
                    err_msg,
                    strerror(errno));
        } else {
            fprintf(stderr, 
                    "%s: failed to copy: %s\n", 
                    pgr_name,
                    strerror(errno));
        }
        free(dest);
        free(err_msg);
        mycp_free_sources(sources, sources_len);
        return EXIT_FAILURE;
    }

    free(err_msg);
    mycp_free_sources(sources, sources_len);
    free(dest);

    return EXIT_SUCCESS;
}

