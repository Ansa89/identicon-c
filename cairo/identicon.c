/**
 * identicon.c - Functions to generate an identicon.
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
/**
 * The original algorithm used in this file is from identicon.js
 * by stewartlord (https://github.com/stewartlord/identicon.js).
 *
 * Copyright (C) 2013, Stewart Lord
 *
 * All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <cairo.h>

#if defined(USE_SODIUM)
#include <sodium.h>
#elif defined(USE_OPENSSL)
#include <openssl/sha.h>
#else
#include "sha512.h"
#endif

#include "identicon.h"


static identicon_RGB_t hsl2rgb(double h, double s, double b) {
	double hsl[6];
	identicon_RGB_t color;

	h *= 6;
	hsl[0] = b += s *= b < 0.5 ? b : 1 - b;
	hsl[1] = b - (int)h % 1 * s * 2;
	hsl[2] = b -= s *= 2;
	hsl[3] = b;
	hsl[4] = b + (int)h % 1 * s;
	hsl[5] = b + s;

	color.red = hsl[(int)h % 6];
	color.green = hsl[((int)h|16) % 6];
	color.blue = hsl[((int)h|8) % 6];

	return color;
}


static unsigned long hex2int(unsigned char *str) {
	unsigned char *buf = NULL;
	unsigned long ret;
	size_t len, i;

	if (str == NULL)
		return 0;

	len = strlen((char *)str);

	if (len > 10)
		len = 10;

	buf = malloc((sizeof(unsigned char) * len * 2) + sizeof(unsigned char));

	for (i = 0; i < len; i++)
		snprintf((char *)&buf[i*2], 3, "%02x", str[i]);

	buf[(i*2)+1] = '\0';
	ret = strtoul((char *)buf, NULL, 16);
	free(buf);

	return ret;
}

static unsigned char *sha512sum(unsigned char *str, unsigned char *salt) {
	unsigned char *hash = NULL;

	if (str == NULL)
		return NULL;

#if defined(USE_SODIUM)
	crypto_hash_sha512_state state;
	hash = malloc(sizeof(unsigned char) * crypto_hash_sha512_BYTES);
	crypto_hash_sha512_init(&state);
	crypto_hash_sha512_update(&state, str, strlen((char *)str));
	if (salt != NULL)
		crypto_hash_sha512_update(&state, salt, strlen((char *)salt));
	crypto_hash_sha512_final(&state, hash);
#elif defined(USE_OPENSSL)
	SHA512_CTX ctx;
	hash = malloc(sizeof(unsigned char) * SHA512_DIGEST_LENGTH);
	SHA512_Init(&ctx);
	SHA512_Update(&ctx, str, strlen((char *)str));
	if (salt != NULL)
		SHA512_Update(&ctx, salt, strlen((char *)salt));
	SHA512_Final(hash, &ctx);
#else
	struct sha512_ctx ctx;
	hash = malloc(sizeof(unsigned char) * SHA512_DIGEST_SIZE);
	sha512_init_ctx(&ctx);
	sha512_process_bytes(str, strlen((char *)str), &ctx);
	if (salt != NULL)
		sha512_process_bytes(salt, strlen((char *)salt), &ctx);
	sha512_finish_ctx(&ctx, hash);
#endif

	return hash;
}

static void draw_part(cairo_t *context, double x, double y, double width, double height, identicon_RGB_t color, bool transparent, bool stroke, bool even) {
	if (transparent && !even)
		return;

	cairo_set_source_rgba(context, color.red, color.green, color.blue, 1);
	cairo_rectangle(context, x, y, width, height);
	cairo_fill(context);

	if (stroke && even) {
		cairo_set_source_rgba(context, color.red, color.green, color.blue, 1);
		cairo_rectangle(context, x, y, width, height);
		cairo_stroke(context);
	}
}

static void draw_identicon(cairo_t *context, identicon_options_t *opts) {
	bool even;
	int i;
	double h;
	unsigned char *hash;
	double base_margin = floor(opts->size * opts->margin);
	double cell = floor((opts->size - (base_margin * 2)) / 5);
	double margin = floor((opts->size - (cell * 5)) / 2);
	identicon_RGB_t background, foreground, color;

	hash = sha512sum((unsigned char *)opts->str, (unsigned char *)opts->salt);

	// Background color
	background.red = 240.0 / 255;
	background.green = 240.0 / 255;
	background.blue = 240.0 / 255;

	if (!opts->transparent)
		draw_part(context, 0, 0, opts->size, opts->size, background, opts->transparent, opts->stroke, false);

	// Foreground color
	h = (double)hex2int(&hash[strlen((char *)hash) - 7]);
	foreground = hsl2rgb(h / 0xfffffff, 0.5, 0.7);

	// The first 15 characters of the hash control the pixels (even/odd)
	// they are drawn down the middle first, then mirrored outwards
	for (i = 0; i < 15; i++) {
		unsigned char c[2];
		c[0] = hash[i];
		c[1] = '\0';
		even = (hex2int(c) % 2) == 0;

		if (even)
			color = foreground;
		else
			color = background;

		if (i < 5) {
			draw_part(context, 2 * cell + margin, i * cell + margin, cell, cell, color, opts->transparent, opts->stroke, even);
		} else if (i < 10) {
			draw_part(context, 1 * cell + margin, (i - 5) * cell + margin, cell, cell, color, opts->transparent, opts->stroke, even);
			draw_part(context, 3 * cell + margin, (i - 5) * cell + margin, cell, cell, color, opts->transparent, opts->stroke, even);
		} else if (i < 15) {
			draw_part(context, 0 * cell + margin, (i - 10) * cell + margin, cell, cell, color, opts->transparent, opts->stroke, even);
			draw_part(context, 4 * cell + margin, (i - 10) * cell + margin, cell, cell, color, opts->transparent, opts->stroke, even);
		}
	}
}

identicon_options_t *new_default_identicon_options() {
	identicon_options_t *opts = malloc(sizeof(identicon_options_t));

	memset(opts->str, 0, MAX_STRING_LENGTH);
	memset(opts->salt, 0, MAX_SALT_LENGTH);
	opts->size = 64;
	opts->margin = 0.08;
	opts->transparent = true;
	opts->stroke = true;

	return opts;
}

cairo_t *new_identicon_context(identicon_options_t *opts) {
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, opts->size, opts->size);
	cairo_t *context = cairo_create(surface);
	draw_identicon(context, opts);
	cairo_save(context);

	return context;
}

void new_identicon_png(identicon_options_t *opts, char *file) {
	if (file == NULL)
		return;

	cairo_t *context = new_identicon_context(opts);
	cairo_surface_write_to_png(cairo_get_target(context), file);

	while (cairo_get_reference_count(context) > 0)
		cairo_destroy(context);
}
