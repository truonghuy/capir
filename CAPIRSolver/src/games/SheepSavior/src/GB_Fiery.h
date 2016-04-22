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



#ifndef __GB_FIERY_H
#define __GB_FIERY_H

#include "Monster.h"

/**
  A GB_Fiery has 5 move actions (unchanged, east, west, north, south) and no special action or property.
*/

class GB_Fiery : public Monster
{
public:
  GB_Fiery(long x, long y, Maze *maze) : Monster(x, y, 5, 0, 0, maze) {
	  //OptimalProb = 1.0;
  };

  /********* Define actions for Fiery when not seeing any player *****/
  void getActNoneSeen(const State& state, long worldNum, std::vector< std::pair<long,double> >& action_prob);
  
  void absGetActNoneSeen(const AbstractState& absState, long humanAct, long aiAct, const AbstractState& prevAbsState, std::vector< std::pair<long,double> >& action_prob);

  // Fiery avoids the exit when seeing only one.
  void moveActsFarthestFromPlayer(long currMonsterX, long currMonsterY,
  	    long agentX, long agentY, unsigned agentIndex, long otherX, long otherY,
  	    std::vector<std::pair<long, double> >& action_prob,const State* state = 0);
};

#endif
