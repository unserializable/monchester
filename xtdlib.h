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

#ifndef __CHESS_XTDLIB_H
#define __CHESS_XTDLIB_H
#include <time.h>
#include <stdint.h>
#include "features.h"
#include "types.h"

long double seconds(clock_t);
uintmax_t ns_to_nps(uintmax_t, long double);
int32_t centipawn_score(int32_t);
int32_t cecp_score(int32_t);
char *program_name_and_version(void);
char piece2SAN(char);
intmax_t to_int(const char *const, intmax_t, intmax_t, bool die, const char *const);
struct MoveCoords *parsed_move(const char *s);
void to_algebraic(char *, struct EngineMove *, struct BoardState *);

void *xmalloc(int);
void *xcalloc(int, int);
void *xrealloc(void *, int);
void xfree(void *);
void xfree_engine_move(struct EngineMove *);

char *xstrdup(const char *);

PV *pv_init(PV*);
void pv_free(PV *pv);
void pv_push(PV*, uint8_t, uint8_t);
void pv_remove(PV*);
void pv_rewrite(PV*, uint8_t, uint8_t *, uint8_t);

void store(Flack *, uint8_t *, int);
#endif
