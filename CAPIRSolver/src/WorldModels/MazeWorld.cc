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



#include "MazeWorld.h"
#include "rapidxml.hpp"
#include "Compression.h"
#include <fstream>

using namespace rapidxml;
using namespace std;

MazeWorld::MazeWorld(MazeWorldDescription& desc) :
	xSize(desc.xSize), ySize(desc.ySize), grid(desc.grid),
			numRegionPerAgent(desc.numRegionPerAgent), gType(desc.gType),
			monsterBlock(desc.monsterBlock),
			agentBlock(desc.agentBlock), visionLimit(desc.visionLimit),
			targetPrecision(desc.targetPrecision),
			displayInterval(desc.displayInterval), Model(desc.discount) {
	worldInitialize();
}
;

void MazeWorld::worldInitialize() {
	// Generate node labels. Every accessible position is a node in the graph.
	numAccessibleLocs = 0;
	gridNodeLabel.resize(xSize);
	reverseGridNodeLabel.resize(0);
	for (long i = 0; i < xSize; i++) {
		gridNodeLabel[i].resize(ySize);
		for (long j = 0; j < ySize; j++) {
			if (grid[i][j] < 0)
				gridNodeLabel[i][j] = -1;
			else {
				gridNodeLabel[i][j] = numAccessibleLocs;
				reverseGridNodeLabel.push_back(pair<long, long> (i, j));
				numAccessibleLocs++;
			}
		}
	}
}
;

void MazeWorld::setUseAbstract(bool uA) {

	for (unsigned i = 0; i < mazes.size(); i++) {
		mazes[i]->getRawWorldGeometricInfo();
		mazes[i]->setUseAbstract(uA);
	}

	if (uA) {
		// 1. Figure out the topology, i.e. the connectivity of the map's regions.
		getTopology();

		// 2. Set use abstract
		for (unsigned i = 0; i < mazes.size(); i++) {
			mazes[i]->getAbstractWorldGeometricInfo();
		}
	}
	// 3. Calculate shortest path for player[0] and player[1].
	player[0]->calcShortestPathMatrix(xSize, ySize, numAccessibleLocs,
			gridNodeLabel);
	player[1]->calcShortestPathMatrix(xSize, ySize, numAccessibleLocs,
			gridNodeLabel);

	// 3. Generate state map
	generateStateMap();
}
;

long MazeWorld::actionFromChar(const char c, int& humanOrAi) {

	if (humanOrAi == humanIndex)
		return player[0]->actionFromChar(c);

	if (humanOrAi == aiIndex)
		return player[1]->actionFromChar(c);

	// not tagged as player[0] or ai yet -> need to search for it
	if (player[0]->isMeaningfulAct(c)) {
		humanOrAi = humanIndex;
		return player[0]->actionFromChar(c);
	}

	if (player[1]->isMeaningfulAct(c)) {
		humanOrAi = aiIndex;
		return player[1]->actionFromChar(c);
	}

	// At this stage, c is not recognized yet, so we'll just return the unchanged action from player[0].
	return player[0]->actionFromChar(c);
}
;

/**
 Construct the connectivity of map space, regions' type, neighbor edge length, area and visible nearby regions
 */
void MazeWorld::getTopology() {

	connectivity.resize(numRegionPerAgent);
	rType.resize(numRegionPerAgent); // junction or not
	borderLength.resize(numRegionPerAgent); // lenghts of common edges

	// region's representative point: The south/west end. The other end is representative point + length - 1 for corridors
	regionRepPoint.resize(numRegionPerAgent);

	std::vector<bool> done(numRegionPerAgent);

	for (long i = 0; i < numRegionPerAgent; i++) {
		connectivity[i].resize(5); // where does it go next
		borderLength[i].resize(5, 0);
		done[i] = false;
	}

	// 1. Process grid to construct borderLength, regionRepPoint, connectivity, and rType
	// neighbor in connectivity < 0 -> wall

	for (long i = 0; i < xSize; i++) {
		for (long j = 0; j < ySize; j++) {
			if (grid[i][j] >= 0) // wall is negative

				if (!done[grid[i][j]]) { // region not already processed
					long k;

					borderLength[grid[i][j]][unchanged] = 1;
					regionRepPoint[grid[i][j]].first = i;
					regionRepPoint[grid[i][j]].second = j;

					// HUY note - since the grid world only consists of horizontal and vertical corridors and 1-cell junctions, by moving east and north, we basically trace the whole corridors
					// HUY note - checking the neighbor at far east by moving along the corridor...
					for (k = 1; grid[i][j] == grid[i + k][j]; k++)
						borderLength[grid[i][j]][unchanged]++; // east
					connectivity[grid[i][j]][east] = grid[i + k][j];
					if (connectivity[grid[i][j]][east] >= 0)
						borderLength[grid[i][j]][east] = 1;

					// HUY note - checking the neighbor at far south. The for loop is not needed because we are moving east/north wise, so the region would have been processed in the direction of south/north
					// for (k = 1; grid[i][j] == grid[i][j-k]; k++) ; // south
					// connectivity[grid[i][j]][south] = grid[i][j-k];
					connectivity[grid[i][j]][south] = grid[i][j - 1];
					if (connectivity[grid[i][j]][south] >= 0)
						borderLength[grid[i][j]][south] = 1;

					// HUY note - checking the neighbor at far west
					// for (k = 1; grid[i][j] == grid[i-k][j]; k++) ; // west
					// connectivity[grid[i][j]][west] = grid[i-k][j];
					connectivity[grid[i][j]][west] = grid[i - 1][j];
					if (connectivity[grid[i][j]][west] >= 0)
						borderLength[grid[i][j]][west] = 1;

					// HUY note - checking the neighbor at far north
					for (k = 1; grid[i][j] == grid[i][j + k]; k++)
						borderLength[grid[i][j]][unchanged]++; // north
					connectivity[grid[i][j]][north] = grid[i][j + k];
					if (connectivity[grid[i][j]][north] >= 0)
						borderLength[grid[i][j]][north] = 1;

					connectivity[grid[i][j]][unchanged] = grid[i][j]; // unchanged

					// Check if it is a junction
					if (borderLength[grid[i][j]][unchanged] == 1) {
						if ((borderLength[grid[i][j]][east] < 1)
								&& (borderLength[grid[i][j]][west] < 1))
							rType[grid[i][j]] = vertical;
						else if ((borderLength[grid[i][j]][south] < 1)
								&& (borderLength[grid[i][j]][north] < 1))
							rType[grid[i][j]] = horizon;
						else
							rType[grid[i][j]] = junction;
					} else {
						if ((borderLength[grid[i][j]][east] == 1)
								|| (borderLength[grid[i][j]][west] == 1))
							rType[grid[i][j]] = horizon;
						else
							rType[grid[i][j]] = vertical;
					}

					done[grid[i][j]] = true;
				}
		}
	}

	return;

}
;

/**
 This routine generates state map. Depending on the useAbstract property, each maze will generate corresponding state map.
 */
void MazeWorld::generateStateMap() {

	for (unsigned i = 0; i < mazes.size(); i++) {

		// if this is an original world
		if (i == equivWorlds[i]) {
			// already generated abstract map for mazes with monster
			mazes[i]->generateStateMap();
		} else {
			mazes[i]->copyStateMap(mazes[equivWorlds[i]]);
		}
	}
}
;

void MazeWorld::generateModel() {
	for (unsigned i = 0; i < mazes.size(); i++) {

		// if this is an original world
		if (i == equivWorlds[i]) {
			// already generated abstract map for mazes with monster
			mazes[i]->generateModel();
		} else {
			mazes[i]->copyValueQFns(mazes[equivWorlds[i]]);
		}
	}
}
;

/***************************** Raw State routines *********************/
/**
 Invoke corresponding routines in player[0], player[1] and mazes to fill the raw state with respective info.
 */
void MazeWorld::getCurrState(State& state) {
	player[humanIndex]->getCurrState(state.playerProperties[humanIndex]);
	player[aiIndex]->getCurrState(state.playerProperties[aiIndex]);

	state.mazeProperties.resize(numWorlds);
	for (unsigned i = 0; i < mazes.size(); i++) {
		mazes[i]->getCurrState(state.mazeProperties[i]);
	}
}
;

