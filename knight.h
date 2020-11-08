/*
	Monchester, chess engine for CECP interfaces and console.
	Copyright (C) 2020 Taimo Peelo

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 3 as
	published by the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see:
	<https://www.gnu.org/licenses/gpl-3.0.html>.
*/

#ifndef __CHESSKNIGHT_H
#define __CHESSKNIGHT_H

#include <stdint.h>

#define _CK_UCS (uint8_t *)

const uint8_t *mKnight[] = {
	_CK_UCS "\x02\x0a\x11",                                 /* a1 -> c2 b3   */
	_CK_UCS "\x03\x12\x0b\x10",                             /* b1 -> c3 d2 a3   */
	_CK_UCS "\x04\x13\x11\x0c\x08",                         /* c1 -> d3 b3 e2 a2   */
	_CK_UCS "\x04\x12\x14\x0d\x09",                         /* d1 -> c3 e3 f2 b2   */
	_CK_UCS "\x04\x13\x15\x0a\x0e",                         /* e1 -> d3 f3 c2 g2   */
	_CK_UCS "\x04\x14\x0b\x16\x0f",                         /* f1 -> e3 d2 g3 h2   */
	_CK_UCS "\x03\x15\x0c\x17",                             /* g1 -> f3 e2 h3   */
	_CK_UCS "\x02\x0d\x16",                                 /* h1 -> f2 g3   */
	_CK_UCS "\x03\x12\x19\x02",                             /* a2 -> c3 b4 c1   */
	_CK_UCS "\x04\x13\x1a\x18\x03",                         /* b2 -> d3 c4 a4 d1   */
	_CK_UCS "\x06\x14\x1b\x19\x10\x04\x00",                 /* c2 -> e3 d4 b4 a3 e1 a1   */
	_CK_UCS "\x06\x15\x1a\x1c\x11\x05\x01",                 /* d2 -> f3 c4 e4 b3 f1 b1   */
	_CK_UCS "\x06\x12\x1b\x1d\x16\x02\x06",                 /* e2 -> c3 d4 f4 g3 c1 g1   */
	_CK_UCS "\x06\x13\x1c\x1e\x03\x17\x07",                 /* f2 -> d3 e4 g4 d1 h3 h1   */
	_CK_UCS "\x04\x14\x1d\x04\x1f",                         /* g2 -> e3 f4 e1 h4   */
	_CK_UCS "\x03\x15\x1e\x05",                             /* h2 -> f3 g4 f1   */
	_CK_UCS "\x04\x1a\x0a\x21\x01",                         /* a3 -> c4 c2 b5 b1   */
	_CK_UCS "\x06\x1b\x22\x0b\x20\x02\x00",                 /* b3 -> d4 c5 d2 a5 c1 a1   */
	_CK_UCS "\x08\x1c\x23\x21\x0c\x18\x03\x08\x01",         /* c3 -> e4 d5 b5 e2 a4 d1 a2 b1   */
	_CK_UCS "\x08\x1d\x22\x24\x19\x0d\x09\x02\x04",         /* d3 -> f4 c5 e5 b4 f2 b2 c1 e1   */
	_CK_UCS "\x08\x1a\x23\x25\x0a\x1e\x03\x0e\x05",         /* e3 -> c4 d5 f5 c2 g4 d1 g2 f1   */
	_CK_UCS "\x08\x1b\x24\x0b\x26\x04\x1f\x06\x0f",         /* f3 -> d4 e5 d2 g5 e1 h4 g1 h2   */
	_CK_UCS "\x06\x1c\x25\x0c\x05\x27\x07",                 /* g3 -> e4 f5 e2 f1 h5 h1   */
	_CK_UCS "\x04\x1d\x0d\x26\x06",                         /* h3 -> f4 f2 g5 g1   */
	_CK_UCS "\x04\x12\x22\x29\x09",                         /* a4 -> c3 c5 b6 b2   */
	_CK_UCS "\x06\x13\x23\x2a\x0a\x28\x08",                 /* b4 -> d3 d5 c6 c2 a6 a2   */
	_CK_UCS "\x08\x14\x24\x2b\x29\x0b\x10\x09\x20",         /* c4 -> e3 e5 d6 b6 d2 a3 b2 a5   */
	_CK_UCS "\x08\x15\x25\x2a\x2c\x21\x0c\x11\x0a",         /* d4 -> f3 f5 c6 e6 b5 e2 b3 c2   */
	_CK_UCS "\x08\x12\x22\x2b\x2d\x0d\x26\x0b\x16",         /* e4 -> c3 c5 d6 f6 f2 g5 d2 g3   */
	_CK_UCS "\x08\x13\x23\x2c\x0c\x2e\x27\x17\x0e",         /* f4 -> d3 d5 e6 e2 g6 h5 h3 g2   */
	_CK_UCS "\x06\x14\x24\x2d\x0d\x2f\x0f",                 /* g4 -> e3 e5 f6 f2 h6 h2   */
	_CK_UCS "\x04\x15\x25\x2e\x0e",                         /* h4 -> f3 f5 g6 g2   */
	_CK_UCS "\x04\x1a\x2a\x11\x31",                         /* a5 -> c4 c6 b3 b7   */
	_CK_UCS "\x06\x12\x1b\x2b\x32\x10\x30",                 /* b5 -> c3 d4 d6 c7 a3 a7   */
	_CK_UCS "\x08\x13\x1c\x2c\x11\x33\x18\x31\x28",         /* c5 -> d3 e4 e6 b3 d7 a4 b7 a6   */
	_CK_UCS "\x08\x12\x14\x1d\x2d\x29\x19\x32\x34",         /* d5 -> c3 e3 f4 f6 b6 b4 c7 e7   */
	_CK_UCS "\x08\x13\x15\x1a\x2a\x1e\x2e\x33\x35",         /* e5 -> d3 f3 c4 c6 g4 g6 d7 f7   */
	_CK_UCS "\x08\x14\x1b\x2b\x16\x34\x2f\x1f\x36",         /* f5 -> e3 d4 d6 g3 e7 h6 h4 g7   */
	_CK_UCS "\x06\x15\x1c\x2c\x35\x17\x37",                 /* g5 -> f3 e4 e6 f7 h3 h7   */
	_CK_UCS "\x04\x1d\x2d\x16\x36",                         /* h5 -> f4 f6 g3 g7   */
	_CK_UCS "\x04\x22\x19\x32\x39",                         /* a6 -> c5 b4 c7 b8   */
	_CK_UCS "\x06\x1a\x23\x33\x18\x3a\x38",                 /* b6 -> c4 d5 d7 a4 c8 a8   */
	_CK_UCS "\x08\x1b\x24\x19\x34\x20\x3b\x39\x30",         /* c6 -> d4 e5 b4 e7 a5 d8 b8 a7   */
	_CK_UCS "\x08\x1a\x1c\x25\x21\x35\x31\x3a\x3c",         /* d6 -> c4 e4 f5 b5 f7 b7 c8 e8   */
	_CK_UCS "\x08\x1b\x1d\x22\x26\x32\x36\x3b\x3d",         /* e6 -> d4 f4 c5 g5 c7 g7 d8 f8   */
	_CK_UCS "\x08\x1c\x23\x1e\x33\x27\x3c\x37\x3e",         /* f6 -> e4 d5 g4 d7 h5 e8 h7 g8   */
	_CK_UCS "\x06\x1d\x24\x34\x1f\x3d\x3f",                 /* g6 -> f4 e5 e7 h4 f8 h8   */
	_CK_UCS "\x04\x25\x1e\x35\x3e",                         /* h6 -> f5 g4 f7 g8   */
	_CK_UCS "\x03\x2a\x21\x3a",                             /* a7 -> c6 b5 c8   */
	_CK_UCS "\x04\x22\x2b\x20\x3b",                         /* b7 -> c5 d6 a5 d8   */
	_CK_UCS "\x06\x23\x2c\x21\x28\x3c\x38",                 /* c7 -> d5 e6 b5 a6 e8 a8   */
	_CK_UCS "\x06\x22\x24\x2d\x29\x3d\x39",                 /* d7 -> c5 e5 f6 b6 f8 b8   */
	_CK_UCS "\x06\x23\x25\x2a\x2e\x3a\x3e",                 /* e7 -> d5 f5 c6 g6 c8 g8   */
	_CK_UCS "\x06\x24\x2b\x26\x2f\x3b\x3f",                 /* f7 -> e5 d6 g5 h6 d8 h8   */
	_CK_UCS "\x04\x25\x2c\x27\x3c",                         /* g7 -> f5 e6 h5 e8   */
	_CK_UCS "\x03\x2d\x26\x3d",                             /* h7 -> f6 g5 f8   */
	_CK_UCS "\x02\x29\x32",                                 /* a8 -> b6 c7   */
	_CK_UCS "\x03\x2a\x33\x28",                             /* b8 -> c6 d7 a6   */
	_CK_UCS "\x04\x2b\x29\x34\x30",                         /* c8 -> d6 b6 e7 a7   */
	_CK_UCS "\x04\x2a\x2c\x35\x31",                         /* d8 -> c6 e6 f7 b7   */
	_CK_UCS "\x04\x2b\x2d\x32\x36",                         /* e8 -> d6 f6 c7 g7   */
	_CK_UCS "\x04\x2c\x2e\x33\x37",                         /* f8 -> e6 g6 d7 h7   */
	_CK_UCS "\x03\x2d\x34\x2f",                             /* g8 -> f6 e7 h6   */
	_CK_UCS "\x02\x2e\x35"                                  /* h8 -> g6 f7   */
};

#undef _CK_UCS

#endif
