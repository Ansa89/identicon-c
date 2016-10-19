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
#if defined(USE_CAIRO)
#include <cairo.h>
#elif defined(USE_LIBPNG)
#include "identicon-c_libpng.h"
#define INCHES_PER_METER (100.0/2.54)
#define DPI 72
#elif defined(USE_STB)
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
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
#elif defined(USE_LIBPNG)
		printf("Creating \"%s\" using LibPNG.\n", filename);
		static FILE *fp;
		uint32_t i;
		png_structp png_ptr;
		png_infop info_ptr;
		png_byte **row_pointers;

		fp = fopen(filename, "wb");
		if (fp == NULL) {
			free(opts);
			free(img);
			return 1;
		}

		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr == NULL) {
			fclose(fp);
			free(opts);
			free(img);
			return 1;
		}

		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL) {
			fclose(fp);
			free(opts);
			free(img);
			return 1;
		}

		if (setjmp(png_jmpbuf(png_ptr))) {
			fclose(fp);
			png_destroy_write_struct(&png_ptr, &info_ptr);
			free(opts);
			free(img);
			return 1;
		}

		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, info_ptr, opts->size, opts->size, 8,
				PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_set_pHYs(png_ptr, info_ptr, DPI * INCHES_PER_METER,
				DPI * INCHES_PER_METER, PNG_RESOLUTION_METER);
		png_write_info(png_ptr, info_ptr);

		row_pointers = png_new_identicon_from_array(img, opts);

		if (row_pointers != NULL) {
			for (i = 0; i < opts->size; i++) {
				png_write_row(png_ptr, row_pointers[i]);
			}
			png_write_end(png_ptr, info_ptr);
		}

		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);

		for (i = 0; i < opts->size; i++)
			free(row_pointers[i]);
		free(row_pointers);
#elif defined(USE_STB)
		printf("Creating \"%s\" using STB.\n", filename);
		stbi_write_png(filename, opts->size, opts->size, 4, img, opts->size * 4);
#else
		printf("Creating \"%s\" using LodePNG.\n", filename);
		lodepng_encode32_file(filename, img, opts->size, opts->size);
#endif

		free(img);
	}

	free(opts);

	return 0;
}
