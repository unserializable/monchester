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

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "compmove.h"
#include "iomain.h"
#include "types.h"
#include "move.h"
#include "globals.h"
#include "xtdlib.h"

int32_t static_score(const struct BoardState *Board, Color judged)
{
	Color opc = OPPONENT(judged);
	int32_t score = Board->material[judged] - Board->material[opc];

	if ((Board->pCount[0] <= 2) && (Board->pCount[0] + Board->pCount[1] == 3)) {
		int32_t md = abs(score);
		if (md < MATVAL[ROOK]) {
			int i = 0;
			bool has_pawn = false;
			for (i = 0; i < 64; i++)
				if (IS_PAWN(Board->Board[i])) {
					has_pawn = true;
					break;
				}

			if (!has_pawn) /* KBK or KNK insufficent material draw. */
				return 0;
		} else { /* Major piece ending against sole king, (#65, GH#11). */
			uint8_t lost_king = Board->king[score > 0 ? opc : judged];
			int32_t lkcd = (
				BOARD_RANK_ABS_DISTANCE(27 /* D4 */, lost_king) +
				BOARD_FILE_ABS_DISTANCE(27 /* D4 */, lost_king) +
				BOARD_RANK_ABS_DISTANCE(36 /* E5 */, lost_king) +
				BOARD_FILE_ABS_DISTANCE(36 /* E5 */, lost_king)
			);
			int32_t kd = (
				BOARD_RANK_ABS_DISTANCE(Board->king[0], Board->king[1]) +
				BOARD_FILE_ABS_DISTANCE(Board->king[0], Board->king[1])
			);

			return score += (score / abs(score)) * (2*lkcd - 3*kd);
		}
	}

	score -= ((Board->iMoves * score) / 1024); /* #82 */
	score += (Board->pbonus[judged] - Board->pbonus[opc]);

#ifndef DISABLE_SCORE_RANDOMIZATION
  #ifndef SCORE_RANDOM_BITS
   #define SCORE_RANDOM_BITS UINT32_C(5)
  #endif
  #define SCORE_RANDOM_HALF ((UINT32_C(1) << (SCORE_RANDOM_BITS - 1)))
	score += (INT32_C(SCORE_RANDOM_HALF) - (CHMINSTD_NXT_BITS(g_cecp_conf.minstd, SCORE_RANDOM_BITS)));
  #undef SCORE_RANDOM_HALF
#endif

	return score;
}

int32_t Score(struct BoardState *Board, struct BoardStateList *bb4, Color judged, uint8_t depth, PV *pv)
{
	uint8_t moves[16][30];
	register int i, j = -1;
	uint8_t acj = (Board->Active == judged);
	uint8_t pv_entry_depth = pv->depth;

	g_nodecount++;

	if (Board->iMoves == 100 && !is_check(Board, Board->Active))
		return 0;

	bool depth_reached = (depth == 0) || (pv_entry_depth == g_engine_conf.depth_max);

	if (depth_reached || !(j = mvs_a(&moves, Board, NULL))) {
		if (!j)
			return Board->check ? (acj ? MATESCORE_LOSS(pv_entry_depth) : MATESCORE_WIN(pv_entry_depth) /* #60 */) : 0;

		int cors = check_or_stalemate(Board);
		if (cors)
			return cors == 1 ? (acj ? MATESCORE_LOSS(pv_entry_depth) : MATESCORE_WIN(pv_entry_depth) /* #60 */) : 0;

		return static_score(Board, judged);
	}

	uint8_t pvm[depth*2];
	Flack flack = { .stack = pvm, .store = pvm, .len = 0, .size = depth*2 };
	int32_t score, r = (acj ? INT32_MIN : INT32_MAX);
	int npieces = Board->pCount[Board->Active];

	for (i = 0; i < npieces; i++) {
		for (j = 0; j < moves[i][0]; j++) {
			struct MoveInfo mInfo;
			pv_push(pv, moves[i][1], moves[i][2+j]); /* real push */
			mInfo = Move(Board, moves[i][1], moves[i][2 + j]);

			bool allowsRepetition = false;
			if (bb4 && Board->iMoves) {
				struct BoardStateList *prb = bb4;
				do {
					if (prb->pocc && same_position(Board, &prb->State)) {
						allowsRepetition = true;
						break;
					}
					prb = prb->LastBoard;
				} while (prb && prb->rsc);
			}

			score = !allowsRepetition ? Score(Board, NULL, judged, depth - 1, pv) : 0;

			undo_move(Board, &mInfo);

			if (acj) {
				if (score > r) {
					r = score;
					store(&flack, pv->pv+pv_entry_depth*2, (pv->depth - pv_entry_depth)*2);
				}
			} else {
				if (score < r) {
					r = score;
					store(&flack, pv->pv+pv_entry_depth*2, (pv->depth - pv_entry_depth)*2);
				}
			}

			pv->depth = pv_entry_depth;
		}
	}

	if (depth) {
		pv_rewrite(pv, pv_entry_depth, flack.store, flack.len);
	}
	return r;
}

