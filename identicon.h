/**
 * identicon.h - Declaration of functions and data types to
 * generate an identicon.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IDENTICON_H
#define IDENTICON_H

#include <stdbool.h>
#include <cairo.h>

#define MAX_STRING_LENGTH 4096
#define MAX_SALT_LENGTH 1024


typedef struct identicon_RGB_t {
	double red;
	double green;
	double blue;
} identicon_RGB_t;

typedef struct identicon_options_t {
	char str[MAX_STRING_LENGTH];
	char salt[MAX_SALT_LENGTH];
	int size;
	double margin;
	bool transparent;
	bool stroke;
} identicon_options_t;


identicon_options_t *new_default_identicon_options();
cairo_t *new_identicon_context(identicon_options_t *opts);
void new_identicon_png(identicon_options_t *opts, char *file);

#endif
