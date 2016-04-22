/*
 * PathFinder.cc
 *
 *  Created on: May 30, 2011
 *      Author: truonghuy
 */

#include <PathFinder.h>
#include <climits>
#include <cmath>
//#include <iostream>

PathFinder::PathFinder() {
  // TODO Auto-generated constructor stub

}

/**
 *
 * @param[in] freeSpace the 2 dimensional array of freeSpace. -1 = obstacle, >=0 free.
 * */
int PathFinder::aStar(int startX, int startY, int endX, int endY,
		  std::vector<std::vector<long> >& freeSpace)
{
	int i,j; // counter
	std::vector<Node*>::iterator it;

	std::vector<Node*> openedList;
	std::vector<Node*> closedList;

	Node *startNode, *endNode, *currNode;

	startNode = new Node();
	endNode = new Node();

	endNode->x = endX;
	endNode->y = endY;

	startNode->x = startX;
	startNode->y = startY;
	startNode->_g = 0;
	startNode->_h = manhattanDist(startNode, endNode);

	openedList.push_back(startNode);

	bool pathFound = false;

	while(!openedList.empty()){
		std::vector<Node*>::iterator minIndex;
		int currCost;
		int minCost = INT_MAX;

		for (it=openedList.begin(); it < openedList.end(); ++it){

			currCost = (*it)->_g + (*it)->_h;

			if (minCost > currCost){
				minCost = currCost;
				minIndex = it;
			}
		}

		// moves the node with min cost from openedList to closedList
		currNode = *minIndex;
		closedList.push_back(currNode);
		openedList.erase(minIndex);

		// if finished
		if (sameNodes(endNode, currNode)){
			endNode->parent = currNode->parent;

			pathFound = true;
			break;
		}

		// checks the neighbourhood
		int neighborX, neighborY;
		Node* node2check;
		for(i=-1; i<2; i++){
			for(j=-1; j<2; j++){
				// only four movement directions
				if (fabs(i)+fabs(j) != 1) continue;

				neighborX = currNode->x + i;
				neighborY = currNode->y + j;

				if (neighborX < 0 || neighborY < 0 || neighborX >= freeSpace.size() || neighborY >= freeSpace[0].size())
					continue;

				// obstacle
				if (freeSpace[neighborX][neighborY] < 0) continue;

				node2check = new Node();
				node2check->x = neighborX;
				node2check->y = neighborY;

				// if already checked
				if (containsNode(openedList, node2check) || containsNode(closedList, node2check)){
					delete node2check;
					continue;
				}

				// calculates cost
				node2check->_g = currNode->_g + manhattanDist(node2check, currNode);
				node2check->_h = manhattanDist(node2check, endNode);

				// keep track of path
				node2check->parent = currNode;

				openedList.push_back(node2check);

			}
		}
	}

	if (pathFound){
		// go back from endNode
		Node *nextNodeFromStart;
		nextNodeFromStart = endNode;
		int action;
		while (!sameNodes(nextNodeFromStart->parent, startNode)){
			getAction(startNode, nextNodeFromStart);
			nextNodeFromStart = nextNodeFromStart->parent;
		}

		return getAction(startNode, nextNodeFromStart);
	}
	else return -1;
};

int PathFinder::getAction(Node* startNode, Node* nextNodeFromStart){
	if (startNode->x > nextNodeFromStart->x){
		//std::cout << "~left~" << std::endl;
		return 3;
	}
	if (startNode->x < nextNodeFromStart->x){
		//std::cout << "~right~" << std::endl;
		return 1;
	}
	if (startNode->y > nextNodeFromStart->y){
		//std::cout << "~down~" << std::endl;
		return 2;
	}
	if (startNode->y < nextNodeFromStart->y){
		//std::cout << "~up~" << std::endl;
		return 4;
	}
};


int PathFinder::manhattanDist(Node* startNode, Node* endNode){
	return fabs(startNode->x - endNode->x) + fabs(startNode->y - endNode->y);
};

bool PathFinder::sameNodes(Node* node1, Node* node2){
	return ((node1->x == node2->x) && (node1->y == node2->y));
};

bool PathFinder::containsNode(std::vector<Node*>& nodeArray, Node* node){

	for (unsigned i=0; i<nodeArray.size(); i++){
		if (sameNodes(node, nodeArray[i]))
			return true;
	}
	return false;
};

PathFinder::~PathFinder() {
  // TODO Auto-generated destructor stub
}