/* Raises unsigned integer /n/ to power /x/. */
uintmax_t upow(uintmax_t n, uint8_t x) {
	uintmax_t r = 1;
	int i;
	for (i = 1; i <= x; i++)
		r *= n;

	return r;
}

/* Selects legal move for color on the move, returns NULL when no moves (check or stalemate). */
struct EngineMove *select_move(struct BoardStateList *bsl, uint8_t depth, const struct TimeControl *tc)
{
#if FEATURE_KEEP_ALL_PVS
	PV *all_pvs = NULL;
#endif /* FEATURE_KEEP_ALL_PVS */
	PV pv;
	struct EngineMove *move;
	struct MoveInfo mInfo;
	Color Active;
	uint8_t Moves[16][30];
	int32_t scores[16][28];
	int *max, npieces, tmax = INT_MIN;
	register int i, j;
	struct BoardState copy = bsl->State;
	struct BoardState *Board = &copy;
	bool has_prev_pv = (bsl->LastBoard != NULL) && (bsl->LastBoard->epv != NULL) && (bsl->LastBoard->epv->pvd > 4);
	uint8_t *prev_pv = has_prev_pv ? bsl->LastBoard->epv->pv+4 : NULL;
	clock_t clock_end, clock_start = clock();
	pv_init(&pv);

	Active = Board->Active;
	npieces = Board->pCount[Active];

	int gen_move_count = mvs_a(&Moves, Board, prev_pv);
	max = (int *) xmalloc(npieces * sizeof(int));
	move = (struct EngineMove *) xmalloc(sizeof(struct EngineMove));
	move->pv = NULL;

	uintmax_t est_nc = (upow(gen_move_count, depth + 1) / 5) * 31;
	uintmax_t est_ms = ((long double)est_nc * 1000L / g_engine_nps);

	while (tc && (tc->time_left > 0) && (tc->time_left < est_ms)) { /* #59. If time is zero or negative, do not care. */
#if DEBUG
		printf("#   DEPTH %d, est_ms was %lu for %lu nodes, time left %d ms.\n", depth + 1, est_ms, est_nc, tc->time_left);
#endif
		depth--;
		est_nc = (upow(gen_move_count, depth + 1) / 5) * 31;
		est_ms = ((long double)est_nc * 1000L / g_engine_nps);
#if DEBUG
		printf("# --DEPTH %d, est_ms was %lu for %lu nodes, time left %d ms.\n", depth + 1, est_ms, est_nc, tc->time_left);
#endif
	};

#if FEATURE_KEEP_ALL_PVS
	for (i = j = 0; i < npieces; i++)
		j += Moves[i][0];

