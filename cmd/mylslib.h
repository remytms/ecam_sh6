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
 * Return the list of the files in a directory as an array of str.
 * This fill in the var filenames_len with the length of the returned array.
 *
 * Args:
 *     char* dir_name: string with the name of the directory.
 *     int* filenames_len: an empty int pointer.
 *
 * Return value:
 *     An array containing the names of the files in the given directory.
 */
char** myls_list_file_in_dir_v3(char dir_name[], int *filenames_len)
{
    int j, dir, nread, bpos;
    char buf[BUF_SIZE];
    char **filenames;
    struct linux_dirent *dir_entries;
    size_t filenames_real_len = 0;

    *filenames_len = 2;
    filenames = calloc(*filenames_len, sizeof(char*));

    printf("Begin In the call: %p\n", filenames);
    
    // Check that it doesn't point to null
    assert(filenames_len);

    if ((dir = open(dir_name, O_RDONLY)) < 0) {
        fprintf(stderr, "Failed to open %s\n", dir_name);
        return NULL;
    } else {
        while ((nread = syscall(SYS_getdents, dir, buf, BUF_SIZE)) > 0) {
            bpos = 0;
            while (bpos < nread) {
                dir_entries = (struct linux_dirent *) (buf + bpos);

                if (filenames_real_len == *filenames_len) {
                    *filenames_len = 2 * *filenames_len;
                    filenames = realloc(filenames, 
                            *filenames_len * sizeof(char*));
                }

                filenames[filenames_real_len] = calloc(
                        (strlen(dir_entries->d_name)+1), sizeof(char*));
                strcpy(filenames[filenames_real_len], dir_entries->d_name);
                filenames_real_len++;

                bpos += dir_entries->d_reclen;
            }
        }
    }
    close(dir);

    filenames = realloc(filenames, filenames_real_len * sizeof(char*));
    *filenames_len = filenames_real_len;

    printf("End In the call: %p\n", filenames);

    return filenames;
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
int myls_list_file_in_dir(char dir_name[], char ***filenames, int *filenames_len)
{
    int dir, nread, bpos;
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

    if ((dir = open(dir_name, O_RDONLY|O_DIRECTORY)) < 0) {
        fprintf(stderr, "cannot open directory %s\n", dir_name);
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
    close(dir);

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
 * Determine if filename is a link or not.
 *
 * Return value:
 *     1 if filename is a link.
 *     0 if not.
 */
int myls_is_link(char *filename)
{
    struct stat filename_stat;
    stat(filename, &filename_stat);
    return S_ISLNK(filename_stat.st_mode);
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
size_t myls_get_mtime(char *filename, char *mtime_str, size_t mtime_str_len)
{
    struct stat filename_stat;
    struct tm mtime_lt;
    struct tm now_lt;
    time_t mtime_raw;
    time_t now_raw;

    stat(filename, &filename_stat);

    mtime_raw = filename_stat.st_mtime;
    now_raw = time(NULL);

    localtime_r(&now_raw, &now_lt);
    localtime_r(&mtime_raw, &mtime_lt);

    if (now_lt.tm_year == mtime_lt.tm_year && 
        now_lt.tm_mon == mtime_lt.tm_mon)
        return strftime(mtime_str, mtime_str_len, "%b %d %H:%M", &mtime_lt);
    else
        return strftime(mtime_str, mtime_str_len, "%b %d  %d", &mtime_lt);
}

/*
 * Get the size of a file.
 */
int myls_get_size(char *filename)
{
    struct stat filename_stat;
    stat(filename, &filename_stat);
    return filename_stat.st_size;
}

/*
 * Get number of hard link of a file.
 */
int myls_get_nlink(char *filename)
{
    struct stat filename_stat;
    stat(filename, &filename_stat);
    return filename_stat.st_nlink;
}

/*
 * Get owner name of a file.
 * See also:
 *     man 2 stat
 *     man 3 getpwuid
 */
int myls_get_username(char *filename, char *str, size_t str_len)
{
    struct stat filename_stat;
    struct passwd *pw;

    stat(filename, &filename_stat);
    pw = getpwuid(filename_stat.st_uid);
    if (pw == NULL)
        return EXIT_FAILURE;

    if (strlen(pw->pw_name) > str_len)
        return EXIT_FAILURE;

    strcpy(str, pw->pw_name);

    return EXIT_SUCCESS;
}

/*
 * Get group name of a file.
 * See also:
 *     man 2 stat
 *     man 3 getgrgid
 */
int myls_get_groupname(char *filename, char *str, size_t str_len)
{
    struct stat filename_stat;
    struct group *grp;

    stat(filename, &filename_stat);
    grp = getgrgid(filename_stat.st_gid);
    if (grp == NULL)
        return EXIT_FAILURE;

    if (strlen(grp->gr_name) > str_len)
        return EXIT_FAILURE;

    strcpy(str, grp->gr_name);

    return EXIT_SUCCESS;
}

/*
 * Get permission in string format for a file.
 * See also:
 *     https://en.wikipedia.org/wiki/Unix_file_types
 *     https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html
 *     man 2 stat
 *     man 3 strcat
 */
int myls_get_permission(char *filename, char *str, size_t str_len)
{
    struct stat filename_stat;
    mode_t filemode;
    stat(filename, &filename_stat);
    filemode = filename_stat.st_mode;
    str[0] = '\0';

    if (str_len < 11)
        return EXIT_FAILURE;

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

    return EXIT_SUCCESS;
}

#endif
