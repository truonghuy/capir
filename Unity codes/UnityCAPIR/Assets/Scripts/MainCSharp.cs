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

/** iPhone Multitouch */
using System.IO;
using System.Text.RegularExpressions;

/// <summary>
/// The main game script.
/// </summary>
public class MainCSharp : MonoBehaviour {
	
	/// <summary>
	/// The AI CAPIR object.
	/// </summary>
	public CapirBox capirBox {get; private set;}
	
	
	/************  Level data *******************************/
	/// <summary>
	/// The level layout, storing the map configuration and 
	/// the starting positions of characters. 
	/// See <see cref="MapData"/> for further information. 
	/// </summary>
	public MapData levelData;
	
	/// <summary>
	/// Current state of the level.
	/// </summary>
	public State state {get; set;}
	/// <summary>
	/// The current level.
	/// </summary>
	private int currLevel;
	/// <summary>
	/// The number of levels.
	/// </summary>
	private int numLevels;
	/// <summary>
	/// The status of levels (solved or not).
	/// </summary>
	private List<bool> solvedLevel;
	/// <summary>
	/// Gets or sets a value indicating whether the current level has ended.
	/// </summary>
	/// <value>
	/// <c>true</c> if the current level has ended; otherwise, <c>false</c>.
	/// </value>
	public bool gameEnded {get; private set;}
	
	/*********** Prefab for Graphic ***************************/
	/// <summary>
	/// The GameObject to represent wall.
	/// </summary>
	public GameObject wallCube;
	/// <summary>
	/// The GameObject to represent floor.
	/// </summary>
	public GameObject floorCube;
	/// <summary>
	/// The GameObject to represent human-controlled character.
	/// </summary>
	public GameObject agentCube;
	/// <summary>
	/// The GameObject to represent AI-controlled character.
	/// </summary>
	public GameObject helperCube;
	/// <summary>
	/// The GameObject to represent Ghost.
	/// </summary>
	public GameObject ghostCube;
	/// <summary>
	/// The GameObject to represent Sheep.
	/// </summary>
	public GameObject sheepCube;
	/// <summary>
	/// The GameObject to represent Fiery.
	/// </summary>
	public GameObject fieryCube;
	/// <summary>
	/// The GameObject to represent Sheep pen.
	/// </summary>
	public GameObject sheepPenCube;
	/// <summary>
	/// The GameObject to represent Fiery pen.
	/// </summary>
	public GameObject fieryPenCube;
	/// <summary>
	/// The GameObject to represent Fiery exit.
	/// </summary>
	public GameObject fieryExitCube;
	
	/// <summary>
	/// The list of NPCs.
	/// </summary>
	private List<GameObject> monsters;
	/// <summary>
	/// The human-controlled character.
	/// </summary>
	private GameObject agent;
	/// <summary>
	/// The AI-controlled character.
	/// </summary>
	private GameObject helper;
	
	
	/************ properties for NPCs *************************/
	/// <summary>
	/// Gets or sets the ghost vision limit.
	/// </summary>
	/// <value>
	/// The ghost vision limit.
	/// </value>
	public int ghostVisLim {get;set;}
	/// <summary>
	/// Gets or sets the Sheep vision limit.
	/// </summary>
	/// <value>
	/// The Sheep vision limit.
	/// </value>
	public int sheepVisLim {get;set;}
	/// <summary>
	/// Gets or sets the Fiery vision limit.
	/// </summary>
	/// <value>
	/// The Fiery vision limit.
	/// </value>
	public int fieryVisLim {get;set;}
	
	/// <summary>
	/// Gets or sets the ghost max HP.
	/// </summary>
	/// <value>
	/// The ghost max HP.
	/// </value>
	public int ghostMaxHP {get; set;}
	/// <summary>
	/// Gets or sets the ghost shot distance, within which the 
	/// human-controlled character can shoot the ghost.
	/// </summary>
	/// <value>
	/// The ghost shot distance.
	/// </value>
	public int ghostShotDistance {get; set;}
	