void MazeWorld::getRandomizedState(State& state, RandSource &randSource) {

	std::vector<long> occupiedGrids;
	long x, y;

	// 1. Randomize a position for player 0 such that it is in a free square
	getRandomizedGrid(x, y, occupiedGrids, randSource);
	player[humanIndex]->getCurrState(state.playerProperties[humanIndex], x, y);

	// 2. Randomize a position for player 1 such that it is in a free square
	getRandomizedGrid(x, y, occupiedGrids, randSource);
	player[aiIndex]->getCurrState(state.playerProperties[aiIndex], x, y);

	// 3. Randomize a position for each maze
	state.mazeProperties.resize(numWorlds);
	for (unsigned i = 0; i < mazes.size(); i++) {
		getRandomizedGrid(x, y, occupiedGrids, randSource);
		mazes[i]->getCurrState(state.mazeProperties[i], x, y);
	}
}
;

void MazeWorld::getRandomizedGrid(long& x, long& y,
		std::vector<long>& occupiedGrids, RandSource &randSource) {

	// ASSERTION: occupiedGrids.size < number of free grid nodes.
	if (occupiedGrids.size() >= numAccessibleLocs) {
		std::cerr << "Not enough grid squares, occupiedGrids.size() = "
				<< occupiedGrids.size() << "and numAccessibleLocs = "
				<< numAccessibleLocs << std::endl;
		exit(EXIT_FAILURE);
	}

	unsigned i;
	bool found;
	long newLoc;

	while (true) {
		newLoc = randSource.get() % numAccessibleLocs;

		// not already in occupiedGrids
		found = false;
		for (i = 0; i < occupiedGrids.size(); i++) {
			if (occupiedGrids[i] == newLoc) {
				found = true;
				break;
			}
		}

		if (found)
			continue;
		// valid new free grid

		x = reverseGridNodeLabel[newLoc].first;
		y = reverseGridNodeLabel[newLoc].second;
		occupiedGrids.push_back(newLoc);
		return;
	}

}
;

/**
 * Based on state, get the max reward of individual mazes, sum them up.
 * Assumption: Mazes do not interact to give more rewards.
 * */
double MazeWorld::getMaxReward(const State& state)
{
	double rewards = 0;
	for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
		// if maze is still alive
		if (!mazes[i]->isTermState(state, i))
			rewards += mazes[i]->getMaxReward();
	}
	return rewards;
};

/**
 * Based on state, get the max reward of individual mazes, sum them up.
 * Assumption: Mazes do not interact to give more rewards.
 * */
double MazeWorld::getMinReward(const State& state)
{
	double rewards = 0;
	for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
		// if maze is still alive
		if (!mazes[i]->isTermState(state, i))
			rewards += mazes[i]->getMinReward();
	}
	return rewards;
};

/**
 * This default version returns valid movement actions.
 * Special actions are assumed to be all valid at any point of time.
 * */
void MazeWorld::getValidCompoundActions(const State& currState,
		std::vector<long>& validActions, long playerAction, int playerIndex) {

	validActions.clear();

	long currHumanX = currState.playerProperties[humanIndex][0];
	long currHumanY = currState.playerProperties[humanIndex][1];

	long currAgentX = currState.playerProperties[aiIndex][0];
	long currAgentY = currState.playerProperties[aiIndex][1];
	long tempX, tempY;
	bool valid;

	// 1. for all pair of movement action, check isValidMove
	for (unsigned act1 = 0; act1 < player[0]->getNumActs(); act1++) {
		if (player[0]->isMoveAct(act1)) {
			tempX = currHumanX + RelativeDirX[act1];
			tempY = currHumanY + RelativeDirY[act1];

			// new position of player[0] can only be updated if canMove is true
			valid = (isValidMove(tempX, tempY, currAgentX, currAgentY, currState)
			    && !player[0]->isImpassable(tempX, tempY, gridNodeLabel));
		}
		// if it's special action, always valid
		else
			valid = true;

		if (valid) {
			for (unsigned act2 = 0; act2 < player[1]->getNumActs(); act2++) {
				if (player[1]->isMoveAct(act2)) {
					tempX = currAgentX + RelativeDirX[act2];
					tempY = currAgentY + RelativeDirY[act2];

					// new position of player[1] can only be updated if canMove is true
					valid = (isValidMove(tempX, tempY, currHumanX, currHumanY, currState)
					    && !player[1]->isImpassable(tempX, tempY, gridNodeLabel));
				} else
					valid = true;

				// if both actions valid, add in.
				if (valid)
					validActions.push_back(act1 * player[1]->getNumActs() + act2);
			}
		}
	}

	// playerAction is supplied, need to strip off all actions that do not have
	// playerAction as a component.
	if (playerAction >= 0) {
		std::vector<long> tempValidActions = validActions;
		validActions.resize(0);
		bool gotSomeAction = false;
		for (unsigned i = 0; i < tempValidActions.size(); i++) {
			if (playerIndex == 0 && tempValidActions[i]
					/ player[1]->getNumActs() == playerAction) {
				validActions.push_back(tempValidActions[i]);
				gotSomeAction = true;
			} else if (playerIndex == 1 && tempValidActions[i]
					% player[1]->getNumActs() == playerAction) {
				validActions.push_back(tempValidActions[i]);
				gotSomeAction = true;
			}
		}

		// this only happens when the script for human model is wrong (due to random?)
		// or when the human press a key to make no move.
		if (!gotSomeAction) {
			playerAction = 0;
			for (unsigned i = 0; i < tempValidActions.size(); i++) {
				if (playerIndex == 0 && tempValidActions[i]
						/ player[1]->getNumActs() == playerAction)
					validActions.push_back(tempValidActions[i]);
				else if (playerIndex == 1 && tempValidActions[i]
						% player[1]->getNumActs() == playerAction)
					validActions.push_back(tempValidActions[i]);
			}
		}

	}
}
;

/**
 policy is invoked to simulate actions, so in here, human is assumed to be player[0], assistant player[1]
 */
double MazeWorld::policy(const State& currState, std::vector<double>& wBelief,
    long& p1act, long& p2act, State& nextState, vector<long>& monsterActions,
    RandSource& randSource) {
	// long bestAiAct;

	// humanAct = -1, meaning we don't know humanAct, and will need to sample later
	//  bool notTerm = policyRoutine(currState, wBelief, p2act, -1, humanIndex,
	//      randSource);
	p2act = -1;
	//  if (notTerm) {
	// in sample, need to infer the most likely world that the ai is working in
	return sample(currState, wBelief, p1act, p2act, nextState, monsterActions,
			randSource);
	//  } else
	//    return 0;
}
;

double MazeWorld::policy_stupidAI(const State& currState,
    std::vector<double>& wBelief, long& p1act, long& p2act, State& nextState,
    vector<long>& monsterActions, RandSource& randSource, int scriptMode) {
	// long bestAiAct;

	// humanAct = -1, meaning we don't know humanAct, and will need to sample later
	//bool notTerm = policyRoutine(currState, wBelief, p2act, -1, humanIndex,
	//    randSource);
	player[aiIndex]->getStupidAct(currState, p1act, humanIndex, p2act,
			randSource, scriptMode);

	// in sample, need to infer the most likely world that the ai is working in
	return sample(currState, wBelief, p1act, p2act, nextState, monsterActions,
			randSource);

}
;

double MazeWorld::policyHuman(const State& currState,
		std::vector<double>& wBelief, long playerAct, int playerIndex,
		long& aiAct, State& nextState, vector<long>& monsterActions,
		RandSource& randSource) {

	// pass playerAct in, which helps in determining the right bestAiAct
	bool notTerm = policyRoutine(currState, wBelief, aiAct, playerAct,
			playerIndex);

	if (notTerm) {
		return moveState(currState, wBelief, playerAct, aiAct, playerIndex,
				nextState, monsterActions, randSource);
	} else
		return 0;
}
;

/**
 *
 * @param[in] scriptMode 0:do nothing, 1:random, 2:follows human, 3:closestToHuman
 *
 * */
