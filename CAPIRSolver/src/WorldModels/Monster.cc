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



#include "Monster.h"
#include "Maze.h"
#include "MazeWorld.h"

/*************** Get monster info ********************/
bool Monster::canReact(const AbstractState& absState) {
	assert( (!absState.monsterProperties.empty()) );

	return (absState.monsterProperties[0] >= 0);
}
;

bool Monster::canReact(const State& state, long worldNum) {
	assert( (!state.mazeProperties[worldNum].empty()) );

	return (state.mazeProperties[worldNum][0] >= 0);
}
;

// TODO ---------------------- Abstract state related
/*************** Abstract related ********************/

void Monster::enumerateProperties(std::vector<AbstractState>& input,
    std::vector<AbstractState>& output, const Agent* dealtAgent) {
	if (properties.empty()) {
		Utilities::copyAbstractStateArray(input, output);
		return;
	}

	output.resize(0);

	AbstractState state;

	vector<vector<long> > tempProperties;
	long currSize;

	while (!input.empty()) {
		state = input.back();
		input.pop_back();

		// 1. Allocate space
		// currSize = state.monsterProperties.size();
		// state.monsterProperties.resize(currSize + properties.size(), 0);

		// 2. Enumerate properties and place in tempProperties
		Agent::enumerateProperties(state.monsterProperties, tempProperties,
		    dealtAgent);

		// 3. Add to output
		for (unsigned j = 0; j < tempProperties.size(); j++) {
			state.monsterProperties = tempProperties[j];
			output.push_back(state);
		}
	}
}
;

void Monster::absReact(std::vector<std::pair<AbstractState, double> >& input,
    long humanAct, long aiAct,
    std::vector<std::pair<AbstractState, double> >& output,
    const AbstractState& prevAbsState) {

	output.resize(0);

	AbstractState currAbsState;
	double prob;
	std::vector<std::pair<AbstractState, double> > tempOutput;

	// 1. If any of the player actions is special, i.e. can affect the monster's welfare.
	if (maze->player[0]->isSpecialAct(humanAct) || maze->player[1]->isSpecialAct(
	    aiAct)) {
		absGetAffectedByPlayersActions(input, humanAct, aiAct, tempOutput);
		Utilities::copyAbstractProbArray(tempOutput, input);
	}

	for (unsigned i = 0; i < input.size(); i++) {

		currAbsState = input[i].first;
		prob = input[i].second;

		absReact(currAbsState, prob, humanAct, aiAct, tempOutput, prevAbsState);

		// add all resulted states to output
		for (unsigned j = 0; j < tempOutput.size(); j++)
			// rewards here are already multiplied by tempOutput[j].second
			Utilities::addAbsStateToVector(tempOutput[j].first, tempOutput[j].second,
			    output);

	}// for i=0 -> input.size
}
;

