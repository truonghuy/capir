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



#ifndef __MAZE_H
#define __MAZE_H

#include "Model.h"
#include "Player.h"
#include "Monster.h"
#include "SpecialLocation.h"
#include "ValueIteration.h"
#include <map>
#include <cmath>


#define getDistance(p1x, p1y, p2x, p2y) (fabs(p1x - p2x) + fabs(p1y - p2y))
#define getMin(x, y) ((x<y)? x : y)

/**
   @class Maze
   @brief This class is the base class for all individual tasks/puzzles that the protagonists need to solve in the whole map.
   @details This class holds
    - One or no Monster.
    - References to the agents and geometrical data, used for planning.
    - All planning is done in child classes of this class.
  
    This class takes care of the following:
    - Deallocate Monster object.

   @author Truong Huy Nguyen
   @date December 2010

*/
class Maze
{
public:
  /**
    Integer identifier for this type of mazes. 
  */
  long worldType;
  /**
    String identifier for this type of mazes. 
  */
  string worldTypeStr;
  
  /**
    The parent game level. 
  */
  MazeWorld* mazeWorld;
  
  /**
    \a monster can be null. if it's null, this world just has special locations and/or items.
  */ 
  Monster* monster;
  
  /**
    SpecialLocation is not shared among mazes of the same type. Shared special locations/items should be represented by other means.
  */
  SpecialLocation* specialLocation;

  /**
    Pointer to the Player objects.
  */
  Player* player[2];
  
  /**
    Number of planned states.
  */
  long virtualSize;
  /**
    Abstraction flag.
  */  
  bool useAbstract;
  /**
    The vision limit of the monster.
  */  
  double visionLimit;
  
  /**
    Pointer to the value function of this maze. If this is an original maze, \a valueFn is allocated here, and should be deallocated by this maze as well.
  */
  vector<double>* valueFn;
  /**
    Pointer to the Q function of this maze. If this is an original maze, \a collabQFn is allocated here, and should be deallocated by this maze as well.
  */
  vector<vector <double> >* collabQFn;
  
  /******** Geometrical info from mazeWorld **********/
  /**
    Pointer to the game's grid, previously constructed in MazeWorld.
  */
  vector<vector <long> >* grid;
  /**
    Pointer to the game's gridNodeLabel, previously constructed in MazeWorld.
  */
  vector<vector <long> >* gridNodeLabel;  
  /**
    Pointer to the game's connectivity, previously constructed in MazeWorld.
  */
  vector<vector <long> >* connectivity;
  /**
    Pointer to the game's borderLength, previously constructed in MazeWorld.
  */
  vector<vector <long> >* borderLength; 
  /**
    Pointer to the game's regionRepPoint, previously constructed in MazeWorld.
  */
  vector< pair<long, long> >* regionRepPoint;
  /**
    Pointer to the game's rType, previously constructed in MazeWorld.
  */
  vector<regionType>* rType;
  /**
    Number of accessible grid squares.
  */
  long numAccessibleLocs;

  /********* Geometrical info of class ***************/
  /**
    Vector of visible nearby regions, indexed by accessible grid node, storing vectors of pair<long, long> (region, numVisibleNodes), indicating the visible nearby regions with number of grid nodes being seen.
  */ 
  std::vector<std::vector < std::pair<long,long> > >* visibleNearByRegions;
  
  /******* for raw state *****************************/
  
  /**
    Maps from AbstractState to \a index. This is only constructed if the maze is solved with abstraction.
  */
  map<AbstractState, long, Utilities::AbstractStateComparator>* vAbsStateMap;
  /**
    Maps from \a index to AbstractState. This is only constructed if the maze is solved with abstraction.
  */
  vector<AbstractState>* reverseVAbsStateMap; // index -> state
  
  /********************************/

public:

