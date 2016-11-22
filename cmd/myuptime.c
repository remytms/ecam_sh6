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

int main(int argc, char *argv[])
{
    int pretty_flag = 0;
    int since_flag = 0;

    int c;
    char *pgr_name;
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

    /*
     * Complete code here.
     */

    return EXIT_SUCCESS;
}

