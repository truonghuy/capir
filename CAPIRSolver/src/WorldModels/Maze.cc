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



#include "Maze.h"
#include "MazeWorld.h"

void Maze::getAbstractWorldGeometricInfo() {
	// TO DO - Move the declaration to header and make them instance attributes.
	// 1. get necessary geographical info from mazeWorld
	connectivity = &(mazeWorld->connectivity);
	borderLength = &(mazeWorld->borderLength);
	regionRepPoint = &(mazeWorld->regionRepPoint);
	rType = &(mazeWorld->rType);
}
;

void Maze::getRawWorldGeometricInfo() {
	grid = &(mazeWorld->grid);
	gridNodeLabel = &(mazeWorld->gridNodeLabel);
	numAccessibleLocs = mazeWorld->numAccessibleLocs;
	// calculate shortestPath for this monster
	monster->calcShortestPathMatrix(mazeWorld->xSize, mazeWorld->ySize,
	    mazeWorld->numAccessibleLocs, mazeWorld->gridNodeLabel);

}
;

// TODO --------------- State map generation
/********************** State Map Generation ******************************/

void Maze::generateStateMap() {

	if (useAbstract) {
		generateAbstractStateMap();
	} else {
		generateRawStateMap();
	}
}
;

void Maze::generateRawStateMap() {
	std::cout << "generate raw stateMap~~~~~~~~~~" << worldTypeStr << "~~~~~"<< std::endl;

	AbstractState temp;
	std::vector<AbstractState> tempV1(0);
	std::vector<AbstractState> tempV2(0);

	reverseVAbsStateMap = new vector<AbstractState> ;
	vAbsStateMap = new map<AbstractState, long, Utilities::AbstractStateComparator> ;

	long stateIndex;

	// 1. dummy terminal state, where playerProperties[humanIndex]'s first item == TermState
	temp.playerProperties[humanIndex].push_back(TermState);
	temp.playerProperties[humanIndex].push_back(TermState);

	reverseVAbsStateMap->push_back(temp);
	vAbsStateMap->insert(std::pair<AbstractState, long>(temp, 0));

	// Because we are enumerating all coordinates
	// so there's no need to enumerate monster's coord first.

	int x = 1, y = 2;
	temp.playerProperties[0][0] = 0;
	temp.playerProperties[0].resize(y + 1, 0);
	temp.playerProperties[1].resize(y + 1, 0);

	if (monster)
		temp.monsterProperties.resize(4, 0);

	stateIndex = 1;

	// all player 1's valid position
	for (long p1x = 0; p1x < mazeWorld->xSize; p1x++) {
		for (long p1y = 0; p1y < mazeWorld->ySize; p1y++) {
			if (playerPassable(humanIndex, p1x, p1y)) {

				temp.playerProperties[humanIndex][x] = p1x;
				temp.playerProperties[humanIndex][y] = p1y;

				// all player 2's valid position
				for (long p2x = 0; p2x < mazeWorld->xSize; p2x++) {
					for (long p2y = 0; p2y < mazeWorld->ySize; p2y++) {
						if (playerPassable(aiIndex, p2x, p2y)) {

							temp.playerProperties[aiIndex][x] = p2x;
							temp.playerProperties[aiIndex][y] = p2y;

							if (monster) {
								for (long mx = 0; mx < mazeWorld->xSize; mx++) {
									for (long my = 0; my < mazeWorld->ySize; my++) {
										if (mx == p1x && my == p1y){
											int blah;
											blah = 1;
										}
										if (monsterPassable(mx, my, p1x, p1y, p2x, p2y)) {
											temp.monsterProperties[0] = mx;
											temp.monsterProperties[1] = my;

											updateVisibility_NoAbs(temp);

											// fill in properties of critters
											tempV1.resize(0);
											tempV2.resize(0);

											tempV1.push_back(temp);

											//std::cout << "human enumerates properties...";
											// 5. enumerate human and ai's properties.
											// Depending on the type of monster human/assistant are dealing with, some properties are not included
											player[0]->enumerateProperties(tempV1, tempV2, monster);
											player[1]->enumerateProperties(tempV2, tempV1, monster);

											// 6. enumerate monster's properties.
											//std::cout << "monster enumerate properties" << std::endl;
											monster->enumerateProperties(tempV1, tempV2, player[0]);

											// 7. If there is specialLocation, enumerate its properties.
											if (specialLocation) {

												specialLocation->enumerateProperties(tempV2, tempV1);
												for (unsigned k = 0; k < tempV1.size(); k++) {
													vAbsStateMap->insert(
													    std::pair<AbstractState, long>(tempV1[k],
													        stateIndex));
													reverseVAbsStateMap->push_back(tempV1[k]);
													stateIndex++;
												}
											} else {

												for (unsigned k = 0; k < tempV2.size(); k++) {
													vAbsStateMap->insert(
													    std::pair<AbstractState, long>(tempV2[k],
													        stateIndex));
													reverseVAbsStateMap->push_back(tempV2[k]);
													stateIndex++;
												}
											} // else
										}
									}
								}
							} else { // there's no monster, i.e. there is only special location
								// fill in properties of critters
								tempV1.resize(0);
								tempV2.resize(0);

								tempV1.push_back(temp);

								//std::cout << "human enumerates properties...";
								// 5. enumerate human and ai's properties.
								// Depending on the type of monster human/assistant are dealing with, some properties are not included
								player[0]->enumerateProperties(tempV1, tempV2, monster);
								player[1]->enumerateProperties(tempV2, tempV1, monster);

								assert( specialLocation != 0);

								// 7. enumerate specialLocation's properties.
								specialLocation->enumerateProperties(tempV1, tempV2);
								for (unsigned k = 0; k < tempV2.size(); k++) {
									vAbsStateMap->insert(
									    std::pair<AbstractState, long>(tempV2[k], stateIndex));
									reverseVAbsStateMap->push_back(tempV2[k]);
									stateIndex++;
								}
							}

						}

					}
				}

			}

		}
	}

	virtualSize = stateIndex;
	std::cout << "Num virtual raw states: " << stateIndex << std::endl;

}
;

/**
 Note: In absMoveOneAgent, when an agent cross region, if there is no monster in that region, its coords is set to the representative coords of that region.
 */
void Maze::generateAbstractStateMap() {
	std::cout << "generateAbstractStateMap~~~~~~~~~~" << worldTypeStr << "~~~~~"<< std::endl;
	constructVisibleNearbyRegion();

	AbstractState temp;
	std::vector<AbstractState> tempV1(0);
	std::vector<AbstractState> tempV2(0);

	reverseVAbsStateMap = new vector<AbstractState> ;
	vAbsStateMap = new map<AbstractState, long,
	    Utilities::AbstractStateComparator> ;

	long stateIndex;

	// A. If there is monster in this world.

	if (monster) {
		// 1. dummy terminal state, where playerProperties[humanIndex]'s first item == TermState

		temp.playerProperties[humanIndex].push_back(TermState);
		reverseVAbsStateMap->push_back(temp);
		vAbsStateMap->insert(std::pair<AbstractState, long>(temp, 0));

		stateIndex = 1;

		// for all monster's coords
		for (long i = 0; i < mazeWorld->xSize; i++) {
			for (long j = 0; j < mazeWorld->ySize; j++) {

				// 2. For all valid coordinates of monster
				if ((*grid)[i][j] >= 0) {

					// for all human's region
					for (long humanRegion = 0; humanRegion < mazeWorld->numRegionPerAgent; humanRegion++) {

						if (playerPassable(humanIndex, humanRegion, i, j))

							// for all ai's region
							for (long aiRegion = 0; aiRegion < mazeWorld->numRegionPerAgent; aiRegion++) {

								if (playerPassable(aiIndex, aiRegion, i, j)) {

									// 3. Allocate memory for temp
									for (long agentIndex = 0; agentIndex < 2; agentIndex++) {
										temp.playerProperties[agentIndex].resize(0);
										temp.playerProperties[agentIndex].resize(3); // regionID, coordX, coordY
									}

									temp.monsterProperties.resize(0);
									temp.monsterProperties.resize(4); // coordX, coordY, seesHuman, seesAssistant

									// 3. Fill in all location-related info
									// regions
									temp.playerProperties[humanIndex][0] = humanRegion;
									temp.playerProperties[aiIndex][0] = aiRegion;

									// monster's coords
									temp.monsterProperties[0] = i;
									temp.monsterProperties[1] = j;

									// 4. enumerate human and ai's visibility and coords where appropriate, e.g. in the same region as monster, and their additional properties.
									tempV1.resize(0);
									tempV2.resize(0);

									tempV1.push_back(temp);

									// TO DO: use 0,1 to determine whether it's human or assistant
									// Then use a vector pointer to point to that vector to enumerate it
									enumerateLocationOneAgent(tempV1, tempV2, humanIndex);
									enumerateLocationOneAgent(tempV2, tempV1, aiIndex);

									//std::cout << "human enumerates properties...";
									// 5. enumerate human and ai's properties.
									// Depending on the type of monster human/assistant are dealing with, some properties are not included
									player[0]->enumerateProperties(tempV1, tempV2, monster);
									player[1]->enumerateProperties(tempV2, tempV1, monster);

									// 6. enumerate monster's properties.
									//std::cout << "monster enumerate properties" << std::endl;
									monster->enumerateProperties(tempV1, tempV2);

									// 7. If there is specialLocation, enumerate its properties.
									if (specialLocation) {

										specialLocation->enumerateProperties(tempV2, tempV1);
										for (unsigned k = 0; k < tempV1.size(); k++) {
											vAbsStateMap->insert(
											    std::pair<AbstractState, long>(tempV1[k], stateIndex));
											reverseVAbsStateMap->push_back(tempV1[k]);
											stateIndex++;
										}
									} else {
										for (unsigned k = 0; k < tempV2.size(); k++) {
											vAbsStateMap->insert(
											    std::pair<AbstractState, long>(tempV2[k], stateIndex));
											reverseVAbsStateMap->push_back(tempV2[k]);
											stateIndex++;
										}
									} // else

								}
							}
					}

				} // if (*grid)[i][j]

			} // for long j

		} // for long i

	}
	// no monster, this means there must be a specialLocation
	// In this case, there's no need to use visionLimit.
	else {
		// 1. dummy terminal state, where playerProperties[humanIndex]'s first item == TermState

		temp.playerProperties[humanIndex].push_back(TermState);
		reverseVAbsStateMap->push_back(temp);
		vAbsStateMap->insert(std::pair<AbstractState, long>(temp, 0));

		stateIndex = 1;

		// for all human's region
		for (long humanRegion = 0; humanRegion < mazeWorld->numRegionPerAgent; humanRegion++) {

			// for all ai's region
			for (long aiRegion = 0; aiRegion < mazeWorld->numRegionPerAgent; aiRegion++) {

				// 2. Allocate memory for temp
				for (long agentIndex = 0; agentIndex < 2; agentIndex++) {
					temp.playerProperties[agentIndex].resize(0);
					temp.playerProperties[agentIndex].resize(3); // regionID, coordX, coordY
				}

				// 3. Fill in all location-related info
				// regions
				temp.playerProperties[humanIndex][0] = humanRegion;
				temp.playerProperties[aiIndex][0] = aiRegion;

				// coords - this may not be necessary
				temp.playerProperties[humanIndex][1]
				    = (*regionRepPoint)[humanRegion].first;
				temp.playerProperties[humanIndex][2]
				    = (*regionRepPoint)[humanRegion].second;

				temp.playerProperties[aiIndex][1] = (*regionRepPoint)[aiRegion].first;
				temp.playerProperties[aiIndex][2] = (*regionRepPoint)[aiRegion].second;

				// 4. enumerate human and ai's properties
				tempV1.resize(0);
				tempV2.resize(0);

				tempV1.push_back(temp);

				player[0]->enumerateProperties(tempV1, tempV2);
				player[1]->enumerateProperties(tempV2, tempV1);

				assert( specialLocation != 0);

				// 7. enumerate specialLocation's properties.
				specialLocation->enumerateProperties(tempV1, tempV2);
				for (unsigned k = 0; k < tempV2.size(); k++) {
					vAbsStateMap->insert(
					    std::pair<AbstractState, long>(tempV2[k], stateIndex));
					reverseVAbsStateMap->push_back(tempV2[k]);
					stateIndex++;
				}

			} // for ai region

		} // for human region


	} // else monster

	virtualSize = stateIndex;
	std::cout << "Num virtual abstract states: " << stateIndex << std::endl;

}
;

