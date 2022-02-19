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

#include "features.h"
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#if FEATURE_KEEP_GAMESCORES
  #include <sys/file.h>
#endif /* FEATURE_KEEP_GAMESCORES */
#include "xtdlib.h"
#include "fen.h"
#include "types.h"
#include "globals.h"
#include "iomain.h"
#include "move.h"
#include "brdlist.h"
#include "compmove.h"

clock_t bench(struct BoardState benchboard, uint8_t depth) {
	struct EngineMove* throwaway;
	clock_t clock_start, clock_end;
	struct BoardStateList bsl = { .State = benchboard };

	clock_start = clock();
	throwaway = select_move(&bsl, depth, NULL, false);
	clock_end = clock();

	xfree_engine_move(throwaway);

	return (clock_end - clock_start);
}

void init_engine_nps(void) {
	uint8_t i, benchdepth = 3, iterations = 5;
	uintmax_t npssum = 0;
	struct BoardState bs;
	init_board(&bs);
#if DEBUG_EVAL
	iterations = 1; /* do just 1 iteration to get less debug mess on-screen */
#endif /* DEBUG_EVAL */

	for (i = 0; i < iterations; i++) {
		g_nodecount = 0;
		clock_t dc = bench(bs, benchdepth);
		npssum += ns_to_nps(g_nodecount, seconds(dc));
	}

	g_engine_nps = npssum / iterations;
	g_nodecount = 0;
}

uint32_t random_seed(void) {
	struct timeval tv;
	uint32_t ms;
	do {
		gettimeofday(&tv, NULL);
		ms = tv.tv_sec * UINT32_C(1000) + (tv.tv_usec / UINT32_C(1000));
	} while (!ms);
	return ms;
}

