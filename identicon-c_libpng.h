/**
 * identicon-c_libpng.h - Declaration of functions and data types to
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

#ifndef IDENTICON_LIBPNG_H
#define IDENTICON_LIBPNG_H

#include <png.h>
#include <identicon-c.h>


// Convert identicon array image to png_byte (facility for libpng)
png_byte **png_new_identicon_from_array(unsigned char *img, identicon_options_t *opts);

// Create a new identicon (facility for libpng)
png_byte **png_new_identicon(identicon_options_t *opts);

#endif