	/* Careful zero-initializing allocation, there will be no other indication about how 
	 * many elements actually arrive in all_pvs array except sudden zeros in PVs. */
	all_pvs = (PV *) xcalloc(j, sizeof(PV));
	move->alternates = all_pvs;
	move->altscores = (int32_t *) xcalloc(j, sizeof(int32_t));
	gen_move_count = 0; /* Re-use for all-move PV tracking! */
#endif /* FEATURE_KEEP_ALL_PVS */

	/* This would be one of the candidate places for rearranging the pieces to e.g. prioritize captures
	   or checks or castling or whatnot. Also, if large material plus, then heavy pieces would
	   be probably best to move. At least against the sole remaining opponent king. */

	g_nodecount = 0;

	for (i = 0; i < npieces; i++) { /* Gets the scores for all moves of all pieces. */
		max[i] = INT_MIN;
		if (Moves[i][0] == 0)
			continue;

		for (j = 0; j < Moves[i][0]; j++) {
				pv_push(&pv, Moves[i][1], Moves[i][j+2]);
				mInfo = Move(Board, Moves[i][1], Moves[i][j+2]);
				scores[i][j] = (Board->iMoves < 100) ? Score(Board, bsl->rsc ? bsl : NULL, Active, depth, &pv) : 0;

				struct BoardStateList *repl;
				repl = (struct BoardStateList *) xmalloc(sizeof(struct BoardStateList));
				/* repl->from/to are irrelevant to fill for repetition detection. */
				repl->LastBoard = CurrentBoard;
				repl->State = *Board;
				if (repetition(repl)) {
					scores[i][j] = 0;
					pv_rewrite(&pv, 1, NULL, 0);
				}
				xfree(repl);

				if (scores[i][j] > max[i]) {
					max[i] = scores[i][j];
					if (max[i] > tmax) {
						tmax = max[i];
						move->pvd = 2 * pv.depth;
						move->pv = (uint8_t *) xrealloc(move->pv, move->pvd * sizeof(uint8_t));
						memcpy(move->pv, pv.pv, 2*pv.depth);
						cecp_print_pv(&pv, tmax, clock_start);
#if DEBUG
						fprintf(stdout, "# PV: ");
						print_pv(stdout, pv.pv, pv.depth, tmax);
#endif
					}
				}
#if FEATURE_KEEP_ALL_PVS
				all_pvs[gen_move_count].size = pv.depth * 2;
				all_pvs[gen_move_count].depth = pv.depth;
				all_pvs[gen_move_count].pv = xmalloc(sizeof(uint8_t) * all_pvs[gen_move_count].size);
				memcpy(all_pvs[gen_move_count].pv, pv.pv, all_pvs[gen_move_count].size);
				move->altscores[gen_move_count] = scores[i][j];
				move->alt_count = ++gen_move_count;
#endif /* FEATURE_KEEP_ALL_PVS */

				undo_move(Board, &mInfo);
				pv.depth = 0;
		}
	}

#if DEBUG_EVAL
	for (i = 0; i < npieces; i++) {
		for (j = 0; j < Moves[i][0]; j++) {
			printf("%s%s->%s: %d\n", g_cecp ? "# ": "", SQUARES[(int)Moves[i][1]], SQUARES[PROMOTIONLESS(Moves[i][j+2])], scores[i][j]);
		}
	}
#endif

	xfree(max);
	pv_free(&pv);

	clock_end = clock();
	g_engine_nps = (g_engine_nps + 3*ns_to_nps(g_nodecount, seconds(clock_end - clock_start)))/4;

#if DEBUG
	fprintf(stdout, "Estimated node count %ju, processed %ju\n", est_nc, g_nodecount);
#endif

	/* #72: return NULL when no moves at all. */
	if (move->pv && move->pvd) {
		move->mvc.from = move->pv[0], move->mvc.to = move->pv[1];
	} else {
		xfree(move);
		return NULL;
	}

	return move;
}

