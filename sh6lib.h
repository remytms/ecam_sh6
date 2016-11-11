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

#ifndef SH6LIB_H
#define SH6LIB_H

/*
 * Return true if the string given in args means 'exit'
 */
int sh6_is_exit(char str[])
{
    return (strcmp(str, "exit") == 0 || strcmp(str, "exit\n") == 0 ||
            strcmp(str, "quit") == 0 || strcmp(str, "quit\n") == 0);
}

#endif
