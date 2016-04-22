/*
 * PathFinder.h
 *
 *  Created on: May 30, 2011
 *      Author: truonghuy
 */

#ifndef PATHFINDER_H_
#define PATHFINDER_H_

#include<vector>

class PathFinder {
private:
	struct Node{
		int x;
		int y;
		int _g; // path cost
		int _h; // heuristic
		Node *parent;
		// Node *child; // no need for now
	};

	static int manhattanDist(Node* startNode, Node* endNode);
	static bool sameNodes(Node* node1, Node* node2);
	static bool containsNode(std::vector<Node*>& nodeArray, Node* node);
	static int getAction(Node* startNode, Node* nextNodeFromStart);

public:
  PathFinder();
  /**
   * A* search for path
   *
   * @return the action the agent should take from startX, startY. -1 if there's no path found
   * */
  static int aStar(int startX, int startY, int endX, int endY,
		  std::vector<std::vector<long> >& freeSpace);
  virtual ~PathFinder();
};

#endif /* PATHFINDER_H_ */
