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

#ifndef __CHESSBISHOP_H
#define __CHESSBISHOP_H

#include <stdint.h>

const uint8_t vBishop[512] = {
    0, 0, 0, 0, 0, 0, 0, 7, 0, 1, 0, 0, 0, 0, 0, 6,
    0, 2, 0, 0, 0, 0, 0, 5, 0, 3, 0, 0, 0, 0, 0, 4,
    0, 4, 0, 0, 0, 0, 0, 3, 0, 5, 0, 0, 0, 0, 0, 2,
    0, 6, 0, 0, 0, 0, 0, 1, 0, 7, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 1, 0, 6, 0, 1, 0, 1, 0, 1, 0, 6,
    0, 2, 0, 1, 0, 1, 0, 5, 0, 3, 0, 1, 0, 1, 0, 4,
    0, 4, 0, 1, 0, 1, 0, 3, 0, 5, 0, 1, 0, 1, 0, 2,
    0, 6, 0, 1, 0, 1, 0, 1, 0, 6, 0, 1, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 2, 0, 5, 0, 1, 0, 1, 0, 2, 0, 5,
    0, 2, 0, 2, 0, 2, 0, 5, 0, 3, 0, 2, 0, 2, 0, 4,
    0, 4, 0, 2, 0, 2, 0, 3, 0, 5, 0, 2, 0, 2, 0, 2,
    0, 5, 0, 2, 0, 1, 0, 1, 0, 5, 0, 2, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 3, 0, 4, 0, 1, 0, 1, 0, 3, 0, 4,
    0, 2, 0, 2, 0, 3, 0, 4, 0, 3, 0, 3, 0, 3, 0, 4,
    0, 4, 0, 3, 0, 3, 0, 3, 0, 4, 0, 3, 0, 2, 0, 2,
    0, 4, 0, 3, 0, 1, 0, 1, 0, 4, 0, 3, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 4, 0, 3, 0, 1, 0, 1, 0, 4, 0, 3,
    0, 2, 0, 2, 0, 4, 0, 3, 0, 3, 0, 3, 0, 4, 0, 3,
    0, 3, 0, 4, 0, 3, 0, 3, 0, 3, 0, 4, 0, 2, 0, 2,
    0, 3, 0, 4, 0, 1, 0, 1, 0, 3, 0, 4, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 5, 0, 2, 0, 1, 0, 1, 0, 5, 0, 2,
    0, 2, 0, 2, 0, 5, 0, 2, 0, 2, 0, 3, 0, 4, 0, 2,
    0, 2, 0, 4, 0, 3, 0, 2, 0, 2, 0, 5, 0, 2, 0, 2,
    0, 2, 0, 5, 0, 1, 0, 1, 0, 2, 0, 5, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 6, 0, 1, 0, 1, 0, 1, 0, 6, 0, 1,
    0, 1, 0, 2, 0, 5, 0, 1, 0, 1, 0, 3, 0, 4, 0, 1,
    0, 1, 0, 4, 0, 3, 0, 1, 0, 1, 0, 5, 0, 2, 0, 1,
    0, 1, 0, 6, 0, 1, 0, 1, 0, 1, 0, 6, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 1, 0, 6, 0, 0,
    0, 0, 0, 2, 0, 5, 0, 0, 0, 0, 0, 3, 0, 4, 0, 0,
    0, 0, 0, 4, 0, 3, 0, 0, 0, 0, 0, 5, 0, 2, 0, 0,
    0, 0, 0, 6, 0, 1, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0
};
#endif
