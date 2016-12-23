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

#ifndef MYUPTIMELIB_H
#define MYUPTIMELIB_H

/*
 * Get number of connected users.
 *
 * Retern value:
 *     the number of connected users. If an error occured, -1 is returned.
 *
 * See also:
 *     http://stackoverflow.com/questions/25628316/osx-yosemite-getutxent-returning-the-same-user-twice
 *     man 3 getutent
 *     man 5 utmp
 */
int myuptime_count_connected_users();

/*
 * Get the time.
 * Don't forget to free the returned string after usage.
 *
 * Return value:
 *     A string representing the time. If an error occured, NULL is returned.
 *
 * See also:
 *     man sysinfo
 */
char* myuptime_get_time();

/*
 * Get uptime (the time elapsed since the system starts).
 * Don't forget to free the returned string after usage.
 *
 * Return value:
 *     A string representing the time elapsed since the system starts.
 *     If an error occured, NULL is returned.
 *
 * See also:
 *     http://advancedlinuxprogramming.com/listings/chapter-7/print-uptime.c
 *     man sysinfo
 *     man strftime
 */
char* myuptime_get_uptime(int pretty_flag);

/*
 * Get the load of the system in a string format.
 * Don't forget to free the returned string after usage.
 *
 * Return value:
 *     A string representing the time elapsed since the system starts.
 *     If an error occured, NULL is returned.
 *
 * See also:
 *     http://stackoverflow.com/questions/4993158/sysinfo-system-call-returns-wrong-load-average-values-on-linux
 *     man sysinfo
 */
char* myuptime_get_load();

/*
 * Get system start date in a string format.
 * Don't forget to free the returned string after usage.
 *
 * Return value:
 *     A string representing the date and time since the system is up.
 *     If an error occured, NULL is returned.
 */
char* myuptime_get_start_date();

#endif
