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

/* 
  Whether all considered moves with their PV-s should kept for later analysis.
  Can be useful for debugging and understanding program decisions & behaviour.
  Not really useful for most end-users.
*/
#ifndef FEATURE_KEEP_ALL_PVS
  #define FEATURE_KEEP_ALL_PVS 0
#endif /* FEATURE_KEEP_ALL_PVS */

/* If this is enabled, game scores will be saved to file in user home folder.
   This is normally not very useful, as most interfaces save the games by
   itself. However, together with FEATURE_KEEP_ALL_PVS this allows for later
   analysis of engine decisions, as the kept PVS are saved as variations
   in the gamescore file. This feature also depends on file locking call
   'flock' that is not available on Windows & as it is duplicating interface
   functionality, it is disabled by default. */
#ifndef FEATURE_KEEP_GAMESCORES
  #define FEATURE_KEEP_GAMESCORES 0
#endif /* FEATURE_KEEP_GAMESCORES */

#endif /* __CHESS_FEATURES_H */
