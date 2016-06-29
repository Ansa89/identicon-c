/**
 * example.c - Example code to show how to generate an identicon.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// PNG functions
#if (defined USE_CAIRO)
#include <cairo.h>
#elif (defined USE_STB)
#ifndef  STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "stb_image_write.h"
#else
#ifndef LODEPNG_NO_COMPILE_CPP
#define LODEPNG_NO_COMPILE_CPP
#endif
#include "lodepng.h"
#endif

#include "identicon-c.h"

int main(int argc, char **argv) {
	unsigned char *img = NULL;
	char *filename = NULL;
	identicon_options_t *opts = new_default_identicon_options();

	if (argc < 4) {
		printf("Usage: %s <md5|sha1|sha256|sha512> string [salt] output.png\n", argv[0]);
		return 1;
	} else if (argc == 4) {
		strncpy(opts->str, argv[2], strlen(argv[2]) % IDENTICON_MAX_STRING_LENGTH);
		opts->str[IDENTICON_MAX_STRING_LENGTH-1] = '\0';
		filename = argv[3];
	} else {
		strncpy(opts->str, argv[2], strlen(argv[2]) % IDENTICON_MAX_STRING_LENGTH);
		opts->str[IDENTICON_MAX_STRING_LENGTH-1] = '\0';
		strncpy(opts->salt, argv[3], strlen(argv[3]) % IDENTICON_MAX_SALT_LENGTH);
		opts->salt[IDENTICON_MAX_SALT_LENGTH-1] = '\0';
		filename = argv[4];
	}

	if (!strcmp(argv[1], "sha1"))
		opts->hash_type = IDENTICON_HASH_SHA1;
	else if (!strcmp(argv[1], "sha256"))
		opts->hash_type = IDENTICON_HASH_SHA256;
	else if (!strcmp(argv[1], "sha512"))
		opts->hash_type = IDENTICON_HASH_SHA512;
	else
		opts->hash_type = IDENTICON_HASH_MD5;
	
	opts->transparent = false;
	opts->stroke = false;
	opts->size = 256;

	img = new_identicon(opts);

	if (img != NULL) {
#if defined(USE_CAIRO)
		printf("Creating \"%s\" using Cairo.\n", filename);
		cairo_surface_t *surface = cairo_image_surface_create_for_data(img, CAIRO_FORMAT_ARGB32,
				opts->size, opts->size, cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, opts->size));

		if (cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS) {
			cairo_surface_flush(surface);
			cairo_surface_write_to_png(surface, filename);

			while (cairo_surface_get_reference_count(surface) > 0)
				cairo_surface_destroy(surface);
		}
#elif defined(USE_STB)
		printf("Creating \"%s\" using STB.\n", filename);
		stbi_write_png(filename, opts->size, opts->size, 4, img, opts->size * 4);
#else
		printf("Creating \"%s\" using LodePNG.\n", filename);
		lodepng_encode32_file(filename, img, opts->size, opts->size);
#endif
	}

	free(opts);
	free(img);

	return 0;
}
