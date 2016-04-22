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



#ifndef __MAZEWORLD_H
#define __MAZEWORLD_H

#include "pugixml.hpp"
#include "Model.h"
#include "Player.h"
#include "Maze.h"
#include "GameTileSheet.h"
#include "MazeWorldDescription.h"
#include <queue>

using namespace std;

/**
 @class MazeWorld
 @brief This class is the base class for a game's level in which the protagonists need to solve puzzles to pass.
 @details This class holds
 - References to the players and geometrical data, used for in-game dynamics.
 - References to all individual Maze's, each of which contains references to at most one Monster.
 Note that in this new version, human = player[0], assistant = player[1]

 @author Truong Huy Nguyen
 @date December 2010
 */
class MazeWorld: public Model {
protected:
	/**
	 Originally, the last state is updated by the MazeWorld, which overrides human's coordinates with TermState. In order to facilitate display for client, the state before being overriden is stored here.
	 */
	AugmentedState lastState;

public:

	/**** Components of a World ********/
	/**
	 Pointer to the Player objects.
	 */
	Player* player[2];

	/**
	 Vector of pointers to individual Mazes.
	 */
	vector<Maze*> mazes;

	/**
	 Number of individual Mazes.
	 */
	long numWorlds;
	/**
	 Type of the game goal. It could take value of Utilities::orType, i.e. the game ends when one of the worlds (Mazes) ends, or Utilities::andType, i.e. the game ends only when all of the worlds end.
	 */
	Utilities::goalType gType;
	/**
	 The vector of original worlds' indices, i.e. earliests model that is equiv to this one.
	 */
	vector<long> equivWorlds;

	/*********** Input Planning info ************/

	/**
	 The default vision limit of all monsters.
	 */
	double visionLimit;

	bool monsterBlock;
	/**
	 Blocking flag for agents. True of agents block each other, false otherwise.
	 */
	bool agentBlock;

	// bool monsterAgentBlock;

	/**
	 Number of regions in the map.
	 */
	long numRegionPerAgent;
	/**
	 Grid's size.
	 */
	long xSize, ySize;
	/**
	 Raster scanned grid.
	 */
	vector<vector<long> > grid;
	/**
	 Value Iteration's epsilon value, below which VI terminates.
	 */
	double targetPrecision;
	/**
	 Time interval to display the value's difference.
	 */
	long displayInterval;

	/******* Computed geographical info ****/
	// for computing shortest path
	/**
	 Integer label of accessible grid squares.
	 */
	vector<vector<long> > gridNodeLabel;

	vector<pair<long, long> > reverseGridNodeLabel;

	/**
	 Number of accessible grid squares in the grid.
	 */
	long numAccessibleLocs;
	// NOT USED - discounted reward as a function of number of steps
	// vector<double> discountedReward;

	/**
	 Array indexed by region index containing neighbor regions on the east, south, west and north sides.
	 */
	vector<vector<long> > connectivity;

	/**
	 The lengths of common edges between neighbor regions, indexed by region id and neighbor region id. borderLength[i][unchanged] is the area of that region, borderLength[i][east/west/south/norht] is either 1 (next to a region) or 0 (next to wall)
	 */
	vector<vector<long> > borderLength;

	/**
	 Representative points of regions, being the westernmost/southernmost coords. Indexed by region.
	 */
	vector<pair<long, long> > regionRepPoint;

	/**
	 Region type. Could be vertical, horizon or junction.
	 */
	vector<regionType> rType;

public:
	/**
	 Constructor. MazeWorldDescription provides initialization for all parameters.
	 */
	MazeWorld(MazeWorldDescription& desc);

	/**
	 NOTE: terminalArray is not updated, why??? Cause it's not needed?

	 Dynamics of the real world. Given current state of the world, players' action, returns a sample of the next state and its reward. realDynamics is always called when currState is not terminal, so there's no need to check for terminus.

	 @return Reward of doing \a act in \a currState.
	 @param[in] currState Current state
	 @param[in] player0Act Index of human action to perform
	 @param[in] player1Act Index of AI action to perform
	 @param[out] nextState Sampled next state. This is random and changes from
	 invocation to invocation.
	 @param[out] succeedArray
	 @param[out] terminalArray
	 @param[in] randSource Source of random numbers
	 */
	double realDynamics(const State& currState, long player0Act, long player1Act,
	    State& nextState, vector<int>& succeedArray, vector<int>& terminalArray,
	    vector<long>& monsterActions, RandSource& randSource);

