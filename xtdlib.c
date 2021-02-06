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
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "xtdlib.h"	/* so that compiler errs when defs don't match decls */
#include "globals.h"
#include "brdlist.h"
#include "types.h"

long double seconds(clock_t timedelta) {
	return ((long double)timedelta)/CLOCKS_PER_SEC;
}

uintmax_t ns_to_nps(uintmax_t nodecount, long double seconds) {
	return nodecount ? (nodecount / seconds) : 0;
}

/* Converts score into score in centipawns, used e.g. in CECP. */
int32_t centipawn_score(int32_t score) {
	return (int32_t)(((double)score / MATVAL[PAWN])*100.0);
}

/* Converts score into score according to CECP conventions, normal
   scores in centipawns, mate scores:
   100000 + N for "mate in N moves", and
   -100000 - N for "mated in N moves".

   where "move" has been interpreted as "ply" internally. */
int32_t cecp_score(int32_t score) {
	if (!IS_MATESCORE(score))
		return centipawn_score(score);

	int32_t base = score < 0 ? -100000 : 100000;
	int32_t plies = MATESCORE_DEPTH(score);
	int32_t moves = (plies + 1) / 2;

	return base < 0 ? base - moves : base + moves;
}

/* Return program name and version cstring from heap, caller to use and free. */
char *program_name_and_version(void) {
	size_t len = strlen(PROGRAM_NAME) + strlen(" ") + strlen(PROGRAM_FULL_VERSION);
	char *r = (char *) xmalloc((1 + len) * sizeof(char));
	sprintf(r, "%s %s", PROGRAM_NAME, PROGRAM_FULL_VERSION);
	return r;
}

/* Returns uppercase PIECE character in SAN (English) -- no pawns accepted. */
char piece2SAN(char piece) {
	switch(TO_WHITE(piece)) {
		case BISHOP	: return 'B';
		case KNIGHT	: return 'N';
		case ROOK	: return 'R';
		case QUEEN	: return 'Q';
		case KING	: return 'K';
		default 	:
				  fprintf(stderr, "unrecognized piece %d\n", piece);
				  return 'Z';
	};
}

/*	Converts cstring /s/ to integer in inclusive range [/min/../max/], optionally dying with /s_desc/
	message if cstring /s/ does not fall into that range or is not numeric / decimal at all */
intmax_t to_int(const char *const s, intmax_t min, intmax_t max, bool die, bool quiet, const char *const s_desc)
{
	errno = 0;
	char *eptr;
	intmax_t n = strtol(s, &eptr, 0);
	if (errno || *eptr != '\0') {
		if (!quiet) {
			fprintf(stderr, "%s", CONVERSION_FAILED_TEXT);

			if (s_desc)
				fprintf(stderr, " %s", s_desc);

			fprintf(stderr, " '%s' to integer", s);

			if (errno)
				fprintf(stderr, " (%s)", strerror(errno));

			fputs(".\n", stderr);
			fflush(stderr);
		}


		if (die)
			exit(!errno ? EINVAL : errno);
		else if (!errno)
			errno = EINVAL;
		return INTMAX_MIN;
	}

	if (n < min || n > max) {
		if (!quiet) {
			if (s_desc)
				fprintf(stderr, "%s ", s_desc);

			fprintf(stderr, "%jd out of %jd..%jd range\n", n, min, max);
			fflush(stderr);
		}

		if (die)
			exit(!errno ? EINVAL : errno);
		else if (!errno)
			errno = EINVAL;
		return INTMAX_MIN;
	}

	return n;
}

/* Returns move coordinates, if /s/ represents move in lowercase algebraic notation.
 Makes no attempt to test move legality (e.g. a1d3 is considered alright).
 Promotion piece suffix may be at the end of /s/, it is accepted only if
 target rank is promotion rank and source rank is rank before promotion rank. */
struct MoveCoords *parsed_move(const char *s)
{
	uint8_t promotion = 0;

	/* coordinate notation ("e2e4"), promotion piece at the end, ("e7d8q") */
	if (strlen(s) == 4 || strlen(s) == 5) {
		if ((!((s[0] >= 'a') && (s[0] <= 'h')))
		   || (!((s[1] >= '1') && (s[1] <= '8')))
		   || (!((s[2] >= 'a') && (s[2] <= 'h')))
		   || (!((s[3] >= '1') && (s[3] <= '8'))))
			return NULL;
		/* else */
		if (strlen(s) == 5) {
			if (!(s[4] == 'q' || s[4] == 'r' || s[4] == 'b' || s[4] == 'n'))
				return NULL;
			promotion = s[4];
		}

		uint8_t from = (s[1] - '1') * 8 + (s[0] - 'a');
		uint8_t to = (s[3] - '1') * 8 + (s[2] - 'a');

		if (from == to)
			return NULL;

		if (promotion) {
			if (!((s[1] == '2' && s[3] == '1') || (s[1] == '7' && s[3] == '8')))
				return NULL;
			int dx = abs(from - to);
			if (!(dx >= 7 && dx <= 9))
				return NULL;
		}
		struct MoveCoords *move = (struct MoveCoords *) xmalloc(sizeof(struct MoveCoords));
		move->from = from, move->to = to;

		return move;
	}
	return NULL;
}

