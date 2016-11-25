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
        int verbose_flag, int update_flag, int dereference_flag,
        int no_clobber_flag, int interactive_flag)
{
    int src, destdir, dest;
    int s_ind, err, ans;
    int dest_exist;
    int allow_copy = 1;
    int linkname_len_max = 1024;
    int linkname_len;
    char *linkname;
    struct stat src_stat;
    struct stat dest_stat;

    if (dest_is_dir) {
        destdir = open(destname, O_RDONLY | O_DIRECTORY);
        if (destdir == -1) {
            err = errno;
            snprintf(*err_msg, err_msg_len, 
                    "failed to open '%s'",
                    destname);
            errno = err;
            return EXIT_FAILURE;
        }
    }

    for (s_ind = 0; s_ind < sources_len; s_ind++) {
        if (dereference_flag)
            err = stat(sources[s_ind], &src_stat);
        else
            err = fstatat(AT_FDCWD, sources[s_ind], 
                    &src_stat, AT_SYMLINK_NOFOLLOW);
        if (err) {
            err = errno;
            snprintf(*err_msg, err_msg_len, 
                    "cannot stat '%s'",
                    sources[s_ind]);
            errno = err;
            return EXIT_FAILURE;
        }

        src = open(sources[s_ind], O_RDONLY);
        if (src == -1) {
            if (dest_is_dir) {
                err = errno;
                snprintf(*err_msg, err_msg_len, 
                        "failed to open '%s'",
                        sources[s_ind]);
                close(destdir);
                errno = err;
            }
            return EXIT_FAILURE;
        }

        if (dest_is_dir) {
            dest_exist = faccessat(destdir, sources[s_ind], 
                        F_OK, 0) == 0;
            if (dest_exist) {
                if (faccessat(destdir, sources[s_ind], 
                            W_OK, 0)) {
                    err = errno;
                    snprintf(*err_msg, err_msg_len, 
                            "cannot create regular file '%s'",
                            sources[s_ind]);
                    close(src);
                    close(destdir);
                    errno = err;
                    return EXIT_FAILURE;
                }

                if (fstatat(destdir, sources[s_ind], 
                            &dest_stat, 0)) {
                    err = errno;
                    snprintf(*err_msg, err_msg_len, 
                            "connot stat '%s'",
                            sources[s_ind]);
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
                        !(src_stat.st_mtime > dest_stat.st_mtime)) &&
                        !no_clobber_flag;

                if (interactive_flag) {
                    printf("%s: overwrite '%s/%s'? ", "", 
                            destname,
                            sources[s_ind]);
                    ans = getc(stdin);
                    allow_copy = ans == 'y' || ans == 'Y';
                }
            }

            if (allow_copy && !S_ISLNK(src_stat.st_mode)) {
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
                    snprintf(*err_msg, err_msg_len, 
                            "failed to open '%s/%s':",
                            destname,
                            sources[s_ind]);
                    close(src);
                    close(destdir);
                    errno = err;
                    return EXIT_FAILURE;
                }
            }
        } else {
            dest_exist = access(destname, F_OK) == 0;
            if (dest_exist) {
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
                    snprintf(*err_msg, err_msg_len, 
                            "connot stat '%s'",
                            destname);
                    close(src);
                    errno = err;
                    return EXIT_FAILURE;
                }

                /* 
                 * If the update_flag is on, allow copy only if source
                 * is newer than dest.
                 */
                allow_copy = !(update_flag && 
                        !(src_stat.st_mtime > dest_stat.st_mtime)) &&
                        !no_clobber_flag;

                if (interactive_flag) {
                    printf("%s: overwrite '%s'? ", "", 
                            destname);
                    ans = getc(stdin);
                    allow_copy = ans == 'y' || ans == 'Y';
                }
            }

            if (allow_copy && !S_ISLNK(src_stat.st_mode)) {
                if (verbose_flag) {
                    printf("'%s' -> '%s'\n",
                            sources[s_ind],
                            destname);
                }

                dest = open(destname, 
                        O_CREAT | O_WRONLY | O_TRUNC, src_stat.st_mode);
                if (dest == -1) {
                    err = errno;
                    snprintf(*err_msg, err_msg_len, 
                            "failed to open '%s'",
                            destname);
                    close(src);
                    errno = err;
                    return EXIT_FAILURE;
                }
            }
        }

        if (allow_copy && !S_ISLNK(src_stat.st_mode)) {
            if (mycp_copy(src, dest)) {
                err = errno;
                snprintf(*err_msg, err_msg_len, 
                        "cannot copy '%s'",
                        destname);
                close(src);
                close(dest);
                if (dest_is_dir)
                    close(destdir);
                errno = err;
                return EXIT_FAILURE;
            }
        }

        if (allow_copy && S_ISLNK(src_stat.st_mode)) {
            linkname = calloc(linkname_len_max, sizeof(char));
            if (linkname == NULL) {
                err = errno;
                close(src);
                close(dest);
                if (dest_is_dir)
                    close(destdir);
                errno = err;
                return EXIT_FAILURE;
            }

            linkname_len = readlink(sources[s_ind], 
                    linkname, linkname_len_max);
            if (linkname_len == -1) {
                err = errno;
                snprintf(*err_msg, err_msg_len, 
                        "failed access link '%s'",
                        sources[s_ind]);
                free(linkname);
                close(src);
                close(dest);
                if (dest_is_dir)
                    close(destdir);
                errno = err;
                return EXIT_FAILURE;
            }
            linkname[linkname_len] = '\0';

            if (dest_exist) {
                if (dest_is_dir) {
                    if (verbose_flag)
                        printf("'%s/%s' removed\n", 
                                destname, sources[s_ind]);

                    if (unlinkat(destdir, sources[s_ind], 0)) {
                        err = errno;
                        snprintf(*err_msg, err_msg_len, 
                                "failed to remove '%s/%s'",
                                destname,
                                sources[s_ind]);
                        free(linkname);
                        close(src);
                        close(dest);
                        if (dest_is_dir)
                            close(destdir);
                        errno = err;
                        return EXIT_FAILURE;
                    }
                } else {
                    if (verbose_flag)
                        printf("'%s' removed\n", destname);

                    if (unlink(destname)) {
                        err = errno;
                        snprintf(*err_msg, err_msg_len, 
                                "failed to remove '%s'",
                                sources[s_ind]);
                        free(linkname);
                        close(src);
                        close(dest);
                        if (dest_is_dir)
                            close(destdir);
                        errno = err;
                        return EXIT_FAILURE;
                    }
                }
            }

            if (dest_is_dir) {
                if (verbose_flag) {
                    printf("'%s' -> '%s/%s'\n",
                            sources[s_ind],
                            destname,
                            sources[s_ind]);
                }

                if (symlinkat(linkname, destdir, sources[s_ind])) {
                    err = errno;
                    snprintf(*err_msg, err_msg_len, 
                            "failed to create link '%s/%s'",
                            destname,
                            sources[s_ind]);
                    free(linkname);
                    close(src);
                    close(dest);
                    if (dest_is_dir)
                        close(destdir);
                    errno = err;
                    return EXIT_FAILURE;
                }
            } else {
                if (verbose_flag) {
                    printf("'%s' -> '%s'\n",
                            sources[s_ind],
                            destname);
                }

                if (symlink(linkname, destname)) {
                    err = errno;
                    snprintf(*err_msg, err_msg_len, 
                            "failed to remove '%s'",
                            sources[s_ind]);
                    free(linkname);
                    close(src);
                    close(dest);
                    if (dest_is_dir)
                        close(destdir);
                    errno = err;
                    return EXIT_FAILURE;
                }
            }
            free(linkname);
        }
        close(src);
        //close(dest);
    }

    if (dest_is_dir)
        close(destdir);

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