#define OPT_PREFIX "--"
#define OPT_PREFIX_LEN (sizeof(OPT_PREFIX)-1)
#define OPT_END OPT_PREFIX
#define IS_OPT(x) (!strncmp((x), OPT_PREFIX, OPT_PREFIX_LEN))
#define IS_OPT_END(x) (!strcmp((x), OPT_END))
void process_cmdline(int ca, const char *args[]) {
	if (ca == 1)
		return;

	int ia = 1;

	int o_unknown = 0;
	int	o_help = 0, o_version = 0, o_bench = 0;
	intmax_t bench_depth = 4;
	char *bench_fen = NULL;
	bool eo = (ia == ca) || IS_OPT_END(args[ia]) || !IS_OPT(args[ia]);
	while (!eo) {
		const char *opt = args[ia];

		if (!strcmp(opt, OPT_PREFIX "help")) {
			o_help = ia;
		} else if (!strcmp(opt, OPT_PREFIX "version")) {
			o_version = ia;
		} else if (!strcmp(opt, OPT_PREFIX "bench")) {
			o_bench = ia;
			if (ca > ia + 1) { /* depth? */
				if (!IS_OPT(args[ia + 1])) {
					bench_depth = to_int(args[ia + 1], 1, 16, true, false, NULL);
					bench_depth--;
					ia++;

					if (ca > ia + 1) { /* position? */
						if (!IS_OPT(args[ia + 1])) {
							struct BoardState *bs = FEN2Board(args[ia + 1]);
							if (bs) {
								bench_fen = xstrdup(args[ia + 1]);
								xfree(bs);
							} else {
								fprintf(stderr, "%s FEN '%s'.\n", CONVERSION_FAILED_TEXT, args[ia + 1]);
								exit(EINVAL);
							}
							ia++;
						}
					}
				}
			}
		} else if (!o_unknown) {
			o_unknown = ia;
		}
		ia++;
		eo = (ia == ca) || IS_OPT_END(args[ia]) || !IS_OPT(args[ia]);
	}

	if (o_version) {
		printf("%s %s\n", PROGRAM_NAME, PROGRAM_FULL_VERSION);
		fputs("Copyright (C) 2020 Taimo Peelo\n", stdout);
		fputs("License GPLv3: GNU GPL version 3 <https://www.gnu.org/licenses/gpl-3.0.html>\n", stdout);
		printf("Source repository: \n%*s%s\n", 2, "", SOURCE_REPO_URL);
		fputs("\nBuild information:\n", stdout);

		/* https://sourceforge.net/p/predef/wiki/Compilers/ */
		printf(
			"%*s%s : %s\n",
			2, "", COMPILER_TEXT,
#if defined(__clang__) && defined(__clang_major__)
	"clang "__clang_version__
#elif defined(__GNUC__) && defined(__GNUC_MINOR__) && defined (__GNUC_PATCHLEVEL__)
	"gcc "__VERSION__
#elif defined(__MINGW32__) || defined(__MINGW64__)
	#if defined(__MINGW32__)
		"MinGW32 " "__MINGW32_MAJOR_VERSION" "__MINGW32_MINOR_VERSION"
	#else
		"MinGW64 " "__MINGW64_VERSION_MAJOR" "__MINGW64_VERSION_MINOR"
	#endif
#else
	"unrecognized"
#endif
		);
		printf("%*sTime%*s: %s %s\n", 2, "", 5, "", __DATE__, __TIME__);

		exit(0);
	}

	if (o_help) {
		printf("%s %s\n", PROGRAM_NAME, PROGRAM_FULL_VERSION);
		printf("Usage: %c%s [flags]\n\n", tolower(PROGRAM_NAME[0]), PROGRAM_NAME + 1);
		printf("Optional [flags] are:\n");
		printf("\n%*s--bench [depth] [FEN]\n", 2, "");
		printf("%*sPerforms benchmark & outputs info (depth:time:nodecount:kN/s:version:FEN).\n", 4, "");
		printf("\n%*s--version\n", 2, "");
		printf("%*sOutputs more info about program version.\n", 4, "");
		printf("\n%*s--help\n", 2, "");
		printf("%*sOutputs program invocation info.\n", 4, "");
		printf("\nReport bugs at: %s/issues\n", SOURCE_REPO_URL);
		exit(0);
	}

	if (o_unknown || (ia < ca)) {
		int mix = o_unknown ? o_unknown : ia;
		printf("Misunderstood '%s'\n", args[mix]);
		exit(EINVAL);
	}

	if (o_bench) { /* TODO: move this somewhere nice */
		struct BoardState bb;
		if (bench_fen) {
			struct BoardState *pbb = FEN2Board(bench_fen);
			bb = *pbb;
			xfree(bench_fen);
			xfree(pbb);
		} else {
			init_board(&bb);
		}

		clock_t clock_delta = bench(bb, bench_depth);
		long double secs = seconds(clock_delta);
		uintmax_t knps = ns_to_nps(g_nodecount, secs) / 1000;
		char *bfen = Board2FEN(&bb);
		printf(
			"%jd:%.3Lf:%ju:%ju:%s:%s\n",
			bench_depth+1, secs, g_nodecount, knps, PROGRAM_FULL_VERSION, bfen
		);
		xfree(bfen);

		exit(0);
	}
}
#undef IS_OPT_END
#undef IS_OPT
#undef OPT_END
#undef OPT_PREFIX
#undef OPT_PREFIX_LEN

/* Reaction with feature adverts to CECP protocol 'protover' announcement. */
void command_protover_cecp(void)
{
	/* We are not using protocol version for anything special. */
	printf(
		"feature "
		"myname=\"%s %s\" "
		"name=1 "
		"setboard=1 "
		"ping=1 "
		"debug=1 " /* Safeguard from legacy like 'draw' interpretations anywhere in line (#113, GH#16). */
		"edit=0 "
		"memory=0 "
		"usermove=0 "
		"analyze=0 "
		"colors=0 "
		"sigint=0 "
		"sigterm=0 "
		"done=1\n",
		PROGRAM_NAME, PROGRAM_FULL_VERSION
	);
	fflush(stdout);
}

