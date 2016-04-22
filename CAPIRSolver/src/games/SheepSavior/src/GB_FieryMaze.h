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



#ifndef __GB_FIERYMAZE_H
#define __GB_FIERYMAZE_H

#include "Maze.h"
#include "GB_Fiery.h"

class GhostBustersLevel;

class GB_FieryMaze : public Maze
{
private:
  static const double FieryCaughtReward = 5;
  static const double FieryEscapedReward = -5;
public:
  pair<long, long> penPosition;
  pair<long, long> exitPosition;

public:
  GB_FieryMaze() {};
  
  /******** Initialization *****************************/
  void initialize(long fieryMazeId, long fieryX, long fieryY, double visLim, GhostBustersLevel* gbLevel);
  
  void copyInfoFrom(Maze* orig){
    
    penPosition = ((GB_FieryMaze*)orig)->penPosition;
    exitPosition = ((GB_FieryMaze*)orig)->exitPosition;
    Maze::copyInfoFrom(orig);
  }
  
  void setProperty(string pName, string pValue);
  
  void setPenPosition(long penX, long penY);
  void setExitPosition(long x, long y);
    
  /***************** Raw State related *************************/
  double getReward(const State& state, bool& vTerminal, int worldNum);
  double getMaxReward(){return FieryCaughtReward;};
  double getMinReward(){return FieryEscapedReward;};
  
  /********* Abstract related **************************/
  double absGetReward(AbstractState& absState);  
  
  /********* Distance function *************************/
  double distance_farthestFromBoth(long monsterNode, long nearestAgentNode,
		  unsigned agentIndex, long fartherAgentNode);
  double distance_farthestFromPlayer(long monsterNode, long agentNode);
  double distance_towardsNode(long monsterNode, long destNode);
};

#endif

