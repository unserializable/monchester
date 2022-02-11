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

#ifndef __CHESSCOMPMOVE_H
#define __CHESSCOMPMOVE_H

#include <stdint.h>
#include "features.h"
#include "types.h"

struct EngineMove *select_move(struct BoardStateList *bsl, uint8_t, const struct TimeControl *, bool);

#endif