/**
 Enumerate human and ai's visibility and coords where appropriate, e.g. in the same region as monster.

 humanIndex is 0, aiIndex is 1

 Optimization - Can enumerate in place?

 */
void Maze::enumerateLocationOneAgent(std::vector<AbstractState>& input,
    std::vector<AbstractState>& output, long agentIndex) {
	output.resize(0);
	AbstractState currAbsState;
	long numVisibleNodes, monsterX, monsterY, agentRegion;

	// in each of the abstract state, human's, ai's regions, and monster's coords are already set
	for (unsigned i = 0; i < input.size(); i++) {
		currAbsState = input[i];

		monsterX = currAbsState.monsterProperties[0];
		monsterY = currAbsState.monsterProperties[1];

		agentRegion = currAbsState.playerProperties[agentIndex][0];

		numVisibleNodes = regionVisible((*gridNodeLabel)[monsterX][monsterY],
		    agentRegion);

		// no need to indicate relative position with monster, use representative point for coords
		currAbsState.playerProperties[agentIndex][1]
		    = (*regionRepPoint)[agentRegion].first;
		currAbsState.playerProperties[agentIndex][2]
		    = (*regionRepPoint)[agentRegion].second;

		// 1. agent is in the same region as monster
		if (agentRegion == (*grid)[monsterX][monsterY]) {

			if ((*borderLength)[agentRegion][unchanged] == 1) {
				// seen for sure
				currAbsState.monsterProperties[2 + agentIndex] = 1;
				output.push_back(currAbsState);
			} else {

				// vertical
				if ((*rType)[agentRegion] == vertical) {

					// if monster is at one end of the corridor, we do not consider this end
					if (fabs(currAbsState.playerProperties[agentIndex][2] - monsterY) > 0) {
						// this side totally seen
						if (fabs(currAbsState.playerProperties[agentIndex][2] - monsterY)
						    <= visionLimit) {
							currAbsState.monsterProperties[2 + agentIndex] = 1;
							output.push_back(currAbsState);
						}
						// this side partially seen
						else {
							currAbsState.monsterProperties[2 + agentIndex] = 0;
							output.push_back(currAbsState);
							currAbsState.monsterProperties[2 + agentIndex] = 1;
							output.push_back(currAbsState);
						}
					}

					// the other end of the region
					currAbsState.playerProperties[agentIndex][2]
					    = (*regionRepPoint)[agentRegion].second
					        + (*borderLength)[agentRegion][unchanged] - 1;

					if (fabs(currAbsState.playerProperties[agentIndex][2] - monsterY) > 0) {
						// this side totally seen
						if (fabs(currAbsState.playerProperties[agentIndex][2] - monsterY)
						    <= visionLimit) {
							currAbsState.monsterProperties[2 + agentIndex] = 1;
							output.push_back(currAbsState);
						}
						// this side partially seen
						else {
							currAbsState.monsterProperties[2 + agentIndex] = 0;
							output.push_back(currAbsState);
							currAbsState.monsterProperties[2 + agentIndex] = 1;
							output.push_back(currAbsState);
						}
					}
				}
				// horizon
				else {

					if (fabs(currAbsState.playerProperties[agentIndex][1] - monsterX) > 0) {
						// this side totally seen
						if (fabs(currAbsState.playerProperties[agentIndex][1] - monsterX)
						    <= visionLimit) {
							currAbsState.monsterProperties[2 + agentIndex] = 1;
							output.push_back(currAbsState);
						}
						// this side partially seen
						else {
							currAbsState.monsterProperties[2 + agentIndex] = 0;
							output.push_back(currAbsState);
							currAbsState.monsterProperties[2 + agentIndex] = 1;
							output.push_back(currAbsState);
						}
					}

					// the other end of the region
					currAbsState.playerProperties[agentIndex][1]
					    = (*regionRepPoint)[agentRegion].first
					        + (*borderLength)[agentRegion][unchanged] - 1;

					if (fabs(currAbsState.playerProperties[agentIndex][1] - monsterX) > 0) {
						// this side totally seen
						if (fabs(currAbsState.playerProperties[agentIndex][1] - monsterX)
						    <= visionLimit) {
							currAbsState.monsterProperties[2 + agentIndex] = 1;
							output.push_back(currAbsState);
						}
						// this side partially seen
						else {
							currAbsState.monsterProperties[2 + agentIndex] = 0;
							output.push_back(currAbsState);
							currAbsState.monsterProperties[2 + agentIndex] = 1;
							output.push_back(currAbsState);
						}
					}
				}// else (*rType) == vertical
			} // else (*rType) == junction

		}
		// agent is not in the same region as monster
		else {
			// if agent's region is seen
			if (numVisibleNodes > 0) {
				currAbsState.monsterProperties[2 + agentIndex] = 1;
				output.push_back(currAbsState);
			}
			// region is not totally seen
			if (numVisibleNodes < (*borderLength)[agentRegion][unchanged]) {
				currAbsState.monsterProperties[2 + agentIndex] = 0;
				output.push_back(currAbsState);
			}
		} // else
	} // for long i
}
;

void Maze::constructVisibleNearbyRegion() {
	// 2. Construct visibleNearByRegions, nearby regions that are within visionLimit
	visibleNearByRegions = new vector<vector<pair<long, long> > > (
	    mazeWorld->numAccessibleLocs);

	long numVisibleNodes, monsterNode, monsterRegion, currRegion, k, nextRegion;

	for (long i = 0; i < mazeWorld->xSize; i++) {
		for (long j = 0; j < mazeWorld->ySize; j++) {

			monsterNode = (*gridNodeLabel)[i][j];
			monsterRegion = (*grid)[i][j];
			if (monsterNode >= 0) {
				(*visibleNearByRegions)[monsterNode].resize(0);

				// add monster's current grid node in the vector
				(*visibleNearByRegions)[monsterNode].push_back(
				    std::pair<long, long>(monsterRegion, 1));

				// neighbor with regions in east/west direction (horizon corridors or junctions)
				if (((*rType)[monsterRegion] == horizon) || ((*rType)[monsterRegion]
				    == junction)) {

					// move west from current point.
					currRegion = monsterRegion;
					numVisibleNodes = 0;
					for (k = 1; (((*grid)[i - k][j] >= 0) && (k <= visionLimit)); k++) // west
					{
						nextRegion = (*grid)[i - k][j];
						if (nextRegion != currRegion) {
							if (currRegion == monsterRegion) {
								(*visibleNearByRegions)[monsterNode][0].second
								    += numVisibleNodes;
							} else {
								(*visibleNearByRegions)[monsterNode].push_back(
								    std::pair<long, long>(currRegion, numVisibleNodes));
							}

							currRegion = nextRegion;
							numVisibleNodes = 1;
						} else
							numVisibleNodes++;
					}

					if (numVisibleNodes > 0) {
						if (currRegion == monsterRegion)
							(*visibleNearByRegions)[monsterNode][0].second += numVisibleNodes;
						else
							(*visibleNearByRegions)[monsterNode].push_back(
							    std::pair<long, long>(currRegion, numVisibleNodes));
					}

					// move east from current point.
					currRegion = monsterRegion;
					numVisibleNodes = 0;
					for (k = 1; (((*grid)[i + k][j] >= 0) && (k <= visionLimit)); k++) // east
					{
						nextRegion = (*grid)[i + k][j];
						if (nextRegion != currRegion) {
							if (currRegion == monsterRegion) {
								(*visibleNearByRegions)[monsterNode][0].second
								    += numVisibleNodes;
							} else {
								(*visibleNearByRegions)[monsterNode].push_back(
								    std::pair<long, long>(currRegion, numVisibleNodes));

							}
							currRegion = nextRegion;
							numVisibleNodes = 1;
						} else
							numVisibleNodes++;
					}

					if (numVisibleNodes > 0) {
						if (currRegion == monsterRegion)
							(*visibleNearByRegions)[monsterNode][0].second += numVisibleNodes;
						else
							(*visibleNearByRegions)[monsterNode].push_back(
							    std::pair<long, long>(currRegion, numVisibleNodes));
					}
				}// if (*rType)

				// neighbor with regions in south/north direction (vertical corridors or junctions)
				if (((*rType)[monsterRegion] == vertical) || ((*rType)[monsterRegion]
				    == junction)) {

					// move south from current point.
					currRegion = monsterRegion;
					numVisibleNodes = 0;
					for (k = 1; (((*grid)[i][j - k] >= 0) && (k <= visionLimit)); k++) // south
					{
						nextRegion = (*grid)[i][j - k];
						if (nextRegion != currRegion) {
							if (currRegion == monsterRegion) {
								(*visibleNearByRegions)[monsterNode][0].second
								    += numVisibleNodes;
							} else {
								(*visibleNearByRegions)[monsterNode].push_back(
								    std::pair<long, long>(currRegion, numVisibleNodes));
							}
							currRegion = nextRegion;
							numVisibleNodes = 1;
						} else
							numVisibleNodes++;
					}

					if (numVisibleNodes > 0) {
						if (currRegion == monsterRegion)
							(*visibleNearByRegions)[monsterNode][0].second += numVisibleNodes;
						else
							(*visibleNearByRegions)[monsterNode].push_back(
							    std::pair<long, long>(currRegion, numVisibleNodes));
					}

					// move north from current point.
					currRegion = monsterRegion;
					numVisibleNodes = 0;
					for (k = 1; (((*grid)[i][j + k] >= 0) && (k <= visionLimit)); k++) // north
					{
						nextRegion = (*grid)[i][j + k];
						if (nextRegion != currRegion) {
							if (currRegion == monsterRegion) {
								(*visibleNearByRegions)[monsterNode][0].second
								    += numVisibleNodes;
							} else {
								(*visibleNearByRegions)[monsterNode].push_back(
								    std::pair<long, long>(currRegion, numVisibleNodes));
							}
							currRegion = nextRegion;
							numVisibleNodes = 1;
						} else
							numVisibleNodes++;
					}

					if (numVisibleNodes > 0) {
						if (currRegion == monsterRegion)
							(*visibleNearByRegions)[monsterNode][0].second += numVisibleNodes;
						else
							(*visibleNearByRegions)[monsterNode].push_back(
							    std::pair<long, long>(currRegion, numVisibleNodes));
					}
				}// if rType

			} // if monsterNode >=0

		} // for j
	} // for i
}
;