/* Fills in EngineMove /em/ algebraic representation to /dest/ when move made from board /bs/. */
void to_algebraic(char *dest, struct EngineMove *em, struct BoardState* bs)
{
	uint8_t rank_to = BOARD_RANK(PROMOTIONLESS(em->mvc.to));
	bool promotion = (rank_to == 0 || rank_to == 7) && IS_PAWN(bs->Board[em->mvc.from]);

	sprintf(dest, "%s%s", SQUARES[em->mvc.from], SQUARES[PROMOTIONLESS(em->mvc.to)]);
	if (promotion) {
		dest[5] = 0;
		dest[4] = tolower(piece2SAN(PROMO_DECODE(em->mvc.to)));
	}
}

const char *UNABLE_TO_ALLOCATE_TEXT = "couldn't allocate needed memory";
const char *ZERO_ALLOCATION_TEXT = "no memory requested";

void *xmalloc(int size)
{
	void *p;
	if (size == 0) {
		fprintf(stderr, "xmalloc() - %s\n", ZERO_ALLOCATION_TEXT);
		print_boardlist_pgn(stderr, CurrentBoard);
		exit(255);
	}
	p = malloc(size);
	if (!p) {
		fprintf(stderr, "xmalloc() %s %d\n", UNABLE_TO_ALLOCATE_TEXT, size);
		print_boardlist_pgn(stderr, CurrentBoard);
		exit(255);
	}

	return p;
}

void *xcalloc(int nmemb, int size)
{
	void *p;
	if (size == 0 || nmemb == 0) {
		fprintf(stderr, "xcalloc() - %s, size = %d, n = %d\n", ZERO_ALLOCATION_TEXT, size, nmemb);
		print_boardlist_pgn(stderr, CurrentBoard);
		exit(255);
	}
	p = calloc(nmemb, size);
	if (!p) {
		fprintf(stderr, "xcalloc() %s for %d * %d\n", UNABLE_TO_ALLOCATE_TEXT, nmemb, size);
		print_boardlist_pgn(stderr, CurrentBoard);
		exit(255);
	}

	return p;
}

void xfree(void *p)
{
	/* don't tolerate freeing NULL pointer! */
	if (!p) {
		fprintf(stderr, "xfree() tried to free NULL pointer\n");
		print_boardlist_pgn(stderr, CurrentBoard);
		exit(255);
	} 
	free(p);
	p = NULL;
}

/* Freeing engine move is somewhat involved, separate routine, NULL safe. */
void xfree_engine_move(struct EngineMove *em) {
	if (em) {
		if (em->pv && em->pvd)
			xfree(em->pv);
#if FEATURE_KEEP_ALL_PVS
			if (em->alternates) {
				int i;
				for (i = 0; i < em->alt_count; i++)
					xfree(em->alternates[i].pv);
				xfree(em->alternates);
				xfree(em->altscores);
			}
#endif /* FEATURE_KEEP_ALL_PVS */
		xfree(em);
	}
}

void *xrealloc(void *p, int size)
{
	if (size == 0) {
		fprintf(stderr, "xrealloc() - %s, size = %d\n", ZERO_ALLOCATION_TEXT, size);
		print_boardlist_pgn(stderr, CurrentBoard);
		exit(255);
	}
	p = realloc(p, size);
	if (!p) {
		fprintf(stderr, "xrealloc() %s %d\n", UNABLE_TO_ALLOCATE_TEXT, size);
		print_boardlist_pgn(stderr, CurrentBoard);
		exit(255);
	}

	return p;
}

/* Implements strdup() which is POSIX but not C standard library. */
char *xstrdup(const char *s) {
	size_t len, i;
	char *r;

	if (NULL == s)
		return NULL;

	for (len = 0; *(s + len) != 0; len++);
	r = xmalloc(sizeof(char) * (len + 1));
	for (i = 0; i < len; i++)
		*(r + i) = *(s + i);
	*(r + len) = 0;
	return r;
}

PV *pv_init(PV *pv) {
	pv->depth = 0;
	pv->size = 24;
	pv->pv = xmalloc(pv->size * sizeof(uint8_t));
	return pv;
}

void pv_free(PV *pv) {
	xfree(pv->pv);
}

void pv_push(PV *pv, uint8_t from, uint8_t to) {
	assert(from <= 63);
	/* TODO: realloc if size is not enough ? */
	if (2*(pv->depth+1) > pv->size) {
		fprintf(stderr, "# PV size %d exceeded by push %d %d\n", (int)pv->size, (int)from, (int)to);
		int i = 0;
		fputc('#', stderr);
		for (i = 0; i < pv->depth; i++) {
			fprintf(stderr, "%s%s ", SQUARES[pv->pv[2*i]], SQUARES[pv->pv[2*i+1]]);
		}
		fputc('\n', stderr);
		fflush(stderr);
		exit(255);
	}
	pv->pv[2*(pv->depth++)] = from;
	pv->pv[2*pv->depth-1] = to;
}

void pv_remove(PV *pv) {
	if (!pv->depth) return;
	pv->depth--;
}

void pv_rewrite(PV *pv, uint8_t at_depth, uint8_t *content, uint8_t count) {
	assert(0 == (count % 2));
	assert(pv->depth >= at_depth); /* Do not allow corrupted gaps. */

	pv->depth = at_depth;
	if (!count)
		return;

	for ( ; count ; count-=2) {
		pv->pv[2*(pv->depth++)] = *(content++);
		pv->pv[2*pv->depth-1] = *(content++);
	}
}

void store(Flack *flack, uint8_t *data, int len)
{
	assert(0 == len % 2);
	assert(len > 0);
	if (len > flack->size) {
		flack->store = (uint8_t *) xrealloc(flack->stack == flack->store ? NULL : flack->store, len * sizeof(uint8_t));
		flack->size = len;
	}
	flack->len = len;
	memcpy(flack->store, data, len);
}
