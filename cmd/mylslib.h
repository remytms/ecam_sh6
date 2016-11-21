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

#ifndef MYLSLIB_H
#define MYLSLIB_H

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#define BUF_SIZE 1024

/*
 * Structure reprensenting a linux directory entry
 * See :
 *     man 2 getdents
 */
struct linux_dirent {
    long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    char d_name[];
};

/*
 * Compare 2 string alphabetically.
 *
 * Args:
 *     char* a: a string.
 *     char* b: a string.
 *
 * Return value:
 *     negative value if a < b.
 *     0 if a == b.
 *     positive value if a > b.
 * 
 * See also:
 *     http://stackoverflow.com/questions/3489139/how-to-qsort-an-array-of-pointers-to-char-in-c
 */
int myls_str_alphanum_cmp(const void* a, const void* b)
{
    int result;
    char *a_str = *(char**)a;
    char *b_str = *(char**)b;
    int a_is_hidden = a_str[0] == '.';
    int b_is_hidden = b_str[0] == '.';

    if (a_is_hidden)
        a_str += sizeof(char);

    if (b_is_hidden)
        b_str += sizeof(char);

    result = strcasecmp(a_str, b_str);
    if (result) {
        return result;
    } else {
        result = -strcmp(a_str, b_str);
        if (result)
            return result;
        if (a_is_hidden)
            return 1;
        if (b_is_hidden)
            return -1;
    }
}

/*
 * This create a the list of the name of the files in a directory.
 * At the end of the excecution of the function,
 *     filenames will containe all the names of the files contained in
 *     dir_name.
 *     filenames_len will be number of files contained in dir_name.
 *
 * Args:
 *     char* dir_name: string with the name of the directory.
 *     char*** filenames: the array to fill in.
 *     int* filenames_len: the length of the array filenames.
 *
 * Return value:
 *     EXIT_SUCCESS if no errors occured.
 *     EXIT_FAILURE if errors occured.
 */
int myls_list_file_in_dir(int dir, char ***filenames, int *filenames_len)
{
    int nread, bpos;
    char buf[BUF_SIZE];
    char **filenames_tmp;
    struct linux_dirent *dir_entries;
    size_t filenames_real_len = 0;

    // Check that it doesn't point to null
    assert(filenames);
    assert(filenames_len);

    filenames_tmp = *filenames;

    *filenames_len = 2;
    filenames_tmp = calloc(*filenames_len, sizeof(char*));

    if (dir < 0) {
        free(filenames_tmp);
        return EXIT_FAILURE;
    } else {
        while ((nread = syscall(SYS_getdents, dir, buf, BUF_SIZE)) > 0) {
            bpos = 0;
            while (bpos < nread) {
                dir_entries = (struct linux_dirent *) (buf + bpos);

                if (filenames_real_len == *filenames_len) {
                    *filenames_len *= 2;
                    filenames_tmp = realloc(
                            filenames_tmp, 
                            *filenames_len * sizeof(char*));
                }

                filenames_tmp[filenames_real_len] = calloc(
                        (strlen(dir_entries->d_name) + 1),
                        sizeof(char*));
                strcpy(filenames_tmp[filenames_real_len], dir_entries->d_name);
                filenames_real_len++;

                bpos += dir_entries->d_reclen;
            }
        }
    }

    filenames_tmp = realloc(filenames_tmp, filenames_real_len * sizeof(char*));
    *filenames_len = filenames_real_len;

    *filenames = filenames_tmp;

    return EXIT_SUCCESS;
}

/*
 * Determine if filename is a regular file or not.
 *
 * Return value:
 *     1 if filename is a regular file.
 *     0 if not.
 *
 * See also:
 *     http://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
 *     man 2 stat
 */
int myls_is_reg_file(char *filename)
{
    struct stat filename_stat;
    stat(filename, &filename_stat);
    return S_ISREG(filename_stat.st_mode);
}

/*
 * Determine if filename is a directory or not.
 *
 * Return value:
 *     1 if filename is a directory.
 *     0 if not.
 */
int myls_is_dir(char *filename)
{
    struct stat filename_stat;
    stat(filename, &filename_stat);
    return S_ISDIR(filename_stat.st_mode);
}

/*
 * Get modification time for the given filename
 * See also:
 *     http://stackoverflow.com/questions/13542345/how-to-convert-st-mtime-which-get-from-stat-function-to-string-or-char
 *     man 3 strftime
 *     man 3 localtime
 *     man 2 stat
 *     man 2 time
 */
size_t myls_get_mtime(struct stat *filename_stat, 
        char **mtime_str)
{
    char str[100];
    struct tm mtime_lt;
    struct tm now_lt;
    time_t mtime_raw;
    time_t now_raw;
    size_t res;

    mtime_raw = filename_stat->st_mtime;
    now_raw = time(NULL);

    localtime_r(&now_raw, &now_lt);
    localtime_r(&mtime_raw, &mtime_lt);

    if (now_lt.tm_year == mtime_lt.tm_year && 
        now_lt.tm_mon == mtime_lt.tm_mon) {
        if ((res = strftime(str, 100, "%b %d %H:%M", &mtime_lt)) < 0)
            return -1;
        *mtime_str = strdup(str);
        return res;
    } else {
        if ((res = strftime(str, 100, "%b %d  %Y", &mtime_lt)) < 0)
            return -1;
        *mtime_str = strdup(str);
        return res;
    }
}