// TODO --------------- Solver related
/******************** Solver related ****************/
/**
 Assumption: There are only corridors and junctions in the map (1.5 dimensions)
 In solver, if generateModel is called with abstract flag on, V and Q functions store values of abstract states.
 If generateModel is called, V and Q functions store values of normal states.
 */
void Maze::generateModel() {

	// UPDATE 09 March 2010: Always check for gType before calculating/using the value and Q functions

	// Algo for Calculating valueFn
	// -------------------------------------------
	// dimensions for virtualQFn: currStateIndex, compoundAct, value
	//std::vector < std::vector <double> > virtualQFn;
	long numActs;

	numActs = player[0]->getNumActs() * player[1]->getNumActs();

	// 2a. Declaration
	// Q: What are the dimensions of transitionMatrices and rewardMatrices?
	// A: transitionMatrix = 3, currStateIndex, compoundAct, (nextStateIndex, probability)
	// rewardMatrix = 2, currStateIndex, compoundAct
	// We no long need to maintain transitionMatrices
	// Remove dimension worldIndex from these two matrices, resize them at the start of each
	// iteration
	std::vector<std::vector<std::vector<pair<long, double> > > > transitionMatrix;
	std::vector<std::vector<double> > rewardMatrix;

	ValueIteration* viSolver;

	// 1. allocate memory
	valueFn = new vector<double> (0);
	collabQFn = new vector<vector<double> > (0);

	// 2. Construct transition matrices and reward matrices
	constructTRCompAct(transitionMatrix, rewardMatrix);
	std::cout << "Now solve~~~~~~~~~" << std::endl;
	// 3. Use resulted rewardMatrices and transitionMatrices to run ValueIteration for valueFns
	viSolver = new ValueIteration(virtualSize, numActs, mazeWorld->discount);
	// targetPrecision = 0.01, displayInterval = 60
	viSolver -> doValueIteration(rewardMatrix, transitionMatrix,
	    mazeWorld->targetPrecision, mazeWorld->displayInterval);

	// 4. Now viSolver stores the values, push it to valueFns
	// valueFn = new std::vector<double>;

	for (unsigned j = 0; j < viSolver->values.size(); j++)
		valueFn->push_back(viSolver->values[j]);

	delete viSolver;

	// 5. compute virtual Q Functions for compound actions first

	// use valueFn
	constructCollabQFns(transitionMatrix, rewardMatrix);

}
;

void Maze::constructTRCompAct(
    std::vector<std::vector<std::vector<std::pair<long, double> > > >& transitionMatrix,
    std::vector<std::vector<double> >& rewardMatrix) {
	std::cout << "constructTRCompAct~~~~~" << worldTypeStr << "~~~~~"
	    << std::endl;
	// Conversion: compoundAct = humanAct * numAiActs + aiAct
	// humanAct = compoundAct / numAiActs
	// aiAct = compoundAct % numAiActs
	long numActs = player[0]->getNumActs() * player[1]->getNumActs();

	// Assumptions:
	// 1. HumanActs are 0...(numHumanActs-1)
	// 2. AiActs are 0...(numAiActs-1)
	// 3. States in each virtual world i are 0...(virtualSize-1)

	sparseStateBelief tranProb;

	transitionMatrix.resize(virtualSize);
	rewardMatrix.resize(virtualSize);

	// 2a. Use virtualDynamics to populate rewardMatrices and transitionMatrices
	for (long j = 0; j < virtualSize; j++) {
		// j is current state in virtualWorld i
		// std::cout << j<<".." << std::endl;
		transitionMatrix[j].resize(0);
		rewardMatrix[j].resize(numActs);

		for (long compAct = 0; compAct < numActs; compAct++) {
			rewardMatrix[j][compAct] = absVirtualDynamics(j,
			    compAct / player[1]->getNumActs(), compAct % player[1]->getNumActs(),
			    tranProb);
			// transitionMatrices
			transitionMatrix[j].push_back(tranProb);

		} // for long compAct

	} // for long j

}
;

void Maze::constructCollabQFns(
    std::vector<std::vector<std::vector<pair<long, double> > > >& transitionMatrix,
    std::vector<std::vector<double> >& rewardMatrix) {

	long numActs = player[0]->getNumActs() * player[1]->getNumActs();

	collabQFn->resize(virtualSize);

	// std::vector < std::vector <double> > virtualQFn;
	// virtualQFn.resize(virtualSize);

	double tempValue;
	long tempCompAct;

	for (long j = 0; j < virtualSize; j++) {
		// j is current state

		// virtualQFn[j].resize(numActs);
		// (*collabQFn)[j].resize(0);
		(*collabQFn)[j].resize(numActs);

		for (long compAct = 0; compAct < numActs; compAct++) {
			double sumValue = 0;
			for (long k = 0; k < (long) transitionMatrix[j][compAct].size(); k++) {
				sumValue += transitionMatrix[j][compAct][k].second
				    * (*valueFn)[transitionMatrix[j][compAct][k].first];
			}

			// virtualQFn[j][compAct] = rewardMatrix[j][compAct] + mazeWorld->discount * sumValue;
			(*collabQFn)[j][compAct] = rewardMatrix[j][compAct] + mazeWorld->discount
			    * sumValue;

#ifdef DEBUG      
			//      cout << virtualQFn[j][compAct] << " ";
#endif
		}

	}// for j = 0

}
;
// TODO --------------- virtualDynamics related
/************************** virtualDynamics related *******************/

/**

 TODO: check world type

 @param[out] tranProb vector of pair<next abs state, prob>
 @return expected reward (analytically)
 */
double Maze::absVirtualDynamics(const long currAbsState, const long hAct,
    const long aAct, sparseStateBelief& tranProb) {
	if (currAbsState == longTermState) { // TermState
		tranProb.resize(0);
		std::pair<long, double> prob(longTermState, 1);
		tranProb.push_back(prob);
		return 0;
	}

	long humanAct = hAct;
	long aiAct = aAct;

	// 1. Get region's id of human, agent and monster, and their relative positions as well as visibility of human and agent
	AbstractState absState = (*reverseVAbsStateMap)[currAbsState];
	// getAbsStateFromLong(currAbsState);

	// This IS meaningful, considering the fact that reverseVAbsStateMap stores all absState. Same as when encountering TermState
	if (isAbstractTerminal(absState)) {
		tranProb.resize(0);
		std::pair<long, double> prob(longTermState, 1);
		tranProb.push_back(prob);
		return 0;
	}

	double reward = 0;
	long tempAbsState;

	AbstractState prevAbsState;
	Utilities::cloneAbsState(absState, prevAbsState);

	// stores the belief of next states
	std::vector<std::pair<AbstractState, double> > temp1, temp2, temp3, temp4;

	// 2. Execute human's action
	temp1.push_back(std::pair<AbstractState, double>(absState, 1));
	// 1. If this is a move act, move and update visibility
	if (player[0]->isMoveAct(humanAct)) {
		// humanAct can be set to unchanged in absExecuteAgentMoveAct
		absExecuteAgentMoveAct(temp1, humanAct, humanIndex, temp2);
	}
	// 2. Otherwise, this is a special act
	else if (player[0]->isSpecialAct(humanAct)) {
		player[0]->absExecuteSpecialAct(temp1, humanAct, temp2);
	}
	// 3. Unsupported action
	else {
		Utilities::copyAbstractProbArray(temp1, temp2);
	}

	// 3. Execute assistant's action
	if (player[1]->isMoveAct(aiAct)) {
		absExecuteAgentMoveAct(temp2, aiAct, aiIndex, temp3);
	}
	// 2. Otherwise, this is a special act
	else if (player[1]->isSpecialAct(aiAct)) {
		player[1]->absExecuteSpecialAct(temp2, aiAct, temp3);
	}
	// 3. Unsupported action
	else {
		Utilities::copyAbstractProbArray(temp2, temp3);
	}

	// 4. Move the maze's entities (monster and update specialLocation's properties).
	absExecuteMazeDynamics(temp3, humanAct, aiAct, temp4, prevAbsState);

	// 5. Quantify the rewards
	std::vector<double> rewards;

	// This routine judges the outcomes and put rewards on them.
	absGetRewards(temp4, rewards);

	// 6. temp4 now stores all next states and probability. Now convert them to long and normalize the probability
	tranProb.resize(0);
	for (unsigned i = 0; i < temp4.size(); i++) {
		reward += rewards[i];

		tempAbsState = getLongFromAbsState(temp4[i].first);
		Utilities::addLongAbsStateToVector(tempAbsState, temp4[i].second, tranProb);
	}

	return reward;
}
;

