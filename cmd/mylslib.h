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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
 */
int myls_str_alphanum_cmp(const char a[], const char b[])
{
    int result = strcasecmp(a, b);
    if (result)
        return result;
    return -strcmp(a, b);
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
 * filenames needs to be an valid memory zone already malloc. But
 * filenames will be increased as needed.
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
    int j, dir, nread, bpos;
    char buf[BUF_SIZE];
    char **filenames_tmp;
    struct linux_dirent *dir_entries;
    size_t filenames_real_len = 0;

    // Check that it doesn't point to null
    assert(filenames);
    assert(filenames_len);

    filenames_tmp = *filenames;

    printf("Begin In the call: %p\n", filenames_tmp);

    if ((dir = open(dir_name, O_RDONLY)) < 0) {
        fprintf(stderr, "Failed to open %s\n", dir_name);
        return EXIT_FAILURE;
    } else {
        while ((nread = syscall(SYS_getdents, dir, buf, BUF_SIZE)) > 0) {
            bpos = 0;
            while (bpos < nread) {
                dir_entries = (struct linux_dirent *) (buf + bpos);

                if (filenames_real_len == *filenames_len) {
                    *filenames_len = 2 * *filenames_len;
                    filenames_tmp = realloc(filenames_tmp, 
                            *filenames_len * sizeof(char*));
                }

                filenames_tmp[filenames_real_len] = calloc(
                        (strlen(dir_entries->d_name)+1), sizeof(char*));
                strcpy(filenames_tmp[filenames_real_len], dir_entries->d_name);
                filenames_real_len++;

                bpos += dir_entries->d_reclen;
            }
        }
    }
    close(dir);

    filenames_tmp = realloc(filenames_tmp, filenames_real_len * sizeof(char*));
    *filenames_len = filenames_real_len;
    printf("-- List in call\n");

    for(j = 0; j < *filenames_len; j++) {
        printf("%s\n", filenames_tmp[j]);
    }

    *filenames = filenames_tmp;

    printf("End In the call: %p\n", filenames_tmp);

    return EXIT_SUCCESS;
}

#endif
