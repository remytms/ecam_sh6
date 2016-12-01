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
#include <stdlib.h>
#include <unistd.h>

#include "myuptimelib.h"

/*
 * See also:
 *     man 2 sysinfo
 *     man 5 utmp
 *     man 3 getutent
 */
int main(int argc, char *argv[])
{
    int pretty_flag = 0;
    int since_flag = 0;

    int c, nbr_user;
    char *pgr_name;
    char *now_time;
    char *uptime;
    char *load;
    char *since;
    struct option options[] = 
    {
        {"pretty", no_argument, NULL, 'p'},
        {"since", no_argument, NULL, 's'},
        {0, 0, 0, 0}
    };

    pgr_name = argv[0];

    while ((c = getopt_long(argc, argv,
                "ps",
                options, NULL)) != -1) {
        switch (c) {
            case 'p':
                pretty_flag = 1;
                break;
            case 's':
                since_flag = 1;
                break;
            default:
                abort();
        }
    }

    if (since_flag) {
        since = myuptime_get_start_date();
        if (since == NULL) {
            perror(pgr_name);
            return EXIT_FAILURE;
        }
        printf("%s\n", since);
        free(since);
        return EXIT_SUCCESS;
    }

    uptime = myuptime_get_uptime(pretty_flag);
    if (uptime == NULL) {
        perror(pgr_name);
        return EXIT_FAILURE;
    }

    if (pretty_flag) {
        printf("%s\n", uptime);
        free(uptime);
        return EXIT_SUCCESS;
    }

    now_time = myuptime_get_time();
    if (now_time == NULL) {
        perror(pgr_name);
        return EXIT_FAILURE;
    }

    nbr_user = myuptime_count_connected_users();
    if (nbr_user == -1) {
        perror(pgr_name);
        return EXIT_FAILURE;
    }

    load = myuptime_get_load();
    if (load == NULL) {
        perror(pgr_name);
        return EXIT_FAILURE;
    }

    printf("%s ", now_time);

    printf("up %s, ", uptime);

    printf("%d user(s), ", nbr_user);

    printf("load average: %s\n", load);

    free(now_time);
    free(uptime);
    free(load);

    return EXIT_SUCCESS;
}

