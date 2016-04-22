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
/// Control script of the human-controlled character.
/// </summary>
/// <remarks>Mostly include navigation routines.</remarks>
public class AgentScript : MonoBehaviour {
	
	/***************  Touch attributes ********************/ 
	private int minMajorDist; 
	private float dx;
	private float dy;
	private Vector3 dVec;
	
	/// <summary>
	/// The current action of human character.
	/// </summary>
	/// <value>
	/// The current action.
	/// </value>
	public int action {get; private set;}

	/// <summary>
	/// The touch trackers used for touch devices.
	/// </summary>
	/// <remarks>All of our active trackers.</remarks>
	private List<TouchCTracker> trackers;

	
	/// <summary>
	/// The tracker Dictionary for lookup.
	/// </summary>
	/// <remarks>Key is finger id, value is a TouchTracker.</remarks>
	private Dictionary<int,TouchCTracker> trackerLookup;
	
	/************* Private data for animation ******************/
	/// <summary>
	/// Indicates whether this character is in the middle of some action.
	/// </summary>
	private bool isMoving;
	/// <summary>
	/// The number of animation steps so far.
	/// </summary>
	private int movingSteps;

	/************* LevelGenerator data ***********************/
	/// <summary>
	/// The parent game script.
	/// </summary>
	private MainCSharp levelGenerator;
	
	/// <summary>
	/// The currently attacked ghost.
	/// </summary>
	private GameObject attackedGhost;
	private Color attackedColor;
	
	/************ Main functions ***************************/
	
	// 
	// 
	/// <summary>
	/// Start this instance.
	/// </summary>
	/// <remarks>
	/// The initialization is placed here so that it is called
	/// after Awake routine in <see cref="MainCSharp"/> is called for game init.
	/// </remarks>
	void Start () {
		
		levelGenerator = GameObject.FindWithTag("LevelGenerator").GetComponent<MainCSharp>();
		//levelData = levelGenerator.GetComponent<MainCSharp>().levelData;
		
		minMajorDist = 15;
		
		isMoving=false;
	  	action = (int)GameConstants.actions.unchanged;
		
		trackers = new List<TouchCTracker>();
		trackerLookup = new Dictionary<int, TouchCTracker>();
		
		attackedGhost = null;
	}
	
	public void Restart(){
		Start();
	}
	
	/// <summary>
	/// Records human's keyboard/touch input.
	/// </summary>
	void Update () {
		
		int inputAction = (int)GameConstants.actions.unchanged;
	
		// clean all touches (so they know if they aren't updated after we pull info)
		foreach(TouchCTracker tracker in trackers)
			tracker.Clean();
		
		// process our touches
		if (Input.touchCount == 1)
		{
			foreach(Touch touch in Input.touches)
			{
				// try to get our tracker for this finger id
				TouchCTracker tracker; 
				
				if(trackerLookup.ContainsKey(touch.fingerId))
				{
					tracker = trackerLookup[touch.fingerId];
					tracker.Update(touch);
					Vector2 movement = touch.position - tracker.firstTouch.position;
					movement /= tracker.totalTime;
					
					dx = Mathf.Abs(movement.x);
					dy = Mathf.Abs(movement.y);
				
					
					/****************************
					 _________
					|         |
					|         |    ----> right (1,0,0)
					|         |
					|         |
				  dy|         |
					|         |    ^
					|         |    |
					|         |    | up vector (0,1,0)
					|_________|     
					     dx        vector perpendicular to the plane is : (0,0,1)
					     
					*****************************/
		 			
		 			if (dx >= minMajorDist && dy <= dx) {
						// swipe in x-axis
						if (movement.x < 0)
							inputAction = (int)GameConstants.actions.west; // 4; Up
						else
							inputAction = (int)GameConstants.actions.east; // 3; Left
						
					} else if (dy >= minMajorDist && dx <= dy) {
						// swipe in y-axis
						if (movement.y < 0)
							inputAction = (int)GameConstants.actions.south; // 2; Down
						else
							inputAction = (int)GameConstants.actions.north; // 1; Right
		 			} 
				}
				else
					tracker = BeginTracking(touch);
			}
		}
		else if (Input.touchCount == 2){
			inputAction = (int)GameConstants.actions.shoot;
		}
		
		if (Input.GetKey (KeyCode.UpArrow))
			inputAction = (int)GameConstants.actions.north;
		else if (Input.GetKey (KeyCode.DownArrow))
			inputAction = (int)GameConstants.actions.south;
		else if (Input.GetKey (KeyCode.RightArrow))
			inputAction = (int)GameConstants.actions.east;
		else if (Input.GetKey (KeyCode.LeftArrow))
			inputAction = (int)GameConstants.actions.west;
		else if (Input.GetKey (KeyCode.Space))
			inputAction = (int)GameConstants.actions.shoot;
		
		if (!isMoving){
			action = inputAction;
		}
		
		UpdateGame();
		
		
		// track which events vanished (without using iPhoneTouchPhase.Ended)
		List<TouchCTracker> ended = new List<TouchCTracker>();
		
		// use an intermediate list because EndTracking removes from trackers arraylist
		foreach(TouchCTracker tracker_ in trackers) 
			if(!tracker_.isDirty)
				ended.Add(tracker_);
		
		foreach(TouchCTracker tracker_ in ended)
			EndTracking(tracker_);
	}
	
