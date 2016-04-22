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



#include "Player.h"
#include "MazeWorld.h"

void Player::enumerateProperties(std::vector<AbstractState>& input,
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
		//currSize = state.playerProperties[agentIndex].size();
		//state.playerProperties[agentIndex].resize(currSize + properties.size(), 0);

		// 2. Enumerate properties and place in tempProperties
		Agent::enumerateProperties(state.playerProperties[agentIndex],
		    tempProperties, dealtAgent);

		// 3. Add to output
		for (unsigned j = 0; j < tempProperties.size(); j++) {
			state.playerProperties[agentIndex] = tempProperties[j];
			output.push_back(state);
		}
	}
}
;

/**
 * bestCompoundActions stores the best compound actions in each of the subworlds.
 * */
void Player::getBestCompoundActions(const State& currState,
    std::vector<std::vector<long> >& bestCompoundActions) {
	// Caller already checked for terminality of the state

	// counters, act = human act, ca = compound act
	long tempVState;
	bestCompoundActions.resize(mazeWorld->numWorlds);
	long numActs = getNumActs() * mazeWorld->player[1 - agentIndex]->getNumActs();

	double maxQValue, value;
	for (unsigned i = 0; i < mazeWorld->numWorlds; i++) {

		bestCompoundActions[i].clear();

		// if this is not terminal state
		if (!mazeWorld->mazes[i]->isTermState(currState, i)) {
			tempVState = (mazeWorld->mazes[i])->realToVirtual(currState, i);
			maxQValue = Distribution::getMaxValue((*(mazeWorld->mazes[i])->collabQFn)[tempVState]);

			for (long act = 0; act < numActs; act++) { // act = compound act
				value = (*(mazeWorld->mazes[i])->collabQFn)[tempVState][act];
				if (fabs(value - maxQValue) < OPTIMAL_EPS)
					bestCompoundActions[i].push_back(act);
			}
		} // if termState
	} // for long i
}
;


/**
 * condProbAct is NOT normalized to retain the Q value.
 * */
void Player::getActionModel(const State& currState,
    std::vector<std::vector<double> >& condProbAct) {
	// Caller already checked for terminality of the state

	// sumProb stores the summation of all probabilities involved to use for normalization
	double maxQValue, tempQValue;
	//double sumProb;

	// counters, act = human act, ca = compound act
	unsigned i, act;
	long tempVState;

	// 5. Construct 2D matrix p( a | i ) with x dimension as worldNum, y dimension as actNum
	// this part is used to infer the world given human's action.
	// Plan:
	// 1. For all worlds
	//       condProbAct of Optimal human actions are set to optimalFactor
	//       condProbAct of Suboptimal human actions are set to suboptimalFactor
	condProbAct.resize(mazeWorld->numWorlds);

	// std::vector<bool> isOptimalAct;

	for (i = 0; i < mazeWorld->numWorlds; i++) {

		// isOptimalAct.resize(0);
		// if this is not terminal state
		if (!mazeWorld->mazes[i]->isTermState(currState, i)) {
			condProbAct[i].resize(getNumActs(), 0);
			tempVState = (mazeWorld->mazes[i])->realToVirtual(currState, i);
			//sumProb = 0;
			for (act = 0; act < getNumActs(); act++) { // act = human act

				// 1. get max value of (*(mazeWorld->mazes[i])->collabQFn)[tempVState][compAct] where act is part of
				maxQValue = (*(mazeWorld->mazes[i])->collabQFn)[tempVState][0];
				for (unsigned partnerAct = 0; partnerAct < mazeWorld->player[1
				    - agentIndex]->getNumActs(); partnerAct++) {
					if (agentIndex == humanIndex)
						tempQValue = (*(mazeWorld->mazes[i])->collabQFn)[tempVState][act
						    * mazeWorld->player[1 - agentIndex]->getNumActs() + partnerAct];
					else
						tempQValue
						    = (*(mazeWorld->mazes[i])->collabQFn)[tempVState][partnerAct
						        * getNumActs() + act];
					if (maxQValue < tempQValue)
						maxQValue = tempQValue;
				}

				// condProbAct[i][act] = exp(actionMult * (*(mazeWorld->mazes[i])->collabQFn)[tempVState][act]);
				condProbAct[i][act] = maxQValue;// exp(actionMult * (maxQValue / mazeWorld->mazes[i]->getMaxReward()));
				//sumProb += condProbAct[i][act];
			}

			// Scale to 0, 1
			Distribution::scaleTo_0_1(condProbAct[i]);

			// Soft max
			for (act = 0; act < getNumActs(); act++)
				condProbAct[i][act] = exp(actionMult * condProbAct[i][act]);

			//for (act = 0; act < getNumActs(); act++) { // act = player act
			//	condProbAct[i][act] /= sumProb;
			//}
		} // if termState
		// if this is terminal state, condProbAct[i][act] are all 0
#ifdef DEBUG
		cout << "p(player action | world) in world " << i << ": ";
		Distribution::printDistrib(condProbAct[i], 0, condProbAct[i].size()-1);
#endif
	} // for long i
}
;

