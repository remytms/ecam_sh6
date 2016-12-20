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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <unistd.h>
#include <utmpx.h>

int main(int argc, char *argv[])
{
    int heading_flag = 0;
    int count_flag = 0;

    int c;
    int nbr_user;
    char *pgr_name;
    int date_len = 100;
    char date[date_len];
    struct utmpx *user_info;
    struct tm user_lt;
    struct option options[] = 
    {
        {"heading", no_argument, NULL, 'H'},
        {"count", no_argument, NULL, 'q'},
        {0, 0, 0, 0}
    };

    pgr_name = argv[0];

    while ((c = getopt_long(argc, argv,
                "Hq",
                options, NULL)) != -1) {
        switch (c) {
            case 'H':
                heading_flag = 1;
                break;
            case 'q':
                count_flag = 1;
                break;
            default:
                abort();
        }
    }

    setutxent();

    nbr_user = 0;
    while ((user_info = getutxent()) != NULL) {
        if (user_info->ut_type == 7) {
            if (count_flag) {
                printf("%s ", user_info->ut_user);
            } else {
                if (heading_flag && nbr_user == 0)
                    printf("NAME\tLINE\tTIME\t\t\tCOMMENT\n");

                printf("%s\t", user_info->ut_user);
                printf("%s\t", user_info->ut_line);

                if (localtime_r(&user_info->ut_tv.tv_sec, &user_lt) == NULL) {
                    perror(pgr_name);
                    return EXIT_FAILURE;
                }

                if (strftime(date, date_len, "%F %R", &user_lt) < 0) {
                    perror(pgr_name);
                    return EXIT_FAILURE;
                }

                printf("%s\t", date);
                printf("(%s)\t", user_info->ut_host);
                printf("\n");
            }
            nbr_user++;
        }
    }

    if (count_flag)
        printf("\n# users=%d\n", nbr_user);

    return EXIT_SUCCESS;
}

