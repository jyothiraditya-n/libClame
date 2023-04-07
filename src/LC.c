/* libClame: Command-line Arguments Made Easy
 * Copyright (C) 2021-2023 Jyothiraditya Nellakra
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>. */

#include <inttypes.h>

#include <LC.h>

/* We go ahead and define the endianness-detection function even if it's only
 * part of the public-facing API on systems where the endianness couldn't be
 * detected using compliler flags. */

int LC_endianness() {
	/* We want an integer greater than 1-byte long where one of the bytes
	 * will have 1s and the other will have all zeros. */
	static const uint16_t integer = 0xFF00;

	/* We trick C into letting us peer into that integer using a view
	 * that's only one byte long. */
	static const uint8_t *view = (uint8_t *) &integer;

	/* If the first byte in the view has 1s, then we're big endian,
	 * since that means the most significant bits were stored first. If it
	 * doesn't, then that means we're little endian, since the least
	 * significant bits were stored first. */
	return *view? LC_BIG_ENDIAN: LC_LITTLE_ENDIAN;

	/* If the compiler is clever, this should all be folded into a single
	 * compile-time constant lol. */
}