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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "iomain.h"
#include "xtdlib.h"
#include "fen.h"
#include "types.h"
#include "globals.h"
#include "brdlist.h"
#include "move.h"  /* init_board() */

/* Reads one line from stream. */
char *getln(FILE *stream)
{
#define CHUNK 16
	char *line = NULL, *temp;
	int len = 0;
	
	*(temp = (char *) xmalloc(CHUNK * sizeof(char))) = '\0';
	while (fgets(temp, CHUNK, stream)) {
		if (line)
			line = (char *) xrealloc(line, ((len += strlen(temp)) + 1) * sizeof(char));
		else
			line = (char *) xcalloc((len = strlen(temp)) + 1, sizeof(char));
		strcat(line, temp);
		if (line[len - 1] == '\n') {
			/* chop and break :) */
			line[len - 1] = '\0';
			break;
		}
	}
	
	xfree(temp);
	return line;
#undef CHUNK
}

/* Writes ASCII representation of chessboard state /Board/ into /stream/. */
void print_board(FILE *stream, const struct BoardState *Board)
{
	register int i, j;
	char c, *fen;

	for (i = 56; i >= 0; i-=8) {
		if (g_cecp) putc('#', stream);
		fprintf(stream, "%d:  ", i / 8 + 1);
		for (j = 0; j < 8; j++)
			if (Board->Board[i+j] != NONE) {
				switch(TO_WHITE(Board->Board[i+j])) {
					case PAWN	: c = 'p';
								  break;
					case BISHOP : c = 'b';
								  break;
					case KNIGHT : c = 'n';
								  break;
					case ROOK	: c = 'r';
								  break;
					case QUEEN	: c = 'q';
								  break;
					case KING	: c = 'k';
								  break;
					default:
							  fprintf(stderr, "unknown piece %d at square %d\n", (int)Board->Board[i+j], i+j);
							  exit(255);
				};
				if (IS_BLACK(Board->Board[i+j]))
					c = toupper(c);
				fprintf(stream, "%c ", c);
			} else
				fprintf(stream, "- ");
#if DEBUG
		/* show some nifty statistics about board */
		if (i == 56)
			fprintf(stream, "      castling(W) : king = %d, queen = %d\n", Board->CWK, Board->CWQ);
		else if (i == 48)
			fprintf(stream, "      castling(B) : king = %d, queen = %d\n", Board->CBK, Board->CBQ);
		else if (i == 40)
			putc('\n', stream);
		else if (i == 32)
			fprintf(stream,
				"      pieces(W) = %d (k@%s), pieces(B) = %d (K@%s)\n",
				Board->pCount[White], SQUARES[Board->king[White]], Board->pCount[Black], SQUARES[Board->king[Black]]);
		else if (i == 24)
			fprintf(stream, "      Active Color: %s, check = %s\n", Board->Active == White ? "WHITE" : "BLACK", Board->check ? SQUARES[Board->check-1] : "NONE");
		else if (i == 16)
			fprintf(stream, "      Move #%d, plies without captures/advances %d\n", Board->Moves, Board->iMoves);
		else if (i == 8)
			fprintf(stream, "      m+b(W) = %d+%d, m+b(B) = %d+%d\n", Board->material[White], Board->pbonus[White],
							Board->material[Black], Board->pbonus[Black]);
		else if (i == 0)
			fprintf(stream, "      epTarget = %s\n", (Board->epTarget == 0) ? "NONE": SQUARES[Board->epTarget]);
#else
		putc('\n', stream);
#endif
	}
	if (g_cecp) putc('#',stream);
	fen = Board2FEN(Board);
	fprintf(stream, "%s\n%s    A B C D E F G H       %s\n", "-------------------", g_cecp ? "#" : "", fen);
	xfree(fen);
	
	fflush(stream);
}

void print_help(void)
{
	if (g_cecp)
		fputs("# ", stdout);
	printf("commands understood: new, resign, help, bench, quit\n");
	fflush(stdout);
}

void print_pv(FILE *stream, const uint8_t *pv, int depth, int score) {
	int i;
	for (i = 0; i < depth; i++) {
		fputs(SQUARES[(int)pv[i*2]], stream);
		fputs(SQUARES[PROMOTIONLESS(pv[i*2+1])], stream);
		/* As no boardstate here, unfortunately bishop promotions cannot be recognized. */
		if (pv[i*2+1] > 63)
			putc(tolower(piece2SAN(PROMO_DECODE(pv[i*2+1]))), stream);

		if (i < depth + 1)
			putc(' ', stream);
	}
	fprintf(stream, " %d\n", score);
	fflush(stream);
}

