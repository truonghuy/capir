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
/// The base class of NPC behavior.
/// </summary>
/// <remarks>
/// Child classes must implement at least five routines.
/// <list type="number">
/// <item><see cref="UpdateSolved"/></item>
/// <item><see cref="ActionSeeingNone"/></item>
/// <item><see cref="ActionSeeingHelper"/></item>
/// <item><see cref="ActionSeeingAgent"/></item>
/// <item><see cref="ActionSeeingBoth"/></item>
/// </list>
/// </remarks>
public class NPCBehavior : MonoBehaviour {
	
	/// <summary>
	/// The level data. See <see cref="MapData"/> class.
	/// </summary>
	protected MapData levelData;
	/// <summary>
	/// The parent game script.
	/// </summary>
	protected MainCSharp levelGenerator;
	
	/// <summary>
	/// The human-controlled character.
	/// </summary>
	protected GameObject agent;
	
	/// <summary>
	/// The AI controlled character.
	/// </summary>
	protected GameObject helper;
	
	/// <summary>
	/// The vision limit, within which the NPC sees the players.
	/// </summary>
	protected int visLim;
	
	/// <summary>
	/// Indicates whether this character is in the middle of some action.
	/// </summary>
	protected bool isMoving;
	/// <summary>
	/// The number of animation steps so far.
	/// </summary>
	protected int movingSteps;
	/// <summary>
	/// The current direction of movement.
	/// </summary>
	protected int direction;
	
	/// <summary>
	/// The index of this NPC.
	/// </summary>
	/// <value>
	/// The index of this NPC.
	/// </value>
	public int myIndex {get; protected set;}
	
	/// <summary>
	/// Gets or sets a value indicating whether this <see cref="NPCBehavior"/> is solved.
	/// </summary>
	/// <value>
	/// <c>true</c> if is solved; otherwise, <c>false</c>.
	/// </value>
	public bool isSolved {get; protected set;}
	
	/// <summary>
	/// The added distance, used to tune the movement of the NPC.
	/// </summary>
	public int addedDistance;
	
	/// <summary>
	/// Sets the status of being solved.
	/// </summary>
	/// <param name='val'>
	/// Value.
	/// </param>
	/// <remarks>Updates the state in <see cref="MainCSharp"/>.</remarks>
	public void setSolved(bool val){
		isSolved = val;
		if (isSolved){
			levelGenerator.state.mazeProperties[myIndex][1] = -1;
			levelGenerator.state.mazeProperties[myIndex][2] = -1;
		}
	}
	
	/// <summary>
	/// Start this instance.
	/// </summary>
	protected virtual void Start () {
		levelGenerator = GameObject.FindWithTag("LevelGenerator").GetComponent<MainCSharp>();
		levelData = levelGenerator.levelData;
		
		isMoving = false;
		isSolved = false;
		
		myIndex = int.Parse(this.gameObject.name);
		
		// get agent and helper
		agent = GameObject.FindGameObjectWithTag("Player");
		helper = GameObject.FindGameObjectWithTag("Helper");
		addedDistance = 0;
	}
	
	public void Restart(){
		Start();
	}
	
	/// <summary>
	/// Update this instance with behaviors corresponding to whether it sees players.
	/// </summary>
	/// <remarks>
	/// Four behaviors: seeing no player, seeing the agent, seeing the assistant and seeing both.
	/// </remarks>
	protected void Update () {
			
		if (!isSolved){
			
			if (!isMoving){
				
				int npcNode = levelData.gridNodeLabel[(int)Mathf.Round(this.transform.position.x), 
					(int)Mathf.Round(this.transform.position.z)];
				int agentNode = levelData.gridNodeLabel[(int)Mathf.Round(agent.transform.position.x), 
					(int)Mathf.Round(agent.transform.position.z)];
				int helperNode = levelData.gridNodeLabel[(int)Mathf.Round(helper.transform.position.x), 
					(int)Mathf.Round(helper.transform.position.z)];
				
				// prioritize to run away more from Helper than agent.
				bool seeingAgent = (levelData.shortestPath[npcNode, agentNode] <= visLim );
				bool seeingHelper = (levelData.shortestPath[npcNode, helperNode] <= visLim + addedDistance);
				
				// 1. If seeing agent and helper
				if (seeingAgent && seeingHelper){
					ActionSeeingBoth();	
				}
					
				// 2. If seeing agent only
				else if (seeingAgent){
					ActionSeeingAgent();
				}
				
				// 3. If seeing helper only
				else if (seeingHelper){
					ActionSeeingHelper();
				}
				// 4. If seeing none
				else{
					ActionSeeingNone();
				}
			}
			// Make movement and update state
			else{
				Utilities.animateMovement(direction, this.gameObject);
				movingSteps++;
				isMoving = !(movingSteps==GameConstants.NumAnimsPerStep);
				
				if (!isMoving){
					levelGenerator.state.mazeProperties[myIndex][1] = (int)Mathf.Round(this.transform.position.x);
					levelGenerator.state.mazeProperties[myIndex][2] = (int)Mathf.Round(this.transform.position.z);
					
					// check whether the NPC has been solved
					UpdateSolved();
				}
			}
		}
	}
	
