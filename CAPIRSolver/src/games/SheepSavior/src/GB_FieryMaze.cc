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



#include "GB_FieryMaze.h"
#include "GhostBustersLevel.h"

void GB_FieryMaze::initialize(long fieryMazeId, long fieryX, long fieryY, double visLim, GhostBustersLevel* gbLevel)
{
  // 1. initialize the monster
  monster = new GB_Fiery(fieryX, fieryY, this);
  
  worldType = fieryMazeId;
  worldTypeStr = "fiery";
  visionLimit = visLim;
  mazeWorld = gbLevel;

};

/**
  There can be a series of if else here to set corresponding property

*/
void GB_FieryMaze::setProperty(string pName, string pValue)
{
  if (pName == "visionLimit")
    visionLimit = atof(pValue.c_str());
};

void GB_FieryMaze::setPenPosition(long penX, long penY)
{
  penPosition.first = penX;
  penPosition.second = penY;
};

void GB_FieryMaze::setExitPosition(long x, long y)
{
  exitPosition.first = x;
  exitPosition.second = y;
};

/***************** Raw State related *************************/
double GB_FieryMaze::getReward(const State& state, bool& vTerminal, int worldNum)
{
  
  if (isTermState(state, worldNum)){
    vTerminal = true;
    return NoReward;
  }

  // 1. Caught in pen
  if ( (state.mazeProperties[worldNum][0] == penPosition.first) && (state.mazeProperties[worldNum][1] == penPosition.second))
  {
    vTerminal = true;
    return FieryCaughtReward;
  }
  
  // 2. Escaped
  if ( (state.mazeProperties[worldNum][0] == exitPosition.first) && (state.mazeProperties[worldNum][1] == exitPosition.second))
  {
    vTerminal = true;
    return FieryEscapedReward;
  }
  
  vTerminal = false;
  return NoReward;
};


/************ Abstract related ****************************/

double GB_FieryMaze::absGetReward(AbstractState& absState)
{
  // 1. already terminal state
  if (isAbstractTerminal(absState))
    return NoReward;

  // 2. not previously terminal 
  if ( (absState.monsterProperties[0] == penPosition.first) && (absState.monsterProperties[1] == penPosition.second) ){
    absState.monsterProperties[0] = TermState;
    return FieryCaughtReward;
  }
  
  if ( (absState.monsterProperties[0] == exitPosition.first) && (absState.monsterProperties[1] == exitPosition.second) ){
    absState.monsterProperties[0] = TermState;
    return FieryEscapedReward;
  }
  
  return NoReward;
};
  

/************** Distance functions ***************************/
double GB_FieryMaze::distance_farthestFromBoth(long monsterNode, 
  long nearestAgentNode, unsigned agentIndex, long fartherAgentNode)
{
  long pNode = (*gridNodeLabel)[penPosition.first][penPosition.second];
  long eNode = (*gridNodeLabel)[exitPosition.first][exitPosition.second];

	if (fartherAgentNode >=0)
		return 8 * (*(monster->shortestPath))[monsterNode][nearestAgentNode] + 4
			* (*(monster->shortestPath))[monsterNode][fartherAgentNode]
			+ 2*(*(monster->shortestPath))[monsterNode][pNode] 
			- (*(monster->shortestPath))[monsterNode][eNode];
	else
		return distance_farthestFromPlayer(monsterNode, nearestAgentNode);

};


double GB_FieryMaze::distance_farthestFromPlayer(long monsterNode, long agentNode)
{
  long pNode = (*gridNodeLabel)[penPosition.first][penPosition.second];
  long eNode = (*gridNodeLabel)[exitPosition.first][exitPosition.second];
  
  return 4*(*(monster->shortestPath))[monsterNode][agentNode] 
  + 2*(*(monster->shortestPath))[monsterNode][pNode] 
  - (*(monster->shortestPath))[monsterNode][eNode];
};

/**
  This routine returns the shortest distance to a node and also farthest from pen node and nearest to exit node.
*/
double GB_FieryMaze::distance_towardsNode(long monsterNode, long destNode)
{
  long pNode = (*gridNodeLabel)[penPosition.first][penPosition.second];
  long eNode = (*gridNodeLabel)[exitPosition.first][exitPosition.second];
  
  return 4*(*(monster->shortestPath))[monsterNode][destNode] 
  + 2*(*(monster->shortestPath))[monsterNode][eNode] 
  - (*(monster->shortestPath))[monsterNode][pNode];
};

