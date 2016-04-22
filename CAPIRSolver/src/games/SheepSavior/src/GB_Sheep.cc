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



#include "GB_Sheep.h"
#include "MazeWorld.h"
#include "Maze.h"
#include "GB_SheepMaze.h"

GB_Sheep::GB_Sheep(long x, long y, Maze *maze) :Monster(x, y, 5, 0, 1, maze) {
	initialMaxValues();
	OptimalProb = 1.0;
	//probRunFromGhost = 0.8;
	// distance to run = a side of the map
	distanceToRun = 3;
};

bool GB_Sheep::canReact(const State& state, long worldNum) {
	// monster hasn't died yet and its HP > 0
	return (Monster::canReact(state, worldNum)
			&& (state.mazeProperties[worldNum][2] > 0));
}
;

void GB_Sheep::moveActsFarthestFromPlayer(long currMonsterX, long currMonsterY,
	    long agentX, long agentY, unsigned agentIndex, long otherX, long otherY,
	    std::vector<std::pair<long, double> >& action_prob, const State* state) {

	action_prob.resize(0);
	std::vector<long> valid_actions(0);

	// 1. get necessary geographical info from mazeWorld
	vector<vector<long> >* gridNodeLabel;
	gridNodeLabel = maze->gridNodeLabel;

	long aNode = (*gridNodeLabel)[agentX][agentY];

	long oNode = -1;
	if (otherX >=0 && otherY>=0)
		oNode = (*gridNodeLabel)[otherX][otherY];

	long currSNode = (*gridNodeLabel)[currMonsterX][currMonsterY];

	// 2. move away from player agentIndex
	// 2. move away from player agentIndex
	long sNode, act, pNode;
	pNode = (*maze->gridNodeLabel)[((GB_SheepMaze*) maze)->penPosition.first]
		                         [((GB_SheepMaze*) maze)->penPosition.second];

	double best_distance, dist;
	best_distance = maze->distance_farthestFromBoth(currSNode, aNode, agentIndex, oNode);

	act = unchanged;
	valid_actions.push_back(act);
	for (long i = east; i < numMoveActs; i++) {
		// action i always moves the sheep, so checking can be done on all sheep position regardless

		if (state)
			sNode = maze->mazeWorld->isValidMonsterMove(this,
					currMonsterX + RelativeDirX[i], currMonsterY + RelativeDirY[i],
					state);
		else sNode = (*gridNodeLabel)[currMonsterX + RelativeDirX[i]]
			                         [currMonsterY + RelativeDirY[i]];

		if (sNode >= 0 && (sNode != pNode) && (!isImpassable(currMonsterX + RelativeDirX[i],
			currMonsterY + RelativeDirY[i], (*gridNodeLabel)))) {

			valid_actions.push_back(i);

			dist = maze->distance_farthestFromBoth(sNode, aNode, agentIndex, oNode);

			if (best_distance < dist) {
				best_distance = dist;
				act = i;
			}
		}
	}

	// 3. Set probability for valid actions
	if (valid_actions.size() > 1 && OptimalProb < 1.0) {
		for (unsigned i = 0; i < valid_actions.size(); i++) {
			// optimal action gets probability OptimalProb
			if (valid_actions[i] == act)
				action_prob.push_back(
					std::pair<long, double>(valid_actions[i], OptimalProb));
			// sub-optimal actions get probability of (1-OptimalProb)/num_subOptimalActions
			else if (1 - OptimalProb > 0)
				action_prob.push_back(
					std::pair<long, double>(valid_actions[i],
						(1 - OptimalProb) / (valid_actions.size() - 1)));
		}
	} else
		action_prob.push_back(std::pair<long, double>(act, 1.0));

}
;

/**
 * Sheep stochastically run away from nearest ghost.
 * */
void GB_Sheep::getActNoneSeen(const State& state, long worldNum,
    std::vector<std::pair<long, double> >& action_prob) {

	long currMonsterX = state.mazeProperties[worldNum][0];
	long currMonsterY = state.mazeProperties[worldNum][1];

	// 2a. if there exists a nearby ghost within distanceToRun, run away from
	// it while avoiding the pen.
	int ghostId = isDangered(state, worldNum);
	if (ghostId >= 0){
		moveActsFarthestFromPlayer(currMonsterX, currMonsterY,
				state.mazeProperties[ghostId][0], state.mazeProperties[ghostId][1],
				humanIndex, -1, -1,
				action_prob, &state);
	}
	// 2b. else, move randomly.
	else randomMoveActs(currMonsterX, currMonsterY, action_prob, &state);

};

int GB_Sheep::isDangered(const State& state, long worldNum)
{
	long mNode = (*(maze->gridNodeLabel))[state.mazeProperties[worldNum][0]]
	                                     [state.mazeProperties[worldNum][1]];

	double nearestDistance = INT_MAX;
	long ghostNode, ghostId = -1;

	// 1. Get the list of all ghosts
	for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
		// if this maze is a sheep, and it is not dead
		if ((maze->mazeWorld->mazes[i]->worldTypeStr == "ghost")
				&& (state.mazeProperties[i][0]	>= 0))
		{
			ghostNode = maze->mazeWorld->gridNodeLabel[state.mazeProperties[i][0]]
			                                          [state.mazeProperties[i][1]];

			if (nearestDistance > (*shortestPath)[mNode][ghostNode]) {
				nearestDistance = (*shortestPath)[mNode][ghostNode];
				ghostId = i;
			}
		}
	}

	// 2. if the nearest ghost is not within distanceToRun,
	// it's not in critical condition
	if (nearestDistance > distanceToRun)
		return -1;

	return ghostId;
};

void GB_Sheep::randomMoveActs(long currMonsterX,
    long currMonsterY, std::vector<std::pair<long, double> >& action_prob,
    const State* state) {
	action_prob.resize(0);
	// get all valid actions
	std::vector<long> validAction;
	//cout << "new random move acts in Sheep " << endl;
	validAction.push_back(unchanged);
	long sNode, pNode;
	pNode = (*maze->gridNodeLabel)[((GB_SheepMaze*) maze)->penPosition.first]
	                               [((GB_SheepMaze*) maze)->penPosition.second];
	for (long i = east; i < numMoveActs; i++) {
		if (state)
			sNode = maze->mazeWorld->isValidMonsterMove(this,
			        currMonsterX + RelativeDirX[i], currMonsterY + RelativeDirY[i],
			        state);
		else
			sNode = (*maze->gridNodeLabel)[currMonsterX + RelativeDirX[i]][currMonsterY
			        + RelativeDirY[i]];

		if (sNode >= 0 && (sNode != pNode) && (!isImpassable(
		    currMonsterX + RelativeDirX[i], currMonsterY + RelativeDirY[i],
		    (*maze->gridNodeLabel))))
			validAction.push_back(i);
	}

	for (unsigned i = 0; i < validAction.size(); i++)
		action_prob.push_back(
		    std::pair<long, double>(validAction[i], 1.0 / validAction.size()));
}
;
