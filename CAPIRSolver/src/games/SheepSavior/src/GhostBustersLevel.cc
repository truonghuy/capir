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



#include "GhostBustersLevel.h"
#include "GB_Human.h"
#include "GB_AiAssistant.h"
#include "GB_SheepMaze.h"
#include "GB_GhostMaze.h"
#include "GB_FieryMaze.h"

#define DANGER_DIST 2

void GhostBustersLevel::initializeHumanAssistantMazes(MazeWorldDescription& desc)
{

  long id, charX, charY;
  pair<long, long> sheepPen, fieryPen, fieryExit;

  equivWorlds.resize(0);

  long ghostOrigWorld = -1; // the first virtual world of ghost
  long sheepOrigWorld = -1; // the first virtual world of sheep
  long fieryOrigWorld = -1; // the first virtual world of fiery
  //long frostyOrigWorld = -1; // the first virtual world of frosty
  
  long numMonsters = 0;
  
  vector< pair<long,long> > humanImpassableBlocks;
  vector< pair<long,long> > assistantImpassableBlocks;
  
  // 1. Go thru list of characters to initialize human, assistant and mazes
  for(unsigned i=0; i<desc.characters.size(); i=i+3){
    id = desc.characters[i];
    charX = desc.characters[i+1];
    charY = desc.characters[i+2];
    
    // initialize characters accordingly based on the id
    switch(id){
      case GB_HumanTileId:
        // NOTE: this is only possible if human is a pointer
        initializeHuman(charX, charY);
        break;
      case GB_AssistantTileId:
        initializeAssistant(charX, charY);
        break;
      case GB_SheepTileId:
        
        // if this is the first sheep encountered, set sheepOrigWorld
        if (sheepOrigWorld < 0){
          sheepOrigWorld = numMonsters;
        }
        // this is not the first world of this type encountered
        equivWorlds.push_back(sheepOrigWorld);
        initializeSheepMaze(charX, charY);
        numMonsters++;
        break;

      case GB_GhostTileId:
        // if this is the first ghost encountered, set ghostOrigWorld
        
        if (ghostOrigWorld < 0){ 
          ghostOrigWorld = numMonsters;
        }  
        // this is not the first world of this type encountered
        equivWorlds.push_back(ghostOrigWorld);
        initializeGhostMaze(charX, charY);
        numMonsters++;
        break;

      case GB_FieryTileId:
        // if this is the first fiery encountered, set fieryOrigWorld
        
        if (fieryOrigWorld < 0){ 
          fieryOrigWorld = numMonsters;
        }  
        // this is not the first world of this type encountered
        equivWorlds.push_back(fieryOrigWorld);
        initializeFieryMaze(charX, charY);
        numMonsters++;
        break;
      /*
	  case GB_FrostyTileId:
        // if this is the first frosty encountered, set frostyOrigWorld
        
        if (frostyOrigWorld < 0){ 
          frostyOrigWorld = numMonsters;
        }  
        // this is not the first world of this type encountered
        equivWorlds.push_back(frostyOrigWorld);
        initializeFrostyMaze(charX, charY);
        numMonsters++;
        break;
        */
      case GB_SheepPenTileId:
        sheepPen.first = charX;
        sheepPen.second = charY;
        break;

      case GB_FieryPenTileId:
        fieryPen.first = charX;
        fieryPen.second = charY;
        
        // use fiery pen as player[0] impassable blocks
        //humanImpassableBlocks.push_back(pair<long,long>(charX, charY));
        
        break;
      
      case GB_FieryExitTileId:
        fieryExit.first = charX;
        fieryExit.second = charY;
        
        // use fiery exit as assistant's impassable blocks
        //assistantImpassableBlocks.push_back(pair<long,long>(charX, charY));
        break;

      default:
        std::cerr << "Unsupported tile id = " << id << " - treated as ground...\n";
        //exit(EXIT_FAILURE);
    }// switch id
    
  } // for characters
  
  numWorlds = numMonsters;
  
  // 2. Set pen and exit positions for original worlds.
  if (sheepOrigWorld>=0)
    ((GB_SheepMaze*)mazes[sheepOrigWorld])->setPenPosition(sheepPen.first, sheepPen.second);

  if (fieryOrigWorld>=0){
    ((GB_FieryMaze*)mazes[fieryOrigWorld])->setPenPosition(fieryPen.first, fieryPen.second);
    ((GB_FieryMaze*)mazes[fieryOrigWorld])->setExitPosition(fieryExit.first, fieryExit.second);
  }
  /*
  //Set frosty pen and exit as opposite of fiery
  if (frostyOrigWorld>=0){
    ((GB_FrostyMaze*)mazes[frostyOrigWorld])->setPenPosition(fieryExit.first, fieryExit.second);
    ((GB_FrostyMaze*)mazes[frostyOrigWorld])->setExitPosition(fieryPen.first, fieryPen.second);
  }
	*/
  // 3. Set properties for original worlds. In GhostBusters, currently we just have visionLimit for each world
  for(unsigned i=0; i<desc.properties.size(); i++){
    switch(desc.properties[i].first){
      case GB_SheepTileId:
        if (sheepOrigWorld>=0)
          mazes[sheepOrigWorld]->setProperty(desc.properties[i].second.first, desc.properties[i].second.second);
        break;

      case GB_GhostTileId:
        if (ghostOrigWorld>=0)
          mazes[ghostOrigWorld]->setProperty(desc.properties[i].second.first, desc.properties[i].second.second);
        break;

      case GB_FieryTileId:
        if (fieryOrigWorld>=0)
          mazes[fieryOrigWorld]->setProperty(desc.properties[i].second.first, desc.properties[i].second.second);
        break;
        /*
	  case GB_FrostyTileId:
        if (frostyOrigWorld>=0)
          mazes[frostyOrigWorld]->setProperty(desc.properties[i].second.first, desc.properties[i].second.second);
        break;
      */
      default:
        //std::cerr << "Property of Unsupported tile id = " << desc.properties[i].first << "\n";
    	  break;
        //exit(EXIT_FAILURE);
    } // switch
    
  }// for property
  

  for (long i=0; i<numWorlds; i++){

    // 4. Copy properties from original worlds to their respective duplicates.
    if (i != equivWorlds[i]){
      mazes[i]->copyInfoFrom(mazes[equivWorlds[i]]);
    }

    // 5. Set human and assistant pointers in mazes
    mazes[i]->setPlayers(player[0], player[1]);
    mazes[i]->monster->setBlocking(desc.monsterAgentBlock, desc.monsterBlock);
  }
  
  // 6. Set players' belief size
  player[0]->setNumWorlds(numWorlds);
  player[1]->setNumWorlds(numWorlds);
  
  // 7. Set human and assistant's impassable blocks
  player[0]->setImpassableLoc(humanImpassableBlocks);
  player[1]->setImpassableLoc(assistantImpassableBlocks);
  
};

