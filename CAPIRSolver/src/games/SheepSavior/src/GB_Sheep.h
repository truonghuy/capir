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



#ifndef __GB_SHEEP_H
#define __GB_SHEEP_H

#include "Monster.h"

/**
 A GB_Sheep has 5 move actions (unchanged, east, west, north, south) and no special action or property.
 It has an OptimalProb of 1.0 to remove any random actions.
 */

class GB_Sheep: public Monster {
public:
	//double probRunFromGhost;
	double distanceToRun;
public:
	static const int MaxSheepHP = 1;

	GB_Sheep(long x, long y, Maze *maze);

	void initialMaxValues() {
		maxValues[0] = MaxSheepHP; // max value for HP
		properties[0] = MaxSheepHP; // initially every ghost has 3 HPs.
		propertyNames[0] = "hitpoints";
	}
	;

	void setDistanceToRun(int dist){distanceToRun = dist;};

	/**
	 * @return the index of the threatening ghost, or -1 if there is no such
	 * ghost.
	 * */
	int isDangered(const State& state, long worldNum);

	bool canReact(const State& state, long worldNum);

	void getActNoneSeen(const State& state, long worldNum,
	    std::vector<std::pair<long, double> >& action_prob);

	void randomMoveActs(long currMonsterX, long currMonsterY,
			std::vector<std::pair<long, double> >& action_prob,
			const State* state = 0);

	void moveActsFarthestFromPlayer(long currMonsterX, long currMonsterY,
		    long agentX, long agentY, unsigned agentIndex, long otherX, long otherY,
		    std::vector<std::pair<long, double> >& action_prob,
		    const State* state = 0);

	// At the point of planning, sheep's HP is ignored
	bool isRelevant(long propertyIndex, const Agent* dealtAgent) {
		return false;
	};
};

#endif