double MazeWorld::policyHuman_stupidAI(const State& currState,
    std::vector<double>& wBelief, long playerAct, int playerIndex, long& aiAct,
    State& nextState, vector<long>& monsterActions, RandSource& randSource,
    int scriptMode) {
	// currState is never terminal here.

	player[1 - playerIndex]->getStupidAct(currState, playerAct, playerIndex,
			aiAct, randSource, scriptMode);

	return moveState(currState, wBelief, playerAct, aiAct, playerIndex,
			nextState, monsterActions, randSource);
}
;

bool MazeWorld::policyRoutine(const State& currState, vector<double>& wBelief,
		long& bestAiAct, long playerAct, int playerIndex) {

	if (isTermState(currState)) {
		return false;
	}

	double maxQValue;
	long bestCompoundAct;

	maxQValue = getBestCompoundAct(currState, wBelief, bestCompoundAct,
			playerAct, playerIndex);

	if (playerIndex == 0)
		bestAiAct = bestCompoundAct % player[1]->getNumActs();
	else
		bestAiAct = bestCompoundAct / player[1]->getNumActs();
	//std::cout << "Chosen optimal ai act =" << bestAiAct << std::endl;

	return true;
}
;

double MazeWorld::getQValue(const State& currState,
		const vector<double>& wBelief, const long compoundAct) {
	double sumQValue = 0;
	long currVState;

	for (long i = 0; i < numWorlds; i++) {
		currVState = mazes[i]->realToVirtual(currState, i);

		if (currVState != longTermState) {
			sumQValue += (*(mazes[i]->collabQFn))[currVState][compoundAct]
					* wBelief[i];
		}
	}
	return sumQValue;
}
;

/**
 * @param[out] QValues output Q values.
 * */
void MazeWorld::getQValues(const State& currState, vector<double>& wBelief,
		vector<pair<long, double> >& QValues, long playerAct, int playerIndex)
{
	QValues.resize(0);

	double sumQValue;

	long compAct, ha, aiAct;

	vector<long> validActions;
	getValidCompoundActions(currState, validActions, playerAct, playerIndex);

	for (unsigned i=0; i<validActions.size(); i++){
		sumQValue = getQValue(currState, wBelief, validActions[i]);
		QValues.push_back(pair<long, double> (validActions[i], sumQValue));
	}
}
;

double MazeWorld::getBestCompoundAct(const State& currState,
    vector<double>& wBelief, long& bestCompoundAct,
    long playerAct, int playerIndex) {

	vector<pair<long, double> > QValues;

	getQValues(currState, wBelief, QValues, playerAct, playerIndex);

	long maxIndex = Distribution::getMaxLongDouble(QValues);

	bestCompoundAct = QValues[maxIndex].first;
	return QValues[maxIndex].second;
}
;

long MazeWorld::getBestCompoundActInSubworld(const State& currState,
		int subWorld, long playerAct, int playerIndex)
{
	long currVState = mazes[subWorld]->realToVirtual(currState, subWorld);

	if (currVState == longTermState)
		return 0;

	if (playerAct < 0)
		return Distribution::getMax( (*(mazes[subWorld]->collabQFn))[currVState], -1, -1);
	else{

		long bestCompoundAct, compAct;
		if (playerIndex == 0){
			bestCompoundAct = playerAct * player[1]->getNumActs();
		}
		else bestCompoundAct = playerAct;

		for (long aiAct = 1; aiAct < player[1-playerIndex]->getNumActs(); aiAct++) {
			if (playerIndex == 0){
				compAct = playerAct * player[1]->getNumActs() + aiAct;
			}
			else compAct = aiAct*player[1]->getNumActs() + playerAct;

			if ((*(mazes[subWorld]->collabQFn))[currVState][bestCompoundAct]
			                  < (*(mazes[subWorld]->collabQFn))[currVState][compAct]) {
				bestCompoundAct = compAct;
			}
		}
		return bestCompoundAct;
	}

};

/**
 * @return the subWorld id if there is exactly one subWorld alive, -1 otherwise
 * */
long MazeWorld::hasOneSubworldAlive(const State& currState)
{
	long aliveWorld;
	long numAliveWorlds = 0;
	for (unsigned i = 0; i < currState.mazeProperties.size(); i++) {
		if (!mazes[i]->isTermState(currState, i)){
			numAliveWorlds++;
			aliveWorld = i;
		}
	}

	if (numAliveWorlds == 0 || numAliveWorlds > 1)
		return -1;

	return aliveWorld;
};

/**
 * bestCompoundActions stores the best compound actions in each of the subworlds.
 * */
void MazeWorld::getBestCompoundActionsInSubtasks(const State& currState,
    std::vector<std::vector<long> >& bestCompoundActions, double tolerance,
    long playerAct, int playerIndex) {
	// Caller already checked for terminality of the state

	// counters, act = human act, ca = compound act
	long tempVState;
	bestCompoundActions.resize(numWorlds);
	long numActs = player[0]->getNumActs() * player[1]->getNumActs();

	double maxQValue, value;
	long compoundAct;
	for (unsigned i = 0; i < numWorlds; i++) {

		bestCompoundActions[i].clear();

		// if this is not terminal state
		if (!mazes[i]->isTermState(currState, i)) {
			tempVState = (mazes[i])->realToVirtual(currState, i);

			if (playerAct < 0){
				maxQValue = Distribution::getMaxValue((*(mazes[i])->collabQFn)[tempVState]);

				for (long act = 0; act < numActs; act++) { // act = compound act
					value = (*(mazes[i])->collabQFn)[tempVState][act];
					if (fabs(value - maxQValue) < tolerance)
						bestCompoundActions[i].push_back(act);
				}
			}
			// Choose maxQValue to be the one associated with the player action
			else{
				// 1. compute the best value with player action as a component
				long numAiActs = player[1-playerIndex]->getNumActs();
				long act = 0;
				compoundAct = getCompoundAct(playerAct, playerIndex, act);
				maxQValue = (*(mazes[i])->collabQFn)[tempVState][compoundAct];
				for (act = 1; act < numAiActs; act++) {
					compoundAct = getCompoundAct(playerAct, playerIndex, act);
					value = (*(mazes[i])->collabQFn)[tempVState][compoundAct];
					if (maxQValue < value)
						maxQValue = value;
				}

				// 2. add all compound actions within the maxQValue's tolerance
				for (act = 0; act < numAiActs; act++) {
					compoundAct = getCompoundAct(playerAct, playerIndex, act);
					value = (*(mazes[i])->collabQFn)[tempVState][compoundAct];
					if (fabs(value - maxQValue) < tolerance)
						bestCompoundActions[i].push_back(compoundAct);
				}
			}
		} // if termState
	} // for long i
}
;

/**
 * If \a playerAct is not furnished, it is assumed to be zero.
 * */
long MazeWorld::getCompoundAct(long playerAct, int playerIndex, long aiAct)
{
	if (playerAct < 0)
		playerAct = 0;

	if (playerIndex == humanIndex)
		return playerAct * player[1]->getNumActs() + aiAct;
	else return aiAct * player[1]->getNumActs() + playerAct;

};

bool MazeWorld::containsAgentAction(long compoundAct, long playerAct, int playerIndex)
{
	long numAiActs = player[1]->getNumActs();
	if (playerIndex == humanIndex){
		if (compoundAct / numAiActs == playerAct) return true;
	}
	else{
		if (compoundAct % numAiActs == playerAct) return true;
	}
	return false;
};

// Note that these functions are meant to check, not to update state
bool MazeWorld::isTermState(const State& state) {
	// 1. Human's X coord is TermState
	if (state.playerProperties[humanIndex][0] == TermState)
		return true;

	if (gType == Utilities::orType) {
		for (unsigned i = 0; i < mazes.size(); i++)
			if (mazes[i]->isTermState(state, i))
				// any world is terminal -> terminal
				return true;
		return false;
	} else {
		for (unsigned i = 0; i < mazes.size(); i++)
			if (!mazes[i]->isTermState(state, i))
				// any world is not terminal -> not terminal
				return false;

		return true;
	}
}
;

bool MazeWorld::someWorldAlive(const State& state) {

	for (unsigned i = 0; i < mazes.size(); i++)
		if (!mazes[i]->isLocalTermState(state, i))
			// any world is not terminal -> not terminal
			return true;

	return false;

}
;


