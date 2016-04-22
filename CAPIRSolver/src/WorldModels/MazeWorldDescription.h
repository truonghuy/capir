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



#ifndef __MAZEWORLDDESCRIPTION_H
#define __MAZEWORLDDESCRIPTION_H

#include "Model.h"
#include "Utilities.h"
#include <cstring>

using namespace std;
/** 
@brief This structure is used to read in parameters to be passed to MazeWorld. 
@param discount Discount factor for MDP
@param xSize Horizontal dimension of grid. 
@param ySize Vertical dimension of grid. The bottom left corner of the grid
 is (0,0) while the top right corner is (\a xSize-1, \a ySize-1)
@param characters id and locations of characters (everything except wall and free tile). 
@param grid Each region is represented by an integer. The wall is represented by
 zero and not considered a region. Every intersection must form its own region.
 Every region is only one cell wide. See example files.
@param numRegionPerAgent Number of regions in the grid, excluding walls (will be removed if we're not using obs).

- Five possible actions: move in 4 directions or stay in place.
- If move into a wall, agent/human will stay in place.
- Sheep currently moves away from nearest agent/human.
- Action encoding used is compoundAct = humanAct * numAiActs + aiAct
	humanAct = compoundAct / numAiActs
	aiAct = compoundAct % numAiActs
- Actions have no additional observations.
*/
typedef vector<long> ID_PosX_PosY; 
typedef pair< string, string > Property;
typedef pair< long, Property > ID_Property;

struct MazeWorldDescription
{
  long xSize;
  long ySize; 
  
  // this stores the list of triple (id, x, y) of characters on the map.
  ID_PosX_PosY characters; 

  // this stores the list of par <id, Property> found in the map
  vector<ID_Property> properties;
  
  std::vector<std::vector <long> > grid;
  // for abstract state
  long numRegionPerAgent;
  
  // These attributes are added thru commandline parameters
  Utilities::goalType gType;
  bool monsterBlock, agentBlock, monsterAgentBlock;
  double visionLimit;
  double targetPrecision;
  double discount;  
  long displayInterval;

  
};

#endif // __MAZEWORLDDESCRIPTION_H