/* CECP-compliant command error reporting to stdout (#109, GH#10). */
void print_cmd_error(const char *erred_cmd, const char *error_desc)
{
	fputs("Error", stdout);
	if (error_desc)
		printf(" (%s)", error_desc);
	fputs(": ", stdout);
	fputs(erred_cmd, stdout);
	putchar('\n');

	fflush(stdout);
}

void cecp_print_pv(const PV *pv, int32_t score, clock_t clock_start)
{
	if (g_cecp && g_cecp_conf.output_thinking) {
		int i;
		clock_t clock_end = clock();
		long double stim = ((long double)clock_end-clock_start)/CLOCKS_PER_SEC;
		uintmax_t cst = stim*100;
		printf(
			"%" PRIu8 " %jd %ju %ju %" PRIu8 " %ju %ju \t",
			pv->depth,
			(intmax_t)cecp_score(score),
			cst,
			g_nodecount,
			pv->depth,
			(uintmax_t)(((1.0 / stim)*g_nodecount)/1000.0), /* kN/s */
			UINTMAX_C(0)/* No endgame tablebases */
		);

		for (i = 0; i < pv->depth; i++) {
			fputs(SQUARES[pv->pv[i*2]], stdout);
			fputs(SQUARES[PROMOTIONLESS(pv->pv[i*2+1])], stdout);
			/* As no boardstate here, unfortunately bishop promotions cannot be recognized. */
			if (pv->pv[i*2+1] > 63)
				putc(tolower(piece2SAN(PROMO_DECODE(pv->pv[i*2+1]))), stdout);

			if (i < pv->depth + 1)
				putchar(' ');
		}
		putchar('\n');

		fflush(stdout);
	}
}

void print_pgn(
	FILE *stream, struct BoardStateList *end_board, int outcome,
	const struct tm *tm, const struct PlayerInfo *white, const struct PlayerInfo *black)
{
	struct BoardState std_start;
	init_board(&std_start);

	struct BoardStateList *start_board = end_board;
	while (start_board->LastBoard)
		start_board = start_board->LastBoard;

	bool is_std_start =
		same_position(&start_board->State, &std_start) &&
		start_board->State.iMoves == std_start.iMoves &&
		start_board->State.Moves == std_start.Moves;

	const char *pgn_result = (
		outcome == UNFINISHED ? RESULT_UNFINISHED_SCORE_TEXT :
			(outcome == DRAW ? RESULT_DRAW_SCORE_TEXT :
				(outcome == BLACKLOSE ? "1-0" : "0-1"))
	);

	fprintf(stream, "[Date \"%d.%02d.%02d\"]\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
	fprintf(stream, "[%s \"%s\"]\n", COLOR_TEXT[White], white->name);
	fprintf(stream, "[%s \"%s\"]\n", COLOR_TEXT[Black], black->name);
	fprintf(stream, "[Result \"%s\"]\n", pgn_result);
	if (!is_std_start) {
		fprintf(stream, "[Setup \"1\"]\n");
		char *start_fen = Board2FEN(&start_board->State);
		fprintf(stream, "[FEN \"%s\"]\n", start_fen);
		xfree(start_fen);
	}
	fprintf(stream, "[%sType \"%s\"]\n", COLOR_TEXT[White], white->type == Computer ? "program" : "human");
	fprintf(stream, "[%sType \"%s\"]\n\n", COLOR_TEXT[Black], black->type == Computer ? "program" : "human");
	print_boardlist_pgn(stream, end_board);
	fprintf(stream, " %s\n\n", pgn_result);
	fflush(stream);
}

/* Prepares for new game of chess on console, gathers players' info from stdin. */
void init_players(void)
{
	int c, i;

	g_players[0].color = White, g_players[1].color = Black;

	for (i = 0; i <= 1; i++) {
		g_players[i].color = (Color) i;
		do {
			printf("Who will play %s, human or computer (h,c)? :", i == 0 ? "WHITE" : "BLACK");
			fflush(stdout);
			c = getchar();
		} while ((tolower(c) != 'h') && (tolower(c) != 'c'));
		/* newline will remain in buffer otherwise */
		xfree(getln(stdin));
		if (c == 'h') {
			g_players[i].type = Human;
			printf("player's name (%s) : ", i == 0 ? "WHITE" : "BLACK");
			fflush(stdout);
			g_players[i].name = getln(stdin);
		} else {
			g_players[i].type = Computer;
			g_players[i].name = program_name_and_version();
		}
	}
}
