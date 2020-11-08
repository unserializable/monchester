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

#ifndef __CHESSBRDLIST_H
#define __CHESSBRDLIST_H
#include "features.h"
#include "types.h"

void clear_board_list(struct BoardStateList *);
struct BoardStateList *append_move(struct BoardStateList **, struct EngineMove *, uint8_t, uint8_t);
void print_boardlist_pgn(FILE *, struct BoardStateList *);

#endif
