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



#include "GB_Ghost.h"
#include "GB_Human.h"
#include "GB_AiAssistant.h"
#include "GhostBustersLevel.h"
#include "Maze.h"
#include "GB_GhostMaze.h"
#include <cmath>

/**
 * When Ghost gets shot by a human in its sight, it reduces its HP by one.
 * Currently, this is deterministic and I'm not taking into account the human's HP or accuracy yet.
 * */
void GB_Ghost::absGetAffectedByPlayersActions(
		std::vector<std::pair<AbstractState, double> >& input, long humanAct,
		long aiAct, std::vector<std::pair<AbstractState, double> >& output) {
	output.resize(0);

	AbstractState currAbsState;

	// 1. if human shoots
	if (humanAct == GB_Human::Shoot) {

		for (unsigned i = 0; i < input.size(); i++) {
			currAbsState = input[i].first;

			// 2. monster sees human when abstract
			if (maze->useAbstract ){
				if (currAbsState.monsterProperties[2])
					// 3. get shot
					currAbsState.monsterProperties[4]--;
			}
			else{
				// compute distance

				long mNode = (*(maze->gridNodeLabel))[currAbsState.monsterProperties[0]]
				                                      [currAbsState.monsterProperties[1]];
				long hNode = (*(maze->gridNodeLabel))[currAbsState.playerProperties[humanIndex][1]]
					                                  [currAbsState.playerProperties[humanIndex][2]];

				if ((*shortestPath)[mNode][hNode] <= ((GB_GhostMaze*)maze)->shotDistance)
					currAbsState.monsterProperties[4]--;
			}

			output.push_back(
					std::pair<AbstractState, double>(currAbsState,
							input[i].second));

		} // for i<input.size()

	} // Otherwise, get the default Monster action
	else
		Monster::absGetAffectedByPlayersActions(input, humanAct, aiAct, output);
}
;

/**
 * No need to check whether the monster sees human before applying GB_AiAss::Shoot
 * because that has been done in GB_AiAss_Attack::executeSpecialAction
 * */
void GB_Ghost::getAffectedByPlayersActions(State& state, long worldNum,
		long humanAct, long aiAct, RandSource& randSource) {
	if (humanAct == GB_Human::Shoot) {
		state.mazeProperties[worldNum][2]--;
	}
}
;

bool GB_Ghost::canReact(const AbstractState& absState) {
	// monster hasn't died yet and its HP > 0
	return (Monster::canReact(absState) && absState.monsterProperties[4] > 0);
}
;

bool GB_Ghost::canReact(const State& state, long worldNum) {
	// monster hasn't died yet and its HP > 0
	return (Monster::canReact(state, worldNum)
			&& (state.mazeProperties[worldNum][2] > 0));
}
;

void GB_Ghost::priorityActions(const State& state, long worldNum,
	    std::vector<std::pair<long, double> >& action_prob){
	long currMonsterX = state.mazeProperties[worldNum][0];
	long currMonsterY = state.mazeProperties[worldNum][1];

	long mNode = (*(maze->gridNodeLabel))[currMonsterX][currMonsterY];


	long sheepNode;

	// 1. Get the sheep that is next to this monster
	for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
		// if this maze is a sheep, and it is not dead
		if ((maze->mazeWorld->mazes[i]->worldTypeStr == "sheep")
				&& (state.mazeProperties[i][0] >= 0)) {

			sheepNode
			= maze->mazeWorld->gridNodeLabel[state.mazeProperties[i][0]][state.mazeProperties[i][1]];

			if ((*shortestPath)[mNode][sheepNode] == 1) {
				action_prob.push_back(std::pair<long, double>(KickTheSheep, 1));
				return;
			}
		}
	}
};

void GB_Ghost::getActNoneSeen(const State& state, long worldNum,
		std::vector<std::pair<long, double> >& action_prob) {

	long currMonsterX = state.mazeProperties[worldNum][0];
	long currMonsterY = state.mazeProperties[worldNum][1];

	long mNode = (*(maze->gridNodeLabel))[currMonsterX][currMonsterY];
	long hNode = (*(maze->gridNodeLabel))[state.playerProperties[humanIndex][0]]
	                                     [state.playerProperties[humanIndex][1]];

	if ((*shortestPath)[mNode][hNode] <= maze->visionLimit
			+ startRunDistance) {
		runAwayFromHuman(state, worldNum, action_prob);
	} else {
			// no need to run away
			double nearestDistance = INT_MAX;
			long sheepNode, attackedSheep = -1;

			// 1. Get the list of all sheep
			for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
				// if this maze is a sheep, and it is not dead
				if ((maze->mazeWorld->mazes[i]->worldTypeStr == "sheep")
						&& (state.mazeProperties[i][0] >= 0)) {

					sheepNode
					= maze->mazeWorld->gridNodeLabel[state.mazeProperties[i][0]][state.mazeProperties[i][1]];

					if (nearestDistance > (*shortestPath)[mNode][sheepNode]) {
						nearestDistance = (*shortestPath)[mNode][sheepNode];
						attackedSheep = i;
					}
				}
			}

			// 2a. if there exists a chasable sheep, chase it
			if (attackedSheep >= 0) {
				moveActsTowardsLocation(currMonsterX, currMonsterY,
						state.mazeProperties[attackedSheep][0],
						state.mazeProperties[attackedSheep][1], action_prob, &state);
			}
			// 2b. else, move randomly.
			else {
				randomMoveActs(currMonsterX, currMonsterY, action_prob, &state);
			}
		}
}
;

void GB_Ghost::executeSpecialAct(State& state, long act, long worldNum) {
	if (act == KickTheSheep) {
		long currMonsterX = state.mazeProperties[worldNum][0];
		long currMonsterY = state.mazeProperties[worldNum][1];

		long mNode = (*(maze->gridNodeLabel))[currMonsterX][currMonsterY];
		long sheepNode;

		// 1. Look for the sheep that is next to this ghost
		for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
			// if this maze is a sheep, and it is not dead
			if ((maze->mazeWorld->mazes[i]->worldTypeStr == "sheep")
					&& (state.mazeProperties[i][0] >= 0)) {

				if ( (fabs(state.mazeProperties[i][0] - state.mazeProperties[worldNum][0]) +
						fabs(state.mazeProperties[i][1] - state.mazeProperties[worldNum][1])) <= 1){
					state.mazeProperties[i][2]--;
					return;
				}
			}
		}

	}
}
;

void GB_Ghost::runAwayFromHuman(const State& state, long worldNum,
    std::vector<std::pair<long, double> >& action_prob) {
	long currMonsterX = state.mazeProperties[worldNum][0];
	long currMonsterY = state.mazeProperties[worldNum][1];
	long humanX = state.playerProperties[humanIndex][0];
	long humanY = state.playerProperties[humanIndex][1];
	long aiX = state.playerProperties[aiIndex][0];
	long aiY = state.playerProperties[aiIndex][1];

	moveActsFarthestFromPlayer(currMonsterX, currMonsterY, humanX,
			humanY, humanIndex, aiX, aiY, action_prob, &state);
};