void GhostBustersLevel::initializeHuman(long x, long y)
{
  player[0] = new GB_Human(x, y, this);
};

void GhostBustersLevel::initializeAssistant(long x, long y)
{
  // player[1] = new GB_AiAssistant(x, y, this);
	player[1] = new GB_AiAssistant(x, y, this);
};

void GhostBustersLevel::initializeSheepMaze(long x, long y)
{
  GB_SheepMaze *maze = new GB_SheepMaze();
  maze->initialize(GB_SheepWorld, x, y, visionLimit, this);
  mazes.push_back(maze);
};

void GhostBustersLevel::initializeGhostMaze(long x, long y)
{
  GB_GhostMaze *maze = new GB_GhostMaze();
  maze->initialize(GB_GhostWorld, x, y, visionLimit, this);
  mazes.push_back(maze);
}; 

void GhostBustersLevel::initializeFieryMaze(long x, long y)
{
  GB_FieryMaze *maze = new GB_FieryMaze();
  maze->initialize(GB_FieryWorld, x, y, visionLimit, this);
  mazes.push_back(maze);
};
/*
void GhostBustersLevel::initializeFrostyMaze(long x, long y)
{
  GB_FrostyMaze *maze = new GB_FrostyMaze();
  maze->initialize(GB_FrostyWorld, x, y, visionLimit, this);
  mazes.push_back(maze);
};
*/
/**
  @return the char array representing the world map (integers separated by space). In GhostBusters, it has the following format:
  - xSize
  - ySize
  - Map of the form <0,1>; 0: ground, 1: wall; -1: sheep's pen; -2: fiery men's pen; -3: fiery men's exit
  - worldType array (1: sheep, 2: ghost, )
*/
char* GhostBustersLevel::getWorldRepresentation()
{
  std::stringstream out;
  // 1. xSize and ySize of the grid
  out << xSize << " " << ySize << " ";
  
  
  // 2. The map in 0,1 form
  vector< vector<long> > tempGrid;
  tempGrid.resize(xSize);
  for (long i = 0; i< xSize; i++)
    tempGrid[i].resize(ySize);
  
  
  for (long i = 0; i< ySize; i++){
    for (long j = 0; j < xSize; j++){
      if (grid[j][ySize-i-1]<0)
        tempGrid[j][ySize-i-1] = 0;
      else tempGrid[j][ySize-i-1] = 1;
    }
  }
  
  // 3. Special locations. Go thru all mazes and get the info of special locations to send to GUI
  for (unsigned i=0; i<mazes.size(); i++){
    // orig world
    if (i == equivWorlds[i])
    {
      switch (mazes[i]->worldType){
        case GB_SheepWorld:
          tempGrid[((GB_SheepMaze*)mazes[i])->penPosition.first] [((GB_SheepMaze*)mazes[i])->penPosition.second] = -1;
          break;

        case GB_FieryWorld:
          tempGrid[((GB_FieryMaze*)mazes[i])->penPosition.first] [((GB_FieryMaze*)mazes[i])->penPosition.second] = -2;
          tempGrid[((GB_FieryMaze*)mazes[i])->exitPosition.first] [((GB_FieryMaze*)mazes[i])->exitPosition.second] = -3;
          break;
          /*
		case GB_FrostyWorld:
          tempGrid[((GB_FrostyMaze*)mazes[i])->penPosition.first] [((GB_FrostyMaze*)mazes[i])->penPosition.second] = -3;
          tempGrid[((GB_FrostyMaze*)mazes[i])->exitPosition.first] [((GB_FrostyMaze*)mazes[i])->exitPosition.second] = -2;
          break;
        */
        default: // ghost, no need        
          break;
      }
    }
  }
  
  // 4. write tempGrid to out
  for (long i = 0; i< ySize; i++){
    for (long j = 0; j < xSize; j++){
      out << tempGrid[j][ySize-i-1] << " ";
      //or: out << tempGrid[j][i] << " ";
    }
  }
  
  // 5. write worldType
  for (unsigned i=0; i<mazes.size(); i++){
    out << mazes[i]->worldType << " ";
  }
  
  // 6. Convert to char array
  char *result;
  std::string tempStr;
  tempStr = out.str();
  result=new char[tempStr.size()+1];
  result[tempStr.size()]=0;
  memcpy(result, tempStr.c_str(), tempStr.size());
  
  // 7. And return 
  return result;
  
};

