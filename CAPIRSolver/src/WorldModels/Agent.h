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



#ifndef __AGENT_H
#define __AGENT_H

#include "RandSource.h"
#include "ObjectWithProperties.h"

#include <vector>
#include <cstring>
#include <queue>

/**
  @class Agent
  @brief Base class of Monster and HumanAssistant.
  @details All Agents have a set of movement actions, a set of special actions (shoot, fight) and a set of properties (inherited from ObjectWithProperties.)
  
  @author Huy Nguyen
  @date Dec 2010
*/

using namespace std;

/********* Constants ***************/
/**
  Movement action names. The human/assistant/monster has the option not to move
*/
enum directions {unchanged, east, south, west, north}; 
/**
  Relative x movement for each action.
*/
const long RelativeDirX[] = {0, 1, 0, -1, 0}; 
/**
  Relative y movement for each aciton.
*/
const long RelativeDirY[] = {0, 0, -1, 0, 1}; 

/***********************************/

class Agent : public ObjectWithProperties
{
public:
  /**
		Initial coordinates.
	*/
	long x, y;
  /**
    Number of movement actions. 0 is always NoAct, so normally an agent has 5 movement actions
  */
  long numMoveActs; 
  /**
    Number of special actions.
  */
  long numSpecialActs;
  /**
    Array of impassable locations of this agent.
  */
  vector< pair<long, long> > impassableLoc;

  /**
    Agent's name.
  */
  string name;
  /**
    Shortest path matrix in the grid. Because of \a impassableLocs, each Agent has its own matrix.
  */
  vector<vector <long> >* shortestPath; 
protected: 
  /**
    Used to calculate \a shortestPath, storing whether node seen before in BFS
  */
  vector<bool> found; 

public:
  /**
    Constructor. 
    @param[in] x initial X coord.
    @param[in] y initial Y coord.
    @param[in] numMoveActs number of movement actions.
    @param[in] numSpecialActs number of special actions.
    @param[in] numProperties number of properties.
  */
  Agent(long x, long y, long numMoveActs, long numSpecialActs, long numProperties) : ObjectWithProperties(numProperties),
  x(x), y(y), numMoveActs(numMoveActs) , numSpecialActs(numSpecialActs) {};
  
  /**
    Destructor.
  */
  ~Agent(){
    if (shortestPath){
      delete shortestPath;
      shortestPath = 0;
    }
  };
  
  /**
    Sets impassable locations.
  */
  void setImpassableLoc(vector< pair<long, long> >& impLoc){
    impassableLoc = impLoc;
  };
  
  /**
    Adds a location (x,y) to \a impassableLocs.
  */
  void addImpassableLoc(long x, long y){
    impassableLoc.push_back(pair<long, long>(x,y));
  };
  
  /**
    Checks if location (x, y) is impassable.
    @param[in] gridNodeLabel node label representation of the grid map.
  */
  bool isImpassable(long x, long y, vector<vector <long> >& gridNodeLabel);
  
  /**
    Calculates shortest path: Single node.
  */
  void calcShortestPath(long i, long j, long numAccessibleLocs, vector<vector <long> >& gridNodeLabel);
  /**
    Calculates shortest path matrix: All pairs' shortest paths.
  */
  void calcShortestPathMatrix(long xSize, long ySize, long numAccessibleLocs, vector<vector <long> >& gridNodeLabel); 
  
  /**
    Returns number of actions.
  */
  inline long getNumActs(){ return (numMoveActs + numSpecialActs); };
  /**
    Returns number of movment actions.
  */
  inline long getNumMoveActs() { return numMoveActs; };
  /**
    Returns number of special actions.
  */
  inline long getNumSpecialActs() { return numSpecialActs; };
  
  /**
    Checks if \a acts is a movement action.
  */
  inline bool isMoveAct(long act){ return ((act>=0) && (act<numMoveActs));};
  /**
    Checks if \a acts is a special action.
  */
  inline bool isSpecialAct(long act){ return ((act>=numMoveActs) && (act<(numMoveActs+numSpecialActs)));};
  /**
    Checks if \a acts is a valid action.
  */
  inline bool isValidAct(long act){return ((act>=0) && (act<(numMoveActs+numSpecialActs)));};

  
  /******* Action mappings ************/
  /**
    Returns mapping of character \a c to action.
  */
  virtual long actionFromChar(const char c){ return unchanged; };
  /**
    Checks if \a acts is meaningful, i.e. not unchanged.
  */
  virtual bool isMeaningfulAct(const char c);
  
  /****** Property Abstraction related ************/  
  /**
    Enumerate properties, i.e. filling the states in input with all possible values of involved properties.
    @param[in] currProperties the partially filled array of properties.
    @param[out] output the array of filled properties.
    @param[in] dealtAgent used to identify properties that could be ignored when solving. Certain dealtAgent could require certain subsets of properties. This helps reduce the state space's size.
  */
  void enumerateProperties(std::vector< long >& currProperties, std::vector< std::vector<long> >& output, const Agent* dealtAgent = 0);
  
  /**
    Fill in relevant properties from raw State's \a rawProperties to AbstractState's \a absProperties.
    @param[in] startingIndex starting index of properties in rawProperties to be copied to absProperties.
    @param[in] endingIndex ending index of properties in rawProperties to be copied to absProperties.
    @param[in] dealtAgent used to identify properties that could be ignored when solving. Certain dealtAgent could require certain subsets of properties. This helps reduce the state space's size.
  */
  void fillProperties(std::vector<long>& absProperties, long startingIndex, long endingIndex, const std::vector<long>& rawProperties, Agent* dealtAgent = 0);
  
  /**
    By default, every property is relevant when dealing with any \a dealtAgent. Certain dealtAgent could be solved without the need to enumerate certain properties. This helps reduce the state space's size.
  */
  virtual bool isRelevant(long propertyIndex, const Agent* dealtAgent){return true;};
  
  /************** Raw state related ***************/
  /**
    Gets initial properties of this Agent.
    @param[out] propertiesState
  */
  virtual void getCurrState(vector<long>& propertiesState, long newX=-1, long newY=-1);
  
  /**
    Executes this Agent's special action \a act. By default, do nothing.
  */
  virtual void executeSpecialAct(const State& currState, long act, State& nextState, RandSource& randSource)
  {};
};

#endif