double MazeWorld::sample(const State& currState, std::vector<double>& wBelief,
		long& humanAct, long& aiAct, State& nextState,
		std::vector<long>& monsterActions, RandSource& randSource) {

	// GOAL: Identify the world the human is in now

	if (isTermState(currState)) {
		nextState = currState;
		return 0;
	}

	double sumProb;
	long i, ha;

	// dimension: worldIndex, actionIndex -> probability
	std::vector<std::vector<double> > condProbAct;

	// sampleAct for human only happens in simulation mode, therefore,
	// we can try to make human&assistant's collaboration more consistent by
	// letting human know aiAct so that it can choose optimal action.

	humanAct = player[0]->sampleAct(currState, aiAct, condProbAct, randSource);
	if (aiAct == -1)
		policyRoutine(currState, wBelief, aiAct, humanAct, humanIndex);

	return rewardFromRealDynamics(currState, wBelief, humanAct, aiAct,
			humanIndex, nextState, condProbAct, monsterActions, randSource);

}
;

double MazeWorld::moveState(const State& currState, vector<double>& wBelief,
		long humanAct, long aiAct, int playerIndex, State& nextState,
		vector<long>& monsterActions, RandSource& randSource) {

	// GOAL: Identify the world the human is in now

	if (isTermState(currState)) {
		nextState = currState;
		return 0;
	}

	double sumProb;
	long i, ha;

	// dimension: worldIndex, actionIndex -> probability
	std::vector<std::vector<double> > condProbAct;

	// getActionModel to populate condProbAct (p(action|world))
	player[playerIndex]->getActionModel(currState, condProbAct);

	return rewardFromRealDynamics(currState, wBelief, humanAct, aiAct,
			playerIndex, nextState, condProbAct, monsterActions, randSource);

}
;

double MazeWorld::rewardFromRealDynamics(const State& currState,
		vector<double>& wBelief, long humanAct, long aiAct, int playerIndex,
		State& nextState, std::vector<std::vector<double> >& condProbAct,
		vector<long>& monsterActions, RandSource& randSource) {
	double sumProb;
	long i, tempVState;

	// succeedArray
	vector<int> succeedArray;
	vector<int> terminalArray;

	// 1. Use realDynamics to give the next state
	double returnedValue;
	if (playerIndex == humanIndex)
		returnedValue = realDynamics(currState, humanAct, aiAct, nextState,
				succeedArray, terminalArray, monsterActions, randSource);
	else
		returnedValue = realDynamics(currState, aiAct, humanAct, nextState,
				succeedArray, terminalArray, monsterActions, randSource);

	// 2. Update belief in assistant
	player[1 - playerIndex]->updateBelief(wBelief, humanAct, condProbAct,
			succeedArray, terminalArray);

	// 10. Return the reward value
	return returnedValue;
}
;

double MazeWorld::realDynamics(const State& currState, long player0Act,
		long player1Act, State& nextState, vector<int>& succeedArray,
		vector<int>& terminalArray, vector<long>& monsterActions,
		RandSource& randSource) {
	// Assumption: Caller must check for terminus before calling this function.
	// randSource isn't used because nextState is deterministic given currState and player0Act, player1Act

	monsterActions.resize(0);
	succeedArray.resize(numWorlds);
	terminalArray.resize(numWorlds);

	// 1. Update position of human and ai agents. Both of them don't jump on monster. Actions that cause them to move to position where a monster is occupying will render as though it's a non-move act.
	Utilities::copyState(currState, nextState);
	executeAgentActions(currState, player0Act, player1Act, nextState,
			succeedArray, randSource);

	// 2. Update mazes' dynamics
	double reward = 0;
	bool vTerminal, terminal;
	terminal = true;

	// Get reward and get terminality of world i.
	// Note: The state can already be terminal in a world long time ago, so reward can still be 0 in those cases

	for (long i = 0; i < numWorlds; i++) {

		// a. Apply monster's action if any.
		if (mazes[i]->monster) {
			// monster react. If the reaction is a movement, monster will invoke MazeWorld's routine.
			// Except for human/assistant related info, nextState carries the same  information of currState.
			mazes[i]->monster->react(nextState, i, monsterActions, randSource);
		}
		// if this maze does not have a monster, action = -1
		else
			monsterActions.push_back(-1);

		// b. Update special locations' properties and final things related to monster
		mazes[i]->executeMazeDynamics(nextState, i, randSource);

		// monster in world i has moved, now, get reward and update virtual world i's terminus
		// getReward should also be in inherited class
		reward += mazes[i]->getReward(nextState, vTerminal, i);

		if (vTerminal) {
			nextState.mazeProperties[i][0] = TermState;
			nextState.mazeProperties[i][1] = TermState;

			/*update terminal array
			 by Qiao Li
			 */
			terminalArray[i] = 1;

			if (gType == Utilities::orType)
				terminal = true;
		} else {
			if (gType != Utilities::orType)
				terminal = false;
		}
	}

	if (terminal) {
		Utilities::convertState2AugState(nextState, lastState, player0Act,
				player1Act, monsterActions);
		nextState.playerProperties[0][0] = TermState;
	}
	// if the game has not ended because of the mazes
	else {
		reward += getReward(nextState, terminal);
		if (terminal) {
			Utilities::convertState2AugState(nextState, lastState, player0Act,
					player1Act, monsterActions);
			nextState.playerProperties[0][0] = TermState;
		}
	}
	return reward;
}
;

void MazeWorld::executeAgentActions(const State& currState, long player0Act,
		long player1Act, State& nextState, vector<int>& succeedArray,
		RandSource& randSource) {

	unsigned i;
	long tempX, tempY;
	bool canMove;

	// 1. Set nextState to currState
	Utilities::copyState(currState, nextState);

	// 2. Update nextState's coordinates if human/player[1] is doing move acts.

	long currHumanX = currState.playerProperties[humanIndex][0];
	long currHumanY = currState.playerProperties[humanIndex][1];

	long currAgentX = currState.playerProperties[aiIndex][0];
	long currAgentY = currState.playerProperties[aiIndex][1];

	// 2.
	if (player[0]->isMoveAct(player0Act)) {
		// Human movement ------------------------------
		// tempX, tempY are always >=0 and <= sizeX, sizeY because the boundary of grid is always wall

		tempX = currHumanX + RelativeDirX[player0Act];
		tempY = currHumanY + RelativeDirY[player0Act];

		// new position of player[0] can only be updated if canMove is true
		canMove = (isValidMove(tempX, tempY, currAgentX, currAgentY, currState,
				&succeedArray) && !player[0]->isImpassable(tempX, tempY,
				gridNodeLabel));

		if (canMove) {
			nextState.playerProperties[humanIndex][0] = tempX;
			nextState.playerProperties[humanIndex][1] = tempY;
			// update player[0] position in order to check for blocking in agent movement
			currHumanX = tempX;
			currHumanY = tempY;

		}
	} else {
		// If player[0] action is special action, typically in this routine, player[0] chooses a monster to apply its special action on.
		player[0]->executeSpecialAct(currState, player0Act, nextState, randSource);
	}

	if (player[1]->isMoveAct(player1Act)) {
		tempX = currAgentX + RelativeDirX[player1Act];
		tempY = currAgentY + RelativeDirY[player1Act];

		// new position of player[1] can only be updated if canMove is true
		canMove = (isValidMove(tempX, tempY, currHumanX, currHumanY, currState)
				&& !player[1]->isImpassable(tempX, tempY, gridNodeLabel));

		if (canMove) {
			nextState.playerProperties[aiIndex][0] = tempX;
			nextState.playerProperties[aiIndex][1] = tempY;
		}
	} else {
		player[1]->executeSpecialAct(currState, player1Act, nextState, randSource);
	}

}
;

bool MazeWorld::isValidMove(long tempX, long tempY, long currAgentX,
		long currAgentY, const State& currState, vector<int>* succeedArray) {
	bool canMove = true;

	if (succeedArray)
		succeedArray->resize(numWorlds, 0); // default to fail in all worlds

	if (grid[tempX][tempY] < 0) { // wall
		return false;
	} else {
		if (agentBlock) {
			// human/agent can't jump on each other

			if ((tempX == currAgentX) && (tempY == currAgentY)) {
				// fail in all worlds
				return false;
			}
		}

		//if (monsterAgentBlock){
		// human can't jump on monster

		for (unsigned i = 0; i < numWorlds; i++) {
			if (mazes[i]->monster && mazes[i]->monster->blocksAgents()
					&& (tempX == currState.mazeProperties[i][0]) && (tempY
					== currState.mazeProperties[i][1])) {
				canMove = false;
			} else if (succeedArray)
				(*succeedArray)[i] = 1;
		}
		//}
	}

	return canMove;
}
;