void Monster::absReact(AbstractState& currAbsState, double prob, long humanAct, long aiAct,
    std::vector<std::pair<AbstractState, double> >& output,
    const AbstractState& prevAbsState) {

	long act;
	bool seesPlayers[2];
	int x = 1, y = 2;

	seesPlayers[0] = currAbsState.monsterProperties[2];
	seesPlayers[1] = currAbsState.monsterProperties[3];

	std::vector<std::pair<long, double> > action_prob;
	output.resize(0);

	if (canReact(currAbsState)) {
		// 1. choose actions for monster depending on its exposure to human/ai
		if (seesPlayers[0]) {
			if (seesPlayers[1]) {
				absGetActHumanAgentSeen(currAbsState, humanAct, aiAct, prevAbsState,
				    action_prob);
			} else {
				absGetAct1AgentSeen(currAbsState, humanAct, aiAct, humanIndex,
				    prevAbsState, action_prob);
			}
		} else {
			if (seesPlayers[1]) {
				absGetAct1AgentSeen(currAbsState, humanAct, aiAct, aiIndex,
				    prevAbsState, action_prob);
			} else {
				absGetActNoneSeen(currAbsState, humanAct, aiAct, prevAbsState,
				    action_prob);
			}
		}

		double probAct;

		// 2. Quantitatively apply the action to currAbsState
		for (unsigned k = 0; k < action_prob.size(); k++) {
			act = action_prob[k].first;
			probAct = action_prob[k].second;

			// 2. Execute the monster's act if its probability is greater than 0.
			if (probAct > 0) {
				if (isMoveAct(act)) {
					// we can count on maze to apply move acts reliably
					maze->absExecuteMonsterMoveAct(currAbsState, prob, humanAct, aiAct,
					    act, probAct, output, prevAbsState);
				}
				// special act
				else {
					// it is up to the monster to determine what the effect of its special act is
					absExecuteSpecialAct(currAbsState, prob, humanAct, aiAct, act,
					    probAct, output, prevAbsState);
				}
			}
		} // for k

	}
	// the monster can't do anything, e.g. already dead or frozen.
	else {
		output.push_back(std::pair<AbstractState, double>(currAbsState, prob));
	}

}
;

/**
 This routine only affects the properties of human/assistant/monster/specialLocation, but not their location info.
 */
void Monster::absExecuteSpecialAct(AbstractState& currAbsState, double prob,
    long humanAct, long aiAct, long act, double probAct,
    std::vector<std::pair<AbstractState, double> >& output,
    const AbstractState& prevAbsState) {
	Utilities::absDoNothing(currAbsState, prob * probAct, output);
}
;

/**
 Monster's movement determined as if the human and ai agent are at anchor points. All blocking is ignored.
 */
void Monster::absGetActHumanAgentSeen(const AbstractState& absState,
    long humanAct, long aiAct, const AbstractState& prevAbsState,
    std::vector<std::pair<long, double> >& action_prob) {

	// 1. Get necessary param
	long currMonsterX = absState.monsterProperties[0];
	long currMonsterY = absState.monsterProperties[1];
	long humanX = absState.playerProperties[humanIndex][1];
	long humanY = absState.playerProperties[humanIndex][2];
	long aiX = absState.playerProperties[aiIndex][1];
	long aiY = absState.playerProperties[aiIndex][2];

	// 2. invoke respective routine
	moveActsFarthestFromBothPlayers(currMonsterX, currMonsterY, humanX, humanY,
	    aiX, aiY, action_prob);
}
;

/**
 Blocking is ignored.
 */
void Monster::absGetAct1AgentSeen(const AbstractState& absState, long humanAct,
    long aiAct, int agentIndex, const AbstractState& prevAbsState,
    std::vector<std::pair<long, double> >& action_prob) {
	long currMonsterX = absState.monsterProperties[0];
	long currMonsterY = absState.monsterProperties[1];
	long agentX = absState.playerProperties[agentIndex][1];
	long agentY = absState.playerProperties[agentIndex][2];

	// 2. invoke respective routine
	moveActsFarthestFromPlayer(currMonsterX, currMonsterY, agentX, agentY, agentIndex, -1, -1,
	    action_prob);
}
;

void Monster::absGetActNoneSeen(const AbstractState& absState, long humanAct,
    long aiAct, const AbstractState& prevAbsState,
    std::vector<std::pair<long, double> >& action_prob) {
	randomMoveActs(absState.monsterProperties[0], absState.monsterProperties[1],
	    action_prob);
}
;

// TODO ------------------ Raw state related
/******************** Raw state related *********************/

