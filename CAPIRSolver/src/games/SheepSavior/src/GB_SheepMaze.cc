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



#include "GB_SheepMaze.h"
#include "GhostBustersLevel.h"

void GB_SheepMaze::initialize(long sheepMazeId,
		long sheepX, long sheepY, double visLim,
		GhostBustersLevel* gbLevel) {
	// 1. initialize the monster
	monster = new GB_Sheep(sheepX, sheepY, this);

	worldType = sheepMazeId;
	worldTypeStr = "sheep";
	// set visionLimit to 1
	visionLimit = visLim;
	//visionLimit = 1;
	mazeWorld = gbLevel;
}
;

/**
 There can be a series of if else here to set corresponding property
 */
void GB_SheepMaze::setProperty(string pName, string pValue) {
	if (pName == "visionLimit")
		visionLimit = atof(pValue.c_str());
	//if (pName == "distanceToRun"){
	//	((GB_Sheep*)monster)->setDistanceToRun(atoi(pValue.c_str()));
	//}
}
;

void GB_SheepMaze::setPenPosition(long penX, long penY) {
	penPosition.first = penX;
	penPosition.second = penY;
}
;

/************ Abstract related ****************************/

double GB_SheepMaze::absGetReward(AbstractState& absState) {
	// 1. already terminal state
	if (isAbstractTerminal(absState))
		return NoReward;

	// 2. not previously terminal
	if ((absState.monsterProperties[0] == penPosition.first)
			&& (absState.monsterProperties[1] == penPosition.second)) {
		absState.monsterProperties[0] = TermState;
		return SheepCaughtReward;
	}
	return NoReward;
}
;

/***************** Raw State related *************************/
double GB_SheepMaze::getReward(const State& state,
		bool& vTerminal, int worldNum) {

	if (isTermState(state, worldNum)) {
		vTerminal = true;
		return NoReward;
	}

	if ((state.mazeProperties[worldNum][0] == penPosition.first)
			&& (state.mazeProperties[worldNum][1] == penPosition.second)) {
		vTerminal = true;
		return SheepCaughtReward;
	}

	if (state.mazeProperties[worldNum][2] <= 0) {
		vTerminal = true;
		return NoReward;
	}

	vTerminal = false;
	return NoReward;
}
;

/************** Distance functions ***************************/
double GB_SheepMaze::distance_farthestFromBoth(long monsterNode,
		long nearestAgentNode, unsigned nearestAgentIndex, long fartherAgentNode) {
	long pNode = (*gridNodeLabel)[penPosition.first][penPosition.second];

	if (fartherAgentNode >=0)
		return 8 * (*(monster->shortestPath))[monsterNode][nearestAgentNode] + 4
			* (*(monster->shortestPath))[monsterNode][fartherAgentNode]
			+ (*(monster->shortestPath))[monsterNode][pNode];
	else
		return distance_farthestFromPlayer(monsterNode, nearestAgentNode);
}
;

double GB_SheepMaze::distance_farthestFromPlayer(long monsterNode,
		long agentNode) {
	long pNode = (*gridNodeLabel)[penPosition.first][penPosition.second];

	return 4 * (*(monster->shortestPath))[monsterNode][agentNode]
			+ (*(monster->shortestPath))[monsterNode][pNode];
}
;

