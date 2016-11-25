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

#ifndef MYCPLIB_H
#define MYCPLIB_H

#include <fcntl.h>
#include <stdio.h>
#include <string.h>

/*
 * Copy sources to destination.
 */
int mycp_do_copy(char **sources, int sources_len, 
        char *destname, int dest_is_dir, char **err_msg, int err_msg_len,
        int verbose_flag, int update_flag)
{
    int src, destdir, dest;
    int s_ind, err;
    int allow_copy = 1;
    struct stat src_stat;
    struct stat dest_stat;

    if (dest_is_dir) {
        destdir = open(destname, O_RDONLY | O_DIRECTORY);
        if (destdir == -1)
            return EXIT_FAILURE;
    }

    for (s_ind = 0; s_ind < sources_len; s_ind++) {
        if (stat(sources[s_ind], &src_stat))
            return EXIT_FAILURE;

        src = open(sources[s_ind], O_RDONLY);
        if (src == -1) {
            if (dest_is_dir) {
                err = errno;
                close(destdir);
                errno = err;
            }
            return EXIT_FAILURE;
        }

        if (dest_is_dir) {
            if (faccessat(destdir, sources[s_ind], 
                        F_OK, AT_SYMLINK_NOFOLLOW) == 0) {
                if (faccessat(destdir, sources[s_ind], 
                            W_OK, AT_SYMLINK_NOFOLLOW)) {
                    err = errno;
                    snprintf(*err_msg, err_msg_len, 
                            "cannot create regular file '%s':",
                            sources[s_ind]);
                    close(src);
                    close(destdir);
                    errno = err;
                    return EXIT_FAILURE;
                }

                if (fstatat(destdir, sources[s_ind], 
                            &dest_stat, AT_SYMLINK_NOFOLLOW)) {
                    err = errno;
                    close(src);
                    close(destdir);
                    errno = err;
                    return EXIT_FAILURE;
                }

                /* 
                 * If the update_flag is on, allow copy only if source
                 * is newer than dest.
                 */
                allow_copy = !(update_flag && 
                        !(src_stat.st_mtime > dest_stat.st_mtime));
            }

            if (allow_copy) {
                if (verbose_flag) {
                    printf("'%s' -> '%s/%s'\n",
                            sources[s_ind],
                            destname,
                            sources[s_ind]);
                }

                dest = openat(destdir, sources[s_ind], 
                        O_CREAT | O_WRONLY | O_TRUNC, src_stat.st_mode);
                if (dest == -1) {
                    err = errno;
                    close(src);
                    close(destdir);
                    errno = err;
                    return EXIT_FAILURE;
                }
            }
        } else {
            if (access(destname, F_OK) == 0) {
                if (access(destname, W_OK)) {
                    err = errno;
                    snprintf(*err_msg, err_msg_len, 
                            "cannot create regular file '%s'",
                            sources[s_ind]);
                    close(src);
                    errno = err;
                    return EXIT_FAILURE;
                }

                if (stat(destname, &dest_stat)) {
                    err = errno;
                    close(src);
                    errno = err;
                    return EXIT_FAILURE;
                }

                /* 
                 * If the update_flag is on, allow copy only if source
                 * is newer than dest.
                 */
                allow_copy = !(update_flag && 
                        !(src_stat.st_mtime > dest_stat.st_mtime));
            }

            if (allow_copy) {
                if (verbose_flag) {
                    printf("'%s' -> '%s'\n",
                            sources[s_ind],
                            destname);
                }

                dest = open(destname, 
                        O_CREAT | O_WRONLY | O_TRUNC, src_stat.st_mode);
                if (dest == -1) {
                    err = errno;
                    close(src);
                    errno = err;
                    return EXIT_FAILURE;
                }
            }
        }

        if (allow_copy) {
            if (mycp_copy(src, dest)) {
                err = errno;
                close(src);
                close(dest);
                if (dest_is_dir)
                    close(destdir);
                errno = err;
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

/*
 * Copy
 *
 * See also:
 *     man 2 read
 *     man 2 write
 */
int mycp_copy(int src, int dest)
{
    ssize_t nread, nwrite;
    int buf_len = 1024;
    char buf[buf_len];

    while (nread = read(src, buf, buf_len)) {
       if (nread == -1)
           return -1;
       nwrite = write(dest, buf, nread);
       if (nwrite == -1)
           return -1;
    }

    return EXIT_SUCCESS;
}

/*
 * Free sources var
 */
void mycp_free_sources(char** array, int ar_len)
{
    int i;
    for (i = 0; i < ar_len; i++)
        free(array[i]);
    free(array);
}

#endif
