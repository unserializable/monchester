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

#ifndef __CHESSQUEEN_H
#define __CHESSQUEEN_H

#include <stdint.h>

const uint8_t vQueen[512] = {
    7, 0, 0, 0, 0, 0, 7, 7, 7, 1, 1, 0, 0, 0, 6, 6,
    7, 2, 2, 0, 0, 0, 5, 5, 7, 3, 3, 0, 0, 0, 4, 4,
    7, 4, 4, 0, 0, 0, 3, 3, 7, 5, 5, 0, 0, 0, 2, 2,
    7, 6, 6, 0, 0, 0, 1, 1, 7, 7, 7, 0, 0, 0, 0, 0,

    6, 0, 0, 0, 1, 1, 7, 6, 6, 1, 1, 1, 1, 1, 6, 6,
    6, 2, 2, 1, 1, 1, 5, 5, 6, 3, 3, 1, 1, 1, 4, 4,
    6, 4, 4, 1, 1, 1, 3, 3, 6, 5, 5, 1, 1, 1, 2, 2,
    6, 6, 6, 1, 1, 1, 1, 1, 6, 6, 7, 1, 1, 0, 0, 0,

    5, 0, 0, 0, 2, 2, 7, 5, 5, 1, 1, 1, 2, 2, 6, 5,
    5, 2, 2, 2, 2, 2, 5, 5, 5, 3, 3, 2, 2, 2, 4, 4,
    5, 4, 4, 2, 2, 2, 3, 3, 5, 5, 5, 2, 2, 2, 2, 2,
    5, 5, 6, 2, 2, 1, 1, 1, 5, 5, 7, 2, 2, 0, 0, 0,

    4, 0, 0, 0, 3, 3, 7, 4, 4, 1, 1, 1, 3, 3, 6, 4,
    4, 2, 2, 2, 3, 3, 5, 4, 4, 3, 3, 3, 3, 3, 4, 4,
    4, 4, 4, 3, 3, 3, 3, 3, 4, 4, 5, 3, 3, 2, 2, 2,
    4, 4, 6, 3, 3, 1, 1, 1, 4, 4, 7, 3, 3, 0, 0, 0,

    3, 0, 0, 0, 4, 4, 7, 3, 3, 1, 1, 1, 4, 4, 6, 3,
    3, 2, 2, 2, 4, 4, 5, 3, 3, 3, 3, 3, 4, 4, 4, 3,
    3, 3, 4, 4, 4, 3, 3, 3, 3, 3, 5, 4, 4, 2, 2, 2,
    3, 3, 6, 4, 4, 1, 1, 1, 3, 3, 7, 4, 4, 0, 0, 0,

    2, 0, 0, 0, 5, 5, 7, 2, 2, 1, 1, 1, 5, 5, 6, 2,
    2, 2, 2, 2, 5, 5, 5, 2, 2, 2, 3, 3, 5, 4, 4, 2,
    2, 2, 4, 4, 5, 3, 3, 2, 2, 2, 5, 5, 5, 2, 2, 2,
    2, 2, 6, 5, 5, 1, 1, 1, 2, 2, 7, 5, 5, 0, 0, 0,

    1, 0, 0, 0, 6, 6, 7, 1, 1, 1, 1, 1, 6, 6, 6, 1,
    1, 1, 2, 2, 6, 5, 5, 1, 1, 1, 3, 3, 6, 4, 4, 1,
    1, 1, 4, 4, 6, 3, 3, 1, 1, 1, 5, 5, 6, 2, 2, 1,
    1, 1, 6, 6, 6, 1, 1, 1, 1, 1, 7, 6, 6, 0, 0, 0,

    0, 0, 0, 0, 7, 7, 7, 0, 0, 0, 1, 1, 7, 6, 6, 0,
    0, 0, 2, 2, 7, 5, 5, 0, 0, 0, 3, 3, 7, 4, 4, 0,
    0, 0, 4, 4, 7, 3, 3, 0, 0, 0, 5, 5, 7, 2, 2, 0,
    0, 0, 6, 6, 7, 1, 1, 0, 0, 0, 7, 7, 7, 0, 0, 0
};
#endif
