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
#include <stdint.h>
#include <stdbool.h>

// Hash functions
#if defined(USE_SODIUM)
#include <sodium.h>
#elif defined(USE_OPENSSL)
#include <openssl/md5.h>
#include <openssl/sha.h>
#else
#include "md5.h"
#include "sha1.h"
#include "sha256.h"
#include "sha512.h"
#endif

#include "identicon-c.h"


/**
 * Convert "Hue Saturation Lightness" to "Red Green Blue" color space.
 *
 * @param[in] h The hue value.
 * @param[in] s The saturation value.
 * @param[in] l The lightness value.
 *
 * @return A variable containig red, green and blue values.
 */
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

	color.red = floor(hsl[(int)h % 6] * 255);
	color.green = floor(hsl[((int)h|16) % 6] * 255);
	color.blue = floor(hsl[((int)h|8) % 6] * 255);

	return color;
}


/**
 * Convert a bytes array to its hexadecimal representation and then to its decimal representation.
 *
 * @param[in] str The array of bytes.
 *
 * @return The decimal representation of input bytes array or 0 if an error occurred.
 */
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

/**
 * Calculate string hash.
 *
 * @param[in] str       The string of which calculating hash.
 * @param[in] salt      An (optional) additional string appended to the main string.
 * @param[in] hash_type The hash algorithm to use.
 *
 * @return The hash of the string (plus the optional salt) or NULL if an error occurred.
 */
static unsigned char *checksum(unsigned char *str, unsigned char *salt, identicon_hash_t hash_type) {
	unsigned char *hash = NULL;

	if (str == NULL)
		return NULL;

	switch (hash_type) {
		case IDENTICON_HASH_MD5: {
#if defined(USE_SODIUM)
			crypto_generichash_state state;
			hash = malloc(sizeof(unsigned char) * 16);
			crypto_generichash_init(&state, NULL, 0, 16);
			crypto_generichash_update(&state, str, strlen((char *)str));
			if (salt != NULL)
				crypto_generichash_update(&state, salt, strlen((char *)salt));
			crypto_generichash_final(&state, hash, 16);
#elif defined(USE_OPENSSL)
			MD5_CTX ctx;
			hash = malloc(sizeof(unsigned char) * MD5_DIGEST_LENGTH);
			MD5_Init(&ctx);
			MD5_Update(&ctx, str, strlen((char *)str));
			if (salt != NULL)
				MD5_Update(&ctx, salt, strlen((char *)salt));
			MD5_Final(hash, &ctx);
#else
			struct md5_ctx ctx;
			hash = malloc(sizeof(unsigned char) * MD5_DIGEST_SIZE);
			md5_init_ctx(&ctx);
			md5_process_bytes(str, strlen((char *)str), &ctx);
			if (salt != NULL)
				md5_process_bytes(salt, strlen((char *)salt), &ctx);
			md5_finish_ctx(&ctx, hash);
#endif
			break;
		}
		case IDENTICON_HASH_SHA1: {
#if defined(USE_SODIUM)
			crypto_generichash_state state;
			hash = malloc(sizeof(unsigned char) * 20);
			crypto_generichash_init(&state, NULL, 0, 20);
			crypto_generichash_update(&state, str, strlen((char *)str));
			if (salt != NULL)
				crypto_generichash_update(&state, salt, strlen((char *)salt));
			crypto_generichash_final(&state, hash, 20);
#elif defined(USE_OPENSSL)
			SHA_CTX ctx;
			hash = malloc(sizeof(unsigned char) * SHA_DIGEST_LENGTH);
			SHA1_Init(&ctx);
			SHA1_Update(&ctx, str, strlen((char *)str));
			if (salt != NULL)
				SHA1_Update(&ctx, salt, strlen((char *)salt));
			SHA1_Final(hash, &ctx);
#else
			struct sha1_ctx ctx;
			hash = malloc(sizeof(unsigned char) * SHA1_DIGEST_SIZE);
			sha1_init_ctx(&ctx);
			sha1_process_bytes(str, strlen((char *)str), &ctx);
			if (salt != NULL)
				sha1_process_bytes(salt, strlen((char *)salt), &ctx);
			sha1_finish_ctx(&ctx, hash);
#endif
			break;
		}
		case IDENTICON_HASH_SHA256: {
#if defined(USE_SODIUM)
			crypto_hash_sha256_state state;
			hash = malloc(sizeof(unsigned char) * crypto_hash_sha256_BYTES);
			crypto_hash_sha256_init(&state);
			crypto_hash_sha256_update(&state, str, strlen((char *)str));
			if (salt != NULL)
				crypto_hash_sha256_update(&state, salt, strlen((char *)salt));
			crypto_hash_sha256_final(&state, hash);
#elif defined(USE_OPENSSL)
			SHA256_CTX ctx;
			hash = malloc(sizeof(unsigned char) * SHA256_DIGEST_LENGTH);
			SHA256_Init(&ctx);
			SHA256_Update(&ctx, str, strlen((char *)str));
			if (salt != NULL)
				SHA256_Update(&ctx, salt, strlen((char *)salt));
			SHA256_Final(hash, &ctx);
#else
			struct sha256_ctx ctx;
			hash = malloc(sizeof(unsigned char) * SHA256_DIGEST_SIZE);
			sha256_init_ctx(&ctx);
			sha256_process_bytes(str, strlen((char *)str), &ctx);
			if (salt != NULL)
				sha256_process_bytes(salt, strlen((char *)salt), &ctx);
			sha256_finish_ctx(&ctx, hash);
#endif
			break;
		}
		case IDENTICON_HASH_SHA512: {
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
			break;
		}
		default: break;
	}

	return hash;
}