long Player::sampleAct(const State& currState, long partnerAct,
    std::vector<std::vector<double> >& condProbAct, RandSource& randSource) {

	// dimension: worldIndex, actionIndex -> probability
	// std::vector < std::vector <double> > condProbAct;

	getActionModel(currState, condProbAct);

	// choose the action that maximizes sum of probabilities conditioned on world.
	// the value is weighted by distance from human to NPC.

	std::vector<double> worldWeight;
	long hNode, mNode, i;
	hNode=mazeWorld->gridNodeLabel[currState.playerProperties[agentIndex][0]][currState.playerProperties[agentIndex][1]];
	worldWeight.resize(mazeWorld->numWorlds, 0);

	for (i = 0; i < mazeWorld->numWorlds; i++) {
		if ((mazeWorld->mazes[i])->isTermState(currState, i))
			worldWeight[i] = 0;
		else {
			mNode = (*(mazeWorld->mazes[i])->gridNodeLabel)[currState.mazeProperties[i][0]][currState.mazeProperties[i][1]];
			worldWeight[i] = 1.0 / (*shortestPath)[hNode][mNode];
		}
	}// for long i

	double sumActProb, maxSumActProb;
	long bestAct;
	maxSumActProb = -FLT_MAX;
	for (long act = 0; act < getNumActs(); act++) {
		sumActProb = 0;
		for (long i = 0; i < numWorlds; i++)
			sumActProb += condProbAct[i][act] * worldWeight[i];

		if (maxSumActProb < sumActProb) {
			maxSumActProb = sumActProb;
			bestAct = act;
		}
	}

	return bestAct;

	/*
	 // 1. Get list of worlds with monster that are not terminal yet.
	 std::vector<long> monsterWorlds(0);
	 unsigned i;
	 for (i = 0; i < mazeWorld->numWorlds; i++) {
	 if ((mazeWorld->mazes[i]->monster)
	 && (!(mazeWorld->mazes[i])->isTermState(currState, i)))
	 monsterWorlds.push_back(i);
	 }

	 if (!monsterWorlds.empty()) {

	 // 2. Choose the monster closest to player's position.

	 vector<vector<long> >* gridNodeLabel;
	 gridNodeLabel = &(mazeWorld->gridNodeLabel);

	 long humanNode =
	 (*gridNodeLabel)[currState.playerProperties[humanIndex][0]][currState.playerProperties[humanIndex][1]];

	 long nearestMonster = 0;

	 long currMonsterX =
	 currState.mazeProperties[monsterWorlds[nearestMonster]][0];
	 long currMonsterY =
	 currState.mazeProperties[monsterWorlds[nearestMonster]][1];

	 long currSNode = (*gridNodeLabel)[currMonsterX][currMonsterY];

	 // distance to human
	 long nearest_distance = (*shortestPath)[currSNode][humanNode];

	 for (i = 1; i < monsterWorlds.size(); i++) {
	 currMonsterX = currState.mazeProperties[monsterWorlds[i]][0];
	 currMonsterY = currState.mazeProperties[monsterWorlds[i]][1];

	 currSNode = (*gridNodeLabel)[currMonsterX][currMonsterY];

	 if (nearest_distance > (*shortestPath)[currSNode][humanNode]) {
	 nearest_distance = (*shortestPath)[currSNode][humanNode];
	 nearestMonster = i;
	 }
	 }

	 // 3. Return the most probable (perhaps most optimal) act in condProbAct[index]

	 return Distribution::getMax(condProbAct[monsterWorlds[nearestMonster]],
	 0, condProbAct[monsterWorlds[nearestMonster]].size() - 1);

	 } else {
	 // 1. Act optimally in the first non-terminal world in maze list
	 for (i = 0; i < mazeWorld->numWorlds; i++) {
	 if (!(mazeWorld->mazes[i])->isTermState(currState, i))
	 return Distribution::getMax(condProbAct[i], 0,
	 condProbAct[i].size() - 1);
	 }
	 }

	 // dummy return
	 return 0;
	 */
}
;

long Player::getNumAliveWorlds(vector<int>& terminalArray)
{
	long numAliveWorlds = 0;

	for (unsigned i = 0; i < terminalArray.size(); i++) {
		if (!terminalArray[i])
			numAliveWorlds++;
	}
	return numAliveWorlds;
};

void Player::makeBelief(vector<double>& wBelief, vector<int>& terminalArray)
{

	for (unsigned i = 0; i < terminalArray.size(); i++) {
		wBelief[i] = 0.0;
		if (!terminalArray[i])
			wBelief[i] = 1.0;

	}
}

