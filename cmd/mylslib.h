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
int myls_str_alphanum_cmp(const void* a, const void* b);

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
int myls_list_file_in_dir(int dir, char ***filenames, int *filenames_len);

/*
 * Determine if filename is a regular file or not.
 *
 * Return value:
 *     1 if filename is a regular file
 *     0 if not
 *     -1 if an error occured
 *
 * See also:
 *     http://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
 *     man 2 stat
 */
int myls_is_reg_file(char *filename);

/*
 * Determine if filename is a directory or not.
 *
 * Return value:
 *     1 if filename is a directory
 *     0 if not
 *     -1 if an error occured
 */
int myls_is_dir(char *filename);

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
        char **mtime_str);

/*
 * Get the size of a file.
 */
int myls_get_size(struct stat *filename_stat);

/*
 * Get number of hard link of a file.
 */
int myls_get_nlink(struct stat *filename_stat);

/*
 * Get owner name of a file.
 * See also:
 *     man 2 stat
 *     man 3 getpwuid
 */
int myls_get_username(struct stat *filename_stat, char **str);

/*
 * Get group name of a file.
 * See also:
 *     man 2 stat
 *     man 3 getgrgid
 */
int myls_get_groupname(struct stat *filename_stat, char **str);

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
        char **result_str);

/*
 * Return a file stat structure in the buffer pointed by stat for the
 * given file (filename) in the given directory (dir). If dir is -1, the
 * filename is assumed to be in the current directory. If dir is a valid
 * file descriptor, the filename is assumed to be in this directory. dir
 * must be a file descriptor to a directory.
 *
 * Args:
 *     int dir: a file descriptor for a directory
 *     char *filename: the name of the file
 *     struct stat **stat: a pointer to a stat structure that will be
 *                         filled in.
 *
 * Return value:
 *     On success it return EXIT_SUCCESS. If an allocation fails it
 *     return EXIT_FAILURE.
 */
int myls_get_file_stat(int dir, char *filename, struct stat **stat);

/*
 * Insert an element after the given position in an array. In the new
 * array the element will be at position given in pos. The array ar is
 * reallocated to include the new element. It needs to be freed after
 * usage. The length of the new array is stored in ar_len.
 *
 * Args:
 *     int pos: the position of elem in the new array
 *     char *elem: the element to add to the array
 *     char ***ar: a pointer to the array that will be modified
 *     int *ar_len: a pointer to the current length of ar
 *
 * Return value:
 *     On success it return EXIT_SUCCESS. If an allocation fails it
 *     return EXIT_FAILURE.
 */
int myls_array_insert(int pos, char *elem, char ***ar, int *ar_len);

/*
 * Concatenate 2 paths. It add the '/' between a and b. This allocate a
 * new string containing the string: a + "/" + b. Don't forget to free
 * it correctly after usage. a and b is not modified and not freed.
 *
 * Args:
 *     char *a: a path name
 *     char *b: a path name
 * 
 * Return value:
 *     A pointer to a new string: a + "/" + b
 *     If an errors occured NULL is returned
 */
char* myls_path_concat(const char *a, const char *b);

#endif