void Maze::absExecuteMazeDynamics(
    std::vector<std::pair<AbstractState, double> >& input, long humanAct,
    long aiAct, std::vector<std::pair<AbstractState, double> >& output,
    const AbstractState& prevAbsState) {
	std::vector<std::pair<AbstractState, double> > tempOutput(0);

	if (monster) {
		monster->absReact(input, humanAct, aiAct, tempOutput, prevAbsState);
	} else {
		Utilities::copyAbstractProbArray(input, tempOutput);
	}

	if (specialLocation) {
		specialLocation->absReact(tempOutput, humanAct, aiAct, output, prevAbsState);
	} else {
		Utilities::copyAbstractProbArray(tempOutput, output);
	}

}
;

bool Maze::monsterPassable(long mx, long my, long p1x, long p1y, long p2x,
    long p2y) {
	if ((monster->isImpassable(mx, my, (*gridNodeLabel)))
	    || (monster->blocksAgents() && ((mx == p1x && my == p1y) || (mx == p2x
	        && my == p2y))))
		return false;

	return true;
}
;

void Maze::absExecuteMonsterMoveAct(AbstractState& currAbsState, double prob,
    long humanAct, long aiAct, long act, double probAct,
    std::vector<std::pair<AbstractState, double> >& output,
    const AbstractState& prevAbsState) {

	AbstractState nextAbsState;
	long nextRegion, monsterGridNode, monsterRegion;
	double probCurr, probNext, reward, sumProb;

	long monsterX, monsterY, tempX, tempY;
	bool terminal;
	int x = 1, y = 2;

	std::vector<std::pair<AbstractState, double> > tempOutput;
	// std::vector< double > tempRewards;

	nextAbsState = currAbsState;

	monsterX = currAbsState.monsterProperties[0];
	monsterY = currAbsState.monsterProperties[1];
	monsterGridNode = (*gridNodeLabel)[monsterX][monsterY];

	monsterRegion = (*grid)[monsterX][monsterY];

	// blocking only happens when sheep sees both human and ai, otherwise it would move away from the nearest agent and would not hit any
	if (act == unchanged) {
		Utilities::addAbsStateToVector(currAbsState, prob * probAct, output);
	} else {
		tempX = monsterX + RelativeDirX[act];
		tempY = monsterY + RelativeDirY[act];

		// no abstraction -> can use the coords of players to check for collision.
		if (!useAbstract) {
			if (!monsterPassable(tempX, tempY, currAbsState.playerProperties[0][x],
			    currAbsState.playerProperties[0][y],
			    currAbsState.playerProperties[1][x],
			    currAbsState.playerProperties[1][y]))
				Utilities::addAbsStateToVector(currAbsState, prob * probAct, output);
			else {
				nextAbsState.monsterProperties[0] = tempX;
				nextAbsState.monsterProperties[1] = tempY;

				// update visibility
				updateVisibility_NoAbs(nextAbsState);

				if (!isAbstractTerminal(nextAbsState))
					Utilities::addAbsStateToVector(nextAbsState, prob * probAct, output);
				else
					Utilities::addAbsStateToVector((*reverseVAbsStateMap)[0],
					    prob * probAct, output);
			}
			return;
		}

		nextRegion = (*grid)[tempX][tempY];

		// check if an agent can block. This happens when:
		// 1. monsterAgentBlock is set
		// 2. monster sees the agent, otherwise they're too far to block each other
		// 3. monster is moving into the agent's region
		// 4. monster is moving towards the agent.

		int possiblyBlockingAgent = -1;

		for (int j = 0; j < 2; j++) {
			if (monster->blocksAgents() && currAbsState.monsterProperties[2 + j]
			    && (nextRegion == currAbsState.playerProperties[j][0])
			    && towardsTheOther(monsterX, monsterY, act,
			        currAbsState.playerProperties[j][1],
			        currAbsState.playerProperties[j][2]))
				possiblyBlockingAgent = j;
		}

		if (possiblyBlockingAgent >= 0) {
			// if an agent can block
			if (nextRegion == monsterRegion) // monster doesn't move to a new region
				probNext
				    = getMin(visionLimit, getDistance(currAbsState.playerProperties[possiblyBlockingAgent][1], currAbsState.playerProperties[possiblyBlockingAgent][2], monsterX, monsterY))
				        - 1;
			else
				probNext = regionVisible(monsterGridNode, nextRegion) - 1;

			// probNext = numBlockingGrids-1;
			probCurr = 1;
		} else { // no blocking
			probCurr = 0;
			probNext = 1;
		}

		// 1. normalize probCurr, probNext
		sumProb = probCurr + probNext;
		probCurr = (probCurr / sumProb) * prob * probAct;
		probNext = (probNext / sumProb) * prob * probAct;

		// 2. add
		if (probCurr > 0) {
			// no reward
			Utilities::addAbsStateToVector(currAbsState, probCurr, output);
		}

		if (probNext > 0) {
			nextAbsState.monsterProperties[0] = tempX;
			nextAbsState.monsterProperties[1] = tempY;

			if (!isAbstractTerminal(nextAbsState)) {
				// update visibility
				updateVisibility(nextAbsState, tempOutput, humanAct, aiAct, act,
				    monsterX, monsterY, prevAbsState);

				for (unsigned j = 0; j < tempOutput.size(); j++)

					// reward here doesn't need to be multiplied by tempOutput[j].second because this reward will be renormalized later by the state's probability (input[i].second * tempOutput[j].second)
					Utilities::addAbsStateToVector(tempOutput[j].first,
					    probNext * tempOutput[j].second, output);
			} else {
				Utilities::addAbsStateToVector((*reverseVAbsStateMap)[0], probNext,
				    output);
			}
		}

	}// else

}
;

void Maze::absExecuteAgentMoveAct(
    std::vector<std::pair<AbstractState, double> >& input, long& action,
    int agentIndex, std::vector<std::pair<AbstractState, double> >& output) {
	assert( (!input.empty()) );

	if (monster) {
		if (useAbstract)
			absExecuteAgentMoveAct_gotMonster(input, action, agentIndex, output);
		else
			executeAgentMoveAct_gotMonster(input, action, agentIndex, output);
	} else
		absExecuteAgentMoveAct_noMonster(input, action, agentIndex, output);
}
;

void Maze::executeAgentMoveAct_gotMonster(
    std::vector<std::pair<AbstractState, double> >& input, long &action,
    int agentIndex, std::vector<std::pair<AbstractState, double> >& output) {
	output.resize(0);
	// AbstractState currAbsState, nextAbsState;

	int x = 1, y = 2;
	long tempX, tempY;
	bool canMove;

	// 3. For all absState in input
	for (unsigned i = 0; i < input.size(); i++) {

		// this agent does not make any move
		if (action == unchanged) {
			// no need to update anything
			Utilities::addAbsStateToVector(input[i].first, input[i].second, output);
			continue;
		}

		long &currAgentX = input[i].first.playerProperties[agentIndex][x];
		long &currAgentY = input[i].first.playerProperties[agentIndex][y];

		long &otherAgentX = input[i].first.playerProperties[1 - agentIndex][x];
		long &otherAgentY = input[i].first.playerProperties[1 - agentIndex][y];

		// Agent movement ------------------------------
		// tempX, tempY are always >=0 and <= sizeX, sizeY because the boundary of grid is always wall

		tempX = currAgentX + RelativeDirX[action];
		tempY = currAgentY + RelativeDirY[action];

		// new position of player[0] can only be updated if canMove is true
		canMove = isValidMove(agentIndex, tempX, tempY, otherAgentX, otherAgentY,
		    input[i].first);

		if (canMove) {
			currAgentX = tempX;
			currAgentY = tempY;
		} else
			action = unchanged;

		// update visibility based on coords
		updateVisibility_NoAbs(input[i].first);

		Utilities::addAbsStateToVector(input[i].first, input[i].second, output);
	}
}
;

void Maze::updateVisibility_NoAbs(AbstractState& absState) {
	int x = 1, y = 2;
	// update visibility based on coords
	absState.monsterProperties[2] = (checkVisibility(
	    absState.monsterProperties[0], absState.monsterProperties[1],
	    absState.playerProperties[0][x],
	    absState.playerProperties[0][y])) ? 1 : 0;
	absState.monsterProperties[3] = (checkVisibility(
	    absState.monsterProperties[0], absState.monsterProperties[1],
	    absState.playerProperties[1][x],
	    absState.playerProperties[1][y])) ? 1 : 0;
}
;

bool Maze::isValidMove(int agentIndex, long tempX, long tempY,
    long otherAgentX, long otherAgentY, const AbstractState& currState) {

	if (!playerPassable(agentIndex, tempX, tempY)) { // wall
		return false;
	} else {
		// human/agent can't jump on each other
		if (mazeWorld->agentBlock && (tempX == otherAgentX) && (tempY
		    == otherAgentY))
			return false;

		if (monster && monster->blocksAgents() && (tempX
		    == currState.monsterProperties[0]) && (tempY
		    == currState.monsterProperties[1]))
			return false;
	}
	return true;
}
;
/**
 This routine is called before monster making its move, so all abs states can't be terminal yet
 agentIndex == humanIndex -> human
 */