/* Reaction upon receiving CECP 'result RESULT {COMMENT}' for ongoing game. */
void command_result_cecp(const char *cmd)
{
	assert(!strncmp(cmd, "result", 6));

	if (g_gamestate != GAME_IN_PROGRESS) {
		print_cmd_error(cmd, NO_GAME_TEXT);
		return;
	}

	size_t cmd_len = strlen(cmd);
	if (*(cmd + 6) != ' ')
		goto bad_result_cmd;

	const char *cmd_end = cmd + cmd_len;
	const char *cecp_result_start = cmd + 7;

	if (cecp_result_start >= cmd_end)
		goto bad_result_cmd;

	if (!strncmp(cecp_result_start, RESULT_DECISIVE_SCORE_TEXT[White], strlen(RESULT_DECISIVE_SCORE_TEXT[White]))) {
		g_outcome = BLACKLOSE;
	} else if (!strncmp(cecp_result_start, RESULT_DECISIVE_SCORE_TEXT[Black], strlen(RESULT_DECISIVE_SCORE_TEXT[Black]))) {
		g_outcome = WHITELOSE;
	} else if (!strncmp(cecp_result_start, RESULT_DRAW_SCORE_TEXT, strlen(RESULT_DRAW_SCORE_TEXT))) {
		g_outcome = DRAW;
	} else if (!strncmp(cecp_result_start, RESULT_UNFINISHED_SCORE_TEXT, strlen(RESULT_UNFINISHED_SCORE_TEXT))) {
		g_outcome = UNFINISHED;
	} else {
		goto bad_result_cmd;
	}

	/* CECP comment of the result is not used for anything ATM. */
	g_gamestate = GAME_ENDED;
	return;

bad_result_cmd:
	print_cmd_error(cmd, *(cmd + 6) == ' ' || cmd_len == 6 ? BAD_FORMAT_TEXT : UNKNOWN_COMMAND_TEXT);
}

/* Set up a new game from standard start position. */
void command_new(void)
{
	/* clean up the board list */
	clear_board_list(CurrentBoard);
	if (g_players[0].name)
		xfree(g_players[0].name);
	if (g_players[1].name)
		xfree(g_players[1].name);

	CurrentBoard = (struct BoardStateList *) xcalloc(1, sizeof(struct BoardStateList));
	init_board(&CurrentBoard->State);
	if (!g_cecp) init_players();
	if (g_cecp) {
		g_cecp_conf.opp_computer = false;
		g_players[0].type = Human;
		g_players[0].name = xstrdup(g_cecp_conf.opp_name ? g_cecp_conf.opp_name : "");
		g_players[1].type = Computer;
		g_players[1].name = program_name_and_version();
#if !FEATURE_FORCE_SCORE_RANDOMIZATION
		g_cecp_conf.randomize_moves = false;
		g_cecp_conf.minstd_last = g_cecp_conf.minstd;
		g_cecp_conf.minstd = 0; /* CECP_DEFAULT_MINSTD */
#endif /* !FEATURE_FORCE_SCORE_RANDOMIZATION */
		/* reset any search depth limit previously set by CECP sd command. */
		g_engine_conf.depth_max = g_engine_defaults.depth_max;
		g_engine_conf.depth_default = g_engine_defaults.depth_default;
	}
	g_gamestate = GAME_IN_PROGRESS;
}

void command_random(void)
{
	/* Only toggle randomization, if it is not forced (#100, GH#3).*/
#if !FEATURE_FORCE_SCORE_RANDOMIZATION
	g_cecp_conf.randomize_moves = !g_cecp_conf.randomize_moves;
	if (!g_cecp_conf.randomize_moves) {
		g_cecp_conf.minstd_last = g_cecp_conf.minstd;
		g_cecp_conf.minstd = 0; /* CECP_DEFAULT_MINSTD */
		return;
	}

	g_cecp_conf.minstd = g_cecp_conf.minstd_last;
	if (!g_cecp_conf.minstd)
		g_cecp_conf.minstd = random_seed();
#endif /* !FEATURE_FORCE_SCORE_RANDOMIZATION */
}

