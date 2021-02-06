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

#ifndef __CHESSTYPES_H
#define __CHESSTYPES_H

#include <stdbool.h>
#include <stdint.h>
#include "features.h"

/* gamestate identifiers */
#define NO_GAME 0
#define GAME_IN_PROGRESS 1
#define GAME_ENDED 2

/** Absolute value of mate score without mate distance adjustments, #60. */
#define MATESCORE_ABS INT32_C(10000000)
#define MATESCORE_WIN(plys)  (MATESCORE_ABS - (plys))
#define MATESCORE_LOSS(plys) (-(MATESCORE_ABS) + (plys))
#define IS_MATESCORE(x) (abs(x) > ((MATESCORE_ABS)/10))
#define MATESCORE_DEPTH(score) (MATESCORE_ABS - abs(score))

/* game outcome */
#define BLACKLOSE 10	/* black lost */
#define WHITELOSE 1		/* white lost */
#define DRAW 0			/* draw */
#define UNFINISHED 44   /* unknown/unfinished */

/* move legality types */
#define VALID 1
#define INVALID -1	     /* purely illegal move */
#define LEAVING_IN_CHECK -2  /* placing king in check */
#define PLACING_IN_CHECK -3  /* leaving king in check */

#define BOARD_RANK(i) ((i) / 8)
#define BOARD_FILE(i) ((i) % 8)

/* Rank/file distances of squares. */
#define BOARD_RANK_DISTANCE(a,b) (BOARD_RANK((a))-BOARD_RANK((b)))
#define BOARD_RANK_ABS_DISTANCE(a,b) (abs(BOARD_RANK_DISTANCE((a),(b))))
#define BOARD_FILE_DISTANCE(a,b) (BOARD_FILE((a))-BOARD_FILE((b)))
#define BOARD_FILE_ABS_DISTANCE(a,b) (abs(BOARD_FILE_DISTANCE((a),(b))))

/* Squares are touching if unchecked king can move between them in zero or one moves. */
#define TOUCHING(a,b) ((BOARD_RANK_ABS_DISTANCE((a),(b)) <= 1) && (BOARD_FILE_ABS_DISTANCE((a),(b)) <= 1))

#define STRAIGHT(a,b) ((BOARD_RANK((a)) == BOARD_RANK((b))) || (BOARD_FILE((a)) == BOARD_FILE((b))))
#define DIAGONAL(a,b) (BOARD_RANK_ABS_DISTANCE((a),(b)) == BOARD_FILE_ABS_DISTANCE((a),(b)))
#define IS_CARDINAL(a,b) (((a)-(b)) && (STRAIGHT((a),(b)) || DIAGONAL((a),(b))))

/* pieces */
#define NONE 0
#define PAWN 1
#define BISHOP 2
#define KNIGHT 3
#define ROOK 4
#define QUEEN 5
#define KING 6

#define PROMO_ENCODE(P, sq) ((((P) - 2) << 6) | (sq))
#define PROMO_DECODE(sq) (((sq) >> 6) + 2)
#define PROMOTIONLESS(sq) ((sq) & 63)

/* 3rd bit shows piece color: 0 - white, 1 - black */
#define TO_WHITE(c) ((c) & 7)
#define TO_BLACK(c) ((c) | 8)
#define DECOY(pc) ((pc) ^= 8) /* Change piece color, needs lvalue parameter. */
#define IS_WHITE(c) ((c) && (((c) & 8) == 0))
#define IS_BLACK(c) ((c) >> 3)
#define IS_COLOR(c,psq) (((psq) != NONE) && (c) == ((psq) >> 3))
#define OPPONENT(c) ((!(c)))
/** This macro requires PIECE arguments, empty squares will not do! */
#define FRIENDLY(pc1,pc2) (((pc1) & 8) == ((pc2) & 8))

#define SQ_COLOR(q) ((BOARD_RANK((q)) % 2) ? (BOARD_FILE((q)) % 2) : (!(BOARD_FILE((q)) % 2)))
#define SAME_COLOR_SQS(q1,q2) (SQ_COLOR((q1)) == SQ_COLOR((q2)) )

/* TO_WHITE(blah) == XYZ is both tedious and prone to cause segmentation faults ... */
#define IS_PAWN(p) (TO_WHITE((p)) == PAWN)
#define IS_BISHOP(p) (TO_WHITE((p)) == BISHOP)
#define IS_KNIGHT(p) (TO_WHITE((p)) == KNIGHT)
#define IS_ROOK(p) (TO_WHITE((p)) == ROOK)
#define IS_QUEEN(p) (TO_WHITE((p)) == QUEEN)
#define IS_KING(p) (TO_WHITE((p)) == KING)

