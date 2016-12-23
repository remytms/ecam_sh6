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

/*
 * Copy src to dest. src and dest must be valid file descriptors.
 *
 * Args:
 *     int scr: a file descriptor to the source
 *     int dest: a file descriptor to the destination
 *
 * Return value:
 *     If an error occured it returns -1 and errno is set apropriatly.
 *     On success it returns EXIT_SUCCESS.
 *
 * See also:
 *     man 2 read
 *     man 2 write
 */
int mycp_copy(int src, int dest);

/*
 * Copy sources to a target destination. If destname is a directory it
 * copies the sources in this directory. If destname is not a directory
 * then the first element of sources it copied to destname. If an error
 * occured, it put an error message in err_msg. err_msg must be already
 * allocated and its maximum size put in err_msg_len. err_msg_len is not
 * modified after writing an error message in err_msg.
 *
 * Args:
 *     char **sources: a list of the names of the files to be copied
 *     int sources_len: the number of file names stored in sources.
 *     char *destname: the name of the target file or directory
 *     int dest_is_dir: set to 1 if destname is a directory, 0 if not.
 *     char **err_msg: a pointer to an empty allocade memory to store
 *                     error messages.
 *     int err_msg_len: the maximum size of the err_msg buffer.
 *     int verbose_flag: set to 1 if verbose_flag is on, 0 if not.
 *     int update_flag: set to 1 if update_flag is on, 0 if not.
 *     int dereference_flag: set to 1 if dereference_flag is on, 0 if not.
 *     int no_clobber_flag: set to 1 if no_clobber_flag is on, 0 if not.
 *     int interactive_flag: set to 1 if interactive_flag is on, 0 if not.
 *
 * Return value:
 *     On sucess it returns EXIT_SUCCESS. If an error occured, it
 *     returns EXIT_FAILURE and errno or err_msg is set appropriately.
 */
int mycp_do_copy(char **sources, int sources_len, 
        char *destname, int dest_is_dir, char **err_msg, int err_msg_len,
        int verbose_flag, int update_flag, int dereference_flag,
        int no_clobber_flag, int interactive_flag);

/*
 * Free sources variable
 *
 * Args:
 *     char **array: the array to empty
 *     int arr_len: the length of the array
 *
 * Return value:
 *     nothing
 */
void mycp_free_sources(char **array, int ar_len);

#endif