	/******************** Functions ***************************/
	
	
	/// <summary>
	/// Wakens this instance.
	/// </summary>
	/// <remarks>
	/// Awake is used for reading the map file and initializing the characters
	/// to make sure that all NPCs are created
	/// before Start routine in each NPC is called. 
	/// </remarks>
	void Awake()
	{
		// widescreen orientation
		Screen.orientation = ScreenOrientation.Portrait;
		
		//levelString = levelFile.text;
		GameObject go = GameObject.FindGameObjectWithTag("LevelChoice");
		currLevel = int.Parse(go.GetComponent<LevelInfoScript>().chosenLevel) - 1;
		//levelString = "level" + go.GetComponent<LevelInfoScript>().chosenLevel + ".tmx";
		
		
		capirBox = new CapirBox(this);
		
		
		numLevels = 4;
		solvedLevel = new List<bool>();
		for(int i=0;i<numLevels; i++)
			solvedLevel.Add(false);
		
		monsters = new List<GameObject>();
		state = new State();
		
		// agent and helper always exist
		agent = Instantiate(agentCube, new Vector3 (0,0, 0), Quaternion.identity) as GameObject;
	    helper = Instantiate(helperCube, new Vector3 (0,0, 0), Quaternion.identity) as GameObject;
			
		
		setUpLevel(currLevel);
		
		startLevel();
	}
	
	/// <summary>
	/// Starts this instance. Invoke initialization routine of <typeparamref name="capirBox"/>.
	/// </summary>
	void Start () {
		capirBox.Start();
	}
	
	/// <summary>
	/// Updates this instance: updates <typeparamref name="capirBox"/> and checks if the game is finished.
	/// </summary>
	void Update()
	{
		UpdateTransparency();
		
		// 0. Check if the game is finished
		if (gameFinished()){
			solvedLevel[currLevel] = true;
			gameEnded = true;
			return;
		}
		
		// update capirBox to update belief, action...
		capirBox.Update();
	}
	
	/// <summary>
	/// Checks if the game is over.
	/// </summary>
	/// <returns>
	/// <c>true</c> if all NPCs have reached terminal states, <c>false</c> otherwise.
	/// </returns>
	bool gameFinished(){

		foreach (GameObject npc in monsters){
			if (!(npc.GetComponent<NPCBehavior>().isSolved))
				return false;
		}
		
		return true;
	}
	
	/// <summary>
	/// Loads the level <paramref name="level"/>.
	/// </summary>
	/// <param name='level'>
	/// The level to be loaded.
	/// </param>
	/// <remarks>
	/// Calls <paramref name="capirBox"/> routine to load level.
	/// </remarks>
	void loadLevel(int level){
		
		// Read the map file and load policy files
		capirBox.readTmxFile("level" + (level+1).ToString() + ".tmx");
	}
	
	/// <summary>
	/// Sets up the given level, i.e., calling <c>loadLevel</c> to read
	/// from .TMX file and invoking <c>createObjects</c> on the initial state.
	/// </summary>
	/// <param name='level'>
	/// The level to be set up.
	/// </param>
	void setUpLevel(int level){
		currLevel = level;
		loadLevel(currLevel);
		createObjects(levelData.initConfig);
	}
	
	/// <summary>
	/// Copies the state from <paramref name="src"/> to <paramref name="dest"/>.
	/// </summary>
	/// <param name='src'>
	/// Source state.
	/// </param>
	/// <param name='dest'>
	/// Destination state.
	/// </param>
	void copyState(State src, State dest){
		for(int i=0; i< 2; i++)
			for(int j=0; j< 2; j++)
				dest.playerProperties[i,j] = src.playerProperties[i,j];
		
		dest.mazeProperties.Clear();
		foreach(List<int> mazeProp in src.mazeProperties){
			List<int> tempProb = new List<int>();
			foreach(int prop in mazeProp){
				tempProb.Add(prop);
			}
			dest.mazeProperties.Add(tempProb);
		}
	}
	