/**
 * Draw a rectangular portion of the final image.
 *
 * @param[in,out] img      The image (already allocated).
 * @param[in]     x        The rectangle top-left corner X coordinate.
 * @param[in]     y        The rectangle top-left corner Y coordinate.
 * @param[in]     width    The rectangle width.
 * @param[in]     height   The rectangle height.
 * @param[in]     color    The color used to paint (as RGB).
 * @param[in]     opts     The identicon options.
 * @param[in]     paint_fg True if we are painting the foreground.
 */
static void draw_rectangle(unsigned char *img, uint32_t x, uint32_t y, uint32_t width, uint32_t height,
		identicon_RGB_t color, identicon_options_t *opts, bool paint_fg) {
	uint32_t i, j, idx;

	if ((img == NULL) || (opts->transparent && !paint_fg))
		return;

	if (opts->stroke && paint_fg) {
		if ((x >= opts->stroke_size) && (width <= opts->size - (2 * opts->stroke_size))) {
			x -= opts->stroke_size;
			width += (2 * opts->stroke_size);
		}
		if ((y >= opts->stroke_size) && (height <= opts->size - (2 * opts->stroke_size))) {
			y -= opts->stroke_size;
			height += (2 * opts->stroke_size);
		}
	}

	for (i = 0; i < width; i++) {
		if (i + x >= opts->size) // opts->size intended as image width
			break;

		for (j = 0; j < height; j++) {
			if (j + y >= opts->size) // opts->size intended as image height
				break;

			idx = 4 * opts->size * (j + y); // opts->size intended as image width
			idx += 4 * (i + x);
			img[idx] = color.red;
			img[idx+1] = color.green;
			img[idx+2] = color.blue;
			img[idx+3] = 255;
		}
	}
}


/**
 * Draw the image.
 *
 * @param[in,out] img  The image (already allocated).
 * @param[in]     opts The identicon options.
 */
static void draw_identicon(unsigned char *img, identicon_options_t *opts) {
	int i;
	double h;
	unsigned char c[2];
	unsigned char *hash = NULL;
	double base_margin = floor(opts->size * opts->margin);
	double cell = floor((opts->size - (base_margin * 2)) / 5);
	double margin = floor((opts->size - (cell * 5)) / 2);
	identicon_RGB_t background, foreground;

	if ((img == NULL) || (opts == NULL))
		return;

	hash = checksum((unsigned char *)opts->str, (unsigned char *)opts->salt, opts->hash_type);

	if (hash == NULL)
		return;

	memset(c, 0, 2);

	// Background color
	background.red = 240;
	background.green = 240;
	background.blue = 240;

	if (!opts->transparent)
		draw_rectangle(img, 0, 0, opts->size, opts->size, background, opts, false);

	// Foreground color
	h = (double)hex2int(&hash[strlen((char *)hash) - 7]);
	foreground = hsl2rgb(h / 0xfffffff, 0.5, 0.7);

	// The first 15 characters of the hash control the pixels (even/odd)
	// they are drawn down the middle first, then mirrored outwards
	for (i = 0; i < 15; i++) {
		c[0] = hash[i];

		if ((hex2int(c) % 2) == 0) {
			if (i < 5) {
				draw_rectangle(img, 2 * cell + margin, i * cell + margin, cell, cell, foreground, opts, true);
			} else if (i < 10) {
				draw_rectangle(img, 1 * cell + margin, (i - 5) * cell + margin, cell, cell, foreground, opts, true);
				draw_rectangle(img, 3 * cell + margin, (i - 5) * cell + margin, cell, cell, foreground, opts, true);
			} else if (i < 15) {
				draw_rectangle(img, 0 * cell + margin, (i - 10) * cell + margin, cell, cell, foreground, opts, true);
				draw_rectangle(img, 4 * cell + margin, (i - 10) * cell + margin, cell, cell, foreground, opts, true);
			}
		}
	}
}


/**
 * Create a new set of default options.
 *
 * @return A new variable containing the default options or NULL if an error occurred.
 */
identicon_options_t *new_default_identicon_options() {
	identicon_options_t *opts = malloc(sizeof(identicon_options_t));

	if (opts != NULL) {
		memset(opts->str, 0, IDENTICON_MAX_STRING_LENGTH);
		memset(opts->salt, 0, IDENTICON_MAX_SALT_LENGTH);
		opts->size = 64;
		opts->margin = 0.08;
		opts->transparent = true;
		opts->stroke = true;
		opts->stroke_size = 1;
		opts->hash_type = IDENTICON_HASH_MD5;
	}

	return opts;
}


/**
 * Create a new identicon.
 *
 * @param[in] opts The identicon options.
 *
 * @return A new variable containing the identicon or NULL if an error occurred.
 */
unsigned char *new_identicon(identicon_options_t *opts) {
	unsigned char *img = NULL;

	if (opts == NULL)
		return NULL;

	img = malloc(sizeof(unsigned char) * opts->size * opts->size * 4);
	draw_identicon(img, opts);

	return img;
}