int main(int argc, const char *argv[])
{
	int i;
	char *command = NULL;
	struct EngineMove *em = NULL;
	bool game_score_shown = false;

	process_cmdline(argc, argv);

	init_engine_nps();
	printf("# %s %s ~(%ju kN/s)\n", PROGRAM_NAME, PROGRAM_FULL_VERSION, g_engine_nps / 1000);

#if FEATURE_FORCE_SCORE_RANDOMIZATION
	g_cecp_conf.minstd = random_seed();
#endif /* FEATURE_FORCE_SCORE_RANDOMIZATION */

	while (1) {
		if (g_gamestate == GAME_IN_PROGRESS) {
			/* check whether game is ended (check&stalemate, draws) */
			if (checkmate(&CurrentBoard->State)) {
				Color winner = OPPONENT(CurrentBoard->State.Active);
				if (!g_cecp)
					printf("Checkmate, %s (%s) wins %s.\n", g_players[winner].name, COLOR_TEXT[winner], g_players[OPPONENT(winner)].name);
				g_gamestate = GAME_ENDED;
				g_outcome = (winner == White) ? BLACKLOSE : WHITELOSE;
				if (g_cecp) {
					printf("%s {%s mates}\n", RESULT_DECISIVE_SCORE_TEXT[winner], COLOR_TEXT[winner]);
				}
			} else if (stalemate(&CurrentBoard->State)) {
				if (!g_cecp) printf("Draw because of stalemate.\n");
				g_gamestate = GAME_ENDED;
				g_outcome = DRAW;
				if (g_cecp) printf("%s {Stalemate}\n", RESULT_DRAW_SCORE_TEXT);
			} else if (CurrentBoard->State.iMoves == 100) {
				if (!g_cecp) printf("Draw : fifty moves played without pawn advances or captures.\n");
				g_gamestate = GAME_ENDED;
				g_outcome = DRAW;
				if (g_cecp) printf("%s {Draw by 50-move rule}\n", RESULT_DRAW_SCORE_TEXT);
			} else if (insufficient_material(&CurrentBoard->State)) {
				if (!g_cecp) printf("Draw : %s.\n", INSUFFICIENT_MATERIAL_TEXT);
				g_gamestate = GAME_ENDED;
				g_outcome = DRAW;
				if (g_cecp) printf("%s {%s}\n", RESULT_DRAW_SCORE_TEXT, INSUFFICIENT_MATERIAL_TEXT);
			}
			else if (Computer == g_players[CurrentBoard->State.Active].type) {
				int rep = repetition(CurrentBoard);
				if (rep) {
					if (!g_cecp) printf("%s (%d).\n", REPETITION_DRAW_TEXT, rep);
					g_gamestate = GAME_ENDED;
					g_outcome = DRAW;
					if (g_cecp) printf("%s {%s}\n", RESULT_DRAW_SCORE_TEXT, REPETITION_DRAW_TEXT);
				}
			}
			fflush(stdout);
		}
		if (g_gamestate == GAME_IN_PROGRESS) {
			if (Computer == g_players[CurrentBoard->State.Active].type) {
				char algebraic_move[6];
				em = select_move(CurrentBoard, g_engine_conf.depth_default, &g_game_time, true);
				to_algebraic(algebraic_move, em, &CurrentBoard->State);

				if (!g_cecp)
					printf("%s plays %s (%ju kN/s)\n", g_players[CurrentBoard->State.Active].name, algebraic_move, g_engine_nps / 1000);
				else {
					printf("move %s\n", algebraic_move);
				}

				append_move(&CurrentBoard, em, em->mvc.from, em->mvc.to);
				print_board(stdout, &CurrentBoard->State);
				continue;
			}
		}
		if (g_gamestate == GAME_ENDED && !game_score_shown && (!g_cecp || FEATURE_KEEP_GAMESCORES)) {
			int sc;
			FILE *tgtf[2] = { g_cecp ? NULL : stdout, NULL };
			time_t t = time(NULL);
			struct tm tm = *localtime(&t);
#if FEATURE_KEEP_GAMESCORES
			char *homedir = getenv("HOME");
			if (homedir) {
				char homedirpath_len = strlen(homedir);
				char *scorefullpath = (char *) xmalloc((homedirpath_len + 1 + strlen(GAMESCORE_FILE) + 1 + 2) * sizeof(char));
				sprintf(scorefullpath, "%s%s%s", homedir, homedir[homedirpath_len-1] != '/' ? "/" : "", GAMESCORE_FILE);
				tgtf[1] = fopen(scorefullpath, "a+");
				if (tgtf[1])
					flock(fileno(tgtf[1]), LOCK_EX);
				xfree(scorefullpath);
			}
#endif /* FEATURE_KEEP_GAMESCORES */

			for (sc = 0; sc < 2; sc++) {
				if (!tgtf[sc])
					continue;

				print_pgn(tgtf[sc], CurrentBoard, g_outcome, &tm, &g_players[White], &g_players[Black]);
			}

#if FEATURE_KEEP_GAMESCORES
			if (tgtf[1]) {
				flock(fileno(tgtf[1]), LOCK_UN);
				fclose(tgtf[1]);
			}
#endif /* FEATURE_KEEP_GAMESCORES */
			game_score_shown = true;
		}
		if (!g_cecp) {
			fputs("command : ", stdout);
		}
		fflush(stdout);
		command = getln(stdin);

		if (NULL == command) /* EOF */
			break;

		if (!g_cecp && (strncmp(command, "setboard", 8)))
			for (i = 0; command[i]; i++)
				command[i] = tolower(command[i]);
		if (!strncmp(command, "bench", 5)) {
			uint8_t bd = g_engine_conf.depth_default;
			clock_t clock_delta;
			long double secs;
			uintmax_t knps;
			struct BoardState benchboard;

			if (strlen(command) > 5) {
				char *cptr;
				uintmax_t ply = strtoull(command+5, &cptr, 10);
				if (*cptr == 0 && ply <= UCHAR_MAX)
					bd = (uint8_t) ply;
				if ((uint8_t)(bd - 1) > (bd)) {
					xfree(command);
					continue;
				}
				bd--;
			}

			init_board(&benchboard);

			clock_delta = bench(benchboard, bd);

			secs = seconds(clock_delta);
			knps = ns_to_nps(g_nodecount, secs) / 1000;
			printf("Nodecount %ju, %.3Lfs, %ju kN/s\n", g_nodecount, secs, knps);
		} else if (!strcmp(command, "xboard")) {
			g_cecp = true;
			printf("# received xboard\n");
			fflush(stdout);
			command_new(); /* #92, allow for "go" directly after xboard. */
			xfree(command);
			continue;
		}
		else if (!strncmp(command, "protover", 8) && g_cecp) {
			command_protover_cecp();
			xfree(command);
			continue;
		} else if (!strncmp(command, "ping", 4) && g_cecp) {
			if (strlen(command) > 5) {
				printf("pong%s\n", command+4);
			}
			fflush(stdout);
			xfree(command);
			continue;
		}
		else if (!strcmp(command, "computer") && g_cecp) {
			g_cecp_conf.opp_computer = true;
			xfree(command);
			continue;
		}
		else if (!strcmp(command, "force") && g_cecp) {
			g_players[0].type = Human;
			g_players[1].type = Human;
			g_gamestate = GAME_IN_PROGRESS;
		} else if (!strcmp(command, "undo") && g_gamestate != NO_GAME && NULL != CurrentBoard->LastBoard) {
			struct BoardStateList *db = CurrentBoard;
			CurrentBoard = db->LastBoard;
			g_gamestate = GAME_IN_PROGRESS;
			xfree(db);
		} else if (!strcmp(command, "remove") && g_gamestate != NO_GAME && NULL != CurrentBoard->LastBoard) {
			if (NULL != CurrentBoard->LastBoard->LastBoard) {
				struct BoardStateList *db1 = CurrentBoard;
				struct BoardStateList *db2 = db1->LastBoard;

				CurrentBoard = db2->LastBoard;
				g_gamestate = GAME_IN_PROGRESS;
				xfree(db1);
				xfree(db2);
			}
		}
		else if (g_cecp && !strncmp(command, "result", 6)) {
			command_result_cecp(command);
		}
		else if (g_cecp && !strcmp(command, "?")) { /* #111, GH#13 */
			if (g_gamestate != GAME_IN_PROGRESS) {
				print_cmd_error(command, NO_GAME_TEXT);
			}
			xfree(command);
			continue; /* Move already sent. */
		}
		else if (!strcmp(command, "resign")) {
			/* Not a command relayed by CECP to engine. */
			if (g_cecp) print_cmd_error(command, UNKNOWN_COMMAND_TEXT);
			else if (g_gamestate == GAME_IN_PROGRESS) {
				g_gamestate = GAME_ENDED;
				g_outcome = (CurrentBoard->State.Active == White) ? WHITELOSE : BLACKLOSE;
			} else {
				print_cmd_error(command, NO_GAME_TEXT);
			}
			xfree(command);
			continue;
		}
		else if (!strcmp(command, "random") && g_cecp) {
			xfree(command);
			command_random();
			continue;
		}
		else if (!strncmp(command, "accepted", 8) && g_cecp) {
			xfree(command);
			continue;
		}
		else if (!strncmp(command, "rejected", 8) && g_cecp) { /* #111, GH#13 */
			/*
				This command is only sent by protocol version 2 interfaces. For them,
				rejections concern currently interface-to-engine features that are
				deprecated anyway and not supported (for CuteChess 1.2.0 'edit', 'colors',
				'sigint', 'sigterm'). So interfaces will not be even trying to use these
				and separate accounting is not needed. This might change when some optional
				engine-to-interface command is implemented.
			*/
			xfree(command);
			continue;
		}

		else if (!strcmp(command, "hint") && g_cecp) {
			xfree(command);
			struct EngineMove* hint = select_move(CurrentBoard, 1, NULL, true);
			if (hint) {
				char algebraic_move[6];
				to_algebraic(algebraic_move, hint, &CurrentBoard->State);
				printf("Hint: %s\n", algebraic_move);
				fflush(stdout);
				xfree_engine_move(hint);
			}
			continue;
		}

		else if (!strncmp(command, "time", 4) && g_cecp) {
			if (strlen(command) > 4) {
				char *cptr;
				long cs = strtol(command+4, &cptr, 10);
				if ((command + 4 != cptr) && *cptr == 0) {
					long ms = 10 * cs;
					g_game_time.time_left = (ms > INT32_MIN && cs < INT32_MAX) ? ms : 0;
				}
			}
			fflush(stdout);
			xfree(command);
			continue;
		}
		else if (!strncmp(command, "otim", 4) && g_cecp) {
			xfree(command);
			continue;
		}
		else if (!strcmp(command, "draw")) {
			/* keep on playing. */
		}
		else if (!strcmp(command, "post") && g_cecp) {
			g_cecp_conf.output_thinking = true;
			xfree(command);
			continue;
		}
		else if (!strcmp(command, "nopost") && g_cecp) {
			g_cecp_conf.output_thinking = false;
			xfree(command);
			continue;
		}
		else if (!strncmp(command, "setboard", 8) && strlen(command) > 9 && command[8] == ' ') {
			struct BoardState *nbs = FEN2Board(command + 9);
			if (nbs) {
				clear_board_list(CurrentBoard);
				CurrentBoard = (struct BoardStateList *) xcalloc(1, sizeof(struct BoardStateList));
				CurrentBoard->State = *nbs;
				xfree(nbs);
			} else if (g_cecp) {
				printf("tellusererror %s\n", ILLEGAL_POSITION_TEXT);
			} else {
				printf("%s\n", ILLEGAL_POSITION_TEXT);
			}
		}
		else if (!strcmp(command, "hard") && g_cecp) {
			g_cecp_conf.ponder = true;
			xfree(command);
			continue;
		}
		else if (!strcmp(command, "easy") && g_cecp) {
			g_cecp_conf.ponder = false;
			xfree(command);
			continue;
		}
		else if (!strncmp(command, "level", 5) && g_cecp) {
			xfree(command);
			continue;
		}
		else if (!strncmp(command, "name", 4) && g_cecp) {
			if (strlen(command) > 5) {
				if (g_cecp_conf.opp_name)
					xfree(g_cecp_conf.opp_name);
				g_cecp_conf.opp_name = xstrdup(command + 5);
				if (!(g_players[0].type == Human && g_players[1].type == Human)) {
					struct PlayerInfo *pi = (g_players[0].type == Human) ? &g_players[0] : &g_players[1];
					if (pi->name) xfree(pi->name);
					pi->name = xstrdup(g_cecp_conf.opp_name);
				}
			}
			xfree(command);
			continue;
		}

		else if (!strncmp(command, "st", 2) && g_cecp) {
			/* No support for this, but do not complain about command. */
		}

		else if (!strncmp(command, "sd", 2) && g_cecp) {
			intmax_t sd_limit = to_int(command + 2, 1, UINT16_MAX, false, true, NULL);
			if (!errno) {
				sd_limit--;
				if (sd_limit > UINT8_MAX) sd_limit = UINT8_MAX;
				g_engine_conf.depth_max = (uint8_t) sd_limit;
				if (sd_limit < g_engine_conf.depth_default)
					g_engine_conf.depth_default = sd_limit;
			} else {
				print_cmd_error(command, UNKNOWN_COMMAND_TEXT);
			}
			xfree(command);
			continue;
		}

		else if (!strcmp(command, "go") && g_cecp) {
			Color ongo = CurrentBoard->State.Active;
			Color held = OPPONENT(ongo);
			if (g_players[ongo].type != Computer) {
				g_players[held].type = Human;
				g_players[held].color = held;
				if (g_cecp_conf.opp_name) {
					if (g_players[held].name) xfree(g_players[held].name);
					g_players[held].name = xstrdup(g_cecp_conf.opp_name);
				} else if (NULL == g_players[held].name) {
					g_players[held].name = xstrdup("");
				}

				g_players[ongo].type = Computer;
				g_players[ongo].color = ongo;
				if (g_players[ongo].name) xfree(g_players[ongo].name);
				g_players[ongo].name = program_name_and_version();
			}
		}

		else if (!strcmp(command, "quit")) {
			clear_board_list(CurrentBoard);
			break;
		}  else if (!strcmp(command, "new")) {
			game_score_shown = false;
			command_new();
		} else if (!strcmp(command, "help")) {
			if (g_cecp) { /* Both reject it with compliant error message ... */
				print_cmd_error(command, UNKNOWN_COMMAND_TEXT);
			}
			print_help(); /* ... and do support it, for rare people doing console speak. */
			xfree(command);
			continue;
		} else if (g_gamestate == GAME_IN_PROGRESS) {
			/* it's humans turn to move, check if input describes a move */
			struct MoveCoords *coords = parsed_move(command);
			if (coords == NULL) {
				print_cmd_error(command, UNKNOWN_COMMAND_TEXT);
			} else if (strlen(command) != 5 &&
					(((command[1] == '2' && command[3] == '1') || (command[1] == '7' && command[3] == '8'))) &&
					PAWN == TO_WHITE(CurrentBoard->State.Board[(int)coords->from])
					) {
				if (!g_cecp) puts("promotion suffix required");
				if (g_cecp) printf("%s (promotion suffix required): %s\n", ILLEGAL_MOVE_TEXT, command);
			} else {
				char promopiece = NONE;
				if (strlen(command) == 5) { /* promotion request */
					const char *prs = "bnrq";
					const char *pos = strchr(prs, command[4]);
					promopiece = PAWN + (pos - prs)  + 1;
				}
				int result;
				/* test for move legality */
				result = validate_move(&CurrentBoard->State, coords);
				if (result == VALID) {
					append_move(&CurrentBoard, NULL, coords->from, PROMO_ENCODE(promopiece, coords->to));
				} else if (result == INVALID) {
					if (!g_cecp) puts("Invalid move, i guess.");
					if (g_cecp) printf("%s: %s\n", ILLEGAL_MOVE_TEXT, command);
				}
				else if (result == LEAVING_IN_CHECK) {
					if (!g_cecp) puts("Hey, watch where you leave your king!");
					if (g_cecp) printf("%s (in check): %s\n", ILLEGAL_MOVE_TEXT, command);
				}
				else if (result == PLACING_IN_CHECK) {
					if (!g_cecp) puts("You can't place your king under fire.");
					if (g_cecp) printf("%s (moving into check): %s\n", ILLEGAL_MOVE_TEXT, command);
				}

				xfree(coords);
			}
		} else
			print_cmd_error(command, UNKNOWN_COMMAND_TEXT);
		if (g_gamestate == GAME_IN_PROGRESS)
			print_board(stdout, &CurrentBoard->State);
		xfree(command);
	}
	if (command) xfree(command);

	return 0;
}