char* GhostBustersLevel::getWorldRepresentationXML()
{
  std::stringstream out;
  
  // 1. formality
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  
  // 2. map: <map width="11" height="11">
  out << "<map width=\"" << xSize << "\" height=\"" << ySize << "\">";
  
  // 3. the layout of the map: <tile x="" y="" type="wall"/>
  out << "<grid>";
  
  // By default, a tile is ground, only non-ground tiles are stored in map
  for (long i = 0; i< ySize; i++){
    for (long j = 0; j < xSize; j++){
      if (grid[j][ySize-i-1]<0)
        //tempGrid[j][ySize-i-1] = 1;
        out << "<tile x=\"" << j << "\" y=\"" << (ySize-i-1) << "\" type=\"wall\"/>";
    }
  }
	
	// 4. the special locations in the map
	for (unsigned i=0; i<mazes.size(); i++){
    // orig world
    if (i == equivWorlds[i])
    {
      switch (mazes[i]->worldType){
        case GB_SheepWorld:
          // sheepPen's tile
          out << "<tile x=\"" << ((GB_SheepMaze*)mazes[i])->penPosition.first << "\" y=\"" <<
          	  ((GB_SheepMaze*)mazes[i])->penPosition.second << "\" type=\"sheepPen\"/>";
          break;

        case GB_FieryWorld:
          // fieryPen
          out << "<tile x=\"" << ((GB_FieryMaze*)mazes[i])->penPosition.first << "\" y=\"" <<
          	  ((GB_FieryMaze*)mazes[i])->penPosition.second << "\" type=\"fieryPen\"/>";
          
          // fieryExit
          out << "<tile x=\"" << ((GB_FieryMaze*)mazes[i])->exitPosition.first << "\" y=\"" <<
        		  ((GB_FieryMaze*)mazes[i])->exitPosition.second << "\" type=\"fieryExit\"/>";
          break;
		/*
        case GB_FrostyWorld:
          // frostyPen
          out << "<tile x=\"" << ((GB_FrostyMaze*)mazes[i])->penPosition.first << "\" y=\"" << ((GB_FrostyMaze*)mazes[i])->penPosition.second << "\" type=\"frostyPen\"/>";
          
          // frostyExit
          out << "<tile x=\"" << ((GB_FrostyMaze*)mazes[i])->exitPosition.first << "\" y=\"" << ((GB_FrostyMaze*)mazes[i])->exitPosition.second << "\" type=\"frostyExit\"/>";
          break;
        */
        default: // ghost, no need        
          break;
      }
    }
  }
  
  // end grid
  out << "</grid>";
  
  // 5. write worldType
  out << "<world_types>";
  for (unsigned i=0; i<mazes.size(); i++){
    out << "<world id=\"" << i << "\" type=\"" << mazes[i]->worldTypeStr << "\"/>";
  }
	out << "</world_types>";
	
  // end map
  out << "</map>";
  
    
  // 6. Convert to char array
  char *result;
  std::string tempStr;
  tempStr = out.str();
  result=new char[tempStr.size()+1];
  result[tempStr.size()]=0;
  memcpy(result, tempStr.c_str(), tempStr.size());
  
  // 7. And return 
  return result;
};

