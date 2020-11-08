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

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "brdlist.h"
#include "globals.h"
#include "types.h"
#include "xtdlib.h"
#include "move.h"
#include "fen.h"
#include "iomain.h"

const char *disambiguate(struct BoardState *bs, char piece, int from, int toenc) {
	int pc = 0, fc = 0, rc = 0, i = 0;
	const char *fromSq = SQUARES[from];

	for (i = 0; i < 64; i++) {
		if (bs->Board[i] != piece)
			continue;

		struct MoveCoords mc = { .from = i, .to = toenc };
		if (VALID == validate_move(bs, &mc)) {
			pc++;
			if (SQUARES[i][0] == fromSq[0]) fc++;
			if (SQUARES[i][1] == fromSq[1]) rc++;
		}
	}

	if (pc == 1)
		return "";

	if (fc == 1)
		return BOARD_FILES[from % 8];

	if (rc == 1)
		return BOARD_RANKS[(from / 8)];

	return SQUARES[from];
}

void clear_board_list(struct BoardStateList *bList)
{
	struct BoardStateList *last;
	while(bList != NULL) {
		last = bList->LastBoard;
		xfree_engine_move(bList->epv);
		xfree(bList);
		bList = last;
	}
}

struct BoardStateList *append_move(struct BoardStateList **b, struct EngineMove *emv, uint8_t from, uint8_t toenc)
{
	struct BoardStateList *bsl = *b;
	struct BoardStateList *t = (struct BoardStateList *) xmalloc(sizeof(struct BoardStateList));
	uint8_t to = PROMOTIONLESS(toenc);
	t->from = from, t->to = to;
	t->LastBoard = bsl;
	t->State = bsl->State;
	t->epv = emv;
	t->pocc = NULL;
	t->rsc = bsl->rsc;

	Move(&t->State, from, toenc);

	if (!t->State.iMoves)
		t->rsc = false;
	else if (t->State.iMoves > 3 && t->LastBoard->LastBoard) { /* must work with setboard! */
		struct BoardStateList *prb = t;
		do {
			prb = prb->LastBoard->LastBoard;

			if (same_position(&t->State, &prb->State)) {
				t->pocc = prb;
				t->rsc = true;
				break;
			}
		} while (prb->State.iMoves && prb->LastBoard && prb->LastBoard->State.iMoves && prb->LastBoard->LastBoard);
	}


	*b = t;

	return t;
}

#if FEATURE_KEEP_ALL_PVS
void printPgnVarPly(FILE *stream, int i, struct BoardState *o, struct BoardState *after, int from, int toenc) {
	int to = PROMOTIONLESS(toenc);
	char *board = o->Board;
	char piece = board[from];
	char capture = (board[to] != NONE);
	char castling = IS_KING(piece) && (abs(from - to)) == 2;
	char promotion = IS_PAWN(piece) && (to <= 7 || to >= 56);

	if ((i-1) % 2  == 0)
		fprintf(stream, "%d. ", ((i-1) / 2) + 1);

	if (IS_PAWN(piece) && (capture || 0 != (abs(from - to) % 8))) {
		fprintf(stream, "%c", SQUARES[from][0]);
	} else if (!IS_PAWN(piece) && !castling)  {
		fprintf(stream, "%c", piece2SAN(piece));
	}

	if (!IS_PAWN(piece)) {
		/* Try to avoid disambiguation when not needed. */
		bool hasambiguity = false;
		int hell;
		for (hell = 0; hell < 64; hell++) {
			if (from == hell) continue;
			if (piece == board[hell]) {
				bool can_move_too = false;
				hasambiguity = true;
				uint8_t damn_moves[30] = {0};
				switch (TO_WHITE(piece)) {
					case QUEEN:
						mvs_q(&damn_moves, o, hell);
						break;
					case ROOK:
						mvs_r(&damn_moves, o, hell);
						break;
					case BISHOP:
						mvs_b(&damn_moves, o, hell);
						break;
					case KNIGHT:
						mvs_n(&damn_moves, o, hell);
						break;
					default:
						break; /* king moves should be non-ambiguous :) */
				}
				if (damn_moves[0]) {
					int di;
					for (di = 0; di < damn_moves[0]; di++) {
						if (damn_moves[2+di] == to) {
							can_move_too = true;
							break;
						}
					}
				}
				if (!can_move_too)
					hasambiguity = false;
			}
			if (hasambiguity)
				break;
		}
		if (hasambiguity) {
			fprintf(stream, "%s", disambiguate(o, piece, from, to));
		}
	}

	if (capture || (IS_PAWN(piece) && (0 != (abs(from - to) % 8))))
		fprintf(stream, "x");

	if (castling)
		fprintf(stream, "%s%s", "O-O", ((from - to) > 0) ? "-O": "");
	else
		fprintf(stream, "%s", SQUARES[to]);

	if (promotion) {
		fprintf(stream, "=%c", piece2SAN(after->Board[to]));
	}
	if (is_check(after,after->Active)) {
		fprintf(stream, "%c", checkmate(after) ? '#' : '+');
	}

	fprintf(stream, " ");
}