  /**
    Full constructor.
  */
  Maze(long wType, MazeWorld* mazeWorld, Monster* monster = 0, SpecialLocation* sLoc = 0, Player* h = 0, Player* a = 0) : worldType(wType), mazeWorld(mazeWorld), monster(monster), specialLocation(sLoc)
  {
    player[0] = h;
    player[1] = a;
  };
  /**
    Default constructor. Not supposed to be used.
  */
  Maze(){ specialLocation = 0; monster=0;};
  
  
  /************ Initialization ******************************/
  /**
    Copies uninitialized data from another maze. In this base class, there is only visionLimit copied.
  */
  virtual void copyInfoFrom(Maze* orig){visionLimit = orig->visionLimit;};
  /**
    Copy state maps from another maze. Help avoid reconstruction of state maps multiple times.
    @param[in] orig the maze to be copied from.
  */
  void copyStateMap(Maze* orig){
  	vAbsStateMap = orig->vAbsStateMap;
  	reverseVAbsStateMap = orig->reverseVAbsStateMap;
  	virtualSize = orig->virtualSize;
  	if (useAbstract) visibleNearByRegions = orig->visibleNearByRegions;
  }
  
  /**
    Copies Q and value functions from another maze. 
    \a Note: I don't think we need to copy valueFn, or use valueFn for that matter, after Q function is constructed. In the future, I will remove valueFn altogether.
    @param[in] orig the maze to be copied from.
  */
  void copyValueQFns(Maze* orig){
    valueFn = orig->valueFn;
    collabQFn = orig->collabQFn;
  };
  
  /**
    Sets property \a pName to corresponding value \a pValue. This routine is empty in this base class; child classes should set \a pName properly.
    @param[in] pName property's name
    @param[in] pValue property's value
  */
  virtual void setProperty(string pName, string pValue){};
  
  /**
    Sets Human pointer.
  */
  void setHuman(Player *h){ player[0] = h;};
  /**
    Sets Assistant pointer.
  */
  void setAssistant(Player *a){ player[1] = a;};
  /**
    Sets both Player and Assistant.
  */
  void setPlayers(Player *h, Player *a)
  {
    player[0] = h;
    player[1] = a;
  };
  /**
    Sets abstract flag.
    @param[in] uA if uA = true, this maze is planned using abstraction. Otherwise, uses raw State.
  */
  void setUseAbstract(bool uA){ useAbstract = uA;};
  
  /************ Maze's characteristics **********************/
  /**
    Checks whether this maze has a monster. A Maze could have at most one Monster and one SpecialLocation, and at least one of the above.
  */
  bool hasMonster(){ return (monster != 0);};
  
  /**
   * get maximum reward achievable in one time step
   * */
  virtual double getMaxReward() = 0;

  /**
	 * get minimum reward achievable in one time step.
	 * Note that this getMinReward is self-contained, i.e.,
	 * does not account for min rewards caused by interactions.
	 * */
	virtual double getMinReward() = 0;

  /**************** Generate state maps *********************/
  /**
    Generates the state maps. It will call generateAbstractStateMap or generateRawStateMap, depending on whether the abstraction flag is set. Generally, there is no reason to call generateRawStateMap.
  */
  void generateStateMap();
  /**
    Generates the abstract state maps.
  */
  void generateAbstractStateMap();
  /**
    Generates the raw state maps. It is NOT implemented yet.
  */
  void generateRawStateMap();
  
  /**
    Enumerates \a agentIndex's visibility and coords where appropriate, e.g. when it is in the \a visionLimit of monster. 
    Optimization - Can enumerate in place?
    @param[in] input the input vector of AbstractState.
    @param[out] output the output vector of AbstractState.
    @param[in] agentIndex the index of the agent. humanIndex is 0, aiIndex is 1.
  */
  void enumerateLocationOneAgent(  std::vector< AbstractState >& input, std::vector< AbstractState >& output, long agentIndex);
  
  /**
    Constructs visible nearby region \a visibleNearByRegions. This is used to track when the agents move to a region that is visible to the NPC.
  */
  void constructVisibleNearbyRegion();
  /**
    Sets all abstract related geometric attributes to those of MazeWorld. These include
    \a connectivity, \a borderLength, \a regionRepPoint,
    \a rType.
  */
  void getAbstractWorldGeometricInfo();

