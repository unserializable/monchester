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
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "move.h"
#include "types.h"
#include "fen.h"
#include "xtdlib.h"
#include "globals.h"

/* MAX is generous for up to 10M-1 moves and 1M-1 captureless plies. */
#define MIN_FEN_LEN 28
#define MAX_FEN_LEN 83

char* Board2FEN(const struct BoardState *bs) {
	int i, j;
	char s[MAX_FEN_LEN];
	char *p = s;
	if (NULL == bs)
		return NULL;

	for (i = 56; i >= 0; i -= 8) {
		int empty = 0;
		for (j = 0; j < 8; j++) {
			char that = bs->Board[i+j];
			if (NONE == that) {
			       empty++;
			       continue;
			}
			if (empty) *(p++) = '0' + empty;
			if (IS_PAWN(that)) {
				*(p++) = IS_WHITE(that) ? 'P' : 'p';
			} else {
				*(p++) = IS_WHITE(that) ? piece2SAN(that) : tolower(piece2SAN(that));
			}
			empty = 0;
		}
		if (empty) *(p++) = '0' + empty;
		if (i-8 >= 0) *(p++) = '/';
	}

	*(p++) = ' ';
	*(p++) = bs->Active == White ? 'w' : 'b';

	*(p++) = ' ';
	if (bs->CWQ || bs->CWK || bs->CBQ || bs->CBK) {
		if (bs->CWK) *(p++) = 'K';
		if (bs->CWQ) *(p++) = 'Q';
		if (bs->CBK) *(p++) = 'k';
		if (bs->CBQ) *(p++) = 'q';
	} else *(p++) = '-';

	*(p++) = ' ';
	if (bs->epTarget) {
		*(p++) = SQUARES[bs->epTarget][0];
		*(p++) = SQUARES[bs->epTarget][1];
	} else *(p++) = '-';

	*(p++) = ' ';
	p += snprintf(p, 4, "%d", bs->iMoves);

	*(p++) = ' ';
	p += snprintf(p, 5, "%d", bs->Moves); /* http://tom7.org/chess/longest.pdf upper bound fits :) */

	*(p++) = 0;
	return xstrdup(s);
}

struct BoardState* FEN2Board(const char *fen) {
	int fenlen;
	char *cptr;
	char *fenend;
	char *f[6];
	struct BoardState bs;
	struct BoardState *result = NULL;

	int i, j, ept;

	if (fen == NULL)
		return NULL;

	fenlen = strlen(fen);
	if (fenlen < MIN_FEN_LEN || fenlen > MAX_FEN_LEN)
		return NULL;

	strncpy(f[0] = (char*) xmalloc((fenlen + 1) * sizeof(char)), fen, fenlen + 1);
	fenend = (f[0] + fenlen);

	for (i = 1; i < 6; i++) {
		if (!(f[i] = (strchr(f[i-1], ' '))))
			goto fenerror;

		if (f[i] > fenend)
			goto fenerror;

		*(f[i]++) = 0;
	}

	for (i = 0, j = 0; f[0][i] != 0 && j < 64; i++) {
		if (isalpha(f[0][i]))
			bs.Board[j++] = f[0][i];
		else if (isdigit(f[0][i])) {
			int x = f[0][i] - '0';
			if (x > 8)
				goto fenerror;
			while (j < 64 && x) bs.Board[j++] = 0, x--;
			if (x)
				goto fenerror;
		} else if ('/' == f[0][i]) {
			if ((j % 8) != 0)
				goto fenerror;
		} else
				goto fenerror;
	}

	if (j != 64 && f[0][i] != 0)
		goto fenerror;

	bs.pCount[0] = bs.pCount[1] = 0;
	bs.material[0] = bs.material[1] = 0;
	bs.pbonus[0] = bs.pbonus[1] = 0;
	for (i = 0; i < 64; i++) {
		int pcval = 0;
		char c = bs.Board[i];
		if (c) {
			switch(tolower(c)) {
				case 'p': bs.Board[i] = isupper(c) ? PAWN : TO_BLACK(PAWN);
					break;
				case 'r': bs.Board[i] = isupper(c) ? ROOK : TO_BLACK(ROOK);
					break;
				case 'b': bs.Board[i] = isupper(c) ? BISHOP : TO_BLACK(BISHOP);
					break;
				case 'n': bs.Board[i] = isupper(c) ? KNIGHT : TO_BLACK(KNIGHT);
					break;
				case 'k': bs.Board[i] = isupper(c) ? KING : TO_BLACK(KING);
					break;
				case 'q': bs.Board[i] = isupper(c) ? QUEEN : TO_BLACK(QUEEN);
					break;

				default:
					 goto fenerror;
			}
			pcval = MATVAL[TO_WHITE(bs.Board[i])];
			bs.pCount[!isupper(c)]++;
			bs.material[!isupper(c)] += pcval;
		}
	}

	/* I misinterpreted FEN rank ordering, but
			 https://www.chessprogramming.org/Vertical_Flipping to the rescue. */
	for (i = 0; i < 32; ++i) {
		char c = bs.Board[i];
		bs.Board[i] = bs.Board[i^56];
		bs.Board[i^56] = c;
	}

	/* Be lenient, but require both kings ... (#97, GH#2) */
	int kc[2] = {0, 0};
	for (i = 0; i < 64; i++) {
		char c = bs.Board[i];
		if (IS_KNIGHT(c))
			bs.pbonus[IS_BLACK(c)] += mKnight[i][0];
		else if (IS_KING(c)) {
			bs.king[IS_BLACK(c)] = i;
			kc[IS_BLACK(c)]++;
		}
	}

	if (1 != kc[0] || 1 != kc[1])
		goto fenerror;

	/* ... and NO pawns on 1st / 8th rank. (#97, GH#2) */
	for (i = 0; i < 8; i++) {
		char p1 = bs.Board[i], p8 = bs.Board[i+56];
		if (IS_PAWN(p1) || IS_PAWN(p8))
			goto fenerror;
	}

	if (!strcmp("w", f[1])) {
		bs.Active = White;
	} else if (!strcmp("b", f[1])) {
		bs.Active = Black;
	} else
		goto fenerror;

	if (strlen(f[2]) > 4)
		goto fenerror;

	bs.CWK = (NULL != strchr(f[2], 'K'));
	bs.CWQ = (NULL != strchr(f[2], 'Q'));
	bs.CBK = (NULL != strchr(f[2], 'k'));
	bs.CBQ = (NULL != strchr(f[2], 'q'));

	if (!strcmp(f[3], "-"))
		ept = 0;
	else if (strlen(f[3]) != 2)
		goto fenerror;
	else if (f[3][1] == '3' || f[3][1] == '6') {
		ept = (int)(f[3][1] - '1') * 8 + ((int)f[3][0] - 'a');
		if (ept < 0 || ept > 64)
			goto fenerror;
	} else
		goto fenerror;
	bs.epTarget = (uint8_t) ept;

	uintmax_t ply = strtoull(f[4], &cptr, 10);
	if (*cptr != 0 || ply > INT_MAX)
		goto fenerror;

	bs.iMoves = ply;

	uintmax_t move = strtoull(f[5], &cptr, 10);
	if (*cptr != 0 || move > INT_MAX)
		goto fenerror;
	bs.Moves = move;
	bs.check = attacked_by(OPPONENT(bs.Active), bs.king[bs.Active], &bs);

	result = (struct BoardState *) xmalloc(sizeof(struct BoardState));
	*result = bs;

fenerror:
	xfree(f[0]);

	return result;
}