	/// <summary>
	/// Clears the level.
	/// </summary>
	/// <remarks>
	/// Destroys all GameObjects on the map except for the player characters.
	/// </remarks>
	void clearLevel(){
		
		GameObject[] gos;
		gos =  GameObject.FindGameObjectsWithTag("MapElement"); 
		foreach(GameObject go in gos){
			Destroy(go);
		}
		
		foreach (GameObject go in monsters){
			Destroy(go);
		}
		monsters.Clear();
		capirBox.clear();
	}
	
	
	/// <summary>
	/// Updates the transparency of the characters if 
	/// they are standing on top of any goal position (pen and exit).
	/// </summary>
	void UpdateTransparency (){
		// the players
		UpdateTransparencyGO(agent);
		UpdateTransparencyGO(helper);
		
		// monsters
		foreach(GameObject npc in monsters){
			UpdateTransparencyGO(npc);
		}
	}
	
	/// <summary>
	/// Updates the transparency of GameObject <paramref name="go"/>.
	/// </summary>
	/// <param name='go'>
	/// The GameObject to be updated.
	/// </param>
	/// <remarks>
	/// If the NPC steps on Sheep's pen or Fiery's pen or exit,
	/// it becomes slightly transparent so that the player can see what underneath.
	/// </remarks>
	void UpdateTransparencyGO(GameObject go){
		int tempX, tempY;
		tempX = Mathf.RoundToInt(go.transform.position.x);
		tempY = Mathf.RoundToInt(go.transform.position.z);
		
		Color colr = go.renderer.material.color;
		// if the go's position matches a goal
		if (levelData.map[tempX, tempY] == GameConstants.SheepPen ||
			levelData.map[tempX, tempY] == GameConstants.FieryPen ||
			levelData.map[tempX, tempY] == GameConstants.FieryExit
			)
		{
			// set alpha to 0.7
			if (colr.a != 0.7f){
				colr.a = 0.7f;
				go.renderer.material.color = colr;
			}
		}
		else{
			if (colr.a != 1.0f){
				colr.a = 1.0f;
				go.renderer.material.color = colr;
			}
		}
	}

	/// <summary>
	/// Starts the level.
	/// </summary>
	void startLevel(){
		
		capirBox.restart();
		
		// 2. copy state from init state levels to state
		copyState(levelData.initConfig, state);
		
		// 3. draw state
		drawState(state);
		
		helper.GetComponent<HelperScript>().Restart();
		agent.GetComponent<AgentScript>().Restart();
		
		foreach(GameObject npc in monsters){
			npc.GetComponent<NPCBehavior>().Restart();
		}
		
		gameEnded = false;
	}
	
	/// <summary>
	/// Moves the positions of characters to the position as described by state <paramref name="s"/>.
	/// </summary>
	/// <param name='s'>
	/// The state.
	/// </param>
	void drawState(State s){
		agent.transform.position = new Vector3(s.playerProperties[0,0], 0, 
			s.playerProperties[0,1]);
		helper.transform.position = new Vector3(s.playerProperties[1,0], 0, 
			s.playerProperties[1,1]);
	    // monster
		for (int i=0; i < s.mazeProperties.Count; i++){
			monsters[i].transform.position = new Vector3(s.mazeProperties[i][1], 0, 
				s.mazeProperties[i][2]);
		}
	}
	