	/// <summary>
	/// To be extended; updates the status of being solved.
	/// </summary>
	protected virtual void UpdateSolved(){}
	/// <summary>
	/// To be extended; updates NPC behavior when seeing no player.
	/// </summary>
	protected virtual void ActionSeeingNone(){}
	/// <summary>
	/// To be extended; updates NPC behavior when seeing the AI-controlled character.
	/// </summary>
	protected virtual void ActionSeeingHelper(){}
	/// <summary>
	/// To be extended; updates NPC behavior when seeing the human-controlled character.
	/// </summary>
	protected virtual void ActionSeeingAgent(){}
	/// <summary>
	/// To be extended; updates NPC behavior when seeing both players.
	/// </summary>
	protected virtual void ActionSeeingBoth(){}
	
	/// <summary>
	/// Gets the action farthest from both nodes, selecting from a set of valid actions.
	/// </summary>
	/// <returns>
	/// The action farthest from both node.
	/// </returns>
	/// <param name='validActions'>
	/// The set of valid actions to select from.
	/// </param>
	/// <param name='node1'>
	/// Node1.
	/// </param>
	/// <param name='node2'>
	/// Node2.
	/// </param>
	/// <remarks>
	/// Priority to get farther away from node1 for tie breaking.
	/// </remarks>
	protected int getActionFarthestFromBothNode(List<int> validActions, int node1, int node2){
		int currX = Mathf.RoundToInt(this.transform.position.x);
		int currY = Mathf.RoundToInt(this.transform.position.z);
		
		int bestAction = validActions[0];
		int tempNode = levelData.gridNodeLabel[currX + GameConstants.RelativeDirX[bestAction], 
				currY + GameConstants.RelativeDirY[bestAction]];
		
		int empFactor = 2;
		
		// run away more from agent than helper
		int bestDistance = empFactor * levelData.shortestPath[tempNode, node1] + 
			levelData.shortestPath[tempNode, node2];
		
		int tempDistance;
		for (int i=1; i<validActions.Count; i++){
			tempNode = levelData.gridNodeLabel[currX + GameConstants.RelativeDirX[validActions[i]], 
				currY + GameConstants.RelativeDirY[validActions[i]]];
			
			tempDistance = empFactor * levelData.shortestPath[tempNode, node1] + 
				 levelData.shortestPath[tempNode, node2];
			
			if (tempDistance > bestDistance){
				bestDistance = tempDistance;
				bestAction = validActions[i];
			}
		}
		return bestAction;
	}
	
	/// <summary>
	/// Gets the action farthest from one node, selecting from a set of valid actions..
	/// </summary>
	/// <returns>
	/// The action farthest from avoidNode.
	/// </returns>
	/// <param name='validActions'>
	/// The set of valid actions.
	/// </param>
	/// <param name='avoidNode'>
	/// The node to get farthest away.
	/// </param>
	protected int getActionFarthestFromNode(List<int> validActions, int avoidNode){
		
		int currX = Mathf.RoundToInt(this.transform.position.x);
		int currY = Mathf.RoundToInt(this.transform.position.z);
		
		int bestAction = validActions[0];
		int tempNode = levelData.gridNodeLabel[currX + GameConstants.RelativeDirX[bestAction], 
				currY + GameConstants.RelativeDirY[bestAction]];
		
		int bestDistance = levelData.shortestPath[tempNode, avoidNode];
		int tempDistance;
		for (int i=1; i<validActions.Count; i++){
			tempNode = levelData.gridNodeLabel[currX + GameConstants.RelativeDirX[validActions[i]], 
				currY + GameConstants.RelativeDirY[validActions[i]]];
			
			tempDistance = levelData.shortestPath[tempNode, avoidNode];
			
			if (tempDistance > bestDistance){
				bestDistance = tempDistance;
				bestAction = validActions[i];
			}
		}
		return bestAction;
	}
	
	/// <summary>
	/// Gets the action towards destNode.
	/// </summary>
	/// <returns>
	/// The action towards destNode.
	/// </returns>
	/// <param name='validActions'>
	/// The set of valid actions to select from.
	/// </param>
	/// <param name='destNode'>
	/// Destination node.
	/// </param>
	protected int getActionTowardsNode(List<int> validActions, int destNode){
		
		int currX = Mathf.RoundToInt(this.transform.position.x);
		int currY = Mathf.RoundToInt(this.transform.position.z);
		
		int bestAction = validActions[0];
		int tempNode = levelData.gridNodeLabel[currX + GameConstants.RelativeDirX[bestAction], 
				currY + GameConstants.RelativeDirY[bestAction]];
		
		int bestDistance = levelData.shortestPath[tempNode, destNode];
		int tempDistance;
		for (int i=1; i<validActions.Count; i++){
			tempNode = levelData.gridNodeLabel[currX + GameConstants.RelativeDirX[validActions[i]], 
				currY + GameConstants.RelativeDirY[validActions[i]]];
			
			tempDistance = levelData.shortestPath[tempNode, destNode];
			
			if (tempDistance < bestDistance){
				bestDistance = tempDistance;
				bestAction = validActions[i];
			}
		}
		return bestAction;
	}
	
}