	/**
	 Sample the other action given one action. wBelief is updated in the process.
	 @param[in] currState Current state
	 @param[in] wBelief world's belief.
	 @param[in] aiAct
	 @param[out] humanAct the sampled human action.
	 @param[out] nextState Sampled next state. This is random and changes from
	 invocation to invocation.
	 @param[in] randSource Source of random numbers
	 */
	double sample(const State& currState, vector<double>& wBelief,
	    long& humanAct, long& aiAct, State& nextState, vector<long>& monsterActions,
	    RandSource& randSource);

	/**
	 Duplicate of sample routine, used for updating belief based on real human actions.

	 @return reward after executing player0Act and player1Act
	 @param[in] currState current state
	 @param[out] nextState next state
	 */
	double moveState(const State& currState, vector<double>& wBelief,
	    long player0Act, long player1Act, int playerIndex, State& nextState,
	    vector<long>& monsterActions, RandSource& randSource);

	/**
	 Used for mapping real world state to virtual world states
	 @return Given the state \a currState of the real world, return the state of the virtual world with number \a worldNum.
	 @param[in] currState Current state
	 @param[in] worldNum Which virtual world
	 */
	long realToVirtual(const State& currState, const long worldNum);

	/**
	 For customized control, inherited class should change this routine.

	 Action map from input keystroke. In this game, there are five actions:
	 w = up
	 s = down
	 a = left
	 d = right
	 anything else = no move

	 @param[in] c the input character keystroke.
	 @param humanOrAi if set (playerIndex), this tells the routine to interprete c as the respective playerIndex's action. Otherwise, the routine will try to map c to human's action first, assistant's second and set humanOrAi to the respective mapped playerIndex.
	 */
	long actionFromChar(const char c, int& humanOrAi);

	/**
	 *
	 * @return the compound action given \a playerAct and \a aiAct
	 * */
	long getCompoundAct(long playerAct, int playerIndex, long aiAct);

	bool containsAgentAction(long compoundAct, long playerAct, int playerIndex);

	/**
	 Write out the QFns to \a filename. Exploit
	 \a equivWorlds - only write a pointer to earlier model when
	 model is equivalent.
	 */
	void writeSolution(string filename);

	/**
	 Read in the QFns from file \a filename and
	 reconstruct collabQFn of all mazes.
	 */
	void readSolution(string filename);

	/**
	 Deallocate resources assigned.
	 */
	~MazeWorld();

public:
	/**************** Initialization *********************/

	/**
	 This routine is to be called after the constructor.

	 Initialize the following properties:
	 - numStateVars
	 - numWorlds
	 - equivWorlds
	 - worldType
	 - numHumanActs
	 - numAiActs
	 - visionLimit // for now, since the abstract state depends on visionLimit, we're using only one visionLimit accross worlds.

	 */
	virtual void initializeHumanAssistantMazes(MazeWorldDescription& desc) = 0;
	/**
	 Set the vision limit to use for abstract state generation.
	 */
	virtual void setDefaultVisionLimit(double visLim) {
		visionLimit = visLim;
	}
	;
	/**
	 Set the problem to be solved using abstract state or not.
	 */
	virtual void setUseAbstract(bool uA);
	/**
	 Initialize and compute geographical info.
	 */
	void worldInitialize();
	/**
	 Compute geographical info.
	 */
	void getTopology();

	/**************** Generate state maps *********************/
	/**
	 Invokes corresponding function of Maze's. Each original world stores its own class state map, which would then be copied over by worlds of the same type.
	 */
	void generateStateMap();
	/**
	 Invokes corresponding function of Maze's. This computes the V and Q functions.
	 */
	void generateModel();

	/************* Related to reading TMX file ******************************/
	/***** and get a char array representation of the world map *************/

