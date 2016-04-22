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
/// Behavior script of Sheep.
/// </summary>
/// <remarks>
/// The sheep moves randomly. It runs away from the players when seeing them. 
/// The objective of the players is to herd it towards its Pen.
/// </remarks>
public class SheepScript : NPCBehavior {
	
	/// <summary>
	/// X coordinate of the Sheep's pen.
	/// </summary>
	/// <value>
	/// X coordinate.
	/// </value>
	public int xPen {get; set;}
	/// <summary>
	/// Y coordinate of the Sheep's pen.
	/// </summary>
	/// <value>
	/// Y coordinate.
	/// </value>
	public int yPen {get; set;}
	
	/// <summary>
	/// Start this instance. Used for initialization.
	/// </summary>
	/// <remarks>
	/// At this point, all NPCs have been created.
	/// </remarks>
	protected override void Start () {
		base.Start();	
		visLim = levelGenerator.sheepVisLim;
		addedDistance = 1;
	}
	
	/// <summary>
	/// Updates the status of being solved.
	/// </summary>
	/// <remarks>
	/// If the Sheep is at its pen, it reaches terminal state.
	/// </remarks>
	protected override void UpdateSolved(){
		if (levelGenerator.state.mazeProperties[myIndex][1] == xPen && 
			levelGenerator.state.mazeProperties[myIndex][2] == yPen){
			setSolved(true);
		}
	}
	
	
	/// <summary>
	/// Behavior when seeing both players: Run away.
	/// </summary>
	protected override void ActionSeeingBoth(){
		
		//Debug.Log("Seeing both");
		
		List<int> validActions = new List<int>();
		
		levelData.getValidActions(validActions, 
			(int)Mathf.Round(this.transform.position.x),
			(int)Mathf.Round(this.transform.position.z));
	
		// Get the point that is farthest away from helper
		int agentNode = levelData.gridNodeLabel[Mathf.RoundToInt(agent.transform.position.x), 
				Mathf.RoundToInt(agent.transform.position.z)];
		int helperNode = levelData.gridNodeLabel[Mathf.RoundToInt(helper.transform.position.x), 
				Mathf.RoundToInt(helper.transform.position.z)];
		
		// Set action
		direction = getActionFarthestFromBothNode(validActions, agentNode, helperNode);
		movingSteps = 0;
		isMoving = true;	
	}
	
	/// <summary>
	/// Behavior when seeing human-controlled player: Run away.
	/// </summary>
	protected override void ActionSeeingAgent(){
		
		//Debug.Log("Seeing Agent");
		
		List<int> validActions = new List<int>();
		
		levelData.getValidActionsExcludePoint(validActions, 
			(int)Mathf.Round(this.transform.position.x),
			(int)Mathf.Round(this.transform.position.z),
			xPen, yPen);
	
		// Get the point that is farthest away from helper
		int agentNode = levelData.gridNodeLabel[Mathf.RoundToInt(agent.transform.position.x), 
				Mathf.RoundToInt(agent.transform.position.z)];
		
		// Set action
		direction = getActionFarthestFromNode(validActions, agentNode);
		movingSteps = 0;
		isMoving = true;	
	}
	
	/// <summary>
	/// Behavior when seeing AI-controlled player: Run away.
	/// </summary>
	protected override void ActionSeeingHelper(){
		
		//Debug.Log("Seeing Helper");
		
		List<int> validActions = new List<int>();
		
		levelData.getValidActionsExcludePoint(validActions, 
			(int)Mathf.Round(this.transform.position.x),
			(int)Mathf.Round(this.transform.position.z),
			xPen, yPen);
	
		// Get the point that is farthest away from helper
		int helperNode = levelData.gridNodeLabel[Mathf.RoundToInt(helper.transform.position.x), 
				Mathf.RoundToInt(helper.transform.position.z)];
		
		// Set action
		direction = getActionFarthestFromNode(validActions, helperNode);
		movingSteps = 0;
		isMoving = true;	
	}
	
	/// <summary>
	/// Behavior when seeing none: Move randomly.
	/// </summary>
	protected override void ActionSeeingNone(){
		
		List<int> validActions = new List<int>();
		
		levelData.getValidActionsExcludePoint(validActions, 
			(int)Mathf.Round(this.transform.position.x),
			(int)Mathf.Round(this.transform.position.z),
			xPen, yPen);
		
		direction = validActions[Random.Range(0, validActions.Count)];
		
		movingSteps = 0;
		isMoving = true;
	}
	
}