void Monster::react(State& state, long worldNum, std::vector<long>& monsterActions, RandSource& randSource) {
	if (canReact(state, worldNum)) {

		long posHX = state.playerProperties[humanIndex][0];
		long posHY = state.playerProperties[humanIndex][1];

		long posAX = state.playerProperties[aiIndex][0];
		long posAY = state.playerProperties[aiIndex][1];

		long currMonsterX = state.mazeProperties[worldNum][0];
		long currMonsterY = state.mazeProperties[worldNum][1];

		bool humanSeen, aiSeen;

		std::vector<std::pair<long, double> > action_prob;

		// 0. Does any priority action first
		priorityActions(state, worldNum, action_prob);

		if (action_prob.empty()){
			// 1. check if human is within monster's visionLimit.

			humanSeen = maze->checkVisibility(currMonsterX, currMonsterY, posHX, posHY);

			// 2. check if agent is within monster's visionLimit.
			aiSeen = maze->checkVisibility(currMonsterX, currMonsterY, posAX, posAY);

			//std::cout << "humanSeen = " << humanSeen << ", aiSeen = " << aiSeen << std::endl;

			// 3. invoke respective routines
			if (humanSeen) {
				if (aiSeen) {
					// both agent and human are seen
					getActHumanAgentSeen(state, worldNum, action_prob);

				} else {
					// agent is not seen, human is seen, move away from human
					getAct1AgentSeen(state, humanIndex, worldNum, action_prob);
				}
			} else if (aiSeen) {
				// agent is seen and human is not seen, move away from agent
				// long best_distance, dist, sNode;
				getAct1AgentSeen(state, aiIndex, worldNum, action_prob);
			} else {
				// human and agent are not seen, monster moves randomly around and not into pen

				getActNoneSeen(state, worldNum, action_prob);
			}
		}

		// 4. Sample the action
		long act = action_prob[Distribution::sampleLongDouble(action_prob,
		    randSource)].first;

		// 5. Execute the monster's act.
		if (isMoveAct(act)) {
			// we can count on mazeWorld to apply move acts reliably
			(maze->mazeWorld)->applyMonsterMoveAct(state, act, worldNum);
			// move acts do not change monster's properties unless it is location-related. In this case, maze will update for monster.
		}
		// special act
		else {
			// it is up to the monster to determine what the effect of its special act is
			executeSpecialAct(state, act, worldNum);
		}

		monsterActions.push_back(act);

	}
	else{
		// can't react
		monsterActions.push_back(-1);
	}
}
;

void Monster::getActHumanAgentSeen(const State& state, long worldNum,
    std::vector<std::pair<long, double> >& action_prob) {

	// 1. Get necessary param
	long currMonsterX = state.mazeProperties[worldNum][0];
	long currMonsterY = state.mazeProperties[worldNum][1];
	long humanX = state.playerProperties[humanIndex][0];
	long humanY = state.playerProperties[humanIndex][1];
	long aiX = state.playerProperties[aiIndex][0];
	long aiY = state.playerProperties[aiIndex][1];

	moveActsFarthestFromBothPlayers(currMonsterX, currMonsterY, humanX, humanY,
	    aiX, aiY, action_prob, &state);
}
;

void Monster::getAct1AgentSeen(const State& state, int agentIndex,
    long worldNum, std::vector<std::pair<long, double> >& action_prob) {

	long currMonsterX = state.mazeProperties[worldNum][0];
	long currMonsterY = state.mazeProperties[worldNum][1];
	long agentX = state.playerProperties[agentIndex][0];
	long agentY = state.playerProperties[agentIndex][1];


	// 2. invoke respective routine
	moveActsFarthestFromPlayer(currMonsterX, currMonsterY, agentX, agentY, agentIndex,
			state.playerProperties[1-agentIndex][0],
			state.playerProperties[1-agentIndex][1],
	    action_prob, &state);
}
;

void Monster::getActNoneSeen(const State& state, long worldNum,
    std::vector<std::pair<long, double> >& action_prob) {
	randomMoveActs(state.mazeProperties[worldNum][0],
	    state.mazeProperties[worldNum][1], action_prob, &state);
}
;

// TODO -------------------- moveActs routines