	/**
	 Reads in problem parameters from TMX file.
	 @param[in] filename the TMX file name
	 @param[in] gts GameTileSheet object that provides game's tile ids.
	 @param[out] desc resulted description object.
	 */
	static void readDescriptionFromTMXFile(string filename,
	    GameTileSheet& gts, MazeWorldDescription& desc);

	/**
	 Mark regions in the map of form <0,1>.
	 @param[in] xSize num cols
	 @param[in] ySize num rows
	 @param[out] grid the map to be marked
	 @param[in] startingJuncIndex starting numeric index of junctions.
	 */
	static long detectRegionsInGrid(long xSize, long ySize,
	    vector<vector<long> >& grid, long startingJuncIndex);

	/**
	 @return the char array representing the world map (integers separated by space). In GhostBusters, it has the following format:
	 - xSize
	 - ySize
	 - Map of the form <0,1>; 0: ground, 1: wall; -1: sheep's pen; -2: fiery men's pen; -3: fiery men's exit
	 - worldType array (1: sheep, 2: ghost, )
	 */
	virtual char* getWorldRepresentation() = 0;
	/**
	 Returns the XML message representing the world map.
	 Sample format:
	 \verbatim
	 <?xml version="1.0" encoding="UTF-8"?>
	 <map width="11" height="11">
	 <grid>
	 <tile x="" y="" type="wall"/>
	 <tile x="" y="" type="player1"/>
	 </grid>
	 <world_types>
	 <world id="0" type="sheep">
	 <world id="1" type="sheep">
	 <world id="2" type="fiery">
	 <world id="3" type="ghost">
	 </world_types>
	 </map>
	 \endverbatim
	 */
	virtual char* getWorldRepresentationXML() = 0;

public:
	/************ State info retrieval functions *********/

	/**
	 @param[in] state current State.
	 @param[in] x X coord.
	 @param[in] y Y coord.
	 @param[out] monsterNum the world id that has monster at location (x,y).
	 @return true if state has a monster still alive at location (x, y) and false otherwise.
	 */
	bool isBlockedAt(const State* state, long x, long y, long& monsterNum);

	/**
	 Check whether this monster coordinates is valid w.r.t the blocking conditions.
	 @param[in] monster the Monster in question.
	 @param[in] monsterX the X coordinate of the questioned position
	 @param[in] monsterX the Y coordinate of the questioned position
	 @param[in] currState current state
	 @return the gridNodeLabel of location (\a monsterX, \a monsterY) if valid, -1 otherwise.
	 */
	long isValidMonsterMove(Monster* monster, long monsterX, long monsterY,
	    const State* currState);

	/**
	 Check whether the location (\a tempX, \a tempY) is a valid destination for  coordinates is valid w.r.t the blocking conditions.
	 @param[in] tempX X coord in question.
	 @param[in] tempY Y coord in question.
	 @param[in] currAgentX the current X coordinate of the agent in question.
	 @param[in] currAgentY the current Y coordinate of the agent in question.
	 @param[in] currState current state
	 @param[out] succeedArray stores value of 1 or 0 for each of the Maze's. 1 means the move is valid in that world, 0 means the move is invalid in that world. This info hints the assistant on which world is the blocking world.
	 @return true if valid, false otherwise.
	 */
	bool isValidMove(long tempX, long tempY, long currAgentX, long currAgentY,
	    const State& currState, vector<int>* succeedArray = 0);

	/**
	 Utility function to print the grid array.
	 */
	void printGrid();

	/**
	 * Print state with grid in ASCII
	 * */
	virtual void displayState(const State& state);

	/**
	 * Utility function to check whether there is an alive monster at x,y
	 * */
	bool hasMonster(const State& state, long x, long y, string monsterName = "");

	/**
	 * Returns the string representing the player's role to be sent to client
	 * */
	char* getPlayerString(int playerIndex);

	/**
	 * Returns the string representing the player's role to be sent to client, in XML
	 * format.
	 * */
	char* getPlayerStringXML(int playerIndex);

	/**
	 * Returns number of sub-worlds that are not terminal.
	 * */
	int getNumAliveWorlds(const State& state);

	/**
	 * Returns the list of dead worlds
	 * */
	void getDeadWorlds(const State& state, std::vector<long> &deadWorlds);

