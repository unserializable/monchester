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

#include <stdlib.h>
#include <stdint.h>
#include "features.h"
#include "types.h"

const char *PROGRAM_NAME = "Monchester";
const char *SOURCE_REPO_URL = "https://github.com/unserializable/monchester";

#if FEATURE_KEEP_GAMESCORES
const char *GAMESCORE_FILE = ".monchester.pgn";
#endif

/* /Effective/ ply depths are one greater than numbers here, 0-based. */
#define MONCHESTER_DEPTH_DEFAULT 3
/* Maximum effective ply depth for engine configuration initialization. This does
   currently not have any effect besides that when CECP 'sd' decreases maximum
   search depth to be lower than default, both default and maximum values search
   depth values are adjusted correspondingly, until the new game begins. */
#define MONCHESTER_DEPTH_MAX 7

/* This is set to true when CECP/XBoard protocol is activated. */
bool g_cecp = false;

/* These keep track of nodecount in current search & engine's average speed. */
uintmax_t g_nodecount = 0;
uintmax_t g_engine_nps = 0;

/* Global game state with current and previous boards, players and time control. */
struct BoardStateList *CurrentBoard = NULL;
struct PlayerInfo g_players[2] = { {White, Computer, NULL}, {Black, Human, NULL}};
struct TimeControl g_game_time = { .time_left = 0 };

#define CECP_DEFAULT_RANDOMIZE false
#define CECP_DEFAULT_PONDER false
#define CECP_DEFAULT_OUTPUT_THINKING false
#define CECP_DEFAULT_MINSTD 0
#define CECP_DEFAULT_OPP_COMPUTER false
#define CECP_DEFAULT_OPP_NAME NULL

/* Engine configuration defaults, for restoration to initial values on request. */
struct EngineSettings g_engine_defaults = {
	.depth_default = MONCHESTER_DEPTH_DEFAULT,
	.depth_max = MONCHESTER_DEPTH_MAX
};

/* Effective engine configuration. */
struct EngineSettings g_engine_conf = {
	.depth_default = MONCHESTER_DEPTH_DEFAULT,
	.depth_max = MONCHESTER_DEPTH_MAX
};

/* CECP configuration defaults, for restoration on request. */
struct CecpSettings cecp_defaults = {
	.randomize_moves = CECP_DEFAULT_RANDOMIZE,
	.ponder = CECP_DEFAULT_PONDER,
	.output_thinking = CECP_DEFAULT_OUTPUT_THINKING,

	.minstd = CECP_DEFAULT_MINSTD,
	.minstd_last = CECP_DEFAULT_MINSTD,

	.opp_computer = CECP_DEFAULT_OPP_COMPUTER,
	.opp_name = CECP_DEFAULT_OPP_NAME
};

/* Effective CECP configuration. */
struct CecpSettings g_cecp_conf = {
	.randomize_moves = CECP_DEFAULT_RANDOMIZE,
	.ponder = CECP_DEFAULT_PONDER,
	.output_thinking = CECP_DEFAULT_OUTPUT_THINKING,

	.minstd = CECP_DEFAULT_MINSTD,
	.minstd_last = CECP_DEFAULT_MINSTD,

	.opp_computer = CECP_DEFAULT_OPP_COMPUTER,
	.opp_name = CECP_DEFAULT_OPP_NAME
};

/* Piece values: PLACEHOLDER, P, B, N, R, Q, K. */
const int32_t MATVAL[7] = { 0, 35, 280, 336, 896, 1498, 30016 };

/* directions cardinal & intercardinal */
const struct { uint8_t N, NW, W, SW, S, SE, E, NE; }
	CARDINAL = {
		0, 1, 2, 3, 4, 5, 6, 7
	};

/* Coordinate addends for moving in (inter)cardinal directions. */
const int8_t CARDINAL_ADDENDS[] = { 8, 7, -1, -9, -8, -7, 1, 9 };

const char *BOARD_FILES[] = {
	"a", "b", "c", "d", "e", "f", "g", "h"
};

const char *BOARD_RANKS[] = {
	"1", "2", "3", "4", "5", "6", "7", "8"
};

const char *SQUARES[] = {
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

int g_gamestate = NO_GAME;
int g_outcome;		/* game outcome */

/* STRING CONSTANTS #69 */
const char *COMPILER_TEXT = "Compiler";
const char *CONVERSION_FAILED_TEXT = "Could not convert";
const char *UNKNOWN_COMMAND_TEXT = "Unrecognized command";
const char *ILLEGAL_MOVE_TEXT = "Illegal move";

const char *COLOR_TEXT[] = {
	"White", "Black"
};

/* Score texts used in PGN and official notation. */
const char *RESULT_DRAW_SCORE_TEXT = "1/2-1/2";
const char *RESULT_DECISIVE_SCORE_TEXT[] = {
	"1-0", "0-1"
};

const char *INSUFFICIENT_MATERIAL_TEXT = "Insufficient mating material";
const char *REPETITION_DRAW_TEXT = "Draw by repetition";
const char *ILLEGAL_POSITION_TEXT = "Illegal position";