void Player::updateBelief(vector<double>& wBelief, long humanAct,
    std::vector<std::vector<double> >& condProbAct, vector<int>& failArray,
    vector<int>& terminalArray) {

	long numAliveWorlds = getNumAliveWorlds(terminalArray);

	//cout << "Num alive worlds = " << numAliveWorlds << endl;
	if (numAliveWorlds <= 1){
		makeBelief(wBelief, terminalArray);
		return;
	}

	double sumProb = 0;
	unsigned i, j;

	// Step 1: Drift model
	vector<double> prevBelief;
	prevBelief = wBelief;

	for (i = 0; i < wBelief.size(); i++) {
		wBelief[i] = 0;
		if (!terminalArray[i]){
			for (j = 0; j < prevBelief.size(); j++) {
				if (j==i)
					wBelief[i] += stayInSameWorld * prevBelief[j];
				else wBelief[i] += (1-stayInSameWorld) * prevBelief[j] / (numAliveWorlds-1);
			}
		}
	}

	// Step 2: Bayesian inference
	// normalize condProbAct
	for (unsigned i = 0; i < condProbAct.size(); i++){
		Distribution::normalize(condProbAct[i]);
	}

	for (i = 0; i < terminalArray.size(); i++) {
		// 0: not terminal
		if (!terminalArray[i])
			// update belief
			if (failArray[i])
				wBelief[i] *= (condProbAct[i][humanAct] * surpriseFactor);
			else
				wBelief[i] *= (condProbAct[i][humanAct] / surpriseFactor);
		//wBelief[i] *= condProbAct[i][humanAct];
		else
			wBelief[i] = 0;
		sumProb += wBelief[i];
	}

	for (i = 0; i < wBelief.size(); i++) {
		if (sumProb > 0)
			wBelief[i] /= sumProb;
		else
			wBelief[i] = 1.0 / wBelief.size();
	}

}
;

long Player::getInitBelief(vector<double>& wBelief, const State *state)
{
	long numAliveWorlds = 0;
	wBelief.resize(0);
	for (long i = 0; i < numWorlds; i++){
		if (!state){
			wBelief.push_back(1.0);
			numAliveWorlds++;
		}
		else{
			if (mazeWorld->mazes[i]->isTermState(*state, i))
				wBelief.push_back(0.0);
			else{
				wBelief.push_back(1.0);
				numAliveWorlds++;
			}
		}
	}

	for(long i=0; i<numWorlds; i++)
		wBelief[i] /= numAliveWorlds;

	return numAliveWorlds;
};

long Player::getInitBeliefDistance(vector<double>& wBelief, const State *state)
{
	int x=0,y=1;
	long numAliveWorlds = 0;
	long otherAgentNode = mazeWorld->gridNodeLabel[state->playerProperties[1-agentIndex][x]]
	                                               [state->playerProperties[1-agentIndex][y]];
	long NPCNode;
	wBelief.resize(0);
	for (long i = 0; i < numWorlds; i++){
		if (mazeWorld->mazes[i]->isTermState(*state, i))
			wBelief.push_back(0.0);
		else{
			NPCNode = mazeWorld->gridNodeLabel[state->mazeProperties[i][x]]
                                               [state->mazeProperties[i][y]];
			// determine the distance from the other agent to this NPC
			wBelief.push_back(1.0 / (*shortestPath)[otherAgentNode][NPCNode]);
			numAliveWorlds++;
		}
	}

	Distribution::normalize(wBelief);
	return numAliveWorlds;
};

/************ Stupid AI *************/

/**
 *
 * @param[in] scriptMode 0:do nothing, 1:random, 2:follows human
 *
 * */
void Player::getStupidAct(const State& currState, long playerAct,
    int playerIndex, long& aiAct, RandSource& randSource, int scriptMode) {

	switch(scriptMode){
	case 1:
		aiAct = randSource.get() % getNumActs();
		break;
	case 2:
		{
			// Move to human's position.
			// if already in, shoot.
			long hx = currState.playerProperties[playerIndex][0];
			long hy = currState.playerProperties[playerIndex][1];

			long ax = currState.playerProperties[1 - playerIndex][0];
			long ay = currState.playerProperties[1 - playerIndex][1];
			// 1. if in the same grid, do nothing
			if (hx == ax && hy == ay){
				aiAct = unchanged;
			}
			// 2. Otherwise, move towards player
			else {
				aiAct = unchanged;
				long aNode = mazeWorld->gridNodeLabel[ax][ay];
				long hNode = mazeWorld->gridNodeLabel[hx][hy];
				double currDistance = (*shortestPath)[aNode][hNode];
				long nextNode;
				for(unsigned i=1; i<5; i++){
					nextNode = mazeWorld->gridNodeLabel[ax + RelativeDirX[i]][ay + RelativeDirY[i]];
					if (nextNode >= 0 && (*shortestPath)[nextNode][hNode] < currDistance){
						currDistance = (*shortestPath)[nextNode][hNode];
						aiAct = i;
					}
				}
			}
			break;
		}
	default:
		aiAct = unchanged;
	}

}
;
