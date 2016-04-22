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



#ifndef __MONSTER_H
#define __MONSTER_H

#include "Agent.h"
#include "Distribution.h"

using namespace std;

class Maze;

/**
 @class Monster
 @brief Base class of all NPCs.
 @details NPCs could move and do special actions such as shoot or figh. Special actions do not affect the visibility of monster in a maze w.r.t the agents. Monster always belongs to one specific maze.

 @author Huy Nguyen
 @date Dec 2010
 */
class Monster: public Agent {
protected:
	/**
	 The parent Maze. \a visionLimit is defined in \a maze.
	 */
	Maze* maze;

public:
	bool blAgents;
	bool blMonsters;

	/**
	 Probability of which this NPC executes its intended action. When OptimalProb is less than 1.0, there's a small chance this NPC acts randomly.
	 */
	double OptimalProb;
	/**
	 Constructor. By default, OptimalProb = 0.9.
	 @param[in] x initial X coord.
	 @param[in] y initial Y coord.
	 @param[in] numMoveActs number of movement actions.
	 @param[in] numSpecialActs number of special actions.
	 @param[in] numProperties number of properties.
	 @param[in] maze the parent Maze's pointer.
	 */
	Monster(long x, long y, long numMoveActs, long numSpecialActs,
	    long numProperties, Maze *maze) :
		Agent(x, y, numMoveActs, numSpecialActs, numProperties), maze(maze),
		blAgents(true), blMonsters(true)
	{
		OptimalProb = 0.9;
	}
	;

	/*
	 * Set whether the movement of this NPC is blocked by the players or fellow NPCs
	 * @param[in] bA blocked by players
	 * @param[in] bM blocked by other NPCs
	 *
	 * */
	void setBlocking(bool bA, bool bM){
		blAgents = bA;
		blMonsters = bM;
	};

	/************ get monster info ************/
	/**
	 Checks if this NPC can react given current AbstractState. Used when planning/solving.
	 @param[in] absState the AbstractState in question.
	 */
	virtual bool canReact(const AbstractState& absState);
	/**
	 Checks if this NPC can react given current raw State. Used when in-game.
	 @param[in] state the State in question.
	 @param[in] worldNum the world index this NPC belongs to.
	 */
	virtual bool canReact(const State& state, long worldNum);
	/**
	 Customized agent blocking condition. By default, blocks all agents.
	 */
	virtual bool blocksAgents() {
		return blAgents;
	}
	;
	/**
	 Customized monster blocking condition. By default, blocks all other monsters.
	 */
	virtual bool blocksMonsters() {
		return blMonsters;
	}
	;
	/************ abstract related ************/
	/**
	 Enumerate properties, i.e. filling the states in input with all possible values of monster's properties.
	 @param[in] input array of input AbstractState.
	 @param[out] output array of output AbstractState.
	 @param[in] dealtAgent used to identify properties that could be ignored when solving.
	 */
	virtual void enumerateProperties(std::vector<AbstractState>& input,
	    std::vector<AbstractState>& output, const Agent* dealtAgent = 0);

	/**
	 absReact on a list of AbtractStates with corresponding probability.
	 Reacts at planning stage given players' actions \a humanAct and \a aiAct.
	 By default, this routine checks for visibility of agents and invokes
	 the corresponding \a absGetAct* routine. It then gather all
	 possible resulted AbstractStates into \a output.
	 */
	void absReact(std::vector<std::pair<AbstractState, double> >& input,
	    long humanAct, long aiAct,
	    std::vector<std::pair<AbstractState, double> >& output,
	    const AbstractState& prevAbsState);

	/*
	 * absReact on single AbstractState.
	 * */
	virtual void absReact(AbstractState& currAbsState, double prob,
	    long humanAct, long aiAct,
	    std::vector<std::pair<AbstractState, double> >& output,
	    const AbstractState& prevAbsState);

