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



#include "GB_GhostMaze.h"
#include "GhostBustersLevel.h"

void GB_GhostMaze::initialize(long ghostMazeId, long ghostX, long ghostY, double visLim, GhostBustersLevel* gbLevel)
{
  // 1. initialize the monster
  monster = new GB_Ghost(ghostX, ghostY, this);
  
  worldType = ghostMazeId;
  worldTypeStr = "ghost";
  visionLimit = visLim;
  // always set to 1
  //visionLimit = 1;
  mazeWorld = gbLevel;
};

/**
  There can be a series of if else here to set corresponding property

*/
void GB_GhostMaze::setProperty(string pName, string pValue)
{
  if (pName == "visionLimit")
    visionLimit = atof(pValue.c_str());
  else if (pName == "shotDistance"){
	  shotDistance = atoi(pValue.c_str());
  }
  // set maxHP
  else if (pName == "maxHP"){
	  monster->properties[0] = monster->maxValues[0] = atoi(pValue.c_str());
  }
};

/************ Abstract related ****************************/

// TODO: The reward related to Ghost here should be evaluated in Ghost class, not here.

double GB_GhostMaze::absGetReward(AbstractState& absState)
{
  // 2. not previously terminal, and HP <=0 
  if (absState.monsterProperties[4] <= 0){
    absState.monsterProperties[0] = TermState;
    return GhostKilledReward;
  }
    
  return NoReward;
};

/********************* raw state **************************/
double GB_GhostMaze::getReward(const State& state, bool& vTerminal, int worldNum)
{ 

  if (isTermState(state, worldNum)){
    vTerminal = true;
    return NoReward;
  }
  
  // 1. No HP left
  vTerminal = false;
  if (state.mazeProperties[worldNum][2] <= 0)
  {
    vTerminal = true;
    return GhostKilledReward;
  }
  
  return NoReward;
};