  /**
   * Sets all basic geometric attributes to those of MazeWorld. These include
   * \a grid, \a gridNodeLabel, \a numAccessibleLocs. Besides, it also invokes
   * Monster::calcShortestPathMatrix.
   * */
  void getRawWorldGeometricInfo();

  
  /************ For MDP solving *************/
  /**
    Assumption: There are only corridors and junctions in the map (1.5 dimensions)
    In solver, if generateModel is called with abstract flag on, V and Q functions store values of AbstractState.
    If generateModel is called, V and Q functions store values of normal State.
  */
  void generateModel();

  // construction methods for value functions and Q functions
  /**
    Constructs transition and reward matrices by invoking a lot of absVirtualDynamics, depending on \a useAbstract flag.
    @param[out] transitionMatrix
    @param[out] rewardMatrix
  */
  void constructTRCompAct(vector < vector < vector < pair<long, double> > > >& transitionMatrix, vector < vector < double > >& rewardMatrix);
  /**
    Constructs Q functions using previously computed transition and reward matrices.
    @param[in] transitionMatrix
    @param[in] rewardMatrix
  */
  void constructCollabQFns(vector < vector < vector < pair<long, double> > > >& transitionMatrix, vector < vector < double > >& rewardMatrix);

  /**
     Implements the dynamics of this Maze, with abstraction flag on.
     @return Reward of taking current action in current state for current world
     @param[in] currAbsState Current virtual world's state.  
     @param[in] hAct Current Human action
     @param[in] aAct Current AI action
     @param[out] tranProb Sparse representation of next abstract state probabilities given current state and current actions. Note that tranProb should be kept sorted ordered by stateNum (first param) WHY???
  */
  virtual double absVirtualDynamics(const long currAbsState, const long hAct, const long aAct, sparseStateBelief& tranProb);
  
  /**
    Returns true if AbstractState \a absState is terminal, false otherwise.
  */
  virtual bool isAbstractTerminal(const AbstractState& absState);
  
  /**
    Returns true if State \a state is terminal, false otherwise.
  */
  virtual bool isTermState(const State& state, long worldNum);
  /**
   * Just check the subworld itself, does not care about the global state.
   * */
  virtual bool isLocalTermState(const State& state, long worldNum);
  
  inline void suicide(State& state, long worldNum){
  	state.mazeProperties[worldNum][0] = TermState;
  };

  /**
    Executes movement action of \a agentIndex.
    @param[in] input the vector of prior AbstractState with probability.
    @param[in] action movement action of \a agentIndex.
    @param[in] agentIndex 0 is Human, 1 is Assistant.
    @param[out] output the vector of posterior AbstractState with probability.
  */
  virtual void absExecuteAgentMoveAct(std::vector< std::pair<AbstractState, double> >& input, long& action, int agentIndex, std::vector< std::pair<AbstractState, double> >& output);
  
  /**
    Executes movement action of \a agentIndex, invoked by absExecuteAgentMoveAct when this Maze has monster.
    @param[in] input the vector of prior AbstractState with probability.
    @param[in] action movement action of \a agentIndex.
    @param[in] agentIndex 0 is Human, 1 is Assistant.
    @param[out] output the vector of posterior AbstractState with probability.
  */
  virtual void absExecuteAgentMoveAct_gotMonster(std::vector< std::pair<AbstractState, double> >& input, long &action, int agentIndex, std::vector< std::pair<AbstractState, double> >& output);
  
  void executeAgentMoveAct_gotMonster(std::vector<std::pair<
      AbstractState, double> >& input, long &action, int agentIndex, std::vector<
      std::pair<AbstractState, double> >& output);


