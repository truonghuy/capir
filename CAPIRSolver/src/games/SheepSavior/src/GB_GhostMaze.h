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



#ifndef __GB_GHOSTMAZE_H
#define __GB_GHOSTMAZE_H

#include "GB_Ghost.h"
#include "Maze.h"

class GhostBustersLevel;

class GB_GhostMaze : public Maze
{
private:
	static const double GhostKilledReward = 5;
public:

	int shotDistance;

  GB_GhostMaze() {
	  // shotDistance is used when there is no abstract.
	  shotDistance = 2;
  };
  double getMinReward(){ return 0;};
  
  /******** Initialization *****************************/
  void initialize(long ghostMazeId, long ghostX, long ghostY, double visLim, GhostBustersLevel* gbLevel);
  
  void setProperty(string pName, string pValue);
  
  /********* Abstract related **************************/
  double absGetReward(AbstractState& state);
  /**************** raw state ******************/
  /**
    @return reward of state.
  */
  double getReward(const State& state, bool& vTerminal, int worldNum);

  double getMaxReward(){return GhostKilledReward;};

};

#endif