void Maze::absExecuteAgentMoveAct_gotMonster(
    std::vector<std::pair<AbstractState, double> >& input, long &action,
    int agentIndex, std::vector<std::pair<AbstractState, double> >& output) {
	// 2. Declaration
	output.resize(0);
	AbstractState currAbsState, nextAbsState;

	long currRegion, nextRegion, monsterGridNode, monsterRegion;
	double probCurr, probNext, sumProb;
	int agentBlocked;
	long monsterX, monsterY;

	int numVisibleNodes;

	// 3. For all absState in input
	for (unsigned i = 0; i < input.size(); i++) {

		currAbsState = input[i].first;
		nextAbsState = currAbsState;

		currRegion = currAbsState.playerProperties[agentIndex][0];

		monsterX = currAbsState.monsterProperties[0];
		monsterY = currAbsState.monsterProperties[1];
		monsterGridNode = (*gridNodeLabel)[monsterX][monsterY];

		monsterRegion = (*grid)[monsterX][monsterY];

		nextRegion = (*connectivity)[currRegion][action];

		// this agent does not make any move
		if (action == unchanged) {
			// no need to update anything
			Utilities::addAbsStateToVector(input[i].first, input[i].second, output);
		} else {
			// when moving to a non-wall or a region that is not in impassableLoc of agentIndex
			if (playerPassable(agentIndex, nextRegion)) {

				// regionVisible returns number of grid nodes visible to monster
				// this should be retrievable using visibleNearByRegions
				numVisibleNodes = regionVisible(monsterGridNode, currRegion);

				// 1. If this agent's region is not within monster's visible regions, no need to calculate exact coordinates
				if (!numVisibleNodes) {

					// calculate probability to move to nextRegion

					probCurr = (*borderLength)[currRegion][unchanged] - 1;
					probNext = (*borderLength)[currRegion][action]; // this must be 1 if nextRegion > 0, so eventually we may remove borderLength

					// 4. if there is anything in nextRegion and respective blocking condition is set, it may be blocked. movingTowardsMonster == false may indicate that this info is not relevant, which is the case here
					agentBlocked = agentMoveCanBeBlocked(currAbsState, agentIndex,
					    nextRegion, false);

					if (agentBlocked == 1)
						probCurr++;
					else if (agentBlocked == 2) { // surely blocks
						probNext = 0;
						probCurr = 1;
						action = unchanged;
					}

					// update probability
					sumProb = probCurr + probNext;
					probCurr = (probCurr / sumProb) * input[i].second;
					probNext = (probNext / sumProb) * input[i].second;

					// 5. Add currAbsState into output if its probability is greater than 0
					if (probCurr > 0) {
						Utilities::addAbsStateToVector(currAbsState, probCurr, output);
					}

					if (probNext > 0) {
						// Update nextAbsState with visibility and coords of the region's end that is next to currRegion

						nextAbsState.playerProperties[agentIndex][0] = nextRegion;

						// this routine fills in the coords for movingAgent with respect to its new region upon executing action

						updateCoords(
						    nextAbsState,
						    agentIndex,
						    action,
						    (*grid)[nextAbsState.monsterProperties[0]][nextAbsState.monsterProperties[1]]);

						// update visibility with respect to new coordinates
						nextAbsState.monsterProperties[2 + agentIndex] = checkVisibility(
						    monsterX, monsterY,
						    nextAbsState.playerProperties[agentIndex][1],
						    nextAbsState.playerProperties[agentIndex][2]);

						// 6. Add nextAbsState into output
						Utilities::addAbsStateToVector(nextAbsState, probNext, output);
					}
				} else {

					// 2. This agent's region is within monster's visible regions.
					// numVisibleNodes denotes the part of currentRegion that is visible

					// a. if this agent is seen, then use coordinates
					if (currAbsState.monsterProperties[2 + agentIndex]) {

						// based on monster's coords and movingAgent's anchor coords and his action, check whether this is a move towards monster or away from monster
						if (towardsTheOther(currAbsState.playerProperties[agentIndex][1],
						    currAbsState.playerProperties[agentIndex][2], action, monsterX,
						    monsterY)) {

							if (currAbsState.playerProperties[agentIndex][0] == monsterRegion) {
								// same region with monster, there's no next region to move to

								// add currAbsState in output

								probCurr = input[i].second;
								Utilities::addAbsStateToVector(currAbsState, probCurr, output);
							} else {
								// there is a next region to move to

								probCurr = numVisibleNodes - 1;
								probNext = 1;

								// agentCanBeBlocked: 0 - not blocked, 1 - could be blocked, 2 - blocked
								agentBlocked = agentMoveCanBeBlocked(currAbsState, agentIndex,
								    nextRegion, true);

								if (agentBlocked == 1)
									probCurr++;
								else if (agentBlocked == 2) { // surely blocks
									probNext = 0;
									probCurr = 1;
									action = unchanged;
								}

								// update probability
								sumProb = probCurr + probNext;
								probCurr = (probCurr / sumProb) * input[i].second;
								probNext = (probNext / sumProb) * input[i].second;

								// 5. Add currAbsState into output if its probability is greater than 0
								if (probCurr > 0) {
									Utilities::addAbsStateToVector(currAbsState, probCurr, output);
								}

								if (probNext > 0) {
									nextAbsState.playerProperties[agentIndex][0] = nextRegion;

									updateCoords(
									    nextAbsState,
									    agentIndex,
									    action,
									    (*grid)[nextAbsState.monsterProperties[0]][nextAbsState.monsterProperties[1]]);

									// update visibility with respect to new coordinates
									nextAbsState.monsterProperties[2 + agentIndex] = 1;

									// 6. Add nextAbsState into output
									Utilities::addAbsStateToVector(nextAbsState, probNext, output);
								}
							}

						}
						// Moving away from monster
						else {

							// fullySeen is true if the region part where movingAgent is in now is fully seen, false otherwise
							bool fullySeen = true;
							if (numVisibleNodes < (*borderLength)[currRegion][unchanged]) {
								if (monsterRegion != currRegion)
									fullySeen = false;
								else
									fullySeen = checkVisibility(monsterX, monsterY,
									    nextAbsState.playerProperties[agentIndex][1],
									    nextAbsState.playerProperties[agentIndex][2]);
							}

							if (!fullySeen) {
								if (monsterRegion != currRegion)
									probCurr = numVisibleNodes - 1;
								else
									probCurr = visionLimit - 1;

								probNext = 1;

								agentBlocked = agentMoveCanBeBlocked(currAbsState, agentIndex,
								    nextRegion, false);

								if (agentBlocked == 1)
									probCurr++;
								else if (agentBlocked == 2) { // surely blocks
									probNext = 0;
									probCurr = 1;
									action = unchanged;
								}

								// update probability
								sumProb = probCurr + probNext;
								probCurr = (probCurr / sumProb) * input[i].second;
								probNext = (probNext / sumProb) * input[i].second;

								// 5. Add currAbsState into output if its probability is greater than 0
								if (probCurr > 0) {
									Utilities::addAbsStateToVector(currAbsState, probCurr, output);
								}

								if (probNext > 0) {
									nextAbsState.monsterProperties[2 + agentIndex] = 0;

									// 6. Add nextAbsState into output
									Utilities::addAbsStateToVector(nextAbsState, probNext, output);
								}
							}
							// current region is fully seen
							else {

								probCurr = numVisibleNodes - 1;
								probNext = 1;

								agentBlocked = agentMoveCanBeBlocked(currAbsState, agentIndex,
								    nextRegion, false);

								if (agentBlocked == 1)
									probCurr++;
								else if (agentBlocked == 2) { // surely blocks
									probNext = 0;
									probCurr = 1;
									action = unchanged;
								}

								// update probability
								sumProb = probCurr + probNext;
								probCurr = (probCurr / sumProb) * input[i].second;
								probNext = (probNext / sumProb) * input[i].second;

								// 5. Add currAbsState into output if its probability is greater than 0
								if (probCurr > 0) {
									Utilities::addAbsStateToVector(currAbsState, probCurr, output);
								}

								if (probNext > 0) {
									nextAbsState.playerProperties[agentIndex][0] = nextRegion;

									// this routine fills in the coords for movingAgent with respect to its new region upon executing action
									updateCoords(
									    nextAbsState,
									    agentIndex,
									    action,
									    (*grid)[nextAbsState.monsterProperties[0]][nextAbsState.monsterProperties[1]]);

									// update visibility with respect to new coordinates
									nextAbsState.monsterProperties[2 + agentIndex]
									    = checkVisibility(monsterX, monsterY,
									        nextAbsState.playerProperties[agentIndex][1],
									        nextAbsState.playerProperties[agentIndex][2]);

									// 6. Add nextAbsState into output
									Utilities::addAbsStateToVector(nextAbsState, probNext, output);
								}
							}

						}
					}

					// b. if this agent is not seen, there is a probability it will be seen upon executing action.
					else {
						// in this case, the coords act as an anchor point of the region

						// if action is towards monster, the agent would not leave current region. Only visibility may change
						if (towardsTheOther(currAbsState.playerProperties[agentIndex][1],
						    currAbsState.playerProperties[agentIndex][2], action, monsterX,
						    monsterY)) {

							probCurr = (*borderLength)[currRegion][unchanged]
							    - numVisibleNodes - 1;
							probNext = 1;

							agentBlocked = agentMoveCanBeBlocked(currAbsState, agentIndex,
							    nextRegion, true);

							if (agentBlocked == 1)
								probCurr++;
							else if (agentBlocked == 2) { // surely blocks
								probNext = 0;
								probCurr = 1;
								action = unchanged;
							}

							// update probability
							sumProb = probCurr + probNext;
							probCurr = (probCurr / sumProb) * input[i].second;
							probNext = (probNext / sumProb) * input[i].second;

							// 5. Add currAbsState into output if its probability is greater than 0
							if (probCurr > 0) {
								Utilities::addAbsStateToVector(currAbsState, probCurr, output);
							}

							if (probNext > 0) {
								nextAbsState.monsterProperties[2 + agentIndex] = 1;

								// 6. Add nextAbsState into output
								Utilities::addAbsStateToVector(nextAbsState, probNext, output);
							}
						}
						// moving away from monster
						else {

							probCurr = (*borderLength)[currRegion][unchanged]
							    - numVisibleNodes - 1;
							probNext = 1;

							agentBlocked = agentMoveCanBeBlocked(currAbsState, agentIndex,
							    nextRegion, false);

							if (agentBlocked == 1)
								probCurr++;
							else if (agentBlocked == 2) { // surely blocks
								probNext = 0;
								probCurr = 1;
								action = unchanged;
							}

							// update probability
							sumProb = probCurr + probNext;
							probCurr = (probCurr / sumProb) * input[i].second;
							probNext = (probNext / sumProb) * input[i].second;

							// 5. Add currAbsState into output if its probability is greater than 0
							if (probCurr > 0) {
								Utilities::addAbsStateToVector(currAbsState, probCurr, output);
							}

							if (probNext > 0) {
								nextAbsState.playerProperties[agentIndex][0] = nextRegion;
								// this routine fills in the coords for movingAgent with respect to its new region upon executing action
								updateCoords(
								    nextAbsState,
								    agentIndex,
								    action,
								    (*grid)[nextAbsState.monsterProperties[0]][nextAbsState.monsterProperties[1]]);

								// nextAbsState.monsterProperties[2+agentIndex] = 0;

								// 6. Add nextAbsState into output
								Utilities::addAbsStateToVector(nextAbsState, probNext, output);
							}
						}
					}
				} // currRegion visible
			} // nextRegion >= 0
			else {
				// hit the wall or impassable block
				action = unchanged;
				probCurr = input[i].second;
				Utilities::addAbsStateToVector(currAbsState, probCurr, output);
			}// else nextRegion >= 0
		} // else action == unchanged
	} // for

}
;

