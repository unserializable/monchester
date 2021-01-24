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

/** @file move.c
Move generation routines, move/undo and various related utility functions.
Functions mvs_(piece SAN) other than mvs_k() do NOT check whether move places
king in check, but mvs_a() (all) produces only legal moves. */

#include <assert.h>
#if DEBUG
#include <stdio.h>
#endif

#include "move.h"

#include "bishop.h"
#include "knight.h"
#include "rook.h"
#include "queen.h"
#include "king.h"

#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "globals.h"
#include "types.h"
#include "xtdlib.h"
#include "iomain.h"

#if DEBUG
#define PIECE_SANITY_CHECK(P, brd, from) \
	do { \
	if (TO_WHITE(((brd)->Board)[from]) != (P)) {\
			fprintf(stderr, "g_" #P "(): NO " #P " on square %d\n", from); \
			exit(255); \
		}\
	if (!IS_COLOR((brd)->Active, (brd)->Board[from])) { \
			fprintf(stderr, "g_" #P "(): active color doesn't match piece color at %d\n", from); \
			exit(255); \
		} \
	} while(0)
#else
#define PIECE_SANITY_CHECK(P, brd, from) do {} while (0)
#endif

static inline union ub16 pawn_cap_sqs(Color c, int8_t psq) {
	register union ub16 r;
	register uint16_t m = UINT16_C(0x180) << (BOARD_FILE(psq)) & UINT16_C(0x8080);
	r.i8[0] = psq + (c ? -9 : 7), r.i8[1] = r.i8[0] + 2;
	r.u16 = r.u16 | m;

	return r;
}

/** Generates slider moves /from/ according to given vector specification. */
int mvs_s(uint8_t (*dst)[30], const struct BoardState *Board, int from, const uint8_t *vectors)
{
	register int i, j, movecount = 0;
	uint8_t *tmoves = *dst;

	tmoves[1] = (uint8_t) from;

	for (j = 0; j < 8; j++) {
		int8_t sq = from;
		for (i = 1; i <= vectors[from * 8 + j]; i++) {
			sq += CARDINAL_ADDENDS[j];
			int8_t pc = Board->Board[sq];
			if (pc == NONE) {
				tmoves[2 + movecount++] = sq;
				continue;
			} else if (!FRIENDLY(pc, Board->Board[from]))
				tmoves[2 + movecount++] = sq;
			break;
		}
	}

	return tmoves[0] = movecount;
}

int mvs_p(uint8_t (*dst)[30], const struct BoardState *Board, int from)
{
	int movecount = 0, i, k;
	uint8_t *moves = *dst;

	PIECE_SANITY_CHECK(PAWN, Board, from);
	*(moves + 1) = from;

	/* capturing moves for pawns standing on a || h files need spec treatment */
	if (!(from % 8)) { 	/* a */
		if ((Board->Active == White) && (IS_BLACK(Board->Board[from+9])))
			moves[2 + movecount++] = from + 9;
		else if ((Board->Active == Black) && (IS_WHITE(Board->Board[from-7])))
			moves[2 + movecount++] = from - 7;
	} else if (!((from+1) % 8)) {	/* h */
		if ((Board->Active == White) && (IS_BLACK(Board->Board[from + 7])))
			moves[2 + movecount++] = from + 7;
		else if ((Board-> Active == Black) && (IS_WHITE(Board->Board[from - 9])))
			moves[2 + movecount++] = from - 9;
	} else {
		if (Board->Active == White) {
			if (IS_BLACK(Board->Board[from + 7]))
				moves[2 + movecount++] = from + 7;
			if (IS_BLACK(Board->Board[from + 9]))
				moves[2 + movecount++] = from + 9;
		}
		else {
			if (IS_WHITE(Board->Board[from - 7]))
				moves[2 + movecount++] = from - 7;
			if (IS_WHITE(Board->Board[from - 9]))
				moves[2 + movecount++] = from - 9;
		}
	}

	for (i = 0, k = movecount; i < k; i++) {
		if (moves[2+i] < 8 || moves[2+i] >= 56) {
			moves[2 + movecount++] = PROMO_ENCODE(QUEEN, moves[2+i]);
			moves[2 + movecount++] = PROMO_ENCODE(ROOK, moves[2+i]);
			moves[2 + movecount++] = PROMO_ENCODE(KNIGHT, moves[2+i]);
		}
	}

	/* infamous ep */
	if ((Board->epTarget) && ((abs(from - Board->epTarget) == 7) || (abs(from - Board->epTarget) == 9)))
		if (BOARD_RANK_ABS_DISTANCE(Board->epTarget, from) == 1 && (((from / 8) == 3) || ((from/8) == 4)))
			moves[2 + movecount++] = Board->epTarget;

	/* generating pawn moves is overly painful, */
	/* but pushing them forward */
	/* is really straightforward */
	/* so we yet do not need to be mournful */
	if (Board->Active == White) {
		if (Board->Board[from + 8] == NONE) {
			moves[2 + movecount++] = from + 8;
			if (((from >=8) && (from <= 15)) && (Board->Board[from + 16] == NONE))
				moves[2+movecount++] = from + 16;
			else if ((from + 8) >= 56) {
				uint8_t tosq = from + 8;
				moves[2 + movecount++] = PROMO_ENCODE(QUEEN, tosq);
				moves[2 + movecount++] = PROMO_ENCODE(ROOK, tosq);
				moves[2 + movecount++] = PROMO_ENCODE(KNIGHT, tosq);
			}
		}
	} else if (Board->Active == Black) {
		if (Board->Board[from - 8] == NONE) {
			moves[2 + movecount++] = from - 8;
			if (((from >=48) && (from <= 55)) && (Board->Board[from - 16] == NONE))
				moves[2+movecount++] = from - 16;
			else if ((from - 8) < 8) {
				uint8_t tosq = from - 8;
				moves[2 + movecount++] = PROMO_ENCODE(QUEEN, tosq);
				moves[2 + movecount++] = PROMO_ENCODE(ROOK, tosq);
				moves[2 + movecount++] = PROMO_ENCODE(KNIGHT, tosq);
			}
		}
	}

	return moves[0] = movecount;
}

int mvs_b(uint8_t (*dst)[30], const struct BoardState *Board, int from)
{
	PIECE_SANITY_CHECK(BISHOP, Board, from);
	return mvs_s(dst, Board, from, &vBishop[0]);
}

int mvs_n(uint8_t (*dst)[30], const struct BoardState *Board, int from)
{
	register int cnt = 0, max_cnt = mKnight[from][0], i;
	PIECE_SANITY_CHECK(KNIGHT, Board, from);
	for (i = 0; i < max_cnt; i++) {
		char tgtsqpc = Board->Board[mKnight[from][i+1]];
		if (!tgtsqpc || !FRIENDLY(Board->Board[from], tgtsqpc)) /* move can be made */
			dst[0][2+cnt++] = mKnight[from][i+1];
	}
	dst[0][1] = from;
	return dst[0][0] = cnt;
}

int mvs_r(uint8_t (*dst)[30], const struct BoardState *Board, int from)
{
	PIECE_SANITY_CHECK(ROOK, Board, from);
	return mvs_s(dst, Board, from, &vRook[0]);
}

int mvs_q(uint8_t (*dst)[30], const struct BoardState *Board, int from)
{
	PIECE_SANITY_CHECK(QUEEN, Board, from);
	return mvs_s(dst, Board, from, &vQueen[0]);
}

/** Generates /legal/ king moves from chessboard /Board/ square /from/. Castling
 is represented as king moving two squares horizontally. */
int mvs_k(uint8_t (*dst)[30], struct BoardState *Board, int from)
{
	static int wqside[] = {1, 2, 3}, wkside[] = { 5, 6 },
			   bqside[] = {57,58, 59}, bkside[] = { 61, 62 };
	int *kside, *qside;
	char ck, cq;
	uint8_t *moves = *dst;
	register int i, j;
	
	PIECE_SANITY_CHECK(KING, Board, from);
	mvs_s(dst, Board, from, &vKing[0]);
	/* Having king do captures of protected pieces messes up evaluation, see #19, #20 */
	/* Remove king to not get it to /block/ ongoing attack to its desired target square */
	int8_t monarch = Board->Board[Board->king[Board->Active]];
	Board->Board[Board->king[Board->Active]] = NONE;
	for (i = j = 0; i < moves[0]; i++) {
		if (!attacked_by(OPPONENT(Board->Active), moves[2 + i], Board)) {
			if (i != j)
			  moves[2+j] = moves[2+i];
			j++;
		}
	}
	/* Restore monarchy. */
	Board->Board[Board->king[Board->Active]] = monarch;
	moves[0] = j;

	if (Board->check) /* Function called for Board->Active only, so cannot castle. */
		return moves[0];

	/* need to check castling moves availability */
	kside = (Board->Active == White) ? &wkside[0] : &bkside[0];
	qside = (Board->Active == White) ? &wqside[0] : &bqside[0];
	cq = Board->cas[Board->Active*2];
	ck = Board->cas[1 + Board->Active*2];

	if ((Board->Board[kside[0]]) || (Board->Board[kside[1]]))
		ck = 0;
	if ((Board->Board[qside[0]]) || (Board->Board[qside[1]]) || (Board->Board[qside[2]]))
		cq = 0;

	if (!(ck || cq))
		return moves[0];

	/* if we still here, we have to find out whether king's square or */
	/* the squares near the king are under opponent's attack */
	/* if so, castling is unavailable for now */

	Color opc = OPPONENT(Board->Active);

	ck = ck && (!attacked_by(opc, kside[0], Board)) && (!attacked_by(opc, kside[1], Board));
	/* #42: queenside square that only rook passes MAY be under attack! */
	cq = cq && (!attacked_by(opc, qside[1], Board)) && (!attacked_by(opc, qside[2], Board));

	/* For time being, now have correct castling values in (ck), (cq). */
	if (!(ck || cq))
		return moves[0];

	/* #33 : add castling in /front/ of king moves, it can never be only move. */
	if (ck) {
		moves[2 + moves[0]++] = moves[2];
		moves[2] = from + 2;
	}

	if (cq) {
		moves[2 + moves[0]++] = moves[2 + ck];
		moves[2 + ck] = from - 2;
	}

	return moves[0];
}

/* Generates all /legal/ possible moves from current board position */
int mvs_a(uint8_t (*dst)[16][30], struct BoardState *Board, const uint8_t *pvmove)
{
	register int i, j, piececount = 0, mc = 0;
	uint8_t pvfrom = pvmove ? pvmove[0] : 144;
	bool doublecheck = false;
	struct { int8_t len; int8_t sqs[8]; } capsblocks;
	capsblocks.len = 0;

	if (Board->check) {
		doublecheck = is_double_check(Board);
		if (doublecheck) {
			mc += mvs_k(&dst[0][piececount++], Board, Board->king[Board->Active]);
			/* ignoring pv */
			for (i = 1; i < 16; i++)
				dst[0][i][0] = 0;
			return mc;
		}

		int8_t checkersq = Board->check - 1;
		int8_t checker = Board->Board[checkersq];
		capsblocks.len = 1;
		capsblocks.sqs[0] = checkersq;
		if (!IS_KNIGHT(checker) && !TOUCHING(Board->king[Board->Active], checkersq)) {
			int8_t dir = cardinal8(checkersq, Board->king[Board->Active]);
			const uint8_t cardislide = vQueen[checkersq * 8 + dir] - 1;
			for (i = 1; i <= cardislide; i++) {
				checkersq += CARDINAL_ADDENDS[dir];
				if (checkersq == Board->king[Board->Active])
					break;
				capsblocks.len++;
				capsblocks.sqs[i] = checkersq;
			}
		}
	}

	for (i = 0; i < 64; i++) {
		if (IS_COLOR(Board->Active, Board->Board[i])) {
			switch (TO_WHITE(Board->Board[i])) {
				case PAWN	: mc += mvs_p(&dst[0][piececount++], Board, i);
							  break;
				case BISHOP	: mc += mvs_b(&dst[0][piececount++], Board, i);
							  break;
				case KNIGHT : mc += mvs_n(&dst[0][piececount++], Board, i);
							  break;
				case ROOK	: mc += mvs_r(&dst[0][piececount++], Board, i);
							  break;
				case QUEEN	: mc += mvs_q(&dst[0][piececount++], Board, i);
							  break;
				case KING	: mc += mvs_k(&dst[0][piececount++], Board, i);
							  break;
			}

			j = dst[0][piececount-1][0];
			if (j) { /* Filter for valid moves only. */
				Color mover = Board->Active;
				register int y, z;
				bool kingmove = Board->king[Board->Active] == i;
				uint8_t *moves = dst[0][piececount-1];
				for (y = z = 0; y < j; y++) {
					bool semisafe = (!capsblocks.len || memchr(capsblocks.sqs, moves[2+y], capsblocks.len));
					semisafe = semisafe || /* isfckep that removes check */
						((moves[2+y] == Board->epTarget) && (IS_PAWN(Board->Board[i])) &&
						memchr(capsblocks.sqs, EP_VICTIM_SQ(Board->epTarget), capsblocks.len));
					if (kingmove || (semisafe && !exposes(Board, Board->king[mover], i, moves[2+y]))) {
						if (y != z)
						  moves[2+z] = moves[2+y];
						z++;
					}
				}
				mc -= moves[0];
				mc += z;
				moves[0] = z;
			}

			if (i == pvfrom && dst[0][piececount-1][0]) {
				if (piececount > 1) { /* bring pv to front */
					uint8_t temp[30];
					memcpy(temp, dst[0][0], 2 + dst[0][0][0]);
					memcpy(dst[0], dst[0][piececount-1], 2 + dst[0][piececount-1][0]);
					memcpy(dst[0][piececount-1], temp, 2 + temp[0]);
				}

				uint8_t cnt = dst[0][0][0], pvto = pvmove[1], fix = 0;
				if (dst[0][0][2] != pvto) {
					for (j = 1; j < cnt; j++) {
						if (dst[0][0][2+j] == pvto) {
							fix = 2+j;
							break;
						}
					}
					if (fix) {
						dst[0][0][fix] = dst[0][0][2];
						dst[0][0][2] = pvto;
					}
				}
			}
		}
	}
	return mc;
}

/*	Returns non-zero if given square /sq/ in /Board/ is attacked by /attacker/ --
	return value encodes square of first discovered /attacker/ + 1. */
int8_t attacked_by(Color attacker, int8_t sq, const struct BoardState *Board)
{
	const char *brd = Board->Board;
	int i, j;
	Color friendly = OPPONENT(attacker);

	/* Knights don't care about shields. */
	for (i = 0; i < mKnight[sq][0]; i++) {
		uint8_t maybe_knsq = mKnight[sq][i+1];
		if (!IS_KNIGHT(brd[maybe_knsq]) || IS_COLOR(friendly, brd[maybe_knsq]))
			continue;

		return maybe_knsq + 1;
	}

	union {
		uint64_t any_unshielded;
		uint8_t unshielded[8]; /* 0 if a) square not on board or b) is occupied by friendly piece */
	} shield;

	memcpy(shield.unshielded, vKing + (sq*8), sizeof(uint8_t) * 8);

	for (i = 0; i < 8; i++) /* nonexistant squares are shielded, so will not be accessed */
		shield.unshielded[i] = shield.unshielded[i] && !IS_COLOR(friendly, brd[sq + CARDINAL_ADDENDS[i]]);

	if (!shield.any_unshielded)
		return 0;

	/* vectors : n nw w sw s se e ne */
	for (i = 0; i < 8; i++) {
		if (!shield.unshielded[i])
			continue;

		int8_t fear_sq = sq;
		for (j = 1; j <= vQueen[sq * 8 + i]; j++) { /* Queen vector covers all directions. */
			fear_sq += CARDINAL_ADDENDS[i];
			char fear_sq_pc = brd[fear_sq];
			if (NONE == fear_sq_pc)
				continue;
			else if (IS_COLOR(friendly, fear_sq_pc) || IS_KNIGHT(fear_sq_pc))
				break;
			else {
				if (IS_PAWN(fear_sq_pc)) {
					union ub16 pcaps = pawn_cap_sqs(attacker, fear_sq);
					if (sq == pcaps.u8[0] || sq == pcaps.u8[1])
						return fear_sq + 1;
					else
						break;
				} else if (IS_QUEEN(fear_sq_pc)) {
					return fear_sq + 1;
				} else if (IS_ROOK(fear_sq_pc)) {
					if (!(i % 2))
						return fear_sq + 1;
					else
						break;
				} else if (IS_BISHOP(fear_sq_pc)) {
					if (i % 2)
						return fear_sq + 1;
					else
						break;
				} else if (IS_KING(fear_sq_pc)) {
					if (j == 1)
						return fear_sq + 1;
					else
						break; /* Far away that it acts as shield now. */
				}
			}
		}
	}

	return 0;
}

/* Sets up standard chess initial position on /Board/. */
void init_board(struct BoardState *Board)
{
	register int i, m;
	char *brd = Board->Board;
	memset(brd, NONE, sizeof(char) * 64);

	brd[0] = brd[7] = ROOK, brd[56] = brd[63] = TO_BLACK(ROOK);
	brd[1] = brd[6] = KNIGHT, brd[57] = brd[62] = TO_BLACK(KNIGHT);
	brd[2] = brd[5] = BISHOP, brd[58] = brd[61] = TO_BLACK(BISHOP);
	brd[3] = QUEEN, brd[4] = KING;
	brd[59] = TO_BLACK(QUEEN), brd[60] = TO_BLACK(KING);

	for (i = 0; i < 8; i++)
		brd[8 + i] = PAWN, brd[48+i] = TO_BLACK(PAWN);

	Board->Active = White;
	Board->CWK = Board->CWQ = Board->CBK = Board->CBQ = 1;
	Board->Moves = 1, Board->iMoves = 0;
	Board->pCount[White] = Board->pCount[Black] = 16;

	m = MATVAL[KING] + MATVAL[QUEEN] + 2*(MATVAL[ROOK] + MATVAL[BISHOP] + MATVAL[KNIGHT]) + 8*MATVAL[PAWN];
	Board->material[White] = Board->material[Black] = m;
	Board->pbonus[White] = Board->pbonus[Black] = mKnight[1][0] + mKnight[6][0];
	Board->king[White] = 4, Board->king[Black] = 60;
	Board->check = Board->epTarget = 0;
}

static inline int materialVal(char c) {
	return MATVAL[TO_WHITE(c)];
}

/* move _from_ square from _to_ square to is expected 2b correct */
/* this is a routine that is used when automagically generating */
/* new positions or applying player move. */
struct MoveInfo Move(struct BoardState *Board, uint8_t from, uint8_t toenc)
{
	struct MoveInfo mi;
	uint8_t to = PROMOTIONLESS(toenc);

	assert(NONE != Board->Board[from]);
	assert(Board->Active == (IS_BLACK((uint8_t)Board->Board[from])));

	/* get values that we can determine straight away */
	mi.piece = Board->Board[from];
	mi.piecepos = from, mi.epiecepos = to;
	memcpy(mi.cas, Board->cas, sizeof(uint8_t)*4); /* #51 */

	mi.epTarget = Board->epTarget;
	mi.iMoves = Board->iMoves;

	mi.captured = 0;	/* if move was capture, we update it later */
	mi.check = Board->check;

	Board->check = atkexp(Board, Board->king[OPPONENT(Board->Active)], from, toenc);

	Board->epTarget = 0;
	/* take care of the nasty pawns capturing en passant */
	if ((IS_PAWN(mi.piece)) && (mi.epTarget != 0 && to == mi.epTarget) && (Board->Board[to] == NONE)) {
		mi.captured = (Board->Active == White) ? TO_BLACK(PAWN) : PAWN;
		mi.capturedpos = (Board->Active == White) ? to - 8 : to + 8;
	} else if (Board->Board[to] != NONE) {
		mi.captured = Board->Board[to];
		mi.capturedpos = to;
	}

	/* start updating the Board */
	if ((IS_PAWN(mi.piece)) && (abs(from - to) == 16))
		Board->epTarget = (from - to) < 0 ? from + 8 : from - 8;
	else if (IS_KNIGHT(mi.piece))
		Board->pbonus[Board->Active] += ((int)mKnight[to][0] - mKnight[from][0]);
	else if (IS_KING(mi.piece)) {
		switch (from - to) { /* if castling, have to move the rook */
			case -2 : Board->Board[to - 1] = Board->Board[from + 3];
					  Board->Board[from + 3] = NONE;
					  break; /* kingside */
			case +2 : Board->Board[to + 1] = Board->Board[from - 4];
					  Board->Board[from - 4] = NONE;
					  break; /* queenside */
		}
		Board->king[Board->Active] = to;
	}

	if (mi.captured) {
		Color opc = OPPONENT(Board->Active);
		Board->pCount[opc]--;
		Board->material[opc] -= materialVal(mi.captured);
		if (IS_KNIGHT(mi.captured))
			Board->pbonus[opc] -= (int)mKnight[to][0];

		Board->Board[(int)mi.capturedpos] = NONE;
		Board->iMoves = 0;
	} else if (IS_PAWN(mi.piece)) {
		Board->iMoves = 0;
	} else {
		Board->iMoves++;
	}

	Board->Board[to] = mi.piece;
	Board->Board[from] = NONE;

	Board->CWQ = Board->CWQ && (from != 4 && (from != 0 && to != 0));
	Board->CWK = Board->CWK && (from != 4 && (from != 7 && to != 7));
	Board->CBQ = Board->CBQ && (from != 60 && (from != 56 && to != 56));
	Board->CBK = Board->CBK && (from != 60 && (from != 63 && to != 63));

	/* promotions */
	if ((IS_PAWN(mi.piece)) && ((to >= 0 && to <= 7) || (to >= 56 && to <= 63))) {
		uint8_t promo_piece = PROMO_DECODE(toenc);
		Board->Board[to] = mi.piece + promo_piece - 1;
		Board->material[Board->Active] += (materialVal(promo_piece) - materialVal(PAWN));
		/* !Promotions to knight are left without positional bonus. */
	}

	Board->Moves += (Board->Active);
	Board->Active = OPPONENT(Board->Active);

	return mi;
}

void undo_move(struct BoardState *Board, const struct MoveInfo *mInfo)
{
	/* Subtract promotion. Promotion to knight is left without bonus here. */
	Board->material[IS_BLACK(mInfo->piece)] +=
		materialVal(mInfo->piece) - materialVal(Board->Board[(int)mInfo->epiecepos]);

	Board->Board[(int)mInfo->piecepos] = mInfo->piece;

	if (IS_KING(mInfo->piece)) {
		if (abs(mInfo->piecepos - mInfo->epiecepos) == 2) {
			/* Last move was castling (duuh) */
			switch (mInfo->piecepos - mInfo->epiecepos) {
				case -2: Board->Board[(int)mInfo->piecepos + 3] = Board->Board[(int)mInfo->epiecepos - 1];
						 Board->Board[(int)mInfo->epiecepos - 1] = NONE;
						 break;
				case +2: Board->Board[(int)mInfo->piecepos - 4] = Board->Board[(int)mInfo->epiecepos + 1];
						 Board->Board[(int)mInfo->epiecepos + 1] = NONE;
						 break;
			}
		}
		Board->king[IS_BLACK(mInfo->piece)] = mInfo->piecepos;
	} else if (IS_KNIGHT(mInfo->piece))
		Board->pbonus[IS_BLACK(mInfo->piece)] += ((int)mKnight[(int)mInfo->piecepos][0] - mKnight[(int)mInfo->epiecepos][0]);

	Board->Board[(int)mInfo->epiecepos] = NONE;
	
	if (mInfo->captured) {
		Board->Board[(int)mInfo->capturedpos] = mInfo->captured;
		Board->pCount[IS_BLACK(mInfo->captured)]++;
		Board->material[IS_BLACK(mInfo->captured)] += materialVal(mInfo->captured);
		if (IS_KNIGHT(mInfo->captured)) {
			Board->pbonus[IS_BLACK(mInfo->captured)] += mKnight[(int)mInfo->epiecepos][0];
		}
	}
	
	memcpy(Board->cas, mInfo->cas, sizeof(uint8_t)*4); /* #51 */
	Board->epTarget = mInfo->epTarget;
	Board->iMoves = mInfo->iMoves;
	Board->check = mInfo->check;

	if (Board->Active == White) {
		Board->Active = Black;
		Board->Moves--;
	} else
		Board->Active = White;
}

/* Finds if king of Color color is in check. */
int is_check(const struct BoardState *Board, Color color)
{
	uint8_t kingpos = Board->king[color];

	return attacked_by(OPPONENT(color), kingpos, Board);
}

/* Checks if given move can be made (is legal). */
int validate_move(struct BoardState *Board, const struct MoveCoords *coords)
{
	uint8_t moves[30], valid_moves[30];

	if (!IS_COLOR(Board->Active, Board->Board[coords->from]))
		return INVALID;

	switch (TO_WHITE(Board->Board[(int)coords->from])) {
		case PAWN	: mvs_p(&moves, Board, coords->from);
				   	  break;
		case BISHOP : mvs_b(&moves, Board, coords->from);
					  break;
		case KNIGHT : mvs_n(&moves, Board, coords->from);
					  break;
		case ROOK 	: mvs_r(&moves, Board, coords->from);
					  break;
		case QUEEN	: mvs_q(&moves, Board, coords->from);
					  break;
		case KING	:
						mvs_s(&moves, Board, coords->from, &vKing[0]);
						mvs_k(&valid_moves, Board, coords->from);
						memcpy(moves+moves[0]+2, valid_moves+2, valid_moves[0]);
						moves[0] += valid_moves[0];
						break;
	}

	if (memchr(moves + 2, coords->to, moves[0])) {
		bool check0 = is_check(Board, Board->Active);
		struct MoveInfo mInfo = Move(Board, coords->from, PROMO_ENCODE(QUEEN, coords->to));
		bool check1 = is_check(Board, OPPONENT(Board->Active));
		undo_move(Board, &mInfo);
		if (check1) {
			return check0 ? LEAVING_IN_CHECK : PLACING_IN_CHECK;
		}
		return VALID;
	}

	return INVALID;
}

/* is active color in a checkmate ? */
int checkmate(struct BoardState *Board)
{
	uint8_t moves[16][30];
	return is_check(Board, Board->Active) && !mvs_a(&moves, Board, NULL);
}

int stalemate(struct BoardState *Board)
{
	uint8_t moves[16][30];
	return !is_check(Board, Board->Active) && !mvs_a(&moves, Board, NULL);
}

bool has_move_from_p(const struct BoardState *bs, uint8_t sq)
{
	/* unfortunate pawn stuff. no captures considered. */
	int8_t pawndir = bs->Active ? -8 : 8;
	return (bs->Board[sq + pawndir] == NONE);
}

bool has_move_from_n(const struct BoardState *bs, uint8_t sq)
{
	int i;

	const uint8_t *nm = mKnight[sq];

	for (i = 1; i <= nm[0]; i++) {
		if (bs->Board[nm[i]] == NONE)
			return true;
	}

	return false;
}

bool has_move_from_b(const struct BoardState *bs, uint8_t sq)
{
	int i;
	const uint8_t *v = vBishop + (sq * 8);

	for (i = 1; i < 8; i+=2) {
		if (!(*(v+i)))
			continue;
		if (bs->Board[sq + CARDINAL_ADDENDS[i]] == NONE)
			return true;
	}

	return false;
}

bool has_move_from_r(const struct BoardState *bs, uint8_t sq)
{
	int i;
	const uint8_t *v = vRook + (sq * 8);

	for (i = 0; i < 8; i+=2) {
		if (!(*(v+i)))
			continue;
		if (bs->Board[sq + CARDINAL_ADDENDS[i]] == NONE)
			return true;
	}

	return false;
}

bool has_move_from_q(const struct BoardState *bs, uint8_t sq)
{
	int i;
	const uint8_t *v = vQueen + (sq * 8);

	for (i = 0; i < 8; i++) {
		if (!(*(v+i)))
			continue;
		if (bs->Board[sq + CARDINAL_ADDENDS[i]] == NONE)
			return true;
	}

	return false;
}

bool (*hmfs[6])(const struct BoardState *, uint8_t) = {
	NULL,
	has_move_from_p,
	has_move_from_b,
	has_move_from_n,
	has_move_from_r,
	has_move_from_q
};

/* This routine should not be used with kings. */
bool has_move_from(const struct BoardState *bs, uint8_t sq)
{
	int8_t pc = bs->Board[sq];

	return hmfs[TO_WHITE(pc)](bs, sq);
}

bool safemove_fast(struct BoardState *bs)
{
	int r, f;
	uint8_t king_sq = bs->king[bs->Active];

	uint8_t rk = BOARD_RANK(king_sq);
	uint8_t fk = BOARD_FILE(king_sq);
	for (r = 0; r < 8; r++) {
		if (rk == r)
			continue;

		uint8_t sq = r * 8;
		uint8_t rd = abs(rk - r);

		for (f = 0; f < 8; f++, sq++) {
			if (f != fk && rd != abs(fk - f)) {
				if (IS_COLOR(bs->Active, bs->Board[sq])) {
						if (has_move_from(bs, sq))
							return true;
				}
			}
		}
	}

	return false;
}

bool king_safemove_fast(struct BoardState *bs)
{
	uint8_t moves[30];
	register int i;
	bool r = false;

	mvs_s(&moves, bs, bs->king[bs->Active], &vKing[0]);
	/* Remove king to not get it to /block/ ongoing attack to its desired target square */
	int8_t monarch = bs->Board[bs->king[bs->Active]];
	bs->Board[bs->king[bs->Active]] = NONE;
	for (i = 0; i < moves[0]; i++) {
		if (!attacked_by(OPPONENT(bs->Active), moves[2 + i], bs)) {
			r = true;
			goto restore_monarchy;
		}
	}

restore_monarchy:
	bs->Board[bs->king[bs->Active]] = monarch;

	return r;
}

/* /Fast/ check for check or stalemate: 0 - neither, 1 - checkmate, 2 - stalemate. */
int check_or_stalemate(struct BoardState *bs)
{
	uint8_t moves[16][30];
	if (bs->check) {
		if (king_safemove_fast(bs))
			return 0;

		return !mvs_a(&moves, bs, NULL);
	}

	if (safemove_fast(bs) || king_safemove_fast(bs))
		return 0;

	return !mvs_a(&moves, bs, NULL) ? 2 : 0;
}

bool same_position(const struct BoardState *c, const struct BoardState *p) {
	/* repetition requires more than matching board -- #15 */
	return  (c && p) &&
		(c->Active == p->Active) &&
		(c->CWK == p->CWK) && (c->CWQ == p->CWQ) &&
		(c->CBK == p->CBK) && (c->CBQ == p->CBQ) &&
		(c->epTarget == p->epTarget) &&
		!memcmp(c->Board, p->Board, 64);
};

/* search for three-fold (or more) position repetitions */
int repetition(const struct BoardStateList *current) {
	if (current->State.iMoves > 4) {
		int repCount = 1;
		struct BoardStateList *prb = current->LastBoard;
		while (prb && repCount < 3) {
			if (same_position(&(current->State), &(prb->State)))
				repCount++;
			prb = prb->LastBoard;
		}

		if (repCount >= 3)
			return repCount;
	}

	return 0;
}

/* #10 detect insufficient mating material, the long way. */
bool insufficient_material(const struct BoardState *bs)
{
	int pct, matd;
	if ((pct = bs->pCount[White] + bs->pCount[Black]) > 4)
		return false;

	/* This should not be used from scoring methods w/o kings. */
	assert(pct > 1);
	assert(bs->pCount[White]);
	assert(bs->pCount[Black]);

	matd = abs(bs->material[White] - bs->material[Black]);
	if (pct == 2 && matd == 0)
		return true;

	if (pct == 3 && matd < MATVAL[ROOK]) {
		int pawnpos = -1, i = 0;
		for (i = 0; i < 64; i++)
			if (IS_PAWN(bs->Board[i])) {
				pawnpos = i;
				break;
			}

		if (pawnpos == -1)
			return true;
	}

	if (pct == 4 && matd == 0) { /* is this KBKB with same-color bishops? */
		int bishops[2] = { -100, -100 }, i, c = 0;
		for (i = 0; i < 64; i++) {
			if (IS_BISHOP(bs->Board[i]))
				bishops[c++] = i;
		}

		if ((c == 2) && SAME_COLOR_SQS(bishops[0], bishops[1]))
			return true;
	}

	return false;
}

int8_t cardinal8(int8_t from, int8_t to)
{
	int fr = BOARD_RANK(from), tr = BOARD_RANK(to);
	int rd = (tr > fr) - (tr < fr) + 1;
	int ff = BOARD_FILE(from), tf = BOARD_FILE(to);
	int fd = (tf > ff) - (tf < ff) + 1;

	assert(((fr == tr) || (ff == tf)) || (abs(fr - tr) == abs(ff - tf))); /* STR8 || DIAGONAL */

	int ternary = rd*3+fd*1;

	int8_t r = 0;
	/* This conversion is necessary until vector directions are reworked. */
	switch(ternary) {
		case 7: r = 0; break;
		case 6: r = 1; break;
		case 3: r = 2; break;
		case 0: r = 3; break;
		case 1: r = 4; break;
		case 2: r = 5; break;
		case 5: r = 6; break;
		case 8: r = 7; break;
		default:
			fprintf(stderr, "cannot happen in slide_direction %d out of (from=%d, to=%d)\n", ternary, from, to);
			exit(0);
	}

	return r;
}

bool is_double_check(struct BoardState *bs)
{
	int8_t checkersq = bs->check;
	if (!(checkersq--))
		return false;

	DECOY(bs->Board[checkersq]);
	bool doublecheck = attacked_by(OPPONENT(bs->Active), bs->king[bs->Active], bs);
	DECOY(bs->Board[checkersq]);

	return doublecheck;
}

bool is_knight_move(int8_t from, int8_t to)
{
	return (NULL != memchr(mKnight[from] + 1, to, mKnight[from][0]));
}

/* Finds and returns first occupied coordinate (+1) in given direction from /sq/ or 0. */
int8_t dirfst(const struct BoardState *bs, int8_t cardinal, int8_t sq)
{
	const uint8_t cardislide = vQueen[sq * 8 + cardinal];
	uint8_t i;

	for (i = 1; i <= cardislide; i++) {
		if (bs->Board[sq += CARDINAL_ADDENDS[cardinal]])
			return sq + 1;
	}

	return 0;
}

/*	If /to/ is attacked from given direction, returns attacker coordinate + 1, otherwise 0. */
int8_t diratk(const struct BoardState *bs, int8_t cardinal, int8_t sq)
{
	int8_t sqpc = bs->Board[sq];
	int8_t opposite = (cardinal + 4) % 8; /* That will cease to work after refactor ... */
	int8_t first = dirfst(bs, opposite, sq);
	if (!first)
		return 0;

	int8_t first_pc = bs->Board[first-1];
	if (IS_KNIGHT(first_pc)) /* No attacks (inter)cardinal */
		return 0;

	if ((sqpc != NONE) && (FRIENDLY(sqpc, first_pc))) /* Need opposite colors (or emptiness at sq).*/
		return 0;

	bool touching = TOUCHING(sq, first-1);
	if (touching && (IS_KING(first_pc) || IS_QUEEN(first_pc)))
		return first;

	if (IS_KING(first_pc))
		return 0; /* No remote attacks. */

	if (IS_PAWN(first_pc)) {
		if (!touching)
			return 0;

		union ub16 pcaps = pawn_cap_sqs(IS_BLACK(((uint8_t)first_pc)), first - 1);
		return (sq == pcaps.u8[0] || sq == pcaps.u8[1]) ? first : 0;
	}

	/* Sliders remaining */
	const uint8_t *v = IS_ROOK(first_pc) ? vRook : (IS_QUEEN(first_pc) ? vQueen : vBishop);
	return v[(first-1) * 8 + cardinal] ? first : 0;
}

/* If move from->to is from the same side as piece on tsq (if there is piece there) and
   EXPOSES tsq to attack from opposing side, returns attacker square + 1. */
int8_t exposes(struct BoardState *bs, const int8_t tsq, const uint8_t from, const uint8_t toenc)
{
	uint8_t to = PROMOTIONLESS(toenc);
	if (!IS_CARDINAL(from, tsq))
		return 0;

	int8_t expdir = cardinal8(from, tsq);
	if (IS_CARDINAL(to, tsq) && expdir == cardinal8(to, tsq))
		return 0;

	/* Mutation, exposure check, restoration. */
	char pc = bs->Board[from];
	char ep_victim = NONE;
	bs->Board[from] = NONE;

	if (IS_PAWN(pc) && to == bs->epTarget) { /* #98, GH#7 */
		int8_t epkill_sq = EP_VICTIM_SQ(bs->epTarget);
		if (IS_CARDINAL(epkill_sq, tsq)) {
			ep_victim = bs->Board[epkill_sq];
			bs->Board[epkill_sq] = NONE;
		}
	}

	int8_t exposure = diratk(bs, expdir, tsq);
	bs->Board[from] = pc;
	if (ep_victim)
		bs->Board[EP_VICTIM_SQ(bs->epTarget)] = ep_victim;

	return exposure;
}

/**
   Move (from -> to) attacks, continues to attack or exposes attack to /tsq/ --
   no move legality checks are done, care to not let bishop hop like knight interally. */
int8_t atkexp(struct BoardState *bs, const int8_t tsq, const uint8_t from, const uint8_t toenc)
{
	uint8_t to = PROMOTIONLESS(toenc);
	if (0 == (tsq - to))
		return 0; /* Direct capture is neither attack nor exposure. */

	if (0 == (tsq - from))
		return 0; /* Self-targeting senseless. */

	int8_t r = 0;
	const int8_t opc = bs->Board[from]; /* Original piece. */
	int8_t epc = opc; /* Effective piece after move. */
	const int8_t xpc = bs->Board[to]; /* Ex-piece or emptyness of /to/ square */
	assert(opc);
	assert(xpc == NONE || (!FRIENDLY(opc, xpc)));

	if (IS_KING(opc) && abs(from - to) == 2) { /* As usual, castling is too horrific. */
		const int8_t tpc = bs->Board[tsq];

		int8_t srooksq = (to == 2 || to == 58) ? to - 2 : to + 1;
		int8_t crooksq = (to == 2 || to == 58) ? to + 1 : to - 1;
		int8_t rook = bs->Board[srooksq];
		bs->Board[from] = bs->Board[srooksq] = NONE;
		bs->Board[to] = epc;
		bs->Board[crooksq] = rook;

		if (tpc) {
			r = attacked_by(OPPONENT(IS_BLACK(tpc)), tsq, bs);
		} else {
			/* Castling king/rook causes no attack *exposures*, only direct attacks. */
			if (!r && IS_CARDINAL(to, tsq))
				r = diratk(bs, cardinal8(to, tsq), tsq);
			int8_t crooksq = (to == 2 || to == 58) ? to + 1 : to - 1;
			if (!r && IS_CARDINAL(crooksq, tsq))
				r = diratk(bs, cardinal8(crooksq, tsq), tsq);
		}

		bs->Board[from] = opc;
		bs->Board[to] = NONE;
		bs->Board[srooksq] = rook;
		bs->Board[crooksq] = NONE;

		return r;
	}

	/* Sneakily mutate */
	bs->Board[from] = NONE;
	if (IS_PAWN(epc)) {
		const int8_t rto = BOARD_RANK(to);
		if ((rto == 0 || rto == 7))
			epc = opc + PROMO_DECODE(toenc) - 1;
		else if (to == bs->epTarget) {
			/* E.p. affects 3 squares... */
			int8_t epkill_sq = EP_VICTIM_SQ(bs->epTarget);
			assert((bs->Board[epkill_sq] != NONE) && (!FRIENDLY(opc, bs->Board[epkill_sq])));
			if (IS_CARDINAL(epkill_sq, tsq)) {
				int8_t epkill = bs->Board[epkill_sq];
				bs->Board[epkill_sq] = NONE;
				bs->Board[to] = epc;
				r = diratk(bs, cardinal8(epkill_sq, tsq), tsq);
				bs->Board[epkill_sq] = epkill;
			}
		}
	}
	bs->Board[to] = epc;

	/* Logic inbetween, freakinknights. */
	const int8_t tpc = bs->Board[tsq];
	if (IS_KNIGHT(epc) && (!tpc || !FRIENDLY(tpc, epc))) {
		r = is_knight_move(to, tsq) ? to + 1 : 0;
	} else if (!r && IS_CARDINAL(to, tsq)) {
		int8_t cto = cardinal8(to, tsq);
		r = diratk(bs, cto, tsq); /* attack */
	}
	if (!r && IS_CARDINAL(from, tsq)) {
		r = diratk(bs, cardinal8(from, tsq), tsq); /* exposure */
	}

	/* Quietly restore. */
	bs->Board[from] = opc;
	bs->Board[to] = xpc;

	return r;
}
