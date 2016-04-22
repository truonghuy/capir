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



#ifndef __GHOSTBUSTERSLEVEL_H
#define __GHOSTBUSTERSLEVEL_H

#include "MazeWorld.h"


/**
  This GhostBustersLevel class extends the MazeWorld class, which represents one level in the GhostBusters game.

*/

/********************* TileId *********************/
#define GB_GroundId 1
#define GB_AssistantTileId 2
#define GB_GhostTileId 3
#define GB_FieryPenTileId 4
#define GB_WallId 5
#define GB_SheepTileId 6
#define GB_FieryTileId 7
#define GB_FrostyTileId 8
#define GB_HumanTileId 9
#define GB_SheepPenTileId 10
#define GB_FieryExitTileId 11

/******************** World Id *******************/
// Sheep should be herded into a pen
#define GB_SheepWorld 0
#define GB_GhostWorld 1
#define GB_FieryWorld 2
#define GB_FrostyWorld 3

/****************** GhostBustersLevel class *********/
class GhostBustersLevel : public MazeWorld
{
private:
	static const double SheepKilledPenalty = -10;
public:
  GhostBustersLevel(MazeWorldDescription& desc) : MazeWorld(desc)
  {};
  
  /**************** Initialization *******************************/

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
  void initializeHumanAssistantMazes(MazeWorldDescription& desc);
  
  void initializeHuman(long x, long y);
  void initializeAssistant(long x, long y);
  void initializeSheepMaze(long x, long y);
  void initializeGhostMaze(long x, long y);
  void initializeFieryMaze(long x, long y);
  //void initializeFrostyMaze(long x, long y);
  
  /******************* Map representation ***************************/
  
  /**
    @return the char array representing the world map (integers separated by space). In GhostBusters, it has the following format:
    - xSize
    - ySize
    - Map of the form <0,1>; 0: ground, 1: wall; -1: sheep's pen; -2: fiery men's pen; -3: fiery men's exit
    - worldType array (1: sheep, 2: ghost, )
  */
  char* getWorldRepresentation();
  
    
  /**
    XML equivalent to getWorldRepresentation. We are going to replace getWorldRepresentation by getWorldRepresentationXML.
    Output:
    \verbatim
    <?xml version="1.0" encoding="UTF-8"?>
    <map width="11" height="11">
      <grid>
        <tile x="" y="" type="wall"/>
        <tile x="" y="" type="player1"/>
        …
      </grid>
      <npc_types>sheep sheep fiery ghost fiery</npc_types>
    </map>
    \endverbatim
  */
  char* getWorldRepresentationXML();
  
  void displayState(const State& state);

  /**************** Full version **************/
  double getReward(const State& state, bool& terminal);

  double getMinReward(const State& state);

  bool negativeInteract(const State& state, long i, long j);

  bool gotInteraction(const State& state);
};

#endif