/**
 The rewards in rewards are already multiplied by the probabilities of the states.
 */
void Maze::absGetRewards(
    std::vector<std::pair<AbstractState, double> >& states,
    std::vector<double>& rewards) {
	rewards.resize(0);
	double reward, tempR;
	bool seesHuman;
	for (unsigned i = 0; i < states.size(); i++) {
		reward = 0;

		reward += player[0]->absGetReward(states[i].first);
		reward += player[1]->absGetReward(states[i].first);
		if (monster)
			reward += monster->absGetReward(states[i].first);
		if (specialLocation)
			reward += specialLocation->absGetReward(states[i].first);

		// Bias player[0]: If the terminal state has player[0] in monster's visionLimit, plus HumanRewardBias
		tempR = absGetReward(states[i].first);

		if ((tempR > 0) && monster && states[i].first.monsterProperties[2])
			tempR += HumanRewardBias;

		reward += tempR;

		reward *= states[i].second;

		rewards.push_back(reward);
	}
}
;

// TODO --------------- Visibility related
/**************** Visibility related ************************************/

void Maze::updateVisibility(AbstractState& absState,
    std::vector<std::pair<AbstractState, double> >& output, long humanAct,
    long aiAct, long monsterAct, long priorMonsterX, long priorMonsterY,
    const AbstractState& prevAbsState) {
	output.resize(0);
	if (monsterAct == unchanged) {
		output.push_back(std::pair<AbstractState, double>(absState, 1.0));
		return;
	}

	std::vector<std::pair<AbstractState, double> > temp;
	temp.resize(0);

	updateCoordsByMonsterMove(absState, monsterAct,
	    (*grid)[priorMonsterX][priorMonsterY]);
	standardizeCoords(absState);

	output.push_back(std::pair<AbstractState, double>(absState, 1.0));

	// 1. Update visibility for player[0].
	updateVisibilityCoordsOneAgent(output, temp, monsterAct, priorMonsterX,
	    priorMonsterY, humanAct, humanIndex, prevAbsState);

	// 2. Update visibility for ai.
	updateVisibilityCoordsOneAgent(temp, output, monsterAct, priorMonsterX,
	    priorMonsterY, aiAct, aiIndex, prevAbsState);

}
;

/**
 Update visibility and coords of agentIndex
 */
void Maze::updateVisibilityCoordsOneAgent(
    std::vector<std::pair<AbstractState, double> >& input,
    std::vector<std::pair<AbstractState, double> >& output, long monsterAct,
    long priorMonsterX, long priorMonsterY, long agentAct, int agentIndex,
    const AbstractState& prevAbsState) {
	output.resize(0);

	AbstractState currAbsState, nextAbsState;
	long priorNumVisibleNodes, numVisibleNodes, monsterX, monsterY, agentRegion,
	    priorMonsterGridNode, monsterGridNode, monsterRegion;

	double probCurr, probNext, sumProb;

	for (unsigned i = 0; i < input.size(); i++) {

		currAbsState = input[i].first;
		nextAbsState = currAbsState;

		agentRegion = currAbsState.playerProperties[agentIndex][0];
		monsterX = currAbsState.monsterProperties[0];
		monsterY = currAbsState.monsterProperties[1];
		monsterGridNode = (*gridNodeLabel)[monsterX][monsterY];

		monsterRegion = (*grid)[monsterX][monsterY];
		priorMonsterGridNode = (*gridNodeLabel)[priorMonsterX][priorMonsterY];

		// Intuition: if monster and agent moves in the same direction on a line, their visibility of each other stays the same

		// this is called when: 1. monsterAct is not unchanged
		// which means monster can move. If agentAct == monsterAct, then it's definitely
		// not unchanged.
		if (monsterAct == agentAct) {

			// this means this agentAct actually moves the agent too
			if ((*connectivity)[prevAbsState.playerProperties[agentIndex][0]][agentAct]
			    >= 0) {

				// move horizontally
				if ((monsterAct == east) || (monsterAct == west)) {
					if (priorMonsterY == currAbsState.playerProperties[agentIndex][2]) {
						currAbsState.monsterProperties[2 + agentIndex]
						    = prevAbsState.monsterProperties[2 + agentIndex];

						Utilities::addAbsStateToVector(currAbsState, input[i].second,
						    output);
						continue;
					}

				}
				// move vertically
				else {
					if (priorMonsterX == currAbsState.playerProperties[agentIndex][1]) {
						currAbsState.monsterProperties[2 + agentIndex]
						    = prevAbsState.monsterProperties[2 + agentIndex];

						Utilities::addAbsStateToVector(currAbsState, input[i].second,
						    output);
						continue;
					}
				}

			}
		}

		// Back to the algo

		priorNumVisibleNodes = regionVisible(priorMonsterGridNode, agentRegion);

		numVisibleNodes = regionVisible(monsterGridNode, agentRegion);

		// 1. Agent region was seen before monster making move
		if (priorNumVisibleNodes) {

			// 1a. Agent region seen after monster move
			if (numVisibleNodes) {

				// Visible area increases
				if ((numVisibleNodes > priorNumVisibleNodes) || towardsTheOther(
				    priorMonsterX, priorMonsterY, monsterAct,
				    currAbsState.playerProperties[agentIndex][1],
				    currAbsState.playerProperties[agentIndex][2])) {

					// if agent has already been seen
					if (currAbsState.monsterProperties[2 + agentIndex]) {

						//        This should have aready been done in updateCoordsByMonsterMove

						// agent is definitely still seen
						// no need to update visibility
						Utilities::addAbsStateToVector(currAbsState, input[i].second,
						    output);
					}
					// agent has not been seen yet, there's a probability agent would be seen. If it's seen, update coords
					else {

						// seen = false
						if (monsterRegion == currAbsState.playerProperties[agentIndex][0])
							probCurr
							    = getMin(0, getDistance(currAbsState.playerProperties[agentIndex][1], currAbsState.playerProperties[agentIndex][2], monsterX, monsterY) - visionLimit);
						else
							probCurr = (*borderLength)[agentRegion][unchanged]
							    - numVisibleNodes;
						// seen = true
						probNext = 1;

						// update probability
						sumProb = probCurr + probNext;
						probCurr = (probCurr / sumProb) * input[i].second;
						probNext = (probNext / sumProb) * input[i].second;

						// 5. Add currAbsState into output if its probability is greater than 0
						if (probCurr > 0) {
							Utilities::addAbsStateToVector(currAbsState, probCurr, output);
						}

						if (probNext > 0) {
							nextAbsState.monsterProperties[2 + agentIndex] = 1;

							// 6. Add nextAbsState into output
							Utilities::addAbsStateToVector(nextAbsState, probNext, output);
						}
					}

				}
				// visible area decreases or monster is moving away from agent.
				else {
					// if agent has not been seen
					if (!currAbsState.monsterProperties[2 + agentIndex]) {

						// agent is definitely not seen
						// no need to update coords and visibility
						Utilities::addAbsStateToVector(currAbsState, input[i].second,
						    output);
					}
					// agent has been seen, there's a probability agent would not be seen. If it's not seen, update coords
					else {

						probCurr = numVisibleNodes;
						probNext = priorNumVisibleNodes - numVisibleNodes;

						// update probability
						sumProb = probCurr + probNext;
						probCurr = (probCurr / sumProb) * input[i].second;
						probNext = (probNext / sumProb) * input[i].second;

						// 5. Add currAbsState into output if its probability is greater than 0
						if (probCurr > 0) {
							Utilities::addAbsStateToVector(currAbsState, probCurr, output);
						}

						if (probNext > 0) {
							nextAbsState.monsterProperties[2 + agentIndex] = 0;

							// 6. Add nextAbsState into output
							Utilities::addAbsStateToVector(nextAbsState, probNext, output);
						}
					}
				} // else visible area decreases
				//  else{

				//  } // else visible area is the same
			} // if numVisibleNodes

			// player[0] region not seen after monster move
			else {
				nextAbsState.monsterProperties[2 + agentIndex] = 0;
				Utilities::addAbsStateToVector(nextAbsState, input[i].second, output);
			} // else numVisibleNodes

		} // if priorNumVisibleNodes

		// 2. Human region was not seen before monster making move
		else {

			// 2a. Human region was seen after making the move
			if (numVisibleNodes) {

				probCurr = numVisibleNodes; // probability seen
				probNext = (*borderLength)[agentRegion][unchanged] - numVisibleNodes; // probability not seen

				// update probability
				sumProb = probCurr + probNext;
				probCurr = (probCurr / sumProb) * input[i].second;
				probNext = (probNext / sumProb) * input[i].second;

				// 5. Add currAbsState into output if its probability is greater than 0
				if (probCurr > 0) {
					currAbsState.monsterProperties[2 + agentIndex] = 1;
					Utilities::addAbsStateToVector(currAbsState, probCurr, output);
				}

				if (probNext > 0) {
					nextAbsState.monsterProperties[2 + agentIndex] = 0;
					// 6. Add nextAbsState into output
					Utilities::addAbsStateToVector(nextAbsState, probNext, output);
				}
			}

			// 2b. Human region was not seen after making the move
			else {
				nextAbsState.monsterProperties[2 + agentIndex] = 0;
				Utilities::addAbsStateToVector(nextAbsState, input[i].second, output);
			}

		}// else priorNumVisibleNodes


#ifdef DEBUGMODEL
		// if ((currAbsState.regions[0]==1)&&(currAbsState.regions[1]==7)&&(currAbsState.coords[4]==1)&&(currAbsState.coords[5]==3)&&(currAbsState.coords[2]==1)&&(currAbsState.coords[3]==2)&&(!currAbsState.seesHumanAi[1])){
		//       currAbsState.regions[0]=1;
		//     }
#endif       
	}
}
;
// TODO --------------- Abstract Move related
/************* Abstract Move related routines *****************************/

