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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mycplib.h"

int main(int argc, char *argv[])
{
    int link_flag = 0;
    int no_clobber_flag = 0;
    int no_target_directory_flag = 0;
    int recursive_flag = 0;
    int target_directory_flag = 0;
    int update_flag = 0;
    int verbose_flag = 0;

    int c, i, j, arg_ind, src_ind;
    int nbr_args;
    char *pgr_name;
    char **sources;
    int sources_len = 0;
    char *dest = NULL;
    int dest_exist;
    int dest_is_dir;
    int dest_must_be_a_dir;
    int source_error = 0;
    struct stat dest_stat;
    struct option options[] = 
    {
        {"link", no_argument, NULL, 'l'},
        {"no-clobber", no_argument, NULL, 'n'},
        {"recursive", no_argument, NULL, 'r'},
        {"target-directory", required_argument, NULL, 't'},
        {"no-target-directory", no_argument, NULL, 'T'},
        {"update", no_argument, NULL, 'u'},
        {"verbose", no_argument, NULL, 'v'},
        {0, 0, 0, 0}
    };

    pgr_name = argv[0];

    while ((c = getopt_long(argc, argv,
                "lnrRt:Tuv",
                options, NULL)) != -1) {
        switch (c) {
            case 'l':
                link_flag = 1;
                break;
            case 'n':
                no_clobber_flag = 1;
                break;
            case 'r':
            case 'R':
                recursive_flag = 1;
                break;
            case 't':
                target_directory_flag = 1;
                if (optarg == NULL) {
                    perror(pgr_name);
                    exit(EXIT_FAILURE);
                }
                dest = strdup(optarg);
                if (dest == NULL) {
                    perror(pgr_name);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'T':
                no_target_directory_flag = 1;
                break;
            case 'u':
                update_flag = 1;
                break;
            case 'v':
                verbose_flag = 1;
                break;
            default:
                abort();
        }
    }

    nbr_args = argc - optind;
    dest_must_be_a_dir = target_directory_flag || (nbr_args > 2);

    if (nbr_args == 0) {
        fprintf(stderr, 
                "%s: missing file operand\n",
                pgr_name);
        exit(EXIT_FAILURE);
    }

    if (nbr_args == 1 && !target_directory_flag) {
        fprintf(stderr, 
                "%s: missing destination file after '%s'\n",
                pgr_name,
                argv[1]);
        exit(EXIT_FAILURE);
    }

    if (!target_directory_flag) {
        dest = strdup(argv[argc-1]);
        if (dest == NULL) {
            perror(pgr_name);
            exit(EXIT_FAILURE);
        }
    }

    if (no_target_directory_flag && dest_must_be_a_dir) {
        fprintf(stderr,
                "%s: extra operand '%s'\n",
                pgr_name,
                dest);
        free(dest);
        exit(EXIT_FAILURE);
    }

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
        exit(EXIT_FAILURE);
    }

    if (no_target_directory_flag && dest_exist) {
        if (dest_is_dir) {
            fprintf(stderr,
                    "%s: cannot overwrite directory '%s' with non-directory\n",
                    pgr_name,
                    dest);
            free(dest);
            exit(EXIT_FAILURE);
        }
    }

    if (target_directory_flag) {
        sources_len = nbr_args;
    } else {
        sources_len = nbr_args - 1;
    }

    sources = malloc(sources_len * sizeof(char*));
    if (sources == NULL) {
        perror(pgr_name);
        exit(EXIT_FAILURE);
    }

    for (arg_ind = optind; arg_ind < optind + sources_len; arg_ind++) {
        src_ind = arg_ind - optind;
        sources[src_ind] = strdup(argv[arg_ind]);
        if (sources[src_ind] == NULL) {
            perror(pgr_name);
            free(dest);
            exit(EXIT_FAILURE);
        }

        if (access(sources[src_ind], R_OK)) {
            fprintf(stderr, 
                    "%s: cannot stat '%s': %s\n", 
                    pgr_name,
                    sources[src_ind],
                    strerror(errno));
            source_error = 1;
        }
    }

    if (source_error) {
        free(dest);
        mycp_free_sources(sources, sources_len);
        exit(EXIT_FAILURE);
    }

    if (mycp_do_copy(sources, sources_len, 
                dest, dest_is_dir, verbose_flag)) {
        fprintf(stderr, 
                "%s: failed to copy: %s\n", 
                pgr_name,
                strerror(errno));
        free(dest);
        mycp_free_sources(sources, sources_len);
        exit(EXIT_FAILURE);
    }

    mycp_free_sources(sources, sources_len);
    free(dest);

    return EXIT_SUCCESS;
}

