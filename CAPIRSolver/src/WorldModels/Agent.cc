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



#include "Agent.h"

bool Agent::isMeaningfulAct(const char c) {
	long act = actionFromChar(c);

	if (act == unchanged)
		return false;

	return true;
}
;

bool Agent::isImpassable(long x, long y, vector<vector<long> >& gridNodeLabel) {
	if (gridNodeLabel[x][y] < 0)
		return true;

	for (unsigned i = 0; i < impassableLoc.size(); i++) {
		if (impassableLoc[i].first == x && impassableLoc[i].second == y)
			return true;
	}

	return false;
}
;

void Agent::calcShortestPathMatrix(long xSize, long ySize,
    long numAccessibleLocs, vector<vector<long> >& gridNodeLabel) {
	shortestPath = new vector<vector<long> > ;
	// alloc memory for all pair shortest path matrix
	(*shortestPath).resize(numAccessibleLocs);
	for (long i = 0; i < numAccessibleLocs; i++) {
		(*shortestPath)[i].resize(numAccessibleLocs, 0);
	}

	// fill in shortest path matrix
	found.resize(numAccessibleLocs);
	for (long i = 0; i < xSize; i++) {
		for (long j = 0; j < ySize; j++) {
			if (!isImpassable(i, j, gridNodeLabel))
				calcShortestPath(i, j, numAccessibleLocs, gridNodeLabel);
		}
	}
}
;

// Breadth first search to calculate shortest path from (i,j)
void Agent::calcShortestPath(long i, long j, long numAccessibleLocs,
    vector<vector<long> >& gridNodeLabel) {
	std::deque<long> fifo_i, fifo_j, dist;

	long sourceNode = gridNodeLabel[i][j];

	// init found vector
	for (long k = 0; k < numAccessibleLocs; k++)
		found[k] = false;

	fifo_i.push_back(i);
	fifo_j.push_back(j);
	dist.push_back(0);
	found[sourceNode] = true;

	while (!fifo_i.empty()) {
		long curr_i = fifo_i.front();
		fifo_i.pop_front();
		long curr_j = fifo_j.front();
		fifo_j.pop_front();
		long currNode = gridNodeLabel[curr_i][curr_j];
		long currDist = dist.front();
		dist.pop_front();
		(*shortestPath)[sourceNode][currNode] = currDist;
		for (long k = east; k <= north; k++) { // k = 0 is noop move
			if (!isImpassable(curr_i + RelativeDirX[k], curr_j + RelativeDirY[k],
			    gridNodeLabel)) {
				long neighbour = gridNodeLabel[curr_i + RelativeDirX[k]][curr_j
				    + RelativeDirY[k]];
				if (!found[neighbour]) {
					found[neighbour] = true;
					fifo_i.push_back(curr_i + RelativeDirX[k]);
					fifo_j.push_back(curr_j + RelativeDirY[k]);
					dist.push_back(currDist + 1);
				}
			}
		}
	}
}
;

/********************* Raw state related *************/
/**
 @param[out] propertiesState the property part of this agent in a state.
 */
void Agent::getCurrState(vector<long>& propertiesState, long newX, long newY) {
	// 1. Coordinates
	propertiesState.resize(2,0);
	if (newX >= 0 && newY >=0){
		propertiesState[0] = newX;
		propertiesState[1] = newY;
	}
	else {
		propertiesState[0] = x;
		propertiesState[1] = y;
	}
	// 2. Other properties appended to the end.
	ObjectWithProperties::getCurrState(propertiesState);
}
;

/*************** Property Abstraction related *********/

void Agent::enumerateProperties(std::vector<long>& currProperties,
    std::vector<std::vector<long> >& output, const Agent* dealtAgent) {
	// Check to see if there is any property that could be ignored
	if (dealtAgent) {
		output.resize(0);
		output.push_back(currProperties);
		// in-place enumeration
		for (unsigned i = 0; i < properties.size(); i++) {
			if (isRelevant(i, dealtAgent))
				enumeratePropertyAt(output, i);
		}
	}
	// everything is relevant
	else
		ObjectWithProperties::enumerateProperties(currProperties, output);
}
;

void Agent::fillProperties(std::vector<long>& absProperties,
    long startingIndex, long endingIndex,
    const std::vector<long>& rawProperties, Agent* dealtAgent) {
	for (unsigned i = startingIndex; i <= endingIndex; i++)
		// if dealtAgent is null or it is relevant (minus startingIndex to offset x and y)
		if ((!dealtAgent) || isRelevant(i - startingIndex, dealtAgent))
			absProperties.push_back(rawProperties[i]);

}
;
