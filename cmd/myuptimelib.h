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

#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <utmpx.h>

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
int myuptime_count_connected_users()
{
    struct utmpx *user_info;
    int nbr_user;

    setutxent();

    nbr_user = 0;
    while ((user_info = getutxent()) != NULL) {
        if (user_info->ut_type == 7)
            nbr_user++;
    }
    return nbr_user;
}

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
char* myuptime_get_time()
{
    int str_len = 100;
    char str[str_len];
    time_t now_raw;
    struct tm now_lt;

    now_raw = time(NULL);
    if (now_raw == -1)
        return NULL;

    if (localtime_r(&now_raw, &now_lt) == NULL)
        return NULL;

    if (strftime(str, str_len, "%H:%M:%S", &now_lt) < 0)
        return NULL;

    return strdup(str);
}

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
char* myuptime_get_uptime(int pretty_flag)
{
    int str_len = 512;
    char *str;
    char *str_tmp;
    char *str_res;
    long uptime;
    struct sysinfo si;
    const long minute = 60;
    const long hour = minute * 60;
    const long day = hour * 24;
    const long week = 7 * day;
    const long year = 365 * day;
    long minutes, hours, days, weeks, years;
    long res;

    str_tmp = calloc(str_len, sizeof(char));
    if (str_tmp == NULL)
        return NULL;

    str = calloc(str_len, sizeof(char));
    if (str == NULL)
        return NULL;

    if (sysinfo(&si) == -1)
        return NULL;

    uptime = si.uptime;

    years = uptime / year;
    weeks = (uptime % year) / week;
    // days change with pretty_flag
    hours = (uptime % day) / hour;
    minutes = (uptime % hour) / minute;

    if (pretty_flag) {
        days = (uptime % week) / day;
        // years
        if (years) {
            sprintf(str_tmp, "%ld years, ", years);
            strcat(str, str_tmp);
        }
        // Weeks
        if (weeks) {
            sprintf(str_tmp, "%ld weeks, ", weeks);
            strcat(str, str_tmp);
        }
        // days
        if (days) {
            sprintf(str_tmp, "%ld days, ", days);
            strcat(str, str_tmp);
        }
        // Hours
        if (hours) {
            sprintf(str_tmp, "%ld hours, ", hours);
            strcat(str, str_tmp);
        }
        // Minutes
        if (minutes) {
            sprintf(str_tmp, "%02ld minutes", minutes);
            strcat(str, str_tmp);
        }
    } else {
        days = (uptime % year) / day;
        // years
        if (years) {
            sprintf(str_tmp, "%ld years, ", years);
            strcat(str, str_tmp);
        }
        // days
        if (days) {
            sprintf(str_tmp, "%ld days, ", days);
            strcat(str, str_tmp);
        }
        // Hours
        if (hours) {
            sprintf(str_tmp, "%ld:", hours);
            strcat(str, str_tmp);
        }
        // Minutes
        if (minutes) {
            if (hours) 
                sprintf(str_tmp, "%02ld", minutes);
            else
                sprintf(str_tmp, "%02ld minutes", minutes);
            strcat(str, str_tmp);
        }
    }

    free(str_tmp);
    str_res = strdup(str);
    free(str);

    return str_res;
}

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
char* myuptime_get_load()
{
    int str_len = 512;
    char *str;
    char *str_res;
    struct sysinfo si;

    str = calloc(str_len, sizeof(char));
    if (str == NULL)
        return NULL;

    if (sysinfo(&si) == -1)
        return NULL;

    sprintf(str, 
            "%.2f, %.2f, %.2f", 
            si.loads[0] / (float)(1 << SI_LOAD_SHIFT),
            si.loads[1] / (float)(1 << SI_LOAD_SHIFT),
            si.loads[2] / (float)(1 << SI_LOAD_SHIFT));

    str_res = strdup(str);

    free(str);

    return str_res;
}

/*
 * Get system start date in a string format.
 * Don't forget to free the returned string after usage.
 *
 * Return value:
 *     A string representing the date and time since the system is up.
 *     If an error occured, NULL is returned.
 */
char* myuptime_get_start_date()
{
    int str_len = 20;
    char str[str_len];
    time_t now_raw;
    time_t start_raw;
    struct tm start_lt;
    struct sysinfo si;

    if (sysinfo(&si) == -1)
        return NULL;

    now_raw = time(NULL);
    if (now_raw == -1)
        return NULL;

    start_raw = now_raw - si.uptime;

    if (localtime_r(&start_raw, &start_lt) == NULL)
        return NULL;

    if (strftime(str, str_len, "%F %T", &start_lt) < 0)
        return NULL;

    return strdup(str);
}

#endif