void MazeWorld::applyMonsterMoveAct(State& state, long act, int worldNum) {
	state.mazeProperties[worldNum][0] += RelativeDirX[act];
	state.mazeProperties[worldNum][1] += RelativeDirY[act];
}
;

long MazeWorld::isValidMonsterMove(Monster* monster, long monsterX,
		long monsterY, const State* currState) {

	long sNode = gridNodeLabel[monsterX][monsterY];

	if (sNode < 0) // wall
		return -1;
	else {
		if (monsterBlock){
			// monster can't jump on monster
			long monsterNum;
			if (isBlockedAt(currState, monsterX, monsterY, monsterNum))
				return -1;
		}

		if (monster->blocksAgents()) {
			// monster can't jump on agent

			if (((monsterX == currState->playerProperties[humanIndex][0])
					&& (monsterY == currState->playerProperties[humanIndex][1]))
					|| ((monsterX == currState->playerProperties[aiIndex][0])
			        && (monsterY == currState->playerProperties[aiIndex][1])))
				return -1;
		}
	}
	return sNode;
}
;

int MazeWorld::getNumAliveWorlds(const State& state) {
	int result = 0;
	for (unsigned i = 0; i < numWorlds; i++) {
		if (!mazes[i]->isTermState(state, i))
			result++;
	}
	return result;
}
;

long MazeWorld::getSubworldAliveStatus(const State& state, std::vector<bool> &aliveStatus) {
	aliveStatus.clear();
	aliveStatus.resize(numWorlds, true);

	long numAlive = numWorlds;
	for (unsigned i = 0; i < numWorlds; i++) {
		if (mazes[i]->isTermState(state, i)){
			aliveStatus[i] = false;
			numAlive--;
		}
	}
	return numAlive;
};

void MazeWorld::getDeadWorlds(const State& state, std::vector<long> &deadWorlds) {
	deadWorlds.resize(0);

	for (unsigned i = 0; i < numWorlds; i++) {
		if (mazes[i]->isTermState(state, i))
			deadWorlds.push_back(i);
	}
}
;

void MazeWorld::getAliveWorlds(const State& state,
		std::vector<long> &aliveWorlds) {
	aliveWorlds.resize(0);

	for (unsigned i = 0; i < numWorlds; i++) {
		if (!mazes[i]->isTermState(state, i))
			aliveWorlds.push_back(i);
	}
}
;

bool MazeWorld::isBlockedAt(const State* state, long x, long y,
		long& monsterNum) {
	// Go thru all mazes, if they have monsters.
	for (unsigned i = 0; i < numWorlds; i++) {
		if ((mazes[i]->monster) && (mazes[i]->monster->blocksMonsters()) && (x
		    == state->mazeProperties[i][0]) && (y == state->mazeProperties[i][1])) {
			monsterNum = i;
			return true;
		}
	}

	return false;
}
;

/* No need for pugi
 // 1. xml declaration
 pugi::xml_node decl = doc.allocate_node(node_declaration);
 decl->append_attribute(doc.allocate_attribute("version", "1.0"));
 decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
 doc.append_node(decl);


 // 2. state node as root node
 xml_node<>* root = doc.allocate_node(node_element, "state");
 // time stamp
 if (timeStamp > -2) {
 sprintf(buffer, "%d", timeStamp);
 root->append_attribute(doc.allocate_attribute("time", buffer));
 }
 // ended?
 if (state.playerProperties[0][0] == TermState) {
 root->append_attribute(doc.allocate_attribute("ended", "yes"));
 } else
 root->append_attribute(doc.allocate_attribute("ended", "no"));

 doc.append_node(root);

 // 3. Players node
 xml_node<>* player;

 for (long i = 0; i < 2; i++) {
 if (i == 0)
 player = doc.allocate_node(node_element, "player1");
 else player = doc.allocate_node(node_element, "player2");

 if (state.playerProperties[i][0] == TermState) {
 player->append_attribute(doc.allocate_attribute("x", buffer));
 }

 root->append_node(player);
 }
 */

void MazeWorld::stateToXMLDoc(pugi::xml_document& doc,
		const AugmentedState& state, const int timeStamp) {

	// 1. state node as root node
	pugi::xml_node root = doc.append_child("state");
	// time stamp
	if (timeStamp > -2) {
		root.append_attribute("time") = timeStamp;
	}
	// ended?
	if (isTermState(state)){
		if (someWorldAlive(state))
			root.append_attribute("ended") = "lost";
		else root.append_attribute("ended") = "won";
	}
	else root.append_attribute("ended") = "no";

	// 2. Players node
	pugi::xml_node node;
	pugi::xml_node propertyNode;

	for (long i = 0; i < 2; i++) {
		if (i == 0)
			node = root.append_child("player1");
		else
			node = root.append_child("player2");

		// position
		if (state.playerProperties[i][0] == TermState) {
			node.append_attribute("x") = (int) lastState.playerProperties[i][0];
			node.append_attribute("y") = (int) lastState.playerProperties[i][1];
			node.append_attribute("prev_act") = (int) lastState.longData[i];
		} else {
			node.append_attribute("x") = (int) state.playerProperties[i][0];
			node.append_attribute("y") = (int) state.playerProperties[i][1];
			node.append_attribute("prev_act") = (int) state.longData[i];
		}

		// properties
		for (unsigned j = 2; j < state.playerProperties[i].size(); j++) {
			propertyNode = node.append_child("property");
			propertyNode.append_attribute("name")
			    = player[i]->propertyNames[j - 2].c_str();
			propertyNode.append_attribute("value")
					= (int) state.playerProperties[i][j];
			propertyNode.append_attribute("max") = (int) player[i]->maxValues[j - 2];
		}
	}

	// 3. npc nodes
	pugi::xml_node worlds = root.append_child("worlds");
	pugi::xml_node world;

	long propIndex;
	for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
		world = worlds.append_child("world");
		world.append_attribute("id") = i;
		world.append_attribute("type") = mazes[i]->worldTypeStr.c_str();

		// there's belief incorporated in this AugmentedState
		if (!state.doubleData.empty()) {
			world.append_attribute("belief") = state.doubleData[i];
		}

		// property index
		propIndex = 0;

		// NPC
		if (mazes[i]->monster) {
			node = world.append_child("npc");
			node.append_attribute("x") = (int) state.mazeProperties[i][0];
			node.append_attribute("y") = (int) state.mazeProperties[i][1];
			if (i + 2 < state.longData.size())
				node.append_attribute("prev_act") = (int) state.longData[2 + i];
			else
				node.append_attribute("prev_act") = -1;

			propIndex = 2;
			for (unsigned j = 0; j < mazes[i]->monster->getNumProperties(); j++) {
				propertyNode = node.append_child("property");
				propertyNode.append_attribute("name")
						= mazes[i]->monster->propertyNames[j].c_str();
				propertyNode.append_attribute("value")
						= (int) state.mazeProperties[i][propIndex];
				propertyNode.append_attribute("max")
						= (int) mazes[i]->monster->maxValues[j];
				propIndex++;
			}
		}

		// Special Location
		if (mazes[i]->specialLocation) {
			node = world.append_child("special_location");

			for (unsigned j = 0; j < mazes[i]->specialLocation->getNumProperties(); j++) {
				propertyNode = node.append_child("property");
				propertyNode.append_attribute("name")
						= mazes[i]->specialLocation->propertyNames[j].c_str();
				propertyNode.append_attribute("value")
						= (int) state.mazeProperties[i][propIndex];
				propertyNode.append_attribute("max")
						= (int) mazes[i]->specialLocation->maxValues[j];
				propIndex++;
			}
		}
	}

	// 3. debug nodes
	if (!state.doubleData.empty()) {
		pugi::xml_node debug = root.append_child("debug");
		pugi::xml_node theRestOfDoubleData;

		for (unsigned i = 0; i < state.doubleData.size(); i++) {
			theRestOfDoubleData = debug.append_child("data");
			theRestOfDoubleData.append_attribute("id") = i;
			theRestOfDoubleData.append_attribute("value") = state.doubleData[i];
		}
	}

	// print to cout to debug
	//std::cout << "Document:\n";
	//doc.save(std::cout);
}
;

