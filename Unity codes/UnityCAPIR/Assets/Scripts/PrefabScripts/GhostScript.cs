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
/// Behavior script of Ghost.
/// </summary>
/// <remarks>
/// Ghosts move like Sheep but must be attacked instead of herded.
/// </remarks>
public class GhostScript : NPCBehavior {
	
	
	/// <summary>
	/// Hitpoint of this NPC.
	/// </summary>
	/// <value>
	/// The hit point.
	/// </value>
	public int HP {get; set;}
	
	/// <summary>
	/// Start this instance, used for initialization.
	/// </summary>
	protected override void Start () {
		base.Start();
		visLim = levelGenerator.ghostVisLim;
		addedDistance = 1;
		HP = levelData.initConfig.mazeProperties[myIndex][3];
		
		// set to white
		Color colr = this.renderer.material.color;
		colr.r = 1f;
		colr.b = 1f;
		colr.g = 1f;
		
		this.renderer.material.color = colr;
	}
	
	/// <summary>
	/// Updates the status of being solved. 
	/// </summary>
	/// <remarks>
	/// If the NPC's hit points are depleted, it reaches terminal state.
	/// </remarks>
	protected override void UpdateSolved(){
		if (levelGenerator.state.mazeProperties[myIndex][3] <= 0)
			setSolved(true);
	}
	
	
	/// <summary>
	/// Behavior when seeing both players: Run away.
	/// </summary>
	protected override void ActionSeeingBoth(){
		
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
		
		levelData.getValidActions(validActions, 
			(int)Mathf.Round(this.transform.position.x),
			(int)Mathf.Round(this.transform.position.z));
	
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
		
		levelData.getValidActions(validActions, 
			(int)Mathf.Round(this.transform.position.x),
			(int)Mathf.Round(this.transform.position.z));
	
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
		levelData.getValidActions(validActions, 
			(int)Mathf.Round(this.transform.position.x),
			(int)Mathf.Round(this.transform.position.z));
		
		direction = validActions[Random.Range(0, validActions.Count)];
		
		movingSteps = 0;
		isMoving = true;
	}
	
}