bool Maze::playerPassable(int agentIndex, long regionIndex, int monsterX, int monsterY) {
	if (regionIndex < 0)
		return false;

	return (!player[agentIndex]->isImpassable((*regionRepPoint)[regionIndex].first,
		    (*regionRepPoint)[regionIndex].second, (*gridNodeLabel)));
}
;

bool Maze::playerPassable(int agentIndex, long x, long y) {
	if ((*grid)[x][y] < 0)
		return false;

	return (!player[agentIndex]->isImpassable(x, y, (*gridNodeLabel)));
}
;

int Maze::agentMoveCanBeBlocked(const AbstractState& currAbsState,
    int agentIndex, long nextRegion, bool movingTowardsMonster) {
	int otherAgent = 1 - agentIndex;

	long monsterX = currAbsState.monsterProperties[0];
	long monsterY = currAbsState.monsterProperties[1];

	// 2. if monsterAgentBlock and this agent is seen by the monster and this agent is moving towards monster, it can be blocked
	if (monster->blocksAgents()) {
		long monsterRegion = (*grid)[monsterX][monsterY];
		if (currAbsState.monsterProperties[2 + agentIndex] && movingTowardsMonster)
			if (monsterRegion == currAbsState.playerProperties[agentIndex][0])
				return 2;

			// if monster's coordinates blocks the movement into nextRegion
			else if (monsterRegion == nextRegion) {
				// nextRegion is of size 1 and monster is in it -> surely block
				if ((*borderLength)[nextRegion][unchanged] == 1)
					return 2;
				// nextRegion is of size > 1, hence an edge, which means currRegion is junction
				else if (getDistance(monsterX, monsterY, (*regionRepPoint)[currAbsState.playerProperties[agentIndex][0]].first, (*regionRepPoint)[currAbsState.playerProperties[agentIndex][0]].second)
				    == 1)
					return 2;
			}
	}

	// 1. if agentBlock and the other agent is in the same region or in the next region, this agent can be blocked
	if (mazeWorld->agentBlock) {

		if (nextRegion == currAbsState.playerProperties[otherAgent][0]) {
			// nextRegion is of size 1 and otherAgent is there, it surely blocks the way
			if ((*borderLength)[nextRegion][unchanged] == 1)
				return 2;
			// size > 1, it may block
			else
				return 1;
		}

		// in the same region, may block
		else if (currAbsState.playerProperties[agentIndex][0]
		    == currAbsState.playerProperties[otherAgent][0])
			return 1;
	}

	return 0;

}
;

void Maze::updateCoords(AbstractState& nextAbsState, int movingAgent,
    long action, long monsterRegion) {
	long destRegion = nextAbsState.playerProperties[movingAgent][0];

	nextAbsState.playerProperties[movingAgent][1]
	    = (*regionRepPoint)[destRegion].first;
	nextAbsState.playerProperties[movingAgent][2]
	    = (*regionRepPoint)[destRegion].second;

	if (monsterRegion == destRegion) {

		if (((*rType)[destRegion] == vertical) && (action == south))
			// coords of the north tip
			nextAbsState.playerProperties[movingAgent][2]
			    = (*regionRepPoint)[destRegion].second
			        + (*borderLength)[destRegion][unchanged] - 1;
		else if (((*rType)[destRegion] == horizon) && (action == west))
			// coords of the east tip
			nextAbsState.playerProperties[movingAgent][1]
			    = (*regionRepPoint)[destRegion].first
			        + (*borderLength)[destRegion][unchanged] - 1;

	}
}
;

bool Maze::towardsTheOther(long p1X, long p1Y, long action, long p2X, long p2Y) {
	if (p1X == p2X) {

		// |p1   |
		// | ... |
		// |p2   |
		if (p1Y > p2Y) {
			if (action == south)
				return true;
		}

		// |p2   |
		// | ... |
		// |p1   |
		else if (p1Y < p2Y) {
			if (action == north)
				return true;
		}
	}

	else if (p1Y == p2Y) {

		// _______________
		// p2 ... p1
		// ---------------
		if (p1X > p2X) {
			if (action == west)
				return true;
		}

		// _______________
		// p1 ... p2
		// ---------------
		else if (p1X < p2X) {
			if (action == east)
				return true;
		}
	}

	return false;
}
;

void Maze::updateCoordsByMonsterMove(AbstractState& absState, long monsterAct,
    long priorMonsterRegion) {

	long monsterX = absState.monsterProperties[0];
	long monsterY = absState.monsterProperties[1];
	long monsterRegion = (*grid)[monsterX][monsterY];

	// this is only needed when monster jumps region
	if (monsterRegion != priorMonsterRegion) {
		for (long agentIndex = 0; agentIndex < 2; agentIndex++) {
			if (monsterRegion == absState.playerProperties[agentIndex][0]) {
				absState.playerProperties[agentIndex][1]
				    = (*regionRepPoint)[monsterRegion].first;
				absState.playerProperties[agentIndex][2]
				    = (*regionRepPoint)[monsterRegion].second;

				if (monsterAct == north)
					absState.playerProperties[agentIndex][2]
					    += (*borderLength)[monsterRegion][unchanged] - 1;

				if (monsterAct == east)
					absState.playerProperties[agentIndex][1]
					    += (*borderLength)[monsterRegion][unchanged] - 1;
			} else if (priorMonsterRegion == absState.playerProperties[agentIndex][0]) {
				// set to the default coords when the monster leaves the region
				absState.playerProperties[agentIndex][1]
				    = (*regionRepPoint)[priorMonsterRegion].first;
				absState.playerProperties[agentIndex][2]
				    = (*regionRepPoint)[priorMonsterRegion].second;
			}
		}
	}

}
;

void Maze::standardizeCoords(AbstractState& absState) {
	long monsterX = absState.monsterProperties[0];
	long monsterY = absState.monsterProperties[1];
	long monsterRegion = (*grid)[monsterX][monsterY];

	// if monster and human are not in the same region, reset human's coords to representative coords of his region, i.e. discard relative position info

	for (long agentIndex = 0; agentIndex < 2; agentIndex++) {
		if (monsterRegion != absState.playerProperties[agentIndex][0]) {
			absState.playerProperties[agentIndex][1]
			    = (*regionRepPoint)[absState.playerProperties[agentIndex][0]].first;
			absState.playerProperties[agentIndex][2]
			    = (*regionRepPoint)[absState.playerProperties[agentIndex][0]].second;
		} else {

			// if monster's coords is at this end, update the agent's coords to the other end
			if (((*rType)[monsterRegion] == vertical) && (monsterY
			    == absState.playerProperties[agentIndex][2])) {
				if (absState.playerProperties[agentIndex][2]
				    == (*regionRepPoint)[monsterRegion].second)
					absState.playerProperties[agentIndex][2]
					    += (*borderLength)[monsterRegion][unchanged] - 1;
				else
					absState.playerProperties[agentIndex][2]
					    = (*regionRepPoint)[monsterRegion].second;
			}

			if (((*rType)[monsterRegion] == horizon) && (monsterX
			    == absState.playerProperties[agentIndex][1]))
				if (absState.playerProperties[agentIndex][1]
				    == (*regionRepPoint)[monsterRegion].first)
					absState.playerProperties[agentIndex][1]
					    += (*borderLength)[monsterRegion][unchanged] - 1;
				else
					absState.playerProperties[agentIndex][1]
					    = (*regionRepPoint)[monsterRegion].first;
		}
	}
}
;
// TODO --------------- Abstract state Utility functions
/************* Abstract state Utility functions **************/

bool Maze::isAbstractTerminal(const AbstractState& absState) {
	if (absState.playerProperties[humanIndex][0] == TermState)
		return true;

	if (!absState.monsterProperties.empty()) {
		if (absState.monsterProperties[0] == TermState)
			return true;
		else
			return false;
	} else {
		assert ( (absState.specialLocationProperties.size() > 0) );
		if (absState.specialLocationProperties[0] == TermState)
			return true;
	}

	return false;
}
;

long Maze::getLongFromAbsState(AbstractState& absState) {
	// 1. If this is a terminal abs state, return virtualTerminalState
	if (isAbstractTerminal(absState))
		return longTermState; // 0
	// Else retrieve from abstract state map its index
	else {

		// 1. standardize absState
		if (useAbstract)
			standardizeCoords(absState);

		// 2. search in abs state map
		//long value = vAbsStateMap->find(absState)->second;
		std::map<AbstractState, long>::iterator currStates = vAbsStateMap->find(
		    absState);
		if (currStates == vAbsStateMap->end()) {
			std::cout << "wrong absState - " << worldTypeStr << ":" << std::endl;
			Utilities::printAbsState(absState);
		}
		// else if (currStates->second == 314){
		//       std::cout << "absState 314" << std::endl;
		//     }
		return currStates->second;
	}
}
;

// TODO --------------- Abstract State Utility routines

// TODO --------------- General Utility routines
/********************* General Utility routines ************************/

/**
 @return the number of grid squares in region that can be seen from monsterGridNode
 */
int Maze::regionVisible(long monsterGridNode, long region) {
	for (unsigned i = 0; i < (*visibleNearByRegions)[monsterGridNode].size(); i++)
		if ((*visibleNearByRegions)[monsterGridNode][i].first == region)
			return (*visibleNearByRegions)[monsterGridNode][i].second;

	return 0;
}
;

bool Maze::checkVisibility(long p1X, long p1Y, long p2X, long p2Y) {

	long tempStartCoord, tempEndCoord;

	// to generalize, this should become
	// if ((distance(currMonsterX, currMonsterY, posHX, posHY) <= visionLimit)&&lineOfSightClear(currMonsterX, currMonsterY, posHX, posHY))

	if ((p1X == p2X) && (fabs(p1Y - p2Y) <= visionLimit)) {
		// human is within visionLimit on the Y direction
		tempStartCoord = (p1Y < p2Y) ? p1Y : p2Y;
		tempEndCoord = (p1Y >= p2Y) ? p1Y : p2Y;

		for (long i = tempStartCoord + 1; i < tempEndCoord; i++)
			if ((*grid)[p1X][i] < 0) // obstacle
				return false;
	} else if ((p1Y == p2Y) && (fabs(p1X - p2X) <= visionLimit)) {
		tempStartCoord = (p1X < p2X) ? p1X : p2X;
		tempEndCoord = (p1X >= p2X) ? p1X : p2X;

		for (long i = tempStartCoord + 1; i < tempEndCoord; i++)
			if ((*grid)[i][p1Y] < 0) // obstacle
				return false;
	} else
		return false;

	return true;
}
;