// if the sheep is dead, the game is over
double GhostBustersLevel::getReward(const State& state, bool& terminal) {

	// 1. browse through all mazes, if the maze is sheep, and sheep is dead.
	for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
		// if this maze is a sheep, and it is dead
		if ((mazes[i]->worldTypeStr == "sheep")
				&& (state.mazeProperties[i][2] <= 0)) {
			terminal = true;
			//std::cout << "sheep is killed, reward is " << SheepKilledPenalty << std::endl;
			return SheepKilledPenalty;
		}
	}

	return 0;
}
;

double GhostBustersLevel::getMinReward(const State& state)
{
	double reward = MazeWorld::getMinReward(state);

	// 1. check if there are alive ghosts and alive sheep
	bool gotAliveSheep = false;
	bool gotAliveGhost = false;

	for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
		// if this maze is a sheep, and it is still alive
		if ((mazes[i]->worldTypeStr == "sheep")
				&& !(mazes[i]->isTermState(state, i) ))
			gotAliveSheep = true;

		if ((mazes[i]->worldTypeStr == "ghost")
				&& !(mazes[i]->isTermState(state, i) ))
			gotAliveGhost = true;
	}


	if (gotAliveGhost && gotAliveSheep)
		reward += SheepKilledPenalty;

	return reward;
};

bool GhostBustersLevel::negativeInteract(const State& state, long i, long j)
{
	if (mazes[i]->isLocalTermState(state, i) || mazes[j]->isLocalTermState(state, j))
		return false;

	if ((mazes[i]->worldTypeStr == "sheep" && mazes[j]->worldTypeStr == "ghost")
	 || (mazes[j]->worldTypeStr == "sheep" && mazes[i]->worldTypeStr == "ghost"))
	{

		long iNode = gridNodeLabel[state.mazeProperties[i][0]][state.mazeProperties[i][1]];
		long jNode = gridNodeLabel[state.mazeProperties[j][0]][state.mazeProperties[j][1]];

		// distance between the two is less than or equal to 2.
		if ((*(mazes[i]->monster->shortestPath))[iNode][jNode] <= DANGER_DIST)
			return true;
	}

	return false;
};

bool GhostBustersLevel::gotInteraction(const State& state)
{
	for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
		// if this maze is a sheep, and it is still alive
		if ((mazes[i]->worldTypeStr == "sheep")
				&& !(mazes[i]->isTermState(state, i) ))
		{
			GB_Sheep* sheep = (GB_Sheep*)(mazes[i]->monster);

			if (sheep->isDangered(state, i) >= 0)
				return true;
		}
	}

	return false;
};

void GhostBustersLevel::displayState(const State& state) {

	// display human-friendly representation of state (H: human, A: AI, @ is NPC)
	cout << "\n";
	long x,y;
	for (long i = 0; i < ySize; i++) {
		for (long j = 0; j < xSize; j++) {
			x = j;
			y = ySize - i - 1;
			if (grid[x][y] == -1) // wall
				cout << "o";
			// Human
			else if ((x == state.playerProperties[0][0]) &&
					(y == state.playerProperties[0][1]))
				cout << "H";
			// AI
			else if ((x == state.playerProperties[1][0]) &&
					(y == state.playerProperties[1][1]))
				cout << "A";
			// NPC
			else{
				if (hasMonster(state, x, y, "ghost")){
					cout << "G";
				}
				else if (hasMonster(state, x, y, "sheep")){
					cout << "S";
				}
				else cout << " ";
			}

		}
		cout << "\n";
	}
	cout << "\n";
};