  /**
    NOT implemented. Executes movement action of \a agentIndex, invoked by absExecuteAgentMoveAct when this Maze does not have monster.
    @param[in] input the vector of prior AbstractState with probability.
    @param[in] action movement action of \a agentIndex.
    @param[in] agentIndex 0 is Human, 1 is Assistant.
    @param[out] output the vector of posterior AbstractState with probability.
  */
  virtual void absExecuteAgentMoveAct_noMonster(std::vector< std::pair<AbstractState, double> >& input, long &action, int agentIndex, std::vector< std::pair<AbstractState, double> >& output)
  { /* to be implemented*/ };
  
  /**
    Executes movement action of monster. Note that this routine is invoked after Human and Assistant's actions have been applied on AbstractState.
    @param[in] currAbsState the current AbstractState to be updated.
    @param[in] prob probability of \a currAbsState.
    @param[in] humanAct previous action of Human.
    @param[in] aiAct previous action of Assistant.
    @param[in] act movement action of monster.
    @param[in] probAct probability of this movement action of monster.
    @param[out] output the vector of posterior AbstractState with probability.
    @param[in] prevAbsState the AbstractState prior to applying \a humanAct and \a aiAct.
  */
  void absExecuteMonsterMoveAct(AbstractState& currAbsState, double prob, long humanAct, long aiAct, long act, double probAct, std::vector< std::pair<AbstractState, double> >& output, const AbstractState& prevAbsState);
  
  /**
    Executes Maze dynamics. This invokes monster and specialLocation's reactions. This routine should take care of the dynamics of shared items as well.
    @param[in] input the vector of prior AbstractState with probability.
    @param[in] humanAct action of Human.
    @param[in] aiAct action of Assistant.
    @param[out] output the vector of posterior AbstractState with probability.
    @param[in] prevAbsState the AbstractState prior to applying \a humanAct and \a aiAct.
  */
  virtual void absExecuteMazeDynamics(std::vector< std::pair<AbstractState, double> >& input, long humanAct, long aiAct, std::vector< std::pair<AbstractState, double> >& output,  const AbstractState& prevAbsState);
  
  /********** For abstract ******************/
  /**
    New! I turned the input \a state to constant, therefore, there's no way to change the value of state in here. Whatever update must be done in Monster::absReact or SpecialLocation::absReact. 
    Each individual Maze must implement this routine, which returns a reward for the current abstract state.
  */
  virtual double absGetReward(AbstractState& state) {return NoReward;};

  /**
    @param[in] states the vector of AbstractState with probability to be evaluated.
    @param[out] rewards the returned vector of probability-weighted rewards corresponding to \a states
  */
  void absGetRewards(std::vector< std::pair<AbstractState, double> >& states, std::vector <double>& rewards);
  /**
    Returns the long index (integer equivalence) of AbstractState \a abState
  */
  long getLongFromAbsState(AbstractState& absState);
  /**
    Based on the actions executed by monster and Human/Assistant, updates the visibility and coordinates of Human and agent in \a absState, which possibly spawns new abstract states stored in tempOutput.
    @param[in] absState the prior AbstractState.
    @param[out] output the vector of posterior AbstractState with probability.
    @param[in] humanAct action of Human.
    @param[in] aiAct action of Assistant.
    @param[in] monsterAct action of NPC.
    @param[in] priorMonsterX prior coordinate X of NPC.
    @param[in] priorMonsterY prior coordinate Y of NPC.
    @param[in] prevAbsState the AbstractState prior to applying \a humanAct, \a aiAct and \a monsterAct.
  */
  void updateVisibility(AbstractState& absState, std::vector< std::pair<AbstractState, double> >& output, long humanAct, long aiAct, long monsterAct, long priorMonsterX, long priorMonsterY, const AbstractState& prevAbsState);
  
  /**
   * Based on the exact coords of monster and players to update monster's visibility.
   * */
  void updateVisibility_NoAbs(AbstractState& absState);
  