	void getAliveWorlds(const State& state, std::vector<long> &aliveWorlds);

	/**
	 * @return number of alive subworlds
	 * */
	long getSubworldAliveStatus(const State& state, std::vector<bool> &aliveStatus);

	/**
	 * @return true if world i and world j can negatively interact one another.
	 * */
	virtual bool negativeInteract(const State& state, long i, long j) = 0;

	/**
	 * @return true if the subworlds at current state interact.
	 * */
	virtual bool gotInteraction(const State& state) = 0;

	/**
	 * This routine returns only the valid actions (those not hitting against wall, etc.)
	 * */
	virtual void getValidCompoundActions(const State& state, vector<long>& validActions,
			long playerAction = -1, int playerIndex = 0);

	/**************** Raw State routines ****************/
	/**
	 Invoke corresponding routines in human, assistant and mazes to fill the raw state with respective info.
	 */
	void getCurrState(State& state);

	/**
	 * Similar to getCurrState but with randomization
	 * */
	void getRandomizedState(State& state, RandSource &randSource);

	/**
	 * Get randomized x, y such that they represent coords of a grid that is
	 * 1. not wall
	 * 2. not in occupiedGrids
	 * */
	void getRandomizedGrid(long& x, long& y, std::vector<long>& occupiedGrids, RandSource &randSource);

	/**
	 State is terminal if:
	 1. Human's X coord is TermState (this is a global terminal sign)
	 2. Depending on gameType, when all sub-world is terminal or just any of them is.
	 @return whether State \a state is terminal
	 */
	virtual bool isTermState(const State& state);

	bool someWorldAlive(const State& state);

	/**
	 * @return the subWorld id if there is exactly one subWorld alive, -1 otherwise
	 * */
	long hasOneSubworldAlive(const State& currState);

	/**
	 * Returns the best compound action in \a bestCompoundAct given current
	 * state \a currState and current world belief \a wBelief, together
	 * with corresponding belief-weighted Q value.
	 *
	 * */
	double getBestCompoundAct(const State& currState,
	    vector<double>& wBelief, long& bestCompoundAct,
	    long playerAct = -1, int playerIndex = 0);

	/**
	 * Return the best compound act in subworld \a subWorld, which
	 * has \a playerAct as part of it.
	 * */
	long getBestCompoundActInSubworld(const State& currState,
			int subWorld, long playerAct, int playerIndex);

	void getBestCompoundActionsInSubtasks(const State& currState,
	    std::vector<std::vector<long> >& bestCompoundActions, double tolerance,
	    long playerAct, int playerIndex);

	double getQValue(const State& currState, const vector<double>& wBelief,
			const long compoundAct);

	/**
	 * Compute the Q values of valid compound acts with respect to the supplied
	 * \a playerAct of \a playerIndex.
	 * */
	void getQValues(const State& currState, vector<double>& wBelief,
			vector<pair<long, double> >& QValues, 
			long playerAct=-1, int playerIndex=0);

	/**
	 Invokes realDynamics to return the reward.

	 @return Reward of doing \a act in \a currState.
	 @param[in] currState Current State
	 @param[in] player0Act Index of human action to perform
	 @param[in] player1Act Index of AI action to perform
	 @param[out] nextState Sampled next state. This is random and changes from
	 invocation to invocation.
	 @param[in] condProbAct this human's action model is obtained to update the assistant's world belief.
	 @param[in] randSource Source of random numbers
	 */
	double rewardFromRealDynamics(const State& currState,
	    vector<double>& wBelief, long player0Act, long player1Act,
	    int playerIndex, State& nextState, vector<vector<double> >& condProbAct,
	    vector<long>& monsterActions, RandSource& randSource);

	/**
	 Applies agents' actions on currState to produce nextState.

	 @param[in] currState Current State
	 @param[in] player0Act Index of Human action to perform
	 @param[in] player1Act Index of AiAssistant action to perform
	 @param[out] nextState Sampled next state. This is random and changes from
	 invocation to invocation.
	 @param[out] succeedArray stores value of 1 or 0 for each of the Maze's. 1 means the move is valid in that world, 0 means the move is invalid in that world. This info hints the assistant on which world is the blocking world.
	 @param[in] randSource Source of random numbers.

	 */
	void executeAgentActions(const State& currState, long player0Act,
	    long player1Act, State& nextState, vector<int>& succeedArray,
	    RandSource& randSource);

