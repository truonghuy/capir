/*
 * Copyright (c) 2012 Truong-Huy D. Nguyen.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Public License v3.0
 * which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/gpl.html
 * 
 * Contributors:
 *     Truong-Huy D. Nguyen - initial API and implementation
 */



#ifndef __GB_GHOST_H
#define __GB_GHOST_H

#include "Monster.h"

class Maze;

/**
 A GB_Ghost has 5 move actions (unchanged, east, west, north, south), 0 special action, and 1 property.
 This ghost chases the nearest sheep if not within visionLimit of players. If there's no sheep, it'll
 start running away from players from visionLimit + startRunDistance.
 */

class GB_Ghost: public Monster {
private:
	//double probChaseSheep;
	int startRunDistance;
	static const int KickTheSheep = 5;
	static const int MaxGhostHP = 2;

public:

	// properties has 1 item: hitpoint
	GB_Ghost(long x, long y, Maze *maze) :
		Monster(x, y, 5, 1, 1, maze) {
		initialMaxValues();
		//OptimalProb = 0.90;
		OptimalProb = 1.0;
		startRunDistance = 2;
		//probChaseSheep = 0.8;
	}
	;
	void initialMaxValues() {
		maxValues[0] = MaxGhostHP; // max value for HP
		properties[0] = MaxGhostHP; // initially every ghost has 3 HPs.
		propertyNames[0] = "hitpoints";
	}
	;

	/**
	 * When Ghost gets shot by a human or ai in its sight, it reduces its HP by one.
	 * */
	void absGetAffectedByPlayersActions(
	    std::vector<std::pair<AbstractState, double> >& input, long humanAct,
	    long aiAct, std::vector<std::pair<AbstractState, double> >& output);
	/**
	 * If the human shoots a ghost, its HP decreases by 1.
	 * */
	void getAffectedByPlayersActions(State& state, long worldNum, long humanAct,
	    long aiAct, RandSource& randSource);

	bool canReact(const AbstractState& absState);
	bool canReact(const State& state, long worldNum);

	/**
	 * Ghost will run away if they are within visionLimit away from players.
	 * */
	/***************** SheepSavior specific:
	 * Ghost goes for sheep when not seen by any body*************************/
	void getActNoneSeen(const State& state, long worldNum,
	    std::vector<std::pair<long, double> >& action_prob);

	void priorityActions(const State& state, long worldNum,
			    std::vector<std::pair<long, double> >& action_prob);

	// Kick the sheep's ass
	void executeSpecialAct(State& state, long act, long worldNum);

	// Run away from human while avoiding the dog
	void runAwayFromHuman(const State& state, long worldNum,
	    std::vector<std::pair<long, double> >& action_prob);


};

#endif
