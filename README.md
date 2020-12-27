# Monchester

![Carved pumpkin with huge teeth and M shaped eyes, glowing from inside.](https://github.com/unserializable/monchester/raw/master/logos/monchester-logo-1-small.jpg)

Happy Halloween 2020! The origins of this little chess engine date back to
16th October 2002 and it has just recently risen from the grave. It plays
strictly on scholastic chess level in 1.0 version and demonstrates that often
the way to win is not making really good moves, but just avoiding bad ones :)

## About
This is a basic chess engine (in C) that is able to play game from console and
via CECP (XBoard) protocol. It should perform on the level suitable mostly
for scholastic chess -- always looking only 4 plys ahead and without extensions
it can be beaten in the opening, middle game and end-game :). It supports
randomization (e.g. CECP ``random`` that is sent by default when new game
starts) for varying play.

It is monstrous only so far as it has risen from the grave, not with regard
to its monstrous playing "quality". Its origins date back to 16th October 2002
when I started writing it -- however it was thrown out after about a half-month
of messing with it as my C-code ran into obscure crashes and memory errors that
I was not able to trace back at the time. I was reminded of that program by a
friend and I managed to locate its source code and went in to make a "quick bug
fix". First errors I truly located easily with better tools of 2020, but one
mysterious crash took long to track down and then I was already hooked in trying
to make "simple" fixes to the program, which contained pretty much every error
and oversight that can occur when writing chess engine from scratch:

  * It strongly preferred stalemating to checkmating -- as the king capture
    was initially allowed and stalemate situations resulted in better material
	position.
  * When it was ahead by too much and pretty much nothing threatened it, it
    never moved towards actual win -- plan was totally missing and principal
	variation remembrance badly desired.
  * Since it was totally deterministic, it managed to end up in positions with
    4 queens and 2 rooks vs single king and still not being able to checkmate.
  * While king capture was allowed, it managed to do totally mysterious move
    choices due to the fact that apparently it internally quite often managed
	to find situations where both kings were taken off the board and then the
	rest of the material battled on.

So all in all much more time was spent on this than planned :), and now the
Monchester is unleashed upon the world.

## Usage

If used from console, ``new`` command starts new game, ``resign`` resigns and
moves must be entered in full algebraic coordinates, e.g. ``e2e4``, promotions
must be denoted by following lowercase letter, e.g. ``e7e8q``.

If used as engine via CECP (XBoard) protocol, the standard engine installation
procedures should apply. In case of XBoard, it can also used from command line
simply with ``xboard -fcp location-of-monchester-binary`` (for example: ``xboard -fcp ./monchester``).

## Repository structure

In GitHub, the ``master`` branch is strictly for README and pre-compiled
binaries. Code is in branches named ``X.Y-branch`` and releases are created from
suitable tags, e.g. for 1.0 there is ``1.0-branch`` where there is ``1.0`` tag
from which source code release is created -- and when there is bugfix release
in ``1.0`` then there will be tag like ``1.0.1`` created on the same branch.

## Compilation

The program should compile under Linux and Mac OS X and has been reported to
compile under Windows using [MSYS2](https://www.msys2.org/). Precondition is
presence of ``make`` and ``gcc`` C-compiler or compatible compiler symlinked
as ``gcc`` (clang) -- in all my computers clang produces MUCH faster binary
than ``gcc``. Compiler can also be changed by passing ``make CC=some_compiler``
argument to make.

### Build (likely) fastest binary
  * ``make CC=clang clean release``

### Build with simplest command line using available ``gcc``
  * ``make clean release``
  * or simply ``make`` if there are no old files to clean

## Features
  * Full FIDE rules for standard chess (castling, en passant, check and stalemate,
  draw by repetition, 50-move draw recognition, insufficient material detection.
  * Minimax search with constant 4-ply search depth.
  * Material evaluation only, with knight placement bonuses :)
  * Nearly instant movement on modern hardware.

## Non-Features

Since reading around, I get a feel that every chess engine must have a long
list of features, so here I present a long list of non-features:

This engine does not use, feature or support: Alpha-Beta, Attack Tables, Bitboards
(rotated, magic, black magic or any other kind), Iterative Deepening, Null-Move
Heuristics / Pruning, Hash Tables, Late Move Reductions, MVV-LVA, Neural Networks,
Opening Books, ProbCut, Razoring, Static Exchange Evaluation, Texel Tuning,
Threads, Tablebases, Transposition Tables, UCI and many other things that I have
either forgotten or never known :)

## Compilation options

### Defines

#### Features

In ``features.h`` there is a list of toggleable features with user-visible effects
that can be toggled or configured at compile time. Currently there are just two,
these are both off (and probably should remain off for casual user).

* ``FEATURE_KEEP_ALL_PVS`` (OFF)
Causes engine to remember PVs for all move branches and is mostly useful for engine
debugging and development. If this and ``FEATURE_KEEP_GAMESCORES`` are both ON, then
written gamescores will include PV variations in gamescore PGN.

* ``FEATURE_KEEP_GAMESCORES`` (OFF)
Causes engine to write finished game scores to file ``.monchester.pgn`` in user's home
folder. It is not generally very useful, as CECP and adapter interfaces for CECP
provide their own game score keeping facilities. This feature also uses ``flock()``
file locking call that is not available on Windows, so enabling it should only be
done if wanting to do some development or testing on Mac/Linux or other compatible
systems.

#### Other

Randomization of play can be disabled by defining:
* ```DISABLE_SCORE_RANDOMIZATION```

at build time, e.g.

```
make EXTFLAGS="-DDISABLE_SCORE_RANDOMIZATION"
```

but this is not recommended, as non-deterministic play actually gives
Monchester much better chances in converting totally won endgames with
major pieces, that it otherwise would draw with deterministic play.

There are couple more possible defines, those interested can probably
easily find out their purpose:

* ```SCORE_RANDOM_BITS```
* ```EXTRA_VERSION```

## Thanks
  * [Henri Lakk](https://github.com/vii5ard) for reminders and MAC compilation for 0.99 preview version.
  * [Juhan](https://github.com/aasaru) and Ingrid "Daddy, it is gonna take your horsie!" Aasaru for playtesting.
  * [Günther Simon](https://rwbc-chess.de/) for first Windows compile and computer chess record-keeping.
  * [Roland Chastain](https://github.com/rchastain/) for problem reports running on CuteChess.

## Acknowledgements
This engine would not have been reanimated, if it weren't for
[XBoard](https://www.gnu.org/software/xboard/) CECP protocol implementation, a GNU project
currently managed by [Harm Geert Muller](https://home.hccnet.nl/h.g.muller/).

For game related bug-squashing, following engines proved to be nice sparring
partners, as they are relatively stable, relatively beatable and relativily compileable:
  * [N.E.G. ("Niet Erg Goed") 1.3](https://home.hccnet.nl/h.g.muller/dwnldpage.html) chess engine by Harm Geert Muller
  * [ChessPuter](https://github.com/smilesbright/ChessPuter) by Miles Bright
  * [GiuChess](https://sourceforge.net/projects/giuchess/files/giuchess/) by Giuliano Ippoliti aka JSorel
