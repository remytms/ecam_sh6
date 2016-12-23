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
#include <unistd.h>

/*
 * Buffer size for getdents
 * See: man 2 getdents
 */
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
    if (result) 
        return result;

    result = -strcmp(a_str, b_str);
    if (result)
        return result;
    if (a_is_hidden)
        return 1;
    if (b_is_hidden)
        return -1;

    return result;
}

int myls_list_file_in_dir(int dir, char ***filenames, int *filenames_len)
{
    int nread, bpos;
    char buf[BUF_SIZE];
    char **filenames_tmp;
    char **tmp;
    struct linux_dirent *dir_entries;
    size_t filenames_real_len = 0;

    // Check that it doesn't point to null
    assert(filenames);
    assert(filenames_len);

    filenames_tmp = *filenames;

    *filenames_len = 2;
    filenames_tmp = calloc(*filenames_len, sizeof(char*));
    if (filenames_tmp == NULL)
        return EXIT_FAILURE;

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
                    tmp = realloc(
                            filenames_tmp, 
                            *filenames_len * sizeof(char*));
                    if (tmp == NULL)
                        return EXIT_FAILURE;
                    filenames_tmp = tmp;
                }

                filenames_tmp[filenames_real_len] = strdup(dir_entries->d_name);
                if (filenames_tmp[filenames_real_len] == NULL)
                    return EXIT_FAILURE;
                filenames_real_len++;

                bpos += dir_entries->d_reclen;
            }
        }
    }

    tmp = realloc(filenames_tmp, filenames_real_len * sizeof(char*));
    if (tmp == NULL)
        return EXIT_FAILURE;
    filenames_tmp = tmp;
    *filenames_len = filenames_real_len;

    *filenames = filenames_tmp;

    return EXIT_SUCCESS;
}

int myls_is_reg_file(char *filename)
{
    struct stat filename_stat;
    if (stat(filename, &filename_stat) == -1) {
        return -1;
    }
    return S_ISREG(filename_stat.st_mode);
}

int myls_is_dir(char *filename)
{
    struct stat filename_stat;
    if (stat(filename, &filename_stat) == -1) {
        return -1;
    }
    return S_ISDIR(filename_stat.st_mode);
}

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
    if ((now_raw = time(NULL)) == -1)
        return -1;

    if (localtime_r(&now_raw, &now_lt) == NULL)
        return -1;
    if (localtime_r(&mtime_raw, &mtime_lt) == NULL)
        return -1;

    if (now_lt.tm_year == mtime_lt.tm_year && 
        now_lt.tm_mon == mtime_lt.tm_mon) {
        if ((res = strftime(str, 100, "%b %d %H:%M", &mtime_lt)) < 0)
            return -1;
        if ((*mtime_str = strdup(str)) == NULL)
            return -1;
        return res;
    } else {
        if ((res = strftime(str, 100, "%b %d  %Y", &mtime_lt)) < 0)
            return -1;
        if ((*mtime_str = strdup(str)) == NULL)
            return -1;
        return res;
    }
}

int myls_get_size(struct stat *filename_stat)
{
    return filename_stat->st_size;
}

int myls_get_nlink(struct stat *filename_stat)
{
    return filename_stat->st_nlink;
}

int myls_get_username(struct stat *filename_stat, char **str)
{
    struct passwd *pw;

    pw = getpwuid(filename_stat->st_uid);
    if (pw == NULL)
        return EXIT_FAILURE;

    *str = strdup(pw->pw_name);
    if (*str == NULL)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int myls_get_groupname(struct stat *filename_stat, char **str)
{
    struct group *grp;

    grp = getgrgid(filename_stat->st_gid);
    if (grp == NULL)
        return EXIT_FAILURE;

    *str = strdup(grp->gr_name);
    if (*str == NULL)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int myls_get_permission(struct stat *filename_stat, 
        char **result_str)
{
    mode_t filemode;
    char *str = calloc(11, sizeof(char));
    if (str == NULL)
        return EXIT_FAILURE;

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
    if (S_ISUID & filemode) strcat(str, "s");
    else if (S_IXUSR & filemode) strcat(str, "x");
    else strcat(str, "-");

    if (S_IRGRP & filemode) strcat(str, "r");
    else strcat(str, "-");
    if (S_IWGRP & filemode) strcat(str, "w");
    else strcat(str, "-");
    if (S_ISGID & filemode) strcat(str, "s");
    else if (S_IXGRP & filemode) strcat(str, "x");
    else strcat(str, "-");

    if (S_IROTH & filemode) strcat(str, "r");
    else strcat(str, "-");
    if (S_IWOTH & filemode) strcat(str, "w");
    else strcat(str, "-");
    if (S_ISVTX & filemode) strcat(str, "s");
    else if (S_IXOTH & filemode) strcat(str, "x");
    else strcat(str, "-");

    *result_str = str;

    return EXIT_SUCCESS;
}

int myls_get_file_stat(int dir, char *filename, struct stat **stat)
{
    struct stat *filename_stat = *stat;
    if (fstatat(dir, filename, filename_stat, AT_SYMLINK_NOFOLLOW))
        return EXIT_FAILURE;
    *stat = filename_stat;
    return EXIT_SUCCESS;
}

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

    if ((ar_new = calloc(ar_old_len + 1, sizeof(char*))) == NULL)
        return EXIT_FAILURE;

    i_old = 0;
    i_new = 0;
    while (i_new < (ar_old_len + 1)) {
        if (i_new == pos) {
            ar_new[i_new] = strdup(elem);
            if (ar_new[i_new] == NULL)
                return EXIT_FAILURE;
            i_new++;
        } else {
            ar_new[i_new] = strdup(ar_old[i_old]);
            if (ar_new[i_new] == NULL)
                return EXIT_FAILURE;
            free(ar_old[i_old]);
            i_new++;
            i_old++;
        }
    }
    free(ar_old);

    *ar_len = ar_old_len + 1;
    *ar = ar_new;

    return EXIT_SUCCESS;
}

char* myls_path_concat(const char *a, const char *b)
{
    char *res;
    if ((res = malloc(strlen(a)+strlen(b)+2 * sizeof(char))) == NULL)
        return res;
    res[0] = '\0';

    strcat(res, a);
    strcat(res, "/");
    strcat(res, b);

    return res;
}