#endif /* FEATURE_KEEP_ALL_PVS */

void print_boardlist_pgn(FILE *stream, struct BoardStateList *bList)
{
	int c=0, x=0, i;
	struct BoardStateList *last = bList;
	if (last == NULL)
		return;

	while(last != NULL) {
		c++;
		last = last->LastBoard;
	}
	struct BoardStateList **o = (struct BoardStateList **) xcalloc(c, sizeof(struct BoardStateList *));

	x = c;
	last = bList;
	while(last != NULL) {
		c--;
		o[c] = last;
		last = last->LastBoard;
	}

	for (i = 1; i < x; i++) {
		char *board = o[i-1]->State.Board;
		char piece = board[(int)o[i]->from];
		char capture = (board[(int)o[i]->to] != NONE);
		char castling = (IS_KING(piece)) && (abs(o[i]->from - o[i]->to)) == 2;
		char promotion = (IS_PAWN(piece) && (o[i]->to <= 7 || o[i]->to >= 56));

		if ((i-1) % 2  == 0)
			fprintf(stream, "%d. ", ((i-1) / 2) + 1);

		if (IS_PAWN(piece) && (capture || 0 !=  (abs(o[i]->from - o[i]->to) % 8))) {
			fprintf(stream, "%c", SQUARES[(int)o[i]->from][0]);
		} else if (!IS_PAWN(piece) && !castling)  {
			fprintf(stream, "%c", piece2SAN(piece));
		}

		if (!IS_PAWN(piece))
			fprintf(stream, "%s", disambiguate(&o[i-1]->State, piece, o[i]->from, o[i]->to));

		if (capture || (IS_PAWN(piece) && (0 != (abs(o[i]->from - o[i]->to) % 8))))
			fprintf(stream, "x");

		if (castling)
			fprintf(stream, "%s%s", "O-O", ((o[i]->from - o[i]->to) > 0) ? "-O": "");
		else
			fprintf(stream, "%s", SQUARES[(int)o[i]->to]);

		if (promotion) {
			fprintf(stream, "=%c", piece2SAN(o[i]->State.Board[(int)o[i]->to]));
		}

		if (is_check(&o[i]->State, o[i]->State.Active)) {
			fprintf(stream, "%c", checkmate(&o[i]->State) ? '#' : '+');
		}

		fprintf(stream, " ");

#if FEATURE_KEEP_ALL_PVS
		if (o[i]->epv && o[i]->epv->alternates && o[i]->epv->alt_count) {
			int j, z;
			fprintf(stream, "\n");
			for (j = 0; j < o[i]->epv->alt_count; j++) {
				PV that = o[i]->epv->alternates[j];
				struct BoardState altstates[that.depth];
				int32_t cp_score = centipawn_score(o[i]->epv->altscores[j]);

				altstates[0] = o[i-1]->State;
				fprintf(stream, "\t( ");
				for (z = 0; z < that.depth; z++) {
					struct BoardState bcopy = altstates[z];

					Move(&bcopy, that.pv[2*z], that.pv[2*z+1]);
					printPgnVarPly(stream, i+z, &altstates[z], &bcopy, that.pv[2*z], that.pv[2*z+1]);

					if (z+1 < that.depth) altstates[z+1] = bcopy;

					if (that.depth < z + 1)
						fprintf(stream, " ");
				}
				fprintf(stream, "{ ");
				if (!IS_MATESCORE(o[i]->epv->altscores[j])) {
					fprintf(stream, "%g", cp_score / 100.0);
				} else {
					putc('#', stream);
					if (o[i]->epv->altscores[j] < 0)
						putc('-', stream);
					fprintf(stream, "%d", MATESCORE_DEPTH(o[i]->epv->altscores[j]));
				}
				fprintf(stream, "/%d }", that.depth);
				fprintf(stream, ")\n");
			}
			fprintf(stream, "\n");
		}
#endif /* FEATURE_KEEP_ALL_PVS */

		if ((i-1) % 14  == 0 && (i > 1))
			fprintf(stream, "\n");
	}

	xfree(o);
}