  /************* Abstract Move related routines *****************************/
  /**
    Returns true if the agent in question can enter region in question. Note that since we force special locations to be junctions, thereby a region on its own, this suffices.
    @param[in] agentIndex 0 = Human, 1 = Assistant.
    @param[in] regionIndex the region in question.
  */
  bool playerPassable(int agentIndex, long regionIndex, int monsterX = -1, int monsterY = -1);

  bool playerPassable(int agentIndex, long x, long y);

  bool monsterPassable(long mx, long my, long p1x, long p1y, long p2x, long p2y);
  /**
    Used in Planning. Checks whether the agent's movement into \a nextRegion is blocked.
    @param[in] currAbsState the current AbtractState
    @param[in] agentIndex 0 = Human, 1 = Assistant.
    @param[in] nextRegion the region in question
    @param[in] movingTowardsMonster true if this agent is moving towards the monster.
    @return 0 if the agent's move is guaranteed not blocked, 1 if it might be blocked and 2 if it is definitely blocked.
  */
  int agentMoveCanBeBlocked(const AbstractState& currAbsState, int agentIndex, long nextRegion, bool movingTowardsMonster);
  
  /**
    Called when a Human/Assistant has just crossed a border to a new region. This routine will update the coords of the moving agent.
    Using regionRepPoint and borderLength (the length of each region) to compute the region's point that indicates from which side of a region the agent has just come from when entering monster's region. 

    @param[out] nextAbsState the AbstractState to be updated.
    @param[in] movingAgent the index of the agent that moves.
    @param[in] action the movement action of the agent in question.
    @param[in] monsterRegion 
  */
  void updateCoords(AbstractState& nextAbsState, int movingAgent, long action, long monsterRegion);
  
  /**
    Updates agents' coordinates to reflect their relative position with NPC.
    @param[out] absState the AbstractState to be updated.
    @param[in] monsterAct NPC's movement act. It is already checked to be anything but unchanged.
    @param[in] priorMonsterRegion NPC's region before making the move.
  */
  void updateCoordsByMonsterMove(AbstractState& absState, long monsterAct, long priorMonsterRegion);

  /**
    Standardizes agents' coordinates. After all movements have been made, the agents' coordinates reflect the direction from which they move, which creates non-standard AbstractState. This routine fixes that.
  */
  void standardizeCoords(AbstractState& absState);
  
  /**
    Updates visibility and coords of \a agentIndex after NPC has made its move \a monsterAct.
    @param[in] input the vector of prior AbstractState with probability.
    @param[out] output the vector of posterior AbstractState with probability.
    @param[in] monsterAct movement action of NPC.
    @param[in] priorMonsterX prior coordinate X of NPC.
    @param[in] priorMonsterY prior coordinate Y of NPC.
    @param[in] agentAct action of the agent in question.
    @param[in] agentIndex 0 = Human, 1 = Assistant.
    @param[in] prevAbsState the AbstractState prior to applying \a humanAct and \a aiAct.
  */
  void updateVisibilityCoordsOneAgent(std::vector< std::pair<AbstractState, double> >& input, std::vector< std::pair<AbstractState, double> >& output, long monsterAct, long priorMonsterX, long priorMonsterY, long agentAct, int agentIndex, const AbstractState& prevAbsState);

  
  /************** Raw state related ***************/
  /**
    Returns the index (integer representation) of AbstractState given State and maze's index. Individual elements of absState are constructed, then getLongFromAbsState is invoked to get long abs state.
    @param[in] state the queried state
    @param[in] worldNum the index of this Maze in MazeWorld's array of Mazes.
  */
  long getLongAbsStateFromState(const State& state, long worldNum);
  
  long getLongAbsStateFromState_NoAbs(const State& state, long worldNum);

  /**
    Called in MazeWorld. Normally this is where maze's status, anything unrelated to Monster and SpecialLocation, gets updated.
  */
  virtual void executeMazeDynamics(State& nextState, int worldNum, RandSource& randSource){};