	/**
	 When either players execute a special action, that can affect this NPC. By default, there's no effect.
	 @param[in] input array of AbstractState with probability.
	 @param[in] humanAct
	 @param[in] aiAct
	 @param[in] output array of AbstractState with probability.
	 */
	virtual void absGetAffectedByPlayersActions(
	    std::vector<std::pair<AbstractState, double> >& input, long humanAct,
	    long aiAct, std::vector<std::pair<AbstractState, double> >& output) {
		Utilities::copyAbstractProbArray(input, output);
	}
	;

	/**
	 Executes special action \a act, given \a humanAct and \a aiAct.
	 @param[in] currAbsState
	 @param[in] prob \a currAbsState's probability.
	 @param[in] humanAct
	 @param[in] aiAct
	 @param[in] act this NPC's special action.
	 @param[in] probAct probability of this action. This will get multiplied in the resulted \a output.
	 @param[in] output array of AbstractState with probability.
	 @param[in] prevAbsState
	 */
	virtual void absExecuteSpecialAct(AbstractState& currAbsState, double prob,
	    long humanAct, long aiAct, long act, double probAct,
	    std::vector<std::pair<AbstractState, double> >& output,
	    const AbstractState& prevAbsState);

	/**
	 Planning: Gets the set of actions with probability that this NPC should execute when seeing both players. The default behavior of a monster is to move farther away from the two.
	 */
	virtual void absGetActHumanAgentSeen(const AbstractState& absState,
	    long humanAct, long aiAct, const AbstractState& prevAbsState,
	    std::vector<std::pair<long, double> >& action_prob);
	/**
	 Planning: Gets the set of actions with probability that this NPC should execute when seeing one of the two players. The default behavior of a monster is to move farther away from the one seen.
	 */
	virtual void absGetAct1AgentSeen(const AbstractState& absState,
	    long humanAct, long aiAct, int agentIndex,
	    const AbstractState& prevAbsState,
	    std::vector<std::pair<long, double> >& action_prob);
	/**
	 Planning: Gets the set of actions with probability that this NPC should execute when not seeing any player. The default behavior of a monster is to move randomly.
	 */
	virtual void absGetActNoneSeen(const AbstractState& absState, long humanAct,
	    long aiAct, const AbstractState& prevAbsState,
	    std::vector<std::pair<long, double> >& action_prob);

	/******************* Raw state related ****************************/
	/**
	 Applies special action \a act on state. By default do nothing.
	 */
	virtual void executeSpecialAct(State& state, long act, long worldNum) {
	}
	;

	/**
	 This routine is invoked when either human or assistant action is special. It's up to the monster to be inflicted by the actions.
	 */
	virtual void getAffectedByPlayersActions(State& state, long worldNum,
	    long humanAct, long aiAct, RandSource& randSource) {
	}
	;

	/**
	 This routine provides a monster's reaction w.r.t whether it sees human or assistant or both of them.
	 @param[in] monsterActions stores the action of this monster.
	 */
	virtual void react(State& state, long worldNum, std::vector<long>& monsterActions, RandSource& randSource);

	/**
	 * In-game: Gets the set of actions that are prioritized to execute before anything.
	 * By default, there's no such thing.
	 * */
	virtual void priorityActions(const State& state, long worldNum,
		    std::vector<std::pair<long, double> >& action_prob){};
	/**
	 In-game: Gets the set of actions with probability that this NPC should execute when seeing both players. The default behavior of a monster is to move farther away from the two.
	 */
	virtual void getActHumanAgentSeen(const State& state, long worldNum,
	    std::vector<std::pair<long, double> >& action_prob);
	/**
	 In-game: Gets the set of actions with probability that this NPC should execute when seeing one of the two players. The default behavior of a monster is to move farther away from the one seen.
	 */
	virtual void getAct1AgentSeen(const State& state, int agentIndex,
	    long worldNum, std::vector<std::pair<long, double> >& action_prob);
	/**
	 In-game: Gets the set of actions with probability that this NPC should execute when not seeing any player. The default behavior of a monster is to move randomly.
	 */
	virtual void getActNoneSeen(const State& state, long worldNum,
	    std::vector<std::pair<long, double> >& action_prob);