char* MazeWorld::stateToPosCharXML(const AugmentedState& state,
		const int timeStamp) {

	Utilities::printState(state);

	pugi::xml_document doc;

	stateToXMLDoc(doc, state, timeStamp);

	// 4. save to stringstream
	std::string tempS;
	std::stringstream out;
	char *posChar;

	doc.save(out, "\t", pugi::format_raw);
	tempS = out.str();
	posChar = new char[tempS.size() + 1];
	posChar[tempS.size()] = 0;
	memcpy(posChar, tempS.c_str(), tempS.size());
	return posChar;
}
;

/*
 char* MazeWorld::stateToPosCharXML(const AugmentedState& state,
 const int timeStamp) {
 std::string tempS;
 std::stringstream out;
 char *posChar;
 Utilities::printState(state);

 //-------------------------------------------
 // 1. formality
 out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

 out << "<state time=\"" << timeStamp << "\" ended=\"";

 if (state.playerProperties[0][0] == TermState) {
 out << "yes\">";
 } else
 out << "no\">";

 // 2. players, player1 = human, player2 = ai_assistant
 for (long i = 0; i < 2; i++) {
 out << "<player" << (i + 1) << " x=\"";
 // if position data is TermState, use lastState's coords
 if (state.playerProperties[i][0] == TermState) {
 out << lastState.playerProperties[i][0] << "\" y=\""
 << lastState.playerProperties[i][1] << "\" prev_act=\""
 << lastState.longData[i] << "\">";
 //out << "-1\" y=\"-1\">";
 } else {
 out << state.playerProperties[i][0] << "\" y=\""
 << state.playerProperties[i][1] << "\" prev_act=\""
 << state.longData[i] << "\">";
 }
 // properties
 for (unsigned j = 2; j < state.playerProperties[i].size(); j++) {
 // ai_assistant
 out << "<property name=\"" << player[i]->propertyNames[j - 2]
 << "\" value=\"" << state.playerProperties[i][j] << "\"/>";
 out << "<property name=\"" << player[i]->propertyNames[j - 2]
 << "_max\" value=\"" << player[i]->maxValues[j - 2]
 << "\"/>";
 // out << state.playerProperties[i][j] << " ";
 }

 out << "</player" << (i + 1) << ">";
 }

 // 3. npcs
 out << "<worlds>";
 long propIndex;
 for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
 out << "<world id=\"" << i << "\" type=\"" << mazes[i]->worldTypeStr
 << "\">";

 // property index
 propIndex = 0;

 // NPC
 if (mazes[i]->monster) {
 out << "<npc x=\"" << state.mazeProperties[i][0] << "\" y=\""
 << state.mazeProperties[i][1] << "\" prev_act=\"";
 if (i + 2 < state.longData.size())
 out << state.longData[2 + i] << "\">";
 else
 out << "-1\">";

 propIndex = 2;
 for (unsigned j = 0; j < mazes[i]->monster->getNumProperties(); j++) {

 out << "<property name=\""
 << mazes[i]->monster->propertyNames[j] << "\" value=\""
 << state.mazeProperties[i][propIndex] << "\"/>";
 out << "<property name=\""
 << mazes[i]->monster->propertyNames[j]
 << "_max\" value=\"" << mazes[i]->monster->maxValues[j]
 << "\"/>";

 propIndex++;
 }

 out << "</npc>";
 }

 // Special Location
 if (mazes[i]->specialLocation) {
 out << "<special_location>";

 for (unsigned j = 0; j
 < mazes[i]->specialLocation->getNumProperties(); j++) {
 out << "<property name=\""
 << mazes[i]->specialLocation->propertyNames[j]
 << "\" value=\"" << state.mazeProperties[i][propIndex]
 << "\"/>";
 out << "<property name=\""
 << mazes[i]->specialLocation->propertyNames[j]
 << "_max\" value=\""
 << mazes[i]->specialLocation->maxValues[j] << "\"/>";

 propIndex++;
 }

 out << "</special_location>";
 }

 out << "</world>";
 }
 out << "</worlds>";
 out << "</state>";

 tempS = out.str();
 posChar = new char[tempS.size() + 1];
 posChar[tempS.size()] = 0;
 memcpy(posChar, tempS.c_str(), tempS.size());
 return posChar;
 }
 ;
 */

char* MazeWorld::stateToPosChar(const AugmentedState& state) {
	std::string tempS;
	std::stringstream out;
	char *posChar;
	//Utilities::printState(state);

	// players
	for (long i = 0; i < 2; i++) {
		// if position data is TermState, put it as -1 -1
		if (state.playerProperties[i][0] == TermState) {
			out << "-1 -1 ";
			//TODO: lastState.playerProperties[i][0] << " " << lastState.playerProperties[i][1] << " ";
		} else {
			out << state.playerProperties[i][0] << " "
					<< state.playerProperties[i][1] << " ";
		}
		for (unsigned j = 2; j < state.playerProperties[i].size(); j++)
			out << state.playerProperties[i][j] << " ";
	}
	// mazes
	for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
		for (unsigned j = 0; j < 2; j++)//state.mazeProperties[i].size(); j++)
			out << state.mazeProperties[i][j] << " ";
	}

	cout << tempS << endl;

	tempS = out.str();
	posChar = new char[tempS.size() + 1];
	posChar[tempS.size()] = 0;
	memcpy(posChar, tempS.c_str(), tempS.size());
	return posChar;
}
;

/****************************** Read/write routines ********************/

void MazeWorld::readDescriptionFromTMXFile(std::string filename,
		GameTileSheet& gts, MazeWorldDescription& desc) {
	FILE *fp; // std::ifstream fp;
	fp = fopen(filename.c_str(), "rb"); // fp.open(filename.c_str(), "rb");
	if (!fp) {// .is_open()){
		std::cerr << "Fail to open " << filename << "\n";
		exit(EXIT_FAILURE);
	}

	// HUY - Reading from xml file generated by Tiled.

	// 1. Read the whole file
	int length;
	char * buffer;

	// get length of file:
	fseek(fp, 0, SEEK_END); // fp.seekg (0, std::ios::end);
	length = ftell(fp); // fp.tellg();
	fseek(fp, 0, SEEK_SET); // fp.seekg (0, std::ios::beg);

	// allocate memory:
	buffer = new char[length + 1];

	// read data as a block
	fread(buffer, length, 1, fp);
	fclose(fp); // fp.close();
	buffer[length] = '\0';
	// std::cout << buffer;

	// 2. Parse the string buffer
	xml_document<> doc; // character type defaults to char
	doc.parse<0> (buffer); // 0 means default parse flags

	xml_node<> *node, *property_node;
	xml_node<> *mapNode = doc.first_node("map");
	xml_attribute<> *attr;

	// 3. Get properties of all tiles if exists
	node = mapNode->first_node("tileset");
	node = node->first_node("tile");

	desc.properties.resize(0);
	long id;
	Property property;
	ID_Property id_property;
	string propName, propValue;
	vector<long> forcedJunctionIds;

	while (node) {
		// there is tile with properties declared

		// get the id
		attr = node->first_attribute("id");
		id = atoi(attr->value());

		// parse the properties
		property_node = node->first_node("properties");
		// there are properties declared for this tile
		if (property_node) {

			property_node = property_node->first_node("property");
			while (property_node) {

				// get the name of the property
				attr = property_node->first_attribute("name");
				property.first = attr->value();

				if (property.first == "junction" || property.first == "specLoc") {
					forcedJunctionIds.push_back((id + 1));
				} else {
					// get the value of the property
					attr = property_node->first_attribute("value");
					property.second = attr->value();

					// TODO: Somehow the id of tile is 1 less than gid
					// add to vector of id_property
					desc.properties.push_back(ID_Property(id + 1, property));
				}

				// get next property
				property_node = property_node->next_sibling("property");
			} // while there is still property

		} // if there is "properties" declared

		node = node->next_sibling("tile");
	} // while node = node->first_node("tile");


	// 4. Extract width and height
	node = mapNode->first_node("layer");

	// width
	attr = node->first_attribute("width");
	desc.xSize = atoi(attr->value());

	// height
	attr = node->first_attribute("height");
	desc.ySize = atoi(attr->value());

	// 5. Initialization for array of characters(which may be human, assistant, monsters and special locations)
	desc.characters.resize(0);

	// 6. Read the grid
	int numRegions;

	desc.grid.resize(desc.xSize);
	for (long i = 0; i < desc.xSize; i++)
		// -2 = this grid is not set yet
		desc.grid[i].resize(desc.ySize, -2);

	// number of regions in the grid, used for abstract states
	numRegions = 0;

	// a. Search for first "tile" node
	node = node->first_node("data");
	node = node->first_node("tile");

	int gridId;
	long forcedJunction = 2;
	unsigned k;
	for (long i = 0; i < desc.ySize; i++) {
		for (long j = 0; j < desc.xSize; j++) {

			// b. Get gid of this node
			attr = node->first_attribute("gid");
			gridId = atoi(attr->value());

			// c. Check the id of this node for the type of the object in that node.
			if (gts.isWallID(gridId)) {
				// 0 = this grid is wall
				desc.grid[j][desc.ySize - i - 1] = 0;
			} else {
				// everything else is on ground
				desc.grid[j][desc.ySize - i - 1] = 1;

				// if this grid square has something other than ground
				if (!gts.isGroundID(gridId)) {

					// add (id, x, y) to desc.characters
					desc.characters.push_back(gridId);
					desc.characters.push_back(j);
					desc.characters.push_back(desc.ySize - i - 1);

					// if this grid square is a forced junction/special location
					for (k = 0; k < forcedJunctionIds.size(); k++) {
						if (gridId == forcedJunctionIds[k]) {
							desc.grid[j][desc.ySize - i - 1] = forcedJunction;
							forcedJunction++;
							break;
						}
					} // for

				}

			} // else

			// Move on to next "tile" node
			node = node->next_sibling("tile");
		}
	}

	delete[] buffer;

	// out: xSize ySize <map in 0, 1 format; 0: wall, 1: ground>

	// 7. Now desc.grid[i][j] stores the map in 0,1 form. Detect regions
	desc.numRegionPerAgent = detectRegionsInGrid(desc.xSize, desc.ySize,
			desc.grid, forcedJunction);

}
;