  /**
    Invoked from MazeWorld::getCurrState. Fills propertiesState with initial values.
    @param[out] propertiesState the property array of this Maze.
    @param[in] newX optional coord
    @param[in] newY optional coord
  */
  virtual void getCurrState(vector<long>& propertiesState, long newX=-1, long newY=-1);
  
  /**
    Used for In-game. Returns the reward of Maze \a worldNum.
    @param[in] state current State.
    @param[out] vTerminal true if Maze \a worldNum is terminal and false otherwise.
    @param[in] worldNum the index of this Maze in MazeWorld's array of Mazes.
    @return reward of State \a state.
  */
  virtual double getReward(const State& state, bool& vTerminal, int worldNum) = 0;
  
  /**
    Converts State to AbstractState w.r.t Maze \a worldNum.
    @param[in] currState current State.
    @param[in] worldNum the index of this Maze in MazeWorld's array of Mazes.
    @return long AbstractState if useAbstract, long virtual State otherwise.
  */
  long realToVirtual(const State& currState, const long worldNum);

  
  /******************* Predefined distance functions ****************/
  /**
    Individual monster classes can extend these functions to use.
  */
  /**
    Implements the distance function used in Monster::moveActsFarthestFromBothPlayers. Child classes can customize this function to suit their needs.
    @param[in] monsterNode the NPC's gridNodeLabel (integer representation of grid square.)
    @param[in] nearestAgentNode the nearest agent's gridNodeLabel (integer representation of grid square.)
    @param[in] fartherAgentNode the farther agent's gridNodeLabel (integer representation of grid square.)
    @return the distance value.
  */
  virtual double distance_farthestFromBoth(long monsterNode, long nearestAgentNode, unsigned nearestAgentIndex,
		  long fartherAgentNode);

  /**
    Implements the distance function used in Monster::moveActsFarthestFromPlayer. Child classes can customize this function to suit their needs.
    @param[in] monsterNode the NPC's gridNodeLabel (integer representation of grid square.)
    @param[in] agentNode the targeted agent's gridNodeLabel (integer representation of grid square.)
    @return the distance value.
  */
  virtual double distance_farthestFromPlayer(long monsterNode, long agentNode);

  /**
    Implements the distance function used to calculate the movement that gets NPC closer to \a destNode. Child classes can customize this function to suit their needs.
    @param[in] monsterNode the NPC's gridNodeLabel (integer representation of grid square.)
    @param[in] destNode the targeted gridNodeLabel (integer representation of grid square.)
    @return the distance value.
  */
  virtual double distance_towardsNode(long monsterNode, long destNode);

  /********************* General Utility routines ************************/

  /**
   * This routine is used when planning is done without abstraction.
   * @return true if the proposed new coordinates of player \a agentIndex (\a tempX, \a tempY)
   * are valid with respect to blocking conditions.
   *
   * */
  bool isValidMove(int agentIndex, long tempX, long tempY,
      long otherAgentX, long otherAgentY, const AbstractState& currState);

  /**
    Checks whether \a region is visible from \a monsterGridNode. If yes, the returned value denotes the number of grid nodes in \a region that are visible from \a monsterGridNode. Otherwise, it is 0.
    @param[in] monsterGridNode the NPC's gridNodeLabel (integer representation of grid square.)
    @param[in] region the region in question.
    @return number of grid nodes in region that are visible from monsterGridNode
  */
  int regionVisible(long monsterGridNode, long region);
  /**
    @return true if coord position p1 sees p2.
  */
  bool checkVisibility(long p1X, long p1Y, long p2X, long p2Y);

  /**
    Checks if critter 1 at p1 executing \a action moves towards critter 2 at p2. This routine is called when a critter's action has already been verified to move it to another grid square. 
  */
  bool towardsTheOther(long p1X, long p1Y, long action, long p2X, long p2Y);
  
  /***************** Cleanup *********************************************/
  /**
    This cleans up the common assets such as state maps and value/q functions. Should only be called on original worlds.
  */
  void deleteCommonPointers();
  ~Maze();
};


#endif