	/******************* Predefined behaviors of Monster **************/

	/**
	 Gets the set of actions with probability that this NPC should do to move away from both players. Invoked by both absGetAct* and getAct*. Best action has probability \a OptimalProb and all others share 1 - OptimalProb.
	 @param[in] state if \a state is non-zero, there'll be a check for collision in real state. Otherwise, it's just planning in sub-world, and no need for collision checks.
	 */
	virtual void moveActsFarthestFromBothPlayers(long currMonsterX,
	    long currMonsterY, long humanX, long humanY, long aiX, long aiY,
	    std::vector<std::pair<long, double> >& action_prob,
	    const State* state = 0);
	/**
	 Gets the set of actions with probability that this NPC should do to move away from a particular players. Invoked by both absGetAct* and getAct*. Best action has probability \a OptimalProb and all others share 1 - OptimalProb.
	 @param[in] state if \a state is non-zero, there'll be a check for collision in real state. Otherwise, it's just planning in sub-world, and no need for collision checks.
	 */
	virtual void moveActsFarthestFromPlayer(long currMonsterX, long currMonsterY,
	    long agentX, long agentY, unsigned agentIndex, long otherX, long otherY,
	    std::vector<std::pair<long, double> >& action_prob,
	    const State* state = 0);

	/*
	 * Similar to \a moveActsFarthestFromPlayer with action moving into point \a exPointX,
	 * \a exPointY excluded.
	 * */
	virtual void moveActsFarthestFromPlayerExcludePoint(long currMonsterX, long currMonsterY,
		long agentX, long agentY, unsigned agentIndex, long otherX, long otherY,
		std::vector<std::pair<long, double> >& action_prob,
		long exPointX, long exPointY,
		const State* state = 0);

	/**
	 Gets the set of actions with probability that this NPC should do to move randomly. Invoked by both absGetAct* and getAct*. Best action has probability \a OptimalProb and all others share 1 - OptimalProb.
	 @param[in] state if \a state is non-zero, there'll be a check for collision in real state. Otherwise, it's just planning in sub-world, and no need for collision checks.
	 */
	virtual void randomMoveActs(long currMonsterX, long currMonsterY,
	    std::vector<std::pair<long, double> >& action_prob,
	    const State* state = 0);
	/**
	 Gets the set of actions with probability that this NPC should do to move towards a specific location. Invoked by both absGetAct* and getAct*. Best action has probability \a OptimalProb and all others share 1 - OptimalProb.
	 @param[in] state if \a state is non-zero, there'll be a check for collision in real state. Otherwise, it's just planning in sub-world, and no need for collision checks.
	 */
	void moveActsTowardsLocation(long currMonsterX, long currMonsterY,
	    long destX, long destY,
	    std::vector<std::pair<long, double> >& action_prob,
	    const State* state = 0);
	/*
	 * Similar to \a moveActsTowardsLocation with action moving into point \a exPointX,
	 * \a exPointY excluded.
	 * */
	void moveActsTowardsLocationExcludePoint(long currMonsterX, long currMonsterY,
		long destX, long destY,
		std::vector<std::pair<long, double> >& action_prob,
		long exPointX, long exPointY,
		const State* state = 0);

	/**
	 Gets the set of actions with probability that this NPC should do to move away from a specific location. Invoked by both absGetAct* and getAct*. Best action has probability \a OptimalProb and all others share 1 - OptimalProb.
	 @param[in] state if \a state is non-zero, there'll be a check for collision in real state. Otherwise, it's just planning in sub-world, and no need for collision checks.
	 */
	void moveActsAwayFromLocation(long currMonsterX, long currMonsterY,
	    long destX, long destY,
	    std::vector<std::pair<long, double> >& action_prob,
	    const State* state = 0) {
		moveActsFarthestFromPlayer(currMonsterX, currMonsterY, destX, destY, humanIndex, -1, -1,
		    action_prob, state);
	}
	;

};

#endif