	/// <summary>
	/// Updates the game.
	/// </summary>
	/// <remarks>
	/// Invoked by Update. Update animation.
	/// </remarks>
	void UpdateGame() {
	
		// 1. Check whether the character is moving, i.e., has not finished movement.
		// This is for the agent's movement
		if(!isMoving){
			int dRow = (int)GameConstants.RelativeDirX[action];
			int dCol = (int)GameConstants.RelativeDirY[action];
			
			int playerRow = Mathf.RoundToInt(this.transform.position.x);  
			int playerCol = Mathf.RoundToInt(this.transform.position.z);
			
			// 0 is floor, 2 is goal 
			if (dRow + dCol != 0){
				
			 	if (levelGenerator.levelData.map[playerRow+dRow,playerCol+dCol]>0){
					isMoving=true;
					movingSteps=0;
				}
				else action = (int)GameConstants.actions.unchanged;
			}
			else if (action == (int)GameConstants.actions.shoot){
				isMoving = true;
				movingSteps = 0;
				
				// if there is a ghost nearby, can shoot
				GameObject[] gos;
				gos =  GameObject.FindGameObjectsWithTag("GhostNPC"); 
				int myNode = levelGenerator.levelData.gridNodeLabel[playerRow, playerCol];
				int ghostNode;
				foreach(GameObject go in gos){
					ghostNode = levelGenerator.levelData.gridNodeLabel
						[Mathf.RoundToInt(go.transform.position.x), 
							Mathf.RoundToInt(go.transform.position.z)];
					
					// reduce HP of attackedGhost
					GhostScript gScript = go.GetComponent<GhostScript>();
				
					if (!gScript.isSolved && levelGenerator.levelData.shortestPath[myNode, ghostNode] <= 
						levelGenerator.ghostShotDistance){
						attackedGhost = go;
						
						gScript.HP--;
						levelGenerator.state.mazeProperties[gScript.myIndex][3]--;
						break;
					}
				}
			}
		}
		else{ // if moving, only animation happens here.
			//debugText = "Moving";
			if (action == (int)GameConstants.actions.shoot){
				// shooting animation
				// TODO
				if (attackedGhost != null){
					if (movingSteps == 0){
						// change color of ghost to red
						attackedColor = attackedGhost.renderer.material.color;
						Color colr = attackedColor;
						GhostScript gScript = attackedGhost.GetComponent<GhostScript>();
						if (gScript.HP <= 0){
							colr.r = 0f;
							colr.b = 1f;
							colr.g = 0f;
						}else{
							colr.r = 1f;
							colr.b = 0f;
							colr.g = 0f;	
						}
						
						attackedGhost.renderer.material.color = colr;
					}
				}
			}
			else Utilities.animateMovement(action, this.gameObject);
			
			movingSteps++;
			
			// Finish moving
			isMoving = !(movingSteps==GameConstants.NumAnimsPerStep);
			
			if (!isMoving){
				// player's coords
				levelGenerator.state.playerProperties[0,0] = (int)Mathf.Round(this.transform.position.x); // playerRow+dRow;
				levelGenerator.state.playerProperties[0,1] = (int)Mathf.Round(this.transform.position.z); // playerRow+dCol;
	 			//action = (int)GameConstants.actions.unchanged;
				
				if (attackedGhost != null){
					// change color of ghost back to normal
					GhostScript gScript = attackedGhost.GetComponent<GhostScript>();
					
					// only when the ghost is still alive do we change the color back
					// to its original color.
					if (gScript.HP > 0)
						attackedGhost.renderer.material.color = attackedColor;
					attackedGhost = null;
				}
			}
		}
		
		
	}
	
	
	/* iPhone Multitouch */
	
	/// <summary>
	/// Starts up a tracker for a fingerid
	/// </summary>
	/// <returns>
	/// The TouchCTracker object.
	/// </returns>
	/// <param name='touch'>
	/// Touch.
	/// </param>
	/// <remarks>Update all of our trackers</remarks>
	TouchCTracker BeginTracking(Touch touch)
	{
		TouchCTracker tracker = new TouchCTracker(touch);
		
		// remember our tracker
		trackers.Add(tracker);
		trackerLookup.Add(touch.fingerId, tracker);
		
		return tracker;
	}
	
	
	/// <summary>
	/// Clears a tracker from being updated, tells it to stop.
	/// </summary>
	/// <param name='tracker'>
	/// TouchCTracker object.
	/// </param>
	void EndTracking(TouchCTracker tracker)
	{
		tracker.End();
		
		trackers.Remove(tracker);
		trackerLookup.Remove(tracker.fingerId);
	}
}