// TODO --------------- Raw state related
/******************** Raw state related ************************/

void Maze::getCurrState(vector<long>& propertiesState, long newX, long newY) {

	propertiesState.resize(0);

	// 1. Monster if any. Monster belongs to individual mazes (not shared among worlds of the same kind)
	if (monster)
		monster->getCurrState(propertiesState, newX, newY);

	// 2. specialLocation if any.
	if (specialLocation)
		specialLocation->getCurrState(propertiesState);

}
;

// SECTION: Raw and abstract translation
long Maze::getLongAbsStateFromState_NoAbs(const State& state, long worldNum) {

	// 1. No abstraction, therefore we just need to fill in the critters' coordinates
	// and properties.
	AbstractState absState;
	int x = 1, y = 2;

	absState.playerProperties[humanIndex].resize(y + 1, 0);
	absState.playerProperties[humanIndex][x]
	    = state.playerProperties[humanIndex][0];
	absState.playerProperties[humanIndex][y]
	    = state.playerProperties[humanIndex][1];

	player[0]->fillProperties(absState.playerProperties[humanIndex], 2,
	    state.playerProperties[humanIndex].size() - 1,
	    state.playerProperties[humanIndex], monster);

	absState.playerProperties[aiIndex].resize(y + 1, 0);
	absState.playerProperties[aiIndex][x] = state.playerProperties[aiIndex][0];
	absState.playerProperties[aiIndex][y] = state.playerProperties[aiIndex][1];

	player[0]->fillProperties(absState.playerProperties[aiIndex], 2,
	    state.playerProperties[aiIndex].size() - 1,
	    state.playerProperties[aiIndex], monster);

	long startIndexSL = 0;
	if (monster) {
		absState.monsterProperties.resize(4, 0);
		absState.monsterProperties[0] = state.mazeProperties[worldNum][0];
		absState.monsterProperties[1] = state.mazeProperties[worldNum][1];

		updateVisibility_NoAbs(absState);

		// Monster's properties
		monster->fillProperties(absState.monsterProperties, 2,
		    2 + monster->getNumProperties() - 1, state.mazeProperties[worldNum]);
		startIndexSL = monster->getNumProperties();
	}

	// TO DO: test specialLocation. I've been ignoring it.
	absState.specialLocationProperties.resize(0);
	// c. SpecialLocation
	if (specialLocation) {
		for (unsigned i = startIndexSL; i < state.mazeProperties[worldNum].size(); i++)
			absState.specialLocationProperties.push_back(
			    state.mazeProperties[worldNum][i]);
	}

	// 2. Get long from AbstractState using the dual state map
	return getLongFromAbsState(absState);

}
;

long Maze::getLongAbsStateFromState(const State& state, long worldNum) {

	// 1. Form AbstractState from state by calculating regions, visibility etc.
	AbstractState absState;
	long humanX, humanY, aiX, aiY, humanRegion, aiRegion;
	unsigned i;

	humanX = state.playerProperties[humanIndex][0];
	humanY = state.playerProperties[humanIndex][1];
	aiX = state.playerProperties[aiIndex][0];
	aiY = state.playerProperties[aiIndex][1];

	// a. Players
	// In absState, there is one additional attribute for playerProperties: regionId
	// Human's coords
	absState.playerProperties[humanIndex].resize(3);

	absState.playerProperties[humanIndex][0] = humanRegion
	    = (*grid)[humanX][humanY];
	absState.playerProperties[humanIndex][1]
	    = (*regionRepPoint)[humanRegion].first;
	absState.playerProperties[humanIndex][2]
	    = (*regionRepPoint)[humanRegion].second;

	// Human's properties. 2 is the offset index in the \a state's property list, to exclude x and y.
	player[0]->fillProperties(absState.playerProperties[humanIndex], 2,
	    state.playerProperties[humanIndex].size() - 1,
	    state.playerProperties[humanIndex], monster);

	// Ai's coords
	absState.playerProperties[aiIndex].resize(3);

	absState.playerProperties[aiIndex][0] = aiRegion = (*grid)[aiX][aiY];
	absState.playerProperties[aiIndex][1] = (*regionRepPoint)[aiRegion].first;
	absState.playerProperties[aiIndex][2] = (*regionRepPoint)[aiRegion].second;
	// Ai's properties. 2 is the offset index in the \a state's property list, to exclude x and y.
	player[1]->fillProperties(absState.playerProperties[aiIndex], 2,
	    state.playerProperties[aiIndex].size() - 1,
	    state.playerProperties[aiIndex], monster);

	long startIndexSL = 0;

	// b. Monsters
	if (monster) {
		long sX, sY, monsterRegion;
		sX = state.mazeProperties[worldNum][0];
		sY = state.mazeProperties[worldNum][1];

		absState.monsterProperties.resize(4);
		absState.monsterProperties[0] = sX;
		absState.monsterProperties[1] = sY;

		monsterRegion = (*grid)[sX][sY];

		// in the same region with human
		if (monsterRegion == humanRegion) {
			if (((*rType)[monsterRegion] == vertical) && (sY <= humanY)) {
				absState.playerProperties[humanIndex][2]
				    = (*regionRepPoint)[monsterRegion].second
				        + (*borderLength)[monsterRegion][unchanged] - 1;
			} else if (((*rType)[monsterRegion] == horizon) && (sX <= humanX)) {
				absState.playerProperties[humanIndex][1]
				    = (*regionRepPoint)[monsterRegion].first
				        + (*borderLength)[monsterRegion][unchanged] - 1;
			}
		}

		// in the same region with ai
		if (monsterRegion == aiRegion) {
			if (((*rType)[monsterRegion] == vertical) && (sY <= aiY)) {
				absState.playerProperties[aiIndex][2]
				    = (*regionRepPoint)[monsterRegion].second
				        + (*borderLength)[monsterRegion][unchanged] - 1;
			} else if (((*rType)[monsterRegion] == horizon) && (sX <= aiX)) {
				absState.playerProperties[aiIndex][1]
				    = (*regionRepPoint)[monsterRegion].first
				        + (*borderLength)[monsterRegion][unchanged] - 1;
			}
		}

		// visibility
		absState.monsterProperties[2 + humanIndex] = checkVisibility(sX, sY,
		    humanX, humanY);
		absState.monsterProperties[2 + aiIndex] = checkVisibility(sX, sY, aiX, aiY);

		// Monster's properties
		monster->fillProperties(absState.monsterProperties, 2,
		    2 + monster->getNumProperties() - 1, state.mazeProperties[worldNum]);

		startIndexSL = 2 + monster->getNumProperties();
	}

	// TO DO: test specialLocation. I've been ignoring it.
	absState.specialLocationProperties.resize(0);
	// c. SpecialLocation
	if (specialLocation) {
		for (i = startIndexSL; i < state.mazeProperties[worldNum].size(); i++)
			absState.specialLocationProperties.push_back(
			    state.mazeProperties[worldNum][i]);
	}

	// 2. Get long from AbstractState using the dual state map
	return getLongFromAbsState(absState);

}
;

/**
 @return long absState if useAbstract, long virtual state otherwise.
 */
long Maze::realToVirtual(const State& currState, const long worldNum) {

	if (isTermState(currState, worldNum))
		return longTermState; // 0 is long terminal state
	else {
		// return abstract state long if useAbstract

		if (useAbstract)
			return getLongAbsStateFromState(currState, worldNum);
		else
			return getLongAbsStateFromState_NoAbs(currState, worldNum);
	}
}
;

/**
 A maze's state is terminal if either the full state was set to terminal or itself was set to terminal.
 */
bool Maze::isTermState(const State& state, long worldNum) {
	if ((state.playerProperties[0][0] == TermState)
	    || (state.mazeProperties[worldNum][0] == TermState))
		return true;
	return false;
}
;

bool Maze::isLocalTermState(const State& state, long worldNum) {
	if (state.mazeProperties[worldNum][0] == TermState)
		return true;
	return false;
}
;

// TODO --------------- Distance functions
/************** Distance function ***************************/
double Maze::distance_farthestFromBoth(long monsterNode, long nearestAgentNode,
		unsigned nearestAgentIndex, long fartherAgentNode) {
	if (fartherAgentNode >= 0)
		return 8 * (*monster->shortestPath)[monsterNode][nearestAgentNode]
	    + (*monster->shortestPath)[monsterNode][fartherAgentNode];
	else return distance_farthestFromPlayer(monsterNode, nearestAgentNode);
}
;

double Maze::distance_farthestFromPlayer(long monsterNode, long agentNode) {
		return (*monster->shortestPath)[monsterNode][agentNode];
}
;

double Maze::distance_towardsNode(long monsterNode, long destNode) {
	return (*monster->shortestPath)[monsterNode][destNode];
}
;

/***************** Cleanup *********************************************/
void Maze::deleteCommonPointers() {

	if (visibleNearByRegions) {
		delete visibleNearByRegions;
	}

	if (vAbsStateMap) {
		delete vAbsStateMap;
	}
	if (reverseVAbsStateMap) {
		delete reverseVAbsStateMap;
	}

	if (valueFn) {
		delete valueFn;
	}
	if (collabQFn) {
		delete collabQFn;
	}
}
;

Maze::~Maze() {

	mazeWorld = 0;
	player[0] = 0;
	player[1] = 0;

	grid = 0;
	gridNodeLabel = 0;
	connectivity = 0;
	borderLength = 0;
	regionRepPoint = 0;
	rType = 0;
	visibleNearByRegions = 0;

	vAbsStateMap = 0;
	reverseVAbsStateMap = 0;
	valueFn = 0;
	collabQFn = 0;

	if (monster) {
		delete monster;
		monster = 0;
	}

	if (specialLocation) {
		delete specialLocation;
		specialLocation = 0;
	}

}
;