long MazeWorld::detectRegionsInGrid(long xSize, long ySize,
		std::vector<std::vector<long> >& grid, long startingJuncIndex) {

	/*
	 // aa. Print grid to check
	 for (long i = 0; i< xSize; i++){
	 for (long j = 0; j < ySize; j++){
	 std::cout << grid[i][j] << "\t";
	 }
	 std::cout << std::endl;
	 }
	 std::cout << std::endl;
	 */
	// a. Mark junctions.
	long direction;
	long regionIndex = startingJuncIndex;
	std::vector<long> connectedDirection;
	std::vector<std::pair<long, long> > junctionPoints(0);
	for (long i = 0; i < xSize; i++) {
		for (long j = 0; j < ySize; j++) {
			if (grid[i][j] == 1) // free space
			{
				// Get connected direction
				connectedDirection.resize(0);
				for (direction = east; direction <= north; direction++) {
					if (grid[i + RelativeDirX[direction]][j + RelativeDirY[direction]]) // free space
						connectedDirection.push_back(direction);
				}

				// Corridors are grids that have exactly two wall opposite
				if ((connectedDirection.size() == 2) && ((connectedDirection[1]
						- connectedDirection[0]) == (west - east)))
					continue;
				else {
					grid[i][j] = regionIndex;
					junctionPoints.push_back(std::pair<long, long>(i, j));
					regionIndex++;
				}
			} else if (grid[i][j] > 1) { // forced junctions
				junctionPoints.push_back(std::pair<long, long>(i, j));
			}
		}
	}
	/*
	 // bb. Print grid to check
	 for (long i = 0; i< xSize; i++){
	 for (long j = 0; j < ySize; j++){
	 std::cout << grid[i][j] << "\t";
	 }
	 std::cout << std::endl;
	 }
	 std::cout << std::endl;
	 */
	// b. Junctions are now marked with numbers >= 2, unmarked grid with 1. Mark corridors.
	long juncX, juncY, tempX, tempY;
	bool markedNewRegion;
	for (unsigned i = 0; i < junctionPoints.size(); i++) {

		// Start from a junction
		juncX = junctionPoints[i].first;
		juncY = junctionPoints[i].second;

		// Move in four direction, if available
		for (direction = east; direction <= north; direction++) {
			// this is a free (not wall) grid not marked as junction/region yet.
			markedNewRegion = false;
			tempX = juncX;
			tempY = juncY;
			while (grid[tempX + RelativeDirX[direction]][tempY
					+ RelativeDirY[direction]] == 1) {
				// move forward in that direction
				markedNewRegion = true;
				grid[tempX + RelativeDirX[direction]][tempY
						+ RelativeDirY[direction]] = regionIndex;
				tempX += RelativeDirX[direction];
				tempY += RelativeDirY[direction];
			}
			if (markedNewRegion)
				regionIndex++;
		}
	}

	// c. Now all grids in free space are marked with >= 2. Change grid to normal
	// format: -1 is wall, and regions start indexing from 0
	for (long i = 0; i < xSize; i++) {
		for (long j = 0; j < ySize; j++) {
			if (grid[i][j])
				grid[i][j] -= 2;
			else
				grid[i][j] = -1;
		}
	}

	return regionIndex - 2;

}
;

void MazeWorld::printGrid() {
	// d. Print grid to check
	for (long i = 0; i < xSize; i++) {
		for (long j = 0; j < ySize; j++) {
			std::cout << grid[i][j] << "\t";
		}
		std::cout << std::endl;
	}
}
;

void MazeWorld::displayState(const State& state) {

	// display human-friendly representation of state (H: human, A: AI, @ is NPC)
	cout << "\n";
	long x,y;
	for (long i = 0; i < ySize; i++) {
		for (long j = 0; j < xSize; j++) {
			x = j;
			y = ySize - i - 1;
			if (grid[x][y] == -1) // wall
				cout << "o";
			// Human
			else if ((x == state.playerProperties[0][0]) &&
					(y == state.playerProperties[0][1]))
				cout << "H";
			// AI
			else if ((x == state.playerProperties[1][0]) &&
					(y == state.playerProperties[1][1]))
				cout << "A";
			// NPC
			else{
				if (hasMonster(state, x, y)){
					cout << "@";
				}
				else cout << " ";
			}

		}
		cout << "\n";
	}
	cout << "\n";
};

bool MazeWorld::hasMonster(const State& state, long x, long y, string monsterName){
	for (unsigned i=0; i< numWorlds; i++){
		if (!mazes[i]->isTermState(state, i) && (x == state.mazeProperties[i][0])
				&& y == state.mazeProperties[i][1]){
			if (monsterName != ""){
				if (mazes[i]->worldTypeStr == monsterName)
					return true;
				else return false;
			}
			else return true;
		}
	}
	return false;
};


char* MazeWorld::getPlayerStringXML(int playerIndex) {

	char *result;
	std::string tempStr;
	// 1. formality
	tempStr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

	// 2. message
	if (playerIndex == humanIndex)
		tempStr += "<player_role>boss</player_role>";
	else
		tempStr += "<player_role>assistant</player_role>";

	// 3. Convert to char array
	result = new char[tempStr.size() + 1];
	result[tempStr.size()] = 0;
	memcpy(result, tempStr.c_str(), tempStr.size());

	return result;
}
;

char* MazeWorld::getPlayerString(int playerIndex) {
	char *result;
	std::string tempStr;

	if (playerIndex == humanIndex)
		tempStr += "boss";
	else
		tempStr += "assistant";

	// 2. Convert to char array
	result = new char[tempStr.size() + 1];
	result[tempStr.size()] = 0;
	memcpy(result, tempStr.c_str(), tempStr.size());

	return result;
}