	/*
	 Samples Human's action, obtains best AiAssistant's action and updates currState to produce nextState. Used in simulation mode. Invokes policyRoutine.
	 @param[in] currState Current State
	 @param[out] nextState Sampled next state. This is random and changes from
	 invocation to invocation.
	 @param[in] randSource Source of random numbers.
	 @param[out] p1act action of player 1
	 @param[out] p2act action of player 2
	 */
	double policy(const State& currState, vector<double>& wBelief,
	    long& p1act, long& p2act, State& nextState, vector<long>& monsterActions,
	    RandSource& randSource);

	double policy_stupidAI(const State& currState,
	      vector<double>& wBelief, long& p1act, long& p2act, State& nextState,
	      vector<long>& monsterActions, RandSource& randSource, int scriptMode = 0);
	/*
	 Given Human's action, obtains best AiAssistant's action and updates currState to produce nextState. Used in HvA mode. Invokes policyRoutine.
	 @param[in] currState Current State
	 @param[in] playerAct Index of Human action to perform
	 @param[out] aiAct the action of ai partner
	 @param[out] nextState Sampled next state. This is random and changes from
	 invocation to invocation.
	 @param[in] randSource Source of random numbers.
	 */
	double policyHuman(const State& currState, vector<double>& wBelief,
	    long playerAct, int playerIndex, long& aiAct, State& nextState,
	    vector<long>& monsterActions, RandSource& randSource);

	double policyHuman_stupidAI(const State& currState,
	    vector<double>& wBelief, long playerAct, int playerIndex,
	    long& aiAct, State& nextState, vector<long>& monsterActions, RandSource& randSource,
	    int scriptMode = 0);

	/**
	 Given Human's action, obtains best AiAssistant's action. Invoked by \a policy and \a policyHuman.

	 @param[in] currState Current State
	 @param[out] bestAiAct the ai action to be returned.
	 @param[in] playerAct Index of Human action to perform
	 @param[in] randSource Source of random numbers.
	 @return true if currState is not terminal, false otherwise.
	 */
	bool policyRoutine(const State& currState, vector<double>& wBelief,
	    long& bestAiAct, long playerAct, int playerIndex);

	/**
	 Applies the monster's move action on \a state. The validity of act was already checked in getAct* routines of Monster.
	 Additional check can be carried out in extending classes.
	 @param[in] state Current State to be applied.
	 @param[in] act Monster's action to perform
	 @param[in] worldNum the Monster's world index.
	 */
	void applyMonsterMoveAct(State& state, long act, int worldNum);

	/**
	 Converts \a state to char array.
	 */
	virtual char* stateToPosChar(const AugmentedState& state);
	/**
	 Converts \a state to XML msg with timeStamp.
	 */
	virtual char* stateToPosCharXML(const AugmentedState& state,
	    const int timeStamp = -2);

	virtual void stateToXMLDoc(pugi::xml_document& doc, const AugmentedState& state,
			const int timeStamp = -2);

	/**
	 Globally assesses the terminal status of \a state. This could be used in the case when human has hitpoints.
	 @param[in] state Current State
	 @param[out] terminal true if terminal, false otherwise.
	 @return reward of \a state.
	 */
	virtual double getReward(const State& state, bool& terminal) {
		terminal = false;
		return 0;
	}
	;

	/**
	 * get maximum reward achievable in one time step
	 * */
	virtual double getMaxReward(const State& state);

	virtual double getMinReward(const State& state);

	/*
	 * Get maximum reward in a game.
	 * Because in a game, reward can only be obtained once, Vmax
	 * concurs with Rmax.
	 * */
	inline double getVmax(const State* state){return getMaxReward(*state);};

	/*
	 * get minimum reward in a game
	 * Because in a game, reward can only be obtained once, Vmin
	 * concurs with Rmin.
	 * */
	inline double getVmin(const State* state){return getMinReward(*state);};
};

#endif