/* These are bit more effective than IS_COLOR && IS_XYZ combination. */
#define IS_COLOR_PAWN(c,p) (IS_PAWN((p)) && ((c) == ((p) >> 3)))
#define IS_COLOR_BISHOP(c,p) (IS_BISHOP((p)) && ((c) == ((p) >> 3)))
#define IS_COLOR_KNIGHT(c,p) (IS_KNIGHT((p)) && ((c) == ((p) >> 3)))
#define IS_COLOR_ROOK(c,p) (IS_ROOK((p)) && ((c) == ((p) >> 3)))
#define IS_COLOR_QUEEN(c,p) (IS_QUEEN((p)) && ((c) == ((p) >> 3)))
#define IS_COLOR_KING(c,p) (IS_KING((p)) && ((c) == ((p) >> 3)))

#define EP_VICTIM_SQ(ep_target_sq) ((ep_target_sq) + ((BOARD_RANK((ep_target_sq)) == 2) ? 8 : -8))

/* usage: boardState.CWQ, boardState->CWQ etc  */
#define CWQ cas[0] /**< White queenside castling availability. */
#define CWK cas[1] /**< White kingside castling availability. */
#define CBQ cas[2] /**< Black queenside castling availability. */
#define CBK cas[3] /**< Black kingside castling availability. */

/** 16 bit union */
union ub16 {
	uint16_t	u16;
	uint8_t		u8[2];
	int16_t		i16;
	int8_t		i8[2];
};

typedef enum { White = 0, Black = 1 } Color;
typedef enum { Computer, Human } PlayerType;

/** Stores single principal variation. */
typedef struct {
	uint8_t size; /**< Bytes available for PV storage. */
	uint8_t depth;/**< Current number of moves in PV. */
	uint8_t *pv;/**< PV storage, current byte-length will be twice the depth. */
} PV;

typedef struct {
	uint8_t *stack;
	uint8_t *store;
	int len;
	int size;
} Flack;

struct PlayerInfo {
	Color color;
	PlayerType type;
	char *name;
};

/** Single game board state representation. */
struct BoardState {
	char Board[64];		/**< Simplest imaginable chessboard representation. */
	uint8_t cas[4];		/**< Castling availability booleans, white king/queen black king/queen. */
	Color Active;		/**< Active color on the move. */
	int32_t material[2];/**< Material values for white and black. */
	int32_t pbonus[2];	/**< Positional bonuses for white and black. */
	uint16_t iMoves;	/**< Number of plies without captures or pawn advances. */
	uint16_t Moves;		/**< Number of full moves. */
	uint8_t epTarget;	/**< Possible en passant target square, 0 = NONE. */
	uint8_t pCount[2];	/**< Number of white/black pieces. */
	uint8_t king[2];	/**< Position of white/black king. */
	uint8_t check;		/**< Non-zero if active color in check, value is square of ONE checker +1! */
};

/** Board state companion encapsulating how that state was reached and thus makes it reversible. */
struct MoveInfo {
	char piece;		/* moved piece */
	char piecepos;		/* where it stood before move */
	char epiecepos;		/* and where it was placed after */
	char captured;		/* captured piece, 0 = NONE */
	char capturedpos;	/* and it's position before capture */

	uint8_t cas[4];		/* castling availability booleans */
	char epTarget;
	
	uint16_t iMoves;
	uint8_t check;
};

/** Represents single move coordinates, with promotion encoded in target square. */
struct MoveCoords {
	uint8_t from;/**< Contains pure start coordinate. */
	uint8_t to;	/**< Upper two bits for promotion, lower bits contain end coordinate. */
};

struct EngineMove {
	struct MoveCoords mvc;
	uint8_t pvd;	/* Depth of principal variation. */
	uint8_t *pv;	/* Principal variation. */
#if FEATURE_KEEP_ALL_PVS
	uint8_t alt_count;	/* Count of move alternatives */
	PV *alternates;	/* PVs for all alternatives. */
	int32_t *altscores; /* Scores for all alternatives. */
#endif /* FEATURE_KEEP_ALL_PVS */
};

/** List of board states forms a full game. */
struct BoardStateList {
	struct BoardState State; /**< Single game board state. */
	struct EngineMove *epv; /**< More last move details if it was engine move. */
	char from, to;	/**< Last moves coordinates (from=to=0 on _first_ move). */
	struct BoardStateList *LastBoard; /**< Link to previous board state in a game. */
	struct BoardStateList *pocc; /**< previous occurrence of kept board state, if any */
	bool rsc;	/**< true if board state duplications have occured since last capture */
};

struct EngineSettings {
	uint8_t depth_default;
	uint8_t depth_max;
};

struct CecpSettings {
	bool randomize_moves;
	bool ponder;
	bool output_thinking;

	/* PRNG state information that is broken into two pieces due to woes #31 */
	uint32_t minstd;
	uint32_t minstd_last;

	/* Information about opponent that CECP gives, if all is well. */
	bool opp_computer;/**< True if CECP has notified that opponent is an engine. */
	char *opp_name;	/**< Contains opponent name, if CECP has sent it. */
};

struct TimeControl {
	int32_t time_left;	/* absolute maximum time to spend on next move, in ms */
};

#endif
