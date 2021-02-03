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

#ifndef __CHESSGLOBALS_H
#define __CHESSGLOBALS_H

#define _CQ(x) #x
#define CQUOTE(x) _CQ(x)

#ifndef MONCHESTER_VERSION
 #define PROGRAM_VERSION "1.0.1-10-ga46d14d"
#else
 #define PROGRAM_VERSION CQUOTE(MONCHESTER_VERSION)
#endif

/* This is for optionally defining version suffix for experimental compiles. */
#ifndef EXTRA_VERSION
 #define PROGRAM_XVERSION ""
#else
 #define PROGRAM_XVERSION "-" CQUOTE(EXTRA_VERSION)
#endif

#define PROGRAM_FULL_VERSION PROGRAM_VERSION PROGRAM_XVERSION

#include <stdint.h>
#include "features.h"
#include "types.h"

#define CHMINSTD_NXT(x) (x = (((uint64_t)(x) * UINT32_C(48271)) % UINT32_C(0x7fffffff)))
#define CHMINSTD_NXT_BITS(x,b) ((CHMINSTD_NXT(x)) >> (31 - (b)))

extern const uint8_t *mKnight[];

extern const char *PROGRAM_NAME;
extern const char *SOURCE_REPO_URL;

#if FEATURE_KEEP_GAMESCORES
extern const char *GAMESCORE_FILE;
#endif

extern uintmax_t g_nodecount;
extern uintmax_t g_engine_nps;

extern bool g_cecp;

extern struct EngineSettings g_engine_defaults;
extern struct EngineSettings g_engine_conf;

extern struct CecpSettings cecp_defaults;
extern struct CecpSettings g_cecp_conf;
extern struct BoardStateList *CurrentBoard;
extern struct PlayerInfo g_players[];
extern struct TimeControl g_game_time;

/* piece values */
extern const int32_t MATVAL[];

/* directions cardinal & intercardinal */
extern const struct { uint8_t N, NW, W, SW, S, SE, E, NE; }
	CARDINAL;

/* Coordinate addends for moving in (inter)cardinal direction. */
extern const int8_t CARDINAL_ADDENDS[];
	
extern const char *BOARD_FILES[];
extern const char *BOARD_RANKS[];
extern const char *SQUARES[];

extern int g_gamestate;
extern int g_outcome;

extern const char *COMPILER_TEXT;
extern const char *CONVERSION_FAILED_TEXT;
extern const char *UNKNOWN_COMMAND_TEXT;
extern const char *ILLEGAL_MOVE_TEXT;

extern const char *COLOR_TEXT[];
extern const char *RESULT_DRAW_SCORE_TEXT;
extern const char *RESULT_DECISIVE_SCORE_TEXT[];

extern const char *INSUFFICIENT_MATERIAL_TEXT;
extern const char *REPETITION_DRAW_TEXT;
extern const char *ILLEGAL_POSITION_TEXT;
#endif