void MazeWorld::writeSolution(std::string filename) {
	/**
	 Only write a pointer to earlier model when
	 model is equivalent.
	 Format:
	 10 //numWorlds
	 0 // index, if equal to the index, it is original
	 5 // virtualSizes[0]: number of states in virtual world 0
	 1.2 2.3 3.4 4.5 5.6 // virtualValueFns[0]
	 1.2 2.3 3.4 4.5 5.6 // vHumanIndeptVFns[0]
	 1.2 2.3 3.4 4.5 5.6 // vAiIndeptVFns[0]
	 0 // index, not equal to the actual index, therefore a duplicate/replica

	 */

	/**
	 * New: Write filename.worldTypeStr.Ftn for each of the world type.
	 * Format:
	 * virtualSize
	 * Q value
	 *
	 * */
	ofstream fp;

	unsigned j, k;
	std::string subWorldFilename;
	for (long i = 0; i < mazes.size(); i++) {
		// This is an original world
		if (i == equivWorlds[i]) {

			// 1. Write Function file

			std::cout << "~~~ Writing Q function ~~~ "
					<< mazes[i]->worldTypeStr << "~~" << std::endl;

			subWorldFilename = filename + "." + mazes[i]->worldTypeStr;

			if (mazes[i]->useAbstract)
				subWorldFilename += ".1.Ftn";
			else
				subWorldFilename += ".0.Ftn";

			fp.open(subWorldFilename.c_str(), ofstream::binary);
			if (!fp.is_open()) {
				cerr << "Fail to open " << subWorldFilename << "\n";
				exit(EXIT_FAILURE);
			}
			stringstream input_string;

			// Set floatfield to 5 digits, i.e. the maximum number of digits after decimal point is 4.
			// weird???? why doesnt input_string.setf(0, ios::floatfield); work?
			// damn the cplusplus examples.
			input_string.setf(ios::fixed, ios::floatfield);
			input_string.precision(5);

			// 3. Write virtualSize
			input_string << mazes[i]->virtualSize << " ";

			// 5. Write collab Q Fns
			for (j = 0; j < (*(mazes[i]->collabQFn)).size(); j++) {
				// no need to write size, because size is always = numActs
				// input_string << (*(mazes[i]->collabQFn))[j].size() << " ";

				for (k = 0; k < (*(mazes[i]->collabQFn))[j].size(); k++) {
					input_string << (*(mazes[i]->collabQFn))[j][k] << " ";
				}
			}

			// std::cout << "Raw string~~~~~~\n"<< input_string.str() << std::endl << "~~~~~~~~~~" << std::endl;

			// Compress the input_string
			// -1 indicates default compression level, which is Z_BEST_COMPRESSION
			std::string raw_str = input_string.str();
			std::string compressed_str = Compression::compress_string(raw_str, -1);

			fp.write(compressed_str.c_str(), compressed_str.size());

			std::cout << "Deflated data: " << raw_str.size() << " -> "
					<< compressed_str.size() << " (" << std::setprecision(1)
					<< std::fixed << ((1.0 - (float) compressed_str.size()
					/ (float) raw_str.size()) * 100.0) << "% saved).\n";

			fp.close();

			/*
			// 2. Write reverseVAbsStateMap
			// Write the number of states first, then the size of each component of AbstractState

			std::cout << "~~~ Writing state map reverseVAbsStateMap ~~~ "
					<< mazes[i]->worldTypeStr << "~~" << std::endl;

			subWorldFilename = filename + "." + mazes[i]->worldTypeStr;

			if (mazes[i]->useAbstract)
				subWorldFilename += ".1.Map";
			else
				subWorldFilename += ".0.Map";

			fp.open(subWorldFilename.c_str(), ofstream::binary);
			if (!fp.is_open()) {
				cerr << "Fail to open " << subWorldFilename << "\n";
				exit(EXIT_FAILURE);
			}
			stringstream mapString;

			mapString.setf(ios::fixed, ios::floatfield);
			mapString.precision(5);

			// a. Write map size
			mapString << mazes[i]->reverseVAbsStateMap->size() << " ";

			// b. Write component size. The first element of reverseVAbsStateMap is reserved for
			// terminal state right?
			mapString << (*(mazes[i]->reverseVAbsStateMap))[1].playerProperties[0].size() << " ";
			mapString << (*(mazes[i]->reverseVAbsStateMap))[1].playerProperties[1].size() << " ";

			mapString << (*(mazes[i]->reverseVAbsStateMap))[1].monsterProperties.size() << " ";
			mapString << (*(mazes[i]->reverseVAbsStateMap))[1].specialLocationProperties.size() << " ";

			// c. Write each state into stringstream
			// state[0] is special
			AbstractState tempState;
			Utilities::cloneAbsState((*(mazes[i]->reverseVAbsStateMap))[1], tempState);
			tempState.playerProperties[0][0] = TermState; // 0 is regionID
			tempState.playerProperties[0][1] = TermState; // 1 is X

			Utilities::AbstractStateToOutStream(tempState, mapString);
			// state[1] onwards are normal
			for (j = 1; j < mazes[i]->reverseVAbsStateMap->size(); j++)
				Utilities::AbstractStateToOutStream((*(mazes[i]->reverseVAbsStateMap))[j], mapString);

			// d. Compress and write to file
			raw_str = mapString.str();
			compressed_str = Compression::compress_string(raw_str, -1);

			fp.write(compressed_str.c_str(), compressed_str.size());

			std::cout << "Deflated data: " << raw_str.size() << " -> "
					<< compressed_str.size() << " (" << std::setprecision(1)
					<< std::fixed << ((1.0 - (float) compressed_str.size()
					/ (float) raw_str.size()) * 100.0) << "% saved).\n";

			fp.close();
			 */
		} // original world
	} // for world
}
;

void MazeWorld::readSolution(std::string filename) {
	/**
	 Note that equivWorlds is not in the memory, we reconstruct it on the fly
	 */

	// Suppose we already have equivWorlds setup here.
	ifstream fp;

	std::string subWorldFilename;
	unsigned worldIndex, j, k, i;
	long vectorSize = player[0]->getNumActs() * player[1]->getNumActs();

	// For each virtual world
	for (i = 0; i < numWorlds; i++) {
		// Check if this world is original
		if (equivWorlds[i] == i) {

			subWorldFilename = filename + "." + mazes[i]->worldTypeStr;

			if (mazes[i]->useAbstract)
				subWorldFilename += ".1.Ftn";
			else
				subWorldFilename += ".0.Ftn";

			fp.open(subWorldFilename.c_str(), ios::in | ios::binary);
			if (!fp.is_open()) {
				cerr << "Fail to open " << subWorldFilename << "\n";
				exit(EXIT_FAILURE);
			}
			std::cout << "~~~ Reading Q function from file~~~ "
					<< mazes[i]->worldTypeStr << "~~" << std::endl;

			// Read and decompress data to string.
			int length;
			char * buffer;

			// get length of file:
			fp.seekg(0, ios::end);
			length = fp.tellg();
			fp.seekg(0, ios::beg);

			// allocate memory:
			buffer = new char[length];

			// read data as a block:
			fp.read(buffer, length);
			fp.close();

			std::string compressed_str(buffer, length);

			delete[] buffer;

			std::string raw_str = Compression::decompress_string(compressed_str);
			stringstream output_string(raw_str);

			// ------

			// By now, all the mazes should have been initialized.
			// I just need to read in their Q fns.

			// start reading numbers from output_string
			output_string >> mazes[i]->virtualSize;

			// Read virtual collab Q Functions
			mazes[i]->collabQFn = new vector<vector<double> > ;
			mazes[i]->collabQFn->resize(mazes[i]->virtualSize);

			for (j = 0; j < mazes[i]->virtualSize; j++) {
				// output_string >> vectorSize;
				(*(mazes[i]->collabQFn))[j].resize(vectorSize);
				for (k = 0; k < vectorSize; k++)
					output_string >> (*(mazes[i]->collabQFn))[j][k];
			}

		} else {

			// NO - this world is a replica of a previous world
			mazes[i]->copyValueQFns(mazes[equivWorlds[i]]);
		}

	} // for long i

	cout << "Done..." << endl;

}
;

// TODO ---------------------- Destructor
MazeWorld::~MazeWorld() {
	if (player[0]) {
		delete player[0];
		player[0] = 0;
	}

	if (player[1]) {
		delete player[1];
		player[1] = 0;
	}

	for (long i = 0; i < numWorlds; i++) {
		if (mazes[i]) {
			delete mazes[i];
			mazes[i] = 0;
		}
	}

}
;
