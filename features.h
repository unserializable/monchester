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

#ifndef __CHESS_FEATURES_H
#define __CHESS_FEATURES_H

/**
  Whether all considered moves with their PV-s should kept for later analysis.
  Can be useful for debugging and understanding program decisions & behaviour.
  Not really useful for most end-users.
*/
#ifndef FEATURE_KEEP_ALL_PVS
  #define FEATURE_KEEP_ALL_PVS 0
#endif /* FEATURE_KEEP_ALL_PVS */

/**
   If this is enabled, game scores will be saved to file in user home folder.
   This is normally not very useful, as most interfaces save the games by
   itself. However, together with FEATURE_KEEP_ALL_PVS this allows for later
   analysis of engine decisions, as the kept PVS are saved as variations
   in the gamescore file. This feature also depends on file locking call
   'flock' that is not available on Windows & as it is duplicating interface
   functionality, it is disabled by default. */
#ifndef FEATURE_KEEP_GAMESCORES
  #define FEATURE_KEEP_GAMESCORES 0
#endif /* FEATURE_KEEP_GAMESCORES */

/**
   Since the 1.0 version is meant as fun scholastic engine, not the one to beat
   with once-discovered winning line, it is desirable to have the slight score
   randomization in effect at all times. That is already always true, when running
   under XBoard/WinBoard, but some CECP interfaces or interfaces pretending to
   be CECP interfaces (e.g. lichess-bot) do not send CECP 'random' command when
   new game is started and play becomes deterministic. Turning on this feature
   flag always forces that slight score randomization, even when CECP 'random'
   is never sent to the engine.

   Score randomization can be disabled entirely too. This is not the same as offered
   by FEATURE_FORCE_SCORE_RANDOMIZATION=0, which still allows for score randomization
   if CECP 'random' command requests it, disabling makes engine play deterministic at
   all times, regardless of interface, done by defining DISABLE_SCORE_RANDOMIZATION. */
#ifndef FEATURE_FORCE_SCORE_RANDOMIZATION /* #100, GH#3 */
  #define FEATURE_FORCE_SCORE_RANDOMIZATION 1
  #ifdef DISABLE_SCORE_RANDOMIZATION
    #error "FEATURE_FORCE_SCORE_RANDOMIZATION conflicts with DISABLE_SCORE_RANDOMIZATION"
  #endif
#else /* Forced randomization could be also set from make-time flags. */
  #ifdef DISABLE_SCORE_RANDOMIZATION
    #if FEATURE_FORCE_SCORE_RANDOMIZATION
      #error "FEATURE_FORCE_SCORE_RANDOMIZATION conflicts with DISABLE_SCORE_RANDOMIZATION"
    #endif
  #endif /* DISABLE_SCORE_RANDOMIZATION */
#endif /* FEATURE_FORCE_SCORE_RANDOMIZATION */

#endif /* __CHESS_FEATURES_H */
