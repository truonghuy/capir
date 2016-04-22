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



#ifndef __GB_SHEEPMAZE_H
#define __GB_SHEEPMAZE_H

#include "Maze.h"
#include "GB_Sheep.h"

class GhostBustersLevel;

class GB_SheepMaze: public Maze  {

private:

	static const double SheepCaughtReward = 10;
public:
	pair<long, long> penPosition;

public:
	GB_SheepMaze() {
	}
	;

	double getMinReward(){return 0;};

	/******** Initialization *****************************/
	void initialize(long sheepMazeId, long sheepX,
					long sheepY, double visLim,
					GhostBustersLevel* gbLevel);

	void copyInfoFrom(Maze* orig) {
		Maze::copyInfoFrom(orig);
		penPosition = ((GB_SheepMaze*) orig)->penPosition;
	}

	void setProperty(string pName, string pValue);

	void setPenPosition(long penX, long penY);

	double getMaxReward(){return SheepCaughtReward;};

	/***************** Raw State related *************************/
	double getReward(const State& state, bool& vTerminal, int worldNum);

	/********* Abstract related **************************/
	double absGetReward(AbstractState& state);

	/********* Distance function *************************/
	double distance_farthestFromBoth(long monsterNode, long nearestAgentNode, unsigned nearestAgentIndex,
			long fartherAgentNode);
	double distance_farthestFromPlayer(long monsterNode, long agentNode);

};

#endif
