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



#ifndef __PLAYER_H
#define __PLAYER_H

#include "Agent.h"
#include "Distribution.h"
#include "RandSource.h"

using namespace std;

// OPTIMAL_FACTOR is used to increase belief
#define OPTIMAL_FACTOR 2
#define OPTIMAL_EPS 0.00001

class MazeWorld;

/**
 @class Player
 @brief Base class of individual game's Human and AiAssistant.
 @details Similar to Monster, players could move and do special actions such as shoot or fight. Special actions do not affect the visibility of monster in a maze w.r.t the agents.

 @author Huy Nguyen
 @date Dec 2010
 */
class Player: public Agent {
private:
	const static int surpriseFactor = 2;

	long numWorlds;

public:

	double stayInSameWorld;

	/**
	 This parameter controls the function getActionModel to translate from Q value to action probability. Default value 10 is set in constructor.
	 */
	double actionMult;

	/**
	 Index of this agent. Can take value of \a humanIndex or \a aiIndex.
	 */
	int agentIndex;

	/**
	 The parent game level.
	 */
	MazeWorld *mazeWorld;

public:
	/**
	 Constructor.
	 @param[in] x initial X coord.
	 @param[in] y initial Y coord.
	 @param[in] numMoveActions number of movement actions.
	 @param[in] numSpecialActions number of special actions.
	 @param[in] numProperties number of properties.
	 @param[in] agentIndex index of this agent.
	 @param[in] mW the parent game level.
	 */
	Player(long x, long y, long numMoveActions, long numSpecialActions,
	    long numProperties, int agentIndex, long numWorlds, MazeWorld *mW) :
		Agent(x, y, numMoveActions, numSpecialActions, numProperties),
		    agentIndex(agentIndex), numWorlds(numWorlds), mazeWorld(mW) {
		// by default, actionMult = 10
		actionMult = 10;
		stayInSameWorld = 0.8;
	}
	;

	virtual void cleanUp(){};

	// utility function that counts the number of alive worlds in terminalArray.
	long getNumAliveWorlds(vector<int>& terminalArray);


	/**
	 Maps from character \a c to a human action. This class does not implement any special action for the human. If the game needs special actions for human, this needs extending.
	 @param[in] c the character input.
	 */
	virtual long actionFromChar(const char c) = 0;

	/*********************** AbstractState related **************/
	/**
	 Enumerates properties, i.e. filling the states in input with all possible values of \a agentIndex's properties.
	 @param[in] input array of input AbstractState.
	 @param[out] output array of output AbstractState.
	 @param[in] dealtAgent used to identify properties that could be ignored when solving.
	 */
	virtual void enumerateProperties(std::vector<AbstractState>& input,
	    std::vector<AbstractState>& output, const Agent* dealtAgent = 0);

	/**
	 Executes special action \a act. By default, does nothing.
	 @param[in] input array of AbstractState with probability.
	 @param[in] act
	 @param[in] output array of AbstractState with probability.
	 */
	virtual void absExecuteSpecialAct(
	    std::vector<std::pair<AbstractState, double> >& input, long act,
	    std::vector<std::pair<AbstractState, double> >& output) {
		Utilities::copyAbstractProbArray(input, output);
	}
	;

	/******* Human-controlled related **********************************/
	/**
	 Returns probability of human actions. This is where we can try different human models. The default one implemented here is an optimal human's model.
	 @param[in] currState current State
	 @param[out] condProbAct Probability of each human action given the Maze's index.
	 */
	virtual void getActionModel(const State& currState,
	    std::vector<std::vector<double> >& condProbAct);

	//virtual void getActionModelCrudeForm(const State& currState,
	//    std::vector<std::vector<double> >& condProbAct);
	virtual void getBestCompoundActions(const State& currState,
			std::vector<std::vector<long> >& bestCompoundActions);

	/**
	 Samples a human action given \a aiAct. By default, the human's action model is trying to chase the nearest monster/NPC.
	 @param[in] currState current State.
	 @param[in] aiAct assistant's action.
	 @param[in] condProbAct Probability of each human action given the Maze's index.
	 @param[in] randSource random source to sample.
	 @return sampled human action.
	 */
	virtual long sampleAct(const State& currState, long aiAct,
	    std::vector<std::vector<double> >& condProbAct, RandSource& randSource);

	/******* AI-controlled related ***********************/
	/**
	 Sets number of worlds. If at construction time, the number of worlds is not known, it can be set here.
	 */
	void setNumWorlds(int noWorlds) {
		numWorlds = noWorlds;
	}
	;

	/**
	 * Returns the initial belief distribution. By default, initial belief is set to uniform.
	 * @return number of alive worlds.
	 * */
	virtual long getInitBelief(vector<double>& wBelief, const State *state = 0);

	/**
	 * Same as \a getInitBelief, except that belief is inversely proportional to distance.
	 * The nearer human is to NPC, the higher the belief.
	 * */
	virtual long getInitBeliefDistance(vector<double>& wBelief, const State *state);

	/**
	 Chooses the world to act optimally. This routine is called by policyRoutine in MazeWorld.
	 @param[in] currState current State.
	 @return the world to act optimally, by default, the most probable one.
	 */
	virtual long getMostLikelyWorld(const State& currState,
	    vector<double>& wBelief) {
		return Distribution::getMax(wBelief, 0, wBelief.size() - 1);
	}
	;

	/**
	 Updates the world belief given observation \a humanAct.
	 @param[in] humanAct human's action.
	 @param[in] condProbAct the human's action model. Dim: worldIndex, action, probability
	 @param[in] failArray 1 if action succeed in that world, 0 otherwise. This is to detect dead locks.
	 @param[in] terminalArray stores terminal statuses of the worlds. 1 if terminal and 0 otherwise.
	 */
	virtual void updateBelief(vector<double>& wBelief, long humanAct,
	    std::vector<std::vector<double> >& condProbAct, vector<int>& failArray,
	    vector<int>& terminalArray);

	/**
	 * Invoked when there is only one alive world in terminalArray.
	 * The updated array wBelief contains 1 for the only alive world.
	 * */
	void makeBelief(vector<double>& wBelief, vector<int>& terminalArray);

	/**
	 * Indicates whether it should switch to online search at the current state.
	 * To be extended by subclasses.
	 * @return true if should do online search, false otherwise.
	 * */
	virtual bool shouldDoOnlineSearch(const State& state){
		return false;
	};

	/********** Stupid AI ***********/
	virtual void getStupidAct(const State& currState, long playerAct,
	    int playerIndex, long& aiAct, RandSource& randSource, int scriptMode = 0);
};

#endif
