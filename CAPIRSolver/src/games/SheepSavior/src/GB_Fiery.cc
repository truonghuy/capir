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
#include "GB_Fiery.h"

void GB_Fiery::getActNoneSeen(const State& state, long worldNum,
		std::vector< std::pair<long,double> >& action_prob)
{ 
  long currMonsterX = state.mazeProperties[worldNum][0];
  long currMonsterY = state.mazeProperties[worldNum][1];
  long exitX = ((GB_FieryMaze*)maze)->exitPosition.first;
  long exitY = ((GB_FieryMaze*)maze)->exitPosition.second;
  
  moveActsTowardsLocationExcludePoint(currMonsterX, currMonsterY, exitX, exitY, action_prob,
		  ((GB_FieryMaze*)maze)->penPosition.first,((GB_FieryMaze*)maze)->penPosition.second,
		  &state);
};

void GB_Fiery::absGetActNoneSeen(const AbstractState& absState, long humanAct, long aiAct,
		const AbstractState& prevAbsState, std::vector< std::pair<long,double> >& action_prob)
{
  long currMonsterX = absState.monsterProperties[0];
  long currMonsterY = absState.monsterProperties[1];  
  long exitX = ((GB_FieryMaze*)maze)->exitPosition.first;
  long exitY = ((GB_FieryMaze*)maze)->exitPosition.second;
  
  moveActsTowardsLocationExcludePoint(currMonsterX, currMonsterY, exitX, exitY, action_prob,
		  ((GB_FieryMaze*)maze)->penPosition.first,((GB_FieryMaze*)maze)->penPosition.second);
};

void GB_Fiery::moveActsFarthestFromPlayer(long currMonsterX, long currMonsterY,
	long agentX, long agentY, unsigned agentIndex, long otherX, long otherY,
	std::vector<std::pair<long, double> >& action_prob, const State* state){
  moveActsFarthestFromPlayerExcludePoint(currMonsterX, currMonsterY,
		  agentX, agentY, agentIndex, otherX, otherY, action_prob,
		  ((GB_FieryMaze*)maze)->penPosition.first,((GB_FieryMaze*)maze)->penPosition.second,
		  state);
};
