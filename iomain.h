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

#ifndef __CHESSIOMAIN_H
#define __CHESSIOMAIN_H

#include <stdio.h> 	/* for FILE */
#include <stdint.h>
#include <time.h>
#include <inttypes.h>
#include "features.h"
#include "types.h"

void init_players(void);
void print_pgn(FILE *, struct BoardStateList *, int, const struct tm *, const struct PlayerInfo *, const struct PlayerInfo *);
void print_board(FILE *, const struct BoardState *);
void print_cmd_error(const char *, const char *);  /* #109, GH#10 */
void print_pv(FILE *, const uint8_t *, int, int);
void cecp_print_pv(const PV *, int32_t, clock_t);
char *getln(FILE *);
void print_help(void);

#endif