	/// <summary>
	/// Creates the game objects for the whole level, as described by <paramref name="levelData"/>.
	/// </summary>
	/// <param name='s'>
	/// The initial state, used to place the characters into initial positions.
	/// </param>
	void createObjects(State s) {
		
		int[] sheepPenPos = new int[2];
		int[] fieryPenPos = new int[2];
		int[] fieryExitPos = new int[2];
		
		// 1. Create MapElements for the map first
		int floorDepth = -1;
		float penDepth = -0.5f;
		
		for (int i = 0; i < levelData.xSize; i++) {
	        for (int j = 0; j < levelData.ySize; j++) {
				Instantiate(floorCube, new Vector3 (i, floorDepth, j), Quaternion.identity);
				switch (levelData.map[i,j]) {
					case GameConstants.Wall:
						Instantiate(wallCube, new Vector3 (i, 0, j), Quaternion.identity);
						break;
					case GameConstants.SheepPen:
						Instantiate(sheepPenCube, new Vector3 (i, penDepth, j), Quaternion.identity);
						sheepPenPos[0] = i;
						sheepPenPos[1] = j;
						break;
					case GameConstants.FieryPen:
						Instantiate(fieryPenCube, new Vector3 (i, penDepth, j), Quaternion.identity);
						fieryPenPos[0] = i;
						fieryPenPos[1] = j;
						break;
					case GameConstants.FieryExit:
						Instantiate(fieryExitCube, new Vector3 (i, penDepth, j), Quaternion.identity);
						fieryExitPos[0] = i;
						fieryExitPos[1] = j;
						break;
				}
	        }
	    }
	    
	    // 3. Monsters
		int monsterIndex = 0;
		foreach(List<int> npcProb in s.mazeProperties){
			GameObject npc;
			
			switch(npcProb[0]){
			case GameConstants.SheepType:
				npc = Instantiate(sheepCube, new Vector3 (npcProb[1], 0, 
					npcProb[2]), Quaternion.identity) as GameObject;
				npc.GetComponent<SheepScript>().xPen = sheepPenPos[0];
				npc.GetComponent<SheepScript>().yPen = sheepPenPos[1];
				
				break;
			case GameConstants.GhostType:
				npc = Instantiate(ghostCube, new Vector3 (npcProb[1], 0, 
					npcProb[2]), Quaternion.identity) as GameObject;
				break;
			case GameConstants.FieryType:
				npc = Instantiate(fieryCube, new Vector3 (npcProb[1], 0, 
					npcProb[2]), Quaternion.identity) as GameObject;
				npc.GetComponent<FieryScript>().xPen = fieryPenPos[0];
				npc.GetComponent<FieryScript>().yPen = fieryPenPos[1];
				npc.GetComponent<FieryScript>().xExit = fieryExitPos[0];
				npc.GetComponent<FieryScript>().yExit = fieryExitPos[1];
				break;
			default:
				npc = new GameObject();
				break;
			}
			
			npc.name = monsterIndex.ToString();
			monsterIndex++;
			monsters.Add(npc);
		}
		
		
	}
	
	/// <summary>
	/// Updates the Menu in the game.
	/// </summary>
	void OnGUI () {
		string caption, temp;
		
		if (gameEnded)
			caption = "You won!\nRestart!";
		else caption = "Give up.\nRestart!";
		
		// 3 buttons
		// Prev level, current level (restart), and next level
		if (currLevel >=1){
			temp = (solvedLevel[currLevel-1])? "(solved)\n" : "(unsolved)\n";
			if (GUI.Button (new Rect (10, 10, (Screen.width-40)/3, 50), temp+"Prev level"))
		    {
				clearLevel();
				setUpLevel((currLevel-1)%numLevels);
				startLevel();
			}
		}
		
		temp = (solvedLevel[currLevel])? "(solved)\n" : "(unsolved)\n";
		if (GUI.Button (new Rect (20+(Screen.width-40)/3, 10,(Screen.width-40)/3,50), temp+caption)) 
			startLevel();
			
		if (currLevel <= numLevels - 2){
			temp = (solvedLevel[(currLevel+1)%numLevels])? "(solved)\n" : "(unsolved)\n";
			if (GUI.Button (new Rect (10 + (10+(Screen.width-40)/3)*2, 10, (Screen.width-40)/3, 50), 
				temp+"Next level"))
		    {
				clearLevel();
				setUpLevel((currLevel+1)%numLevels);
				startLevel();
			}
		}
	}
	
	
}
