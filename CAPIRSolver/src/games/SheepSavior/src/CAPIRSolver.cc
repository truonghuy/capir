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
#include <sstream>
#include <iostream>
#include <cstdlib>

using namespace std;

int main(int argc, char **argv)
{
  ostringstream message;
  double discount = 0.99;
  double targetPrecision = 0.01;
  long displayInterval = 1;
  double visionLimit = 3;
  Utilities::goalType gType = Utilities::andType;
  
  bool monsterBlock = false;
  bool agentBlock = false;
  bool monsterAgentBlock = false;
  bool useAbstract = false;

  message << "Usage:\n"
	  << "  -m mapfile\n"
	  << "  -u useAbstract (default: 0)\n"
	  << "  -p targetPrecision (default: 0.01)\n"
	  << "  -d discountFactor (default: 0.99)\n"
	  << "  -v visionLimit (default visionLimit for all mazes: 3)\n"
	  << "  -g gType (default: 1, 0 = orType, 1 = and)\n" 
	  << "  -1 monsterBlock (0 or 1, default = 1 meaning monster are blocked among each other)\n"
	  << "  -2 agentBlock (0 or 1, default = 0 meaning agent/human do not block each other)\n"
	  << "  -3 monsterAgentBlock (0 or 1, default = 1 meaning monster are blocked from agent/human and vice versa)\n"
	  << "  -i displayInterval (default: 1)\n";
  
  if (argc == 1){
    cout << message.str() << endl;
    exit(1);
  }

  string map_file, vqFns_file;
  for (long i=1; i<argc; i++) {
    if (argv[i][0] != '-') {
      cout << message.str() << endl;
      exit(1);
    }
    i++;
    switch(argv[i-1][1]) {
    case 'm': 
      map_file = argv[i];
      vqFns_file = map_file;
      break;
    case 'u':
      useAbstract = (atoi(argv[i]) == 1);
      break;
    case '1':
      monsterBlock = (atoi(argv[i]) == 1);
      break;
    case '2':
      agentBlock = (atoi(argv[i]) == 1);
      break;
    case '3':
      monsterAgentBlock = (atoi(argv[i]) == 1);
      break;
    case 'd':
      discount = atof(argv[i]);
      break;
    case 'v':
      visionLimit = atof(argv[i]);
      break;
    case 'g':
      {
      int type = atoi(argv[i]);
      gType = ( (type==Utilities::orType)? Utilities::orType : Utilities::andType);
      break;
      }
    case 'p':
      targetPrecision = atof(argv[i]);
      break;
    case 'i':
      displayInterval = atoi(argv[i]);
      break;
    default:
      cout << message.str() << endl;
      exit(1);
    }
  }

  MazeWorldDescription currDescription;
  currDescription.discount = discount;
  currDescription.visionLimit = visionLimit; // default visionLimit
  currDescription.targetPrecision = targetPrecision;
  currDescription.displayInterval = displayInterval;
  currDescription.gType = gType;
  currDescription.monsterBlock = monsterBlock;
  currDescription.agentBlock = agentBlock;
  currDescription.monsterAgentBlock = monsterAgentBlock;

  // 1. Read problem from file
  
  if(map_file.substr(map_file.find_last_of(".") + 1) == "tmx") {
    GameTileSheet gts;
    GhostBustersLevel::readDescriptionFromTMXFile(map_file, gts, currDescription);
  } else {
    cout << "Wrong map_file format detected!!" << endl;
    exit(1);
  }
  
  // 2. Initialize GhostBustersLevel
  GhostBustersLevel currLevel(currDescription);
  currLevel.initializeHumanAssistantMazes(currDescription);
  
  // 3. Set to use abstract states or not. 
  currLevel.setUseAbstract(useAbstract);
  
  // 4. Solve
  currLevel.generateModel(); 

  // 5. Write resultant policy to file
  currLevel.writeSolution(vqFns_file); // used to be writeModels: write Q functions to file

};