/*
 * Get the size of a file.
 */
int myls_get_size(struct stat *filename_stat)
{
    return filename_stat->st_size;
}

/*
 * Get number of hard link of a file.
 */
int myls_get_nlink(struct stat *filename_stat)
{
    return filename_stat->st_nlink;
}

/*
 * Get owner name of a file.
 * See also:
 *     man 2 stat
 *     man 3 getpwuid
 */
int myls_get_username(struct stat *filename_stat, char **str)
{
    struct passwd *pw;

    pw = getpwuid(filename_stat->st_uid);
    if (pw == NULL)
        return EXIT_FAILURE;

    *str = strdup(pw->pw_name);

    return EXIT_SUCCESS;
}

/*
 * Get group name of a file.
 * See also:
 *     man 2 stat
 *     man 3 getgrgid
 */
int myls_get_groupname(struct stat *filename_stat, char **str)
{
    struct group *grp;

    grp = getgrgid(filename_stat->st_gid);
    if (grp == NULL)
        return EXIT_FAILURE;

    *str = strdup(grp->gr_name);

    return EXIT_SUCCESS;
}

/*
 * Get permission in string format for a file.
 * See also:
 *     https://en.wikipedia.org/wiki/Unix_file_types
 *     https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html
 *     man 2 stat
 *     man 3 strcat
 *     man 3 errno
 */
int myls_get_permission(struct stat *filename_stat, 
        char **result_str)
{
    mode_t filemode;
    char *str = calloc(11, sizeof(char));

    filemode = filename_stat->st_mode;
    str[0] = '\0';

    if (S_ISREG(filemode)) strcat(str, "-");
    if (S_ISDIR(filemode)) strcat(str, "d");
    if (S_ISCHR(filemode)) strcat(str, "c");
    if (S_ISBLK(filemode)) strcat(str, "b");
    if (S_ISFIFO(filemode)) strcat(str, "p");
    if (S_ISLNK(filemode)) strcat(str, "l");
    if (S_ISSOCK(filemode)) strcat(str, "s");

    if (S_IRUSR & filemode) strcat(str, "r");
    else strcat(str, "-");
    if (S_IWUSR & filemode) strcat(str, "w");
    else strcat(str, "-");
    if (S_IXUSR & filemode) strcat(str, "x");
    else strcat(str, "-");

    if (S_IRGRP & filemode) strcat(str, "r");
    else strcat(str, "-");
    if (S_IWGRP & filemode) strcat(str, "w");
    else strcat(str, "-");
    if (S_IXGRP & filemode) strcat(str, "x");
    else strcat(str, "-");

    if (S_IROTH & filemode) strcat(str, "r");
    else strcat(str, "-");
    if (S_IWOTH & filemode) strcat(str, "w");
    else strcat(str, "-");
    if (S_IXOTH & filemode) strcat(str, "x");
    else strcat(str, "-");

    *result_str = str;

    return EXIT_SUCCESS;
}

/*
 * Get a file stat.
 */
int myls_get_file_stat(int dir, char *filename, struct stat **stat)
{
    struct stat *filename_stat = *stat;
    if (dir < 0) {
        if (lstat(filename, filename_stat))
            return EXIT_FAILURE;
    } else {
        if (fstatat(dir, filename, filename_stat, AT_SYMLINK_NOFOLLOW))
            return EXIT_FAILURE;
    }
    *stat = filename_stat;
    return EXIT_SUCCESS;
}

/*
 * Insert the element after the given position.
 */
int myls_array_insert(int pos, char *elem, char ***ar, int *ar_len)
{
    char **ar_old;
    char **ar_new;
    int i_old, i_new;
    int ar_old_len;

    // Check that it doesn't point to null
    assert(ar);
    assert(ar_len);

    ar_old = *ar;
    ar_old_len = *ar_len;

    ar_new = calloc(ar_old_len + 1, sizeof(char*));

    i_old = 0;
    i_new = 0;
    while (i_new < (ar_old_len + 1)) {
        if (i_new == pos) {
            ar_new[i_new] = strdup(elem);
            i_new++;
        } else {
            ar_new[i_new] = strdup(ar_old[i_old]);
            free(ar_old[i_old]);
            i_new++;
            i_old++;
        }
    }
    free(ar_old);

    *ar_len = ar_old_len + 1;
    *ar = ar_new;
}

/*
 * Concatenate 2 path.
 */
char* myls_path_concat(const char *a, const char *b)
{
    char *res = malloc(strlen(a)+strlen(b)+2 * sizeof(char));
    res[0] = '\0';

    strcat(res, a);
    strcat(res, "/");
    strcat(res, b);

    return res;
}

#endif
