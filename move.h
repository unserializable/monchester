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

#ifndef __CHESSMOVE_H
#define __CHESSMOVE_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "features.h"
#include "types.h"

int mvs_a(uint8_t (*)[16][30], struct BoardState *, const uint8_t *);
int mvs_p(uint8_t (*)[30], const struct BoardState *, int);
int mvs_b(uint8_t (*)[30], const struct BoardState *, int);
int mvs_n(uint8_t (*)[30], const struct BoardState *, int);
int mvs_r(uint8_t (*)[30], const struct BoardState *, int);
int mvs_q(uint8_t (*)[30], const struct BoardState *, int);
int mvs_k(uint8_t (*)[30], struct BoardState *, int);

int8_t attacked_by(Color, int8_t, const struct BoardState *);
bool is_knight_move(int8_t, int8_t);
bool is_double_check(struct BoardState *);
int8_t exposes(struct BoardState *, const int8_t, const uint8_t, const uint8_t);
int8_t atkexp(struct BoardState *, const int8_t, const uint8_t, const uint8_t);

int check_or_stalemate(struct BoardState *);

void init_board(struct BoardState *);
struct MoveInfo Move(struct BoardState *, uint8_t, uint8_t);
void undo_move(struct BoardState *, const struct MoveInfo *);
int validate_move(struct BoardState *, const struct MoveCoords *);
int is_check(const struct BoardState *, Color);
int checkmate(struct BoardState *);
int stalemate(struct BoardState *);
bool same_position(const struct BoardState *, const struct BoardState *);
int repetition(const struct BoardStateList *);
bool insufficient_material(const struct BoardState *);
int8_t cardinal8(int8_t, int8_t);
#endif