void Monster::moveActsFarthestFromBothPlayers(long currMonsterX,
    long currMonsterY, long humanX, long humanY, long aiX, long aiY,
    std::vector<std::pair<long, double> >& action_prob, const State* state) {
	action_prob.resize(0);
	std::vector<long> valid_actions(0);

	// 1. get necessary geographical info from mazeWorld
	vector<vector<long> >* gridNodeLabel;
	gridNodeLabel = maze->gridNodeLabel;

	// 1. calculate nearest agent/human
	long hNode = (*gridNodeLabel)[humanX][humanY];
	long aNode = (*gridNodeLabel)[aiX][aiY];

	long currSNode = (*gridNodeLabel)[currMonsterX][currMonsterY];

	long s_h_Dist = (*shortestPath)[currSNode][hNode];
	long s_a_Dist = (*shortestPath)[currSNode][aNode];
	long nearestAgentNode, fartherAgentNode;

	nearestAgentNode = (s_h_Dist < s_a_Dist) ? hNode : aNode;
	fartherAgentNode = (s_h_Dist < s_a_Dist) ? aNode : hNode;
	unsigned nearestAgentIndex = (s_h_Dist < s_a_Dist) ? humanIndex : aiIndex;

	// 2. move away from nearest agent/human
	long sNode, act;
	double best_distance, dist;
	best_distance = maze->distance_farthestFromBoth(currSNode, nearestAgentNode, nearestAgentIndex,
	    fartherAgentNode);

	act = unchanged;
	valid_actions.push_back(act);
	for (long i = east; i < numMoveActs; i++) {
		// action i always moves the sheep, so checking can be done on all sheep position regardless

		if (state)
			sNode
			    = maze->mazeWorld->isValidMonsterMove(this,
			        currMonsterX + RelativeDirX[i], currMonsterY + RelativeDirY[i],
			        state);
		else
			sNode = (*gridNodeLabel)[currMonsterX + RelativeDirX[i]][currMonsterY
			    + RelativeDirY[i]];

		if (sNode >= 0 && (!isImpassable(currMonsterX + RelativeDirX[i],
		    currMonsterY + RelativeDirY[i], (*gridNodeLabel)))) {

			valid_actions.push_back(i);

			dist = maze->distance_farthestFromBoth(sNode, nearestAgentNode, nearestAgentIndex,
			    fartherAgentNode);

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
 * optional otherX and otherY, if set to >=0, will be the other player's coords,
 * which the monster should avoid too.
 *
 * */
void Monster::moveActsFarthestFromPlayer(long currMonsterX, long currMonsterY,
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
	long sNode, act;
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
		else sNode = (*gridNodeLabel)[currMonsterX + RelativeDirX[i]][currMonsterY
			    + RelativeDirY[i]];

		if (sNode >= 0 && (!isImpassable(currMonsterX + RelativeDirX[i],
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

void Monster::moveActsFarthestFromPlayerExcludePoint(long currMonsterX, long currMonsterY,
    long agentX, long agentY, unsigned agentIndex, long otherX, long otherY,
    std::vector<std::pair<long, double> >& action_prob,
    long exPointX, long exPointY,
    const State* state) {
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
	long sNode, act;
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
		else sNode = (*gridNodeLabel)[currMonsterX + RelativeDirX[i]][currMonsterY
			    + RelativeDirY[i]];

		if (sNode >= 0
				&& (*gridNodeLabel)[exPointX][exPointY] != sNode
				&& (!isImpassable(currMonsterX + RelativeDirX[i],
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


void Monster::moveActsTowardsLocation(long currMonsterX, long currMonsterY,
    long destX, long destY, std::vector<std::pair<long, double> >& action_prob,
    const State* state) {
	action_prob.resize(0);
	std::vector<long> valid_actions(0);

	// 1. get necessary geographical info from mazeWorld
	vector<vector<long> >* gridNodeLabel;
	gridNodeLabel = maze->gridNodeLabel;

	long dNode = (*gridNodeLabel)[destX][destY];
	long currSNode = (*gridNodeLabel)[currMonsterX][currMonsterY];

	// 2. move towards from player agentIndex
	long sNode, act;
	double best_distance, dist;
	best_distance = maze->distance_towardsNode(currSNode, dNode);

	act = unchanged;
	valid_actions.push_back(act);
	for (long i = east; i < numMoveActs; i++) {
		// action i always moves the sheep, so checking can be done on all sheep position regardless

		if (state)
			sNode
			    = maze->mazeWorld->isValidMonsterMove(this,
			        currMonsterX + RelativeDirX[i], currMonsterY + RelativeDirY[i],
			        state);
		else
			sNode = (*gridNodeLabel)[currMonsterX + RelativeDirX[i]][currMonsterY
			    + RelativeDirY[i]];

		if (sNode >= 0 && (!isImpassable(currMonsterX + RelativeDirX[i],
		    currMonsterY + RelativeDirY[i], (*gridNodeLabel)))) {

			valid_actions.push_back(i);

			dist = maze->distance_towardsNode(sNode, dNode);

			if (best_distance > dist) {
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

void Monster::moveActsTowardsLocationExcludePoint(long currMonsterX, long currMonsterY,
    long destX, long destY, std::vector<std::pair<long, double> >& action_prob,
    long exPointX, long exPointY,
    const State* state) {
	action_prob.resize(0);
	std::vector<long> valid_actions(0);

	// 1. get necessary geographical info from mazeWorld
	vector<vector<long> >* gridNodeLabel;
	gridNodeLabel = maze->gridNodeLabel;

	long dNode = (*gridNodeLabel)[destX][destY];
	long currSNode = (*gridNodeLabel)[currMonsterX][currMonsterY];

	// 2. move towards from player agentIndex
	long sNode, act;
	double best_distance, dist;
	best_distance = maze->distance_towardsNode(currSNode, dNode);

	act = unchanged;
	valid_actions.push_back(act);
	for (long i = east; i < numMoveActs; i++) {
		// action i always moves the sheep, so checking can be done on all sheep position regardless

		if (state)
			sNode
			    = maze->mazeWorld->isValidMonsterMove(this,
			        currMonsterX + RelativeDirX[i], currMonsterY + RelativeDirY[i],
			        state);
		else
			sNode = (*gridNodeLabel)[currMonsterX + RelativeDirX[i]][currMonsterY
			    + RelativeDirY[i]];

		if (sNode >= 0
				&& (*gridNodeLabel)[exPointX][exPointY] != sNode
				&& (!isImpassable(currMonsterX + RelativeDirX[i],
		    currMonsterY + RelativeDirY[i], (*gridNodeLabel)))) {

			valid_actions.push_back(i);

			dist = maze->distance_towardsNode(sNode, dNode);

			if (best_distance > dist) {
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

void Monster::randomMoveActs(long currMonsterX, long currMonsterY,
    std::vector<std::pair<long, double> >& action_prob, const State* state) {
	action_prob.resize(0);
	// get all valid actions
	std::vector<long> validAction;

	validAction.push_back(unchanged);
	long sNode;
	for (long i = east; i < numMoveActs; i++) {
		if (state)
			sNode
			    = maze->mazeWorld->isValidMonsterMove(this,
			        currMonsterX + RelativeDirX[i], currMonsterY + RelativeDirY[i],
			        state);
		else
			sNode
			    = (*maze->gridNodeLabel)[currMonsterX + RelativeDirX[i]][currMonsterY
			        + RelativeDirY[i]];

		if (sNode >= 0 && (!isImpassable(currMonsterX + RelativeDirX[i],
		    currMonsterY + RelativeDirY[i], (*maze->gridNodeLabel))))
			validAction.push_back(i);
	}

	for (unsigned i = 0; i < validAction.size(); i++)
		action_prob.push_back(
		    std::pair<long, double>(validAction[i], 1.0 / validAction.size()));
}
;

