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

using UnityEngine;
using System.Collections;
using System.Collections.Generic;

/// <summary>
/// This class represents the level's map and initial state.
/// </summary>
[System.Serializable]
public class MapData{
	
	/// <summary>
	/// Intermediate variable to compute shortest paths.
	/// </summary>
	private bool[] found;
	
	/// <summary>
	/// The map layout.
	/// </summary>
	public int[,] map {get; set;}
	/// <summary>
	/// The map width.
	/// </summary>
	public int xSize {get; set;}
	/// <summary>
	/// The map height.
	/// </summary>
	public int ySize {get; set;}
	/// <summary>
	/// The initial state.
	/// </summary>
	public State initConfig {get; set;}
	/// <summary>
	/// The free grid nodes.
	/// </summary>
	public int[,] gridNodeLabel {get; private set;}
	/// <summary>
	/// The number of accessible locations, i.e. free grid nodes.
	/// </summary>
	public int numAccessibleLocs;
	
	/// <summary>
	/// The inverse mapping from freeGridIndex to grid coords.
	/// </summary>
	public List<int[]> freeGrids {get; private set;}
	
	/// <summary>
	/// The shortest path. The index is gridNodeLabel[startx, starty], and
	/// gridNodeLabel[endx, endy].
	/// </summary>
	public int[,] shortestPath {get; private set;}
	
	/// <summary>
	/// Initializes a new instance of the <see cref="MapData"/> class.
	/// </summary>
	/// <param name='sizeX'>
	/// The map width.
	/// </param>
	/// <param name='sizeY'>
	/// The map height.
	/// </param>
	public MapData(int sizeX, int sizeY){
		initConfig = new State();
		this.xSize = sizeX;
		this.ySize = sizeY;
		this.map = new int[xSize, ySize];
		freeGrids = new List<int[]>();
	}
	
	/// <summary>
	/// Calculates the shortest path matrix.
	/// </summary>
	/// <remarks>
	/// It should be invoked after the map has been constructed.
	/// </remarks>
	public void calcShortestPathMatrix(){
		freeGrids.Clear();
		int xSize = map.GetLength(0);
		int ySize = map.GetLength(1);
		
		// 1. compute gridNodeLabel
		gridNodeLabel = new int[xSize, ySize];
		
		numAccessibleLocs = 0;
		for (int i = 0; i < xSize; i++) {
			for (int j = 0; j < ySize; j++) {
				if (map[i,j] == GameConstants.Wall)
					gridNodeLabel[i,j] = -1;
				else {
					gridNodeLabel[i,j] = numAccessibleLocs;
					freeGrids.Add(new int[]{i,j});
					numAccessibleLocs++;
				}
			}
		}
		
		// 2. compute the matrix 
		shortestPath = new int[numAccessibleLocs, numAccessibleLocs];
		Utilities.PopulateMatrix<int>(shortestPath, 0);
		
		// fill in shortest path matrix
		found = new bool[numAccessibleLocs];
		for (int i = 0; i < xSize; i++) {
			for (int j = 0; j < ySize; j++) {
				if (gridNodeLabel[i,j] >= 0)
					calcShortestPath(i, j);
			}
		}
		
	}
	
	/// <summary>
	/// Calculates the shortest path between grid node <paramref name="i"/>
	/// and <paramref name="j"/>.
	/// </summary>
	private void calcShortestPath(int i, int j){
		
		Queue<int> fifo_i, fifo_j, dist;
		fifo_i = new Queue<int>();
		fifo_j = new Queue<int>();
		dist = new Queue<int>();
		
		int sourceNode = gridNodeLabel[i,j];
	
		// init found vector
		Utilities.Populate<bool>(found, false);
		
		fifo_i.Enqueue(i);
		fifo_j.Enqueue(j);
		dist.Enqueue(0);
		
		found[sourceNode] = true;
	
		while (fifo_i.Count > 0) {
			
			
			int curr_i = fifo_i.Dequeue();
			int curr_j = fifo_j.Dequeue();
			
			int currNode = gridNodeLabel[curr_i,curr_j];
			int currDist = dist.Dequeue();
			
			shortestPath[sourceNode, currNode] = currDist;
			
			for (GameConstants.actions k = GameConstants.actions.east; 
				k <= GameConstants.actions.north; k++) { 
				// k = 0 is noop move
				if (gridNodeLabel[curr_i + GameConstants.RelativeDirX[(int)k],curr_j + 
					GameConstants.RelativeDirY[(int)k]] >= 0) {
					
					int neighbour = gridNodeLabel[curr_i + GameConstants.RelativeDirX[(int)k], curr_j
					    + GameConstants.RelativeDirY[(int)k]];
					
					if (!found[neighbour]) {
						found[neighbour] = true;
						fifo_i.Enqueue(curr_i + GameConstants.RelativeDirX[(int)k]);
						fifo_j.Enqueue(curr_j + GameConstants.RelativeDirY[(int)k]);
						dist.Enqueue(currDist + 1);
					}
				}
			}
		}
	}
	
	/// <summary>
	/// Returns valid actions at point <paramref name="xCoord"/>, <paramref name="yCoord"/> 
	/// that do not move into the point <paramref name="exX"/>, <paramref name="exY"/>.
	/// </summary>
	/// <param name='actions'>
	/// Returned list of valid actions.
	/// </param>
	public void getValidActionsExcludePoint(List<int> actions, 
		int xCoord, int yCoord, 
		int exX, int exY){
		
		actions.Clear();
		
		// nochanged is always a valid action
		actions.Add((int)GameConstants.actions.unchanged);
		
		// check the movement actions
		for (GameConstants.actions i=GameConstants.actions.east; i <= GameConstants.actions.north; i++){
			if (isFreeSpace(xCoord + GameConstants.RelativeDirX[(int)i], yCoord + GameConstants.RelativeDirY[(int)i]) 
				&& (xCoord + GameConstants.RelativeDirX[(int)i] != exX || yCoord + GameConstants.RelativeDirY[(int)i] != exY) )	
				actions.Add((int)i);
		}
	}
	
	/// <summary>
	/// Returns valid actions at point <paramref name="xCoord"/>, <paramref name="yCoord"/>.
	/// </summary>
	/// <param name='actions'>
	/// Returned list of valid actions.
	/// </param>
	public void getValidActions(List<int> actions, int xCoord, int yCoord){
		actions.Clear();
		// nochanged is always a valid action
		actions.Add((int)GameConstants.actions.unchanged);
		
		// check the movement actions
		for (GameConstants.actions i=GameConstants.actions.east; i <= GameConstants.actions.north; i++){
			//if (levelData.map[xCoord + GameConstants.RelativeDirX[(int)i], yCoord + GameConstants.RelativeDirY[(int)i]] 
			//	> GameConstants.Wall)
			if (isFreeSpace(xCoord + GameConstants.RelativeDirX[(int)i], yCoord + GameConstants.RelativeDirY[(int)i]))	
				actions.Add((int)i);
		}
	}
	
	/// <summary>
	/// Checks if the point <paramref name="xCoord"/>, <paramref name="yCoord"/> 
	/// is a free grid.
	/// </summary>
	protected bool isFreeSpace(int xCoord, int yCoord){
		if (map[xCoord, yCoord] == GameConstants.Wall)
			return false;
		return true;
	}
	
}

