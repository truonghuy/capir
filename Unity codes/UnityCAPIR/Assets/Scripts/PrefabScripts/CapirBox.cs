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
using System.IO;
using System.Diagnostics;
using System;

/** For XML */
using System.Xml;
using System.Text;

/// <summary>
/// The CAPIR object for action query.
/// </summary>
/// <remarks>
/// The main CAPIR box that assistant script can query to obtain
/// best collaborative action.
/// </remarks>
public class CapirBox {
	
	// npcTypes can be retrieved at caller.levelData.initConfig.mazeProperties
	//public int[] npcTypes;
	
	/// <summary>
	/// The MainCSharp object that uses CapirBox for query.
	/// </summary>
	private MainCSharp parentScript; 
	/// <summary>
	/// The human-controlled character.
	/// </summary>
	private AgentScript agent;
	
	/// <summary>
	/// This array stores the index of the Q function for each NPC type.
	/// </summary>
	/// <remarks>
	/// Some map could contain less NPC types than normally. 
	/// existingNPCTypes[i] = -1 when there is no such NPC type in the map.
	/// existingNPCTypes[i] = j >=0 when QValues[j] is the Q-function of NPCType i.
	/// </remarks>
	private int[] existingNPCTypes;
	
	/// <summary>
	/// The npc types, i.e. Sheep or Ghost.
	/// </summary>
	private List<int> npcTypes; 
	
	/**
	 * \brief 
	 * 
	 * 
	 * 
	 * */
	/// <summary>
	/// Stores the Q values of the NPC types existing in the map.
	/// </summary>
	/// <remarks>
	/// This matrix is used to access Qvalues of an NPC given its index. Indices of QValues are 
	/// QValues[existingNPCTypes[npcTypes[i]]][state, agentAction, helperAction]. 
	/// </remarks>
	public List<double[,,]> QValues;
	//public List<Dictionary<VirtualState, int> > StateMaps;
	
	/* For belief update and action selection */
	
	/// <summary>
	/// The belief on human's intention.
	/// </summary>
	private double[] belief;
	
	/// <summary>
	/// The preset human persistence. 
	/// </summary>
	/// <remarks>
	/// Used in Belief Update. Currently set to 0.8.
	/// </remarks>
	private double agentPersistence;
	
	/// <summary>
	/// Gets or sets the helper action.
	/// </summary>
	/// <value>
	/// The helper action.
	/// </value>
	public int helperAction {get; private set;}
	
	/** Timer */
	//public static Stopwatch stopWatch;
	
	/*************** Functions ***********************/
	
	/// <summary>
	/// Initializes a new instance of the <see cref="CapirBox"/> class.
	/// </summary>
	/// <param name='caller'>
	/// The main game script which initialize CapirBox.
	/// </param>
	public CapirBox(MainCSharp caller){
		parentScript = caller;
		existingNPCTypes = new int[GameConstants.NumNPCTypes];
		// initialize to existingNPCTypes
		Utilities.Populate<int>(existingNPCTypes, -1); 
		
		npcTypes = new List<int>();
		
		QValues = new List<double[,,]>(); // new List<List<List<double>>>();
		//StateMaps = new List<Dictionary<VirtualState, int>>();
		
		//stopWatch = new Stopwatch();
	}
	
	/// <summary>
	/// Start this instance.
	/// </summary>
	/// <remarks>
	/// Invoked by MainCSharp to get agentPersistence from <see cref="HelperScript"/>.
	/// </remarks>
	public void Start(){
		agentPersistence = parentScript.helperCube.GetComponent<HelperScript>().agentPersistence;
		agent = parentScript.agentCube.GetComponent<AgentScript>();
	}
	
	/// <summary>
	/// Clear this instance: clear npcTypes, QValues and existingNPCTypes.
	/// </summary>
	public void clear(){
		Utilities.Populate<int>(existingNPCTypes, -1);
		npcTypes.Clear();
		QValues.Clear();
	}
	
	/// <summary>
	/// Restart this instance: initialize belief to uniform distribution.
	/// </summary>
	public void restart(){
		Utilities.Populate<double>(belief, 1 / (double)npcTypes.Count);
	}
	
	/// <summary>
	/// Reads the tmx file. Use this for initialization.
	/// </summary>
	/// <param name='filename'>
	/// The path to TMX file.
	/// </param>
	public void readTmxFile(string filename){
		
		XmlDocument root = new XmlDocument();
		string fullFilename = Path.Combine( Application.dataPath, "Resources/"+filename);
		//string fullFilename = Path.Combine( Application.dataPath, "Resources/test_level.tmx");
		root.Load(fullFilename);
		
		XmlNodeList layers = root.GetElementsByTagName("layer");
		
		// there's actually only 1 layer, ie. layers.Count = 1.
		int xSize = int.Parse(layers[0].Attributes["width"].Value);
		int ySize = int.Parse(layers[0].Attributes["height"].Value);
		
		XmlNodeList tileSetNodes = root.SelectNodes("/map/tileset/tile"); // root.GetElementsByTagName("tileset");
		//Debug.Log("number of elements in tileSetNodes = " + tileSetNodes.Count);
		XmlNode propNode;
		
		// 1. Properties
		parentScript.ghostVisLim = GameConstants.DefaultVisibility;
		parentScript.sheepVisLim = GameConstants.DefaultVisibility;
		parentScript.fieryVisLim = GameConstants.DefaultVisibility;
		
		// 2. Ghost properties
		parentScript.ghostMaxHP = GameConstants.DefaultGhostHP;
		parentScript.ghostShotDistance = GameConstants.DefaultGhostShotDist;
		
		foreach(XmlNode tileNode in tileSetNodes){
			
			// visionLimit applies for all NPCs
			propNode = tileNode.SelectSingleNode("properties/property[@name='visionLimit']");
			
			if (propNode != null){
				switch (int.Parse(tileNode.Attributes["id"].Value)){
				case GameConstants.GB_GhostTileId:
					parentScript.ghostVisLim = int.Parse(propNode.Attributes["value"].Value);
					break;
				case GameConstants.GB_SheepDeterministicTileId:
					parentScript.sheepVisLim = int.Parse(propNode.Attributes["value"].Value);
					break;
				case GameConstants.GB_FieryTileId:
					parentScript.fieryVisLim = int.Parse(propNode.Attributes["value"].Value);
					break;
				}
			}
			
			// For ghosts
			propNode = tileNode.SelectSingleNode("properties/property[@name='shotDistance']");
			if (propNode != null && 
				int.Parse(tileNode.Attributes["id"].Value) == GameConstants.GB_GhostTileId)
			{
				parentScript.ghostShotDistance = int.Parse(propNode.Attributes["value"].Value);
			}
			
			propNode = tileNode.SelectSingleNode("properties/property[@name='maxHP']");
			if (propNode != null && 
				int.Parse(tileNode.Attributes["id"].Value) == GameConstants.GB_GhostTileId)
			{
				parentScript.ghostMaxHP = int.Parse(propNode.Attributes["value"].Value);
			}
		}
		
		XmlNodeList tiles = root.SelectNodes("/map/layer/data/tile");
		parentScript.levelData = new MapData(xSize, ySize);
		
		int tileIndex = 0;
		bool gotSheep = false, gotGhost = false, gotFiery = false;
		
		// QIndex keeps track of which Q-values belong to which npc type
		int QIndex = 0;
		
		for (int i = 0; i < ySize; i++) {
			for (int j = 0; j < xSize; j++) {
				parentScript.levelData.map[j, ySize - i - 1] = GameConstants.Ground;
				
				switch(int.Parse(tiles[tileIndex].Attributes["gid"].Value)){
				case GameConstants.GB_WallId:
					parentScript.levelData.map[j, ySize - i - 1] = GameConstants.Wall;
					break;
				case GameConstants.GB_FieryExitTileId:
					parentScript.levelData.map[j, ySize - i - 1] = GameConstants.FieryExit;
					break;
				case GameConstants.GB_FieryPenTileId:
					parentScript.levelData.map[j, ySize - i - 1] = GameConstants.FieryPen;
					break;
				case GameConstants.GB_SheepDeterministicPenTileId:
					parentScript.levelData.map[j, ySize - i - 1] = GameConstants.SheepPen;
					break;
				case GameConstants.GB_HumanTileId:
					parentScript.levelData.initConfig.playerProperties[0,0] = j;
					parentScript.levelData.initConfig.playerProperties[0,1] = ySize - i - 1;
					break;
				case GameConstants.GB_AssistantTileId:
					parentScript.levelData.initConfig.playerProperties[1,0] = j;
					parentScript.levelData.initConfig.playerProperties[1,1] = ySize - i - 1;
					break;
				case GameConstants.GB_GhostTileId:
					// ghost by default has 3 hit points
					parentScript.levelData.initConfig.mazeProperties.Add(new List<int>(){GameConstants.GhostType, 
						j, ySize - i - 1, parentScript.ghostMaxHP});
					npcTypes.Add(GameConstants.GhostType);
					if (!gotGhost){
						gotGhost = true;
						existingNPCTypes[GameConstants.GhostType] = QIndex;
						//readQMap(fullFilename + "."+GameConstants.GhostTypeStr);
						QIndex++;
					}
					break;
				case GameConstants.GB_SheepDeterministicTileId:
					parentScript.levelData.initConfig.mazeProperties.Add(new List<int>(){GameConstants.SheepType, 
						j, ySize - i - 1});
					npcTypes.Add(GameConstants.SheepType);
					if (!gotSheep){
						gotSheep = true;
						existingNPCTypes[GameConstants.SheepType] = QIndex;
						//readQMap(fullFilename + "."+GameConstants.SheepTypeStr);
						QIndex++;
					}
					break;
				case GameConstants.GB_FieryTileId:
					parentScript.levelData.initConfig.mazeProperties.Add(new List<int>(){GameConstants.FieryType, 
						j, ySize - i - 1});
					npcTypes.Add(GameConstants.FieryType);
					if (!gotFiery){
						gotFiery = true;
						existingNPCTypes[GameConstants.FieryType] = QIndex;
						//readQMap(fullFilename + "."+GameConstants.FieryTypeStr);
						QIndex++;
					}					
					break;
				}
				tileIndex++;
			}
		}
		
		parentScript.levelData.calcShortestPathMatrix();
		
		// construct state map and read Q functions
		
		for(int i=0; i< npcTypes.Count; i++){
			if (existingNPCTypes[npcTypes[i]] >= QValues.Count){
				readQMap(fullFilename, npcTypes[i]);
			}
		}	
		
		// init belief
		belief = new double[npcTypes.Count];
	}
	
	/// <summary>
	/// Reads the Q map.
	/// </summary>
	/// <param name='filename'>
	/// The path to TMX file.
	/// </param>
	/// <param name='npcType'>
	/// Npc type, i.e. Sheep or Ghost.
	/// </param>
	private void readQMap(string filename, int npcType){
		QValues.Add(readPolicyFile(filename + "."+ GameConstants.NPCTypeStr[npcType] + ".0.Ftn"));
	}
	
	/// <summary>
	/// For debugging purpose: Prints the first elements.
	/// </summary>
	/// <param name='dict'>
	/// Dict.
	/// </param>
	/// <param name='numElements'>
	/// Number of elements to print.
	/// </param>
	public void printFirstElements(Dictionary<VirtualState, int> dict, int numElements){
		List<VirtualState> keys = new List<VirtualState>(dict.Keys);
		
		int i=0;
		foreach (VirtualState key in keys){
			if (i >= numElements)
				break;
			UnityEngine.Debug.Log("Virtual State " + i +  ": " + key.ToString());
		}
	}
	
	/// <summary>
	/// Reads the policy file to initialize QValues.
	/// </summary>
	/// <returns>
	/// The matrix representing the Q values.
	/// </returns>
	/// <param name='filename'>
	/// The path to Ftn file. The filename should be level.tmx.ghost/sheep/fiery.0.Ftn.
	/// </param>
	public double[,,] readPolicyFile(string filename){

		string uncompressedStr = ZlibDecompression.decompress(filename);
		string[] strArray = uncompressedStr.Split(' ');
	
		// 1. read number of virtual states
		int numVirtualStates = int.Parse(strArray[0]);
		
		double[,,] result = new double[numVirtualStates, GameConstants.NumAgentActions, GameConstants.NumHelperActions];
		
		int i = 1;
		for (int j=0; j <numVirtualStates; j++){
			for (int k=0; k<GameConstants.NumAgentActions; k++){
				for (int l=0; l<GameConstants.NumHelperActions; l++){
					result[j,k,l] = double.Parse(strArray[i]);
					i++;
				}	
			}
		}
		
		return result;
	}
	
	/// <summary>
	/// Deprecated! Generates the state map.
	/// </summary>
	/// <returns>
	/// The Dictionary representing the state map.
	/// </returns>
	/// <param name='npcType'>
	/// Npc type.
	/// </param>
	public Dictionary<VirtualState, int> generateStateMap(int npcType){
		//UnityEngine.Debug.Log("generateStateMap");
		VirtualState state;
		Dictionary<VirtualState, int> result = new Dictionary<VirtualState, int>();
		
		int sizeMonster = GameConstants.numProperties[npcType];
		//state = new VirtualState(sizeMonster);
		// stateIndex 0 is termState, no need to store.
		int stateIndex = 1;
		
		// 1. construct a state
		// TODO: may need to change to ySize first, xSize next, also the coordinates is i, ySize - j -1
		for (int p1x = 1; p1x < parentScript.levelData.xSize-1; p1x++){
			for (int p1y = 1; p1y < parentScript.levelData.ySize-1; p1y++){
				if (parentScript.levelData.map[p1x,parentScript.levelData.ySize - p1y -1] != GameConstants.Wall){
					
					
					for (int p2x = 1; p2x < parentScript.levelData.xSize-1; p2x++){
						for (int p2y = 1; p2y < parentScript.levelData.ySize-1; p2y++){
							if (parentScript.levelData.map[p2x,parentScript.levelData.ySize - p2y-1] != GameConstants.Wall){
								
								
								for (int mx = 1; mx < parentScript.levelData.xSize-1; mx++){
									for (int my = 1; my < parentScript.levelData.ySize-1; my++){
		
										if (parentScript.levelData.map[mx,parentScript.levelData.ySize - my-1] != GameConstants.Wall){
											
											state = new VirtualState(sizeMonster);
											
											state.playerProperties[0,0] = p1x;
											state.playerProperties[0,1] = p1y;
											state.playerProperties[1,0] = p2x;
											state.playerProperties[1,1] = p2y;
											
											state.monsterProperty[0] = mx;
											state.monsterProperty[1] = my;
											
											// if NPC is ghost, there is HP.
											if (npcType == GameConstants.GhostType){
												for (int hp=0; hp<= parentScript.ghostMaxHP; hp++){
													state.monsterProperty[2] = hp;
													try{
														result.Add(state, stateIndex);
//														if (stateIndex <= 10)
//															UnityEngine.Debug.Log("Virtual State " + stateIndex +  
//																": " + state.ToString());
														stateIndex++;
													}
													catch
													{
														UnityEngine.Debug.Log("Duplicated Virtual State " + 
															stateIndex + ": " + state.ToString());
													}
												}
											
											}
											// Otherwise, no additional property
											else{
												
												try{
													result.Add(state, stateIndex);
//													if (stateIndex <= 10)
//															UnityEngine.Debug.Log("Virtual State " + stateIndex +  
//																": " + state.ToString());
													stateIndex++;
												}
												catch
												{
													UnityEngine.Debug.Log("Duplicated Virtual State " + 
														stateIndex + ": " + state.ToString());
												}
												
											}
										}
									}	
								}			
							}
						}	
					}			
				}
			}	
		}
		UnityEngine.Debug.Log("generate state map for "+GameConstants.NPCTypeStr[npcType]+
			", num Virtual States ="+ stateIndex);
		
		return result;
		
	}
	
	/// <summary>
	/// Deprecated! Reads the state map from file.
	/// </summary>
	/// <returns>
	/// The Dictionary representing the state map.
	/// </returns>
	/// <param name='filename'>
	/// The path to state map file.
	/// </param>
	/// <remarks>
	/// File format: <numVirtualStates> <numAgentProp> <numHelperProp> <numMonsterProp> <numSpecLocProp>.
	/// In the current implementation, numAgentProb = numHelperProp = 3, numMonsterProp = 4 or 5 depending
	/// on whether there is any hit point.
	///
	/// Thereafter are: <agentRegion> <agentX> <agentY> <helperRegion> <helperX> <helperY>.
	/// Then: <monsterX> <monsterY> <seesAgent> <seesHelper> <additional props>[].
	/// </remarks>
	public Dictionary<VirtualState, int> readStateMapping(string filename){
		
		//UnityEngine.Debug.Log("readStateMapping from "+filename);
		string uncompressedStr = ZlibDecompression.decompress(filename);
		
		string[] strArray = uncompressedStr.Split(' ');
		
		Dictionary<VirtualState, int> result = new Dictionary<VirtualState, int>();
		
		//int xSize = parentScript.levelData.xSize;
		//int ySize = parentScript.levelData.ySize;
		
		// 1. read number of virtual states
		int numVirtualStates = int.Parse(strArray[0]);
		UnityEngine.Debug.Log("readStateMapping from "+filename+", num Virtual States ="+ numVirtualStates);
		
		
		VirtualState state; // = new VirtualState();
		//state.monsterProperty.
		int sizeMonster = int.Parse(strArray[3]) - 2; // exclude seesAgent and seesHelper
		
		int sizePerState = int.Parse(strArray[1]) + int.Parse(strArray[2]) 
			+ int.Parse(strArray[3]) + int.Parse(strArray[4]);
		
		int offset;
		for (int i=0; i<numVirtualStates; i++){
			state = new VirtualState(sizeMonster);
			offset = 4 + i*sizePerState;
			// a. agent and helper
			// exclude the first element, being the region id
			state.playerProperties[0,0] = int.Parse(strArray[offset + 2]);
			state.playerProperties[0,1] = int.Parse(strArray[offset + 3]);
			state.playerProperties[1,0] = int.Parse(strArray[offset + 5]);
			state.playerProperties[1,1] = int.Parse(strArray[offset + 6]);
			
			// b. NPC, exclude seesAgent and seesHelper because we are dealing with no abstraction here.
			offset = offset + 7;
			state.monsterProperty[0] = int.Parse(strArray[offset]);
			state.monsterProperty[1] = int.Parse(strArray[offset + 1]);
			for (int j=4; j < sizeMonster + 2; j++){
				state.monsterProperty[j-2] = int.Parse(strArray[offset + j]);
			}
			
			// c. I'm not dealing with specialLocation here.
			// d. Add into result
			try{
				result.Add(state, i);
			}
			catch
			{
				UnityEngine.Debug.Log("Duplicated Virtual State " + i + ": " + state.ToString());
			}
		}
		
		return result;
	}
	
	
//	public int realToVirtual(State state, int npcIndex){
//		
//		int stateMapIndex = existingNPCTypes[npcTypes[npcIndex]];
//		
//		// 1. construct state
//		VirtualState vState = new VirtualState(state.mazeProperties[npcIndex].Count - 1);
//		
//		vState.playerProperties = state.playerProperties;
//		
//		// exclude the first property in maze, because it is the NPCType.
//		for (int i=0; i<state.mazeProperties[npcIndex].Count - 1; i++){
//			vState.monsterProperty[i] = state.mazeProperties[npcIndex][i+1];
//		}
//		
//		int result;
//		if (StateMaps[stateMapIndex].TryGetValue(vState, out result)){
//			return result;
//		}
//		else {
//			//Debug.Log("state not found: "+ vState.ToString());
//			return 0;
//		}
//	}
	
	/// <summary>
	/// Return the integer representation of virtualState of NPC indexed at npcIndex.
	/// </summary>
	/// <returns>
	/// The integer representation.
	/// </returns>
	/// <param name='state'>
	/// Full game state.
	/// </param>
	/// <param name='npcIndex'>
	/// Npc index.
	/// </param>
	public int realToVirtualFunction(State state, int npcIndex){
		
		int npcType = npcTypes[npcIndex];
		
		// 1. get the freeSpaceIndex of coords.
		int X1 = parentScript.levelData.gridNodeLabel[state.playerProperties[0,0], 
			state.playerProperties[0,1]];
		int X2 = parentScript.levelData.gridNodeLabel[state.playerProperties[1,0], 
			state.playerProperties[1,1]];
		int X3 = parentScript.levelData.gridNodeLabel[state.mazeProperties[npcIndex][1], 
			state.mazeProperties[npcIndex][2]];
		
		// num free grids
		int NF = parentScript.levelData.numAccessibleLocs;
		int result = (X1 * NF + X2)*NF + X3;
		
		// if it is ghost, there is HP
		if (npcType == GameConstants.GhostType){
			result = result * (parentScript.ghostMaxHP+1) + state.mazeProperties[npcIndex][3];
		}
		
		return result+1;
	}
	
	/// <summary>
	/// Gets the best helper action.
	/// </summary>
	/// <returns>
	/// The best helper action.
	/// </returns>
	/// <remarks>
	/// When this routine is invoked, belief is updated and action selection is carried out.
	/// </remarks>
	public int getBestHelperAction(){ //(State state, int agentAction, double[] belief){
		
		// 1. compute condProbAct
		double[,] condProbAct;
		
		getActionModel(parentScript.state, out condProbAct);
		
		// 2. update belief
		beliefUpdate(parentScript.state, agent.action, belief, condProbAct);
		
		int bestAct = (int)GameConstants.actions.unchanged;
		double bestQValue = totalQValue(parentScript.state, belief, agent.action, bestAct);
		
		double qValue;
			
		List<int> validActions = new List<int>();
		
		parentScript.levelData.getValidActions(validActions, 
			parentScript.state.playerProperties[1,0], 
			parentScript.state.playerProperties[1,1]);
	
		//string debugMsg = "";
		foreach(int hAct in validActions){
			
			// get the total Q value for each helper action
			qValue = totalQValue(parentScript.state, belief, agent.action, hAct);

			//debugMsg += qValue.ToString() + ", ";
			// Then get the max
			if (qValue > bestQValue){ 
				bestQValue = qValue;
				bestAct = hAct;
			}
		}
		//UnityEngine.Debug.Log("QValues: "+debugMsg);
		// Then return the action
		return bestAct;
	}
	
	/// <summary>
	/// Computes the total Q value.
	/// </summary>
	/// <returns>
	/// The total Q value.
	/// </returns>
	/// <param name='s'>
	/// Current state.
	/// </param>
	/// <param name='belief'>
	/// Current belief on the human's intention.
	/// </param>
	/// <param name='agentAction'>
	/// Human action.
	/// </param>
	/// <param name='hAct'>
	/// Helper action.
	/// </param>
	private double totalQValue(State s, double[] belief, int agentAction, int hAct){
		double result = 0.0;
		
		int virtualState;
		for (int i=0; i< belief.Length; i++){
			if (s.mazeProperties[i][1] >=0 && belief[i] > 0){
				virtualState = realToVirtualFunction(s, i);
				result += belief[i] * QValues[existingNPCTypes[npcTypes[i]]]
										[virtualState, agentAction, hAct];
			}
		}
		
		return result;
	}
	
	/// <summary>
	/// Gets the action model of the human-controlled character.
	/// </summary>
	/// <param name='currState'>
	/// Current state.
	/// </param>
	/// <param name='condProbAct'>
	/// Conditional probability of human action.
	/// </param>
	private void getActionModel(State currState, out double[,] condProbAct) {
		// Caller already checked for terminality of the state
	
		// sumProb stores the summation of all probabilities involved to use for normalization
		double maxQValue, tempQValue;
		maxQValue = -1;
		//double sumProb;
	
		// counters, act = human act, ca = compound act
		int i, act, tempVState;
	
		// 5. Construct 2D matrix p( a | i ) with x dimension as worldNum, y dimension as actNum
		// this part is used to infer the world given human's action.
		condProbAct = new double[currState.mazeProperties.Count, GameConstants.NumAgentActions];
		Utilities.PopulateMatrix<double>(condProbAct, 0.0);
		
		for (i = 0; i < currState.mazeProperties.Count; i++) {
	
			// isOptimalAct.resize(0);
			// if this is not terminal state
			//if (currState.mazeProperties[i][1] >= 0) {
			if (currState.mazeProperties[i][1] >= 0) {	
				//tempVState = realToVirtual(currState, i);
				tempVState = realToVirtualFunction(currState, i);
				
				//sumProb = 0;
				// act = human act
				for (act = (int)GameConstants.actions.unchanged; act <= (int)GameConstants.actions.shoot; act++) { 
					
					// 1. get max value of (*(mazeWorld->mazes[i])->collabQFn)[tempVState][compAct] where act is part of
					
					maxQValue = QValues[existingNPCTypes[npcTypes[i]]][tempVState,act,0];
					
					for (int partnerAct = (int)GameConstants.actions.unchanged; 
						partnerAct <= (int)GameConstants.actions.north; partnerAct++) {

						tempQValue = QValues[existingNPCTypes[npcTypes[i]]][tempVState, act, partnerAct];
						
						if (maxQValue < tempQValue)
							maxQValue = tempQValue;
					}
					condProbAct[i, act] = Math.Exp(GameConstants.ActionMult * maxQValue);
					//condProbAct[i, act] = Math.Exp(maxQValue);
				}
			} // if termState
			// if this is terminal state, condProbAct[i][act] are all 0
		} // for long i
	}
	
	/// <summary>
	/// Belief Update.
	/// </summary>
	/// <param name='state'>
	/// Current state.
	/// </param>
	/// <param name='agentAction'>
	/// Human action.
	/// </param>
	/// <param name='belief'>
	/// Current belief on the human player's intention.
	/// </param>
	/// <param name='condProbAct'>
	/// The action model of human player computed from <see cref="getActionModel"/>.
	/// </param>
	public void beliefUpdate(State state, int agentAction, double[] belief, double[,] condProbAct){
		
		int i, j;
		int numAliveWorlds = Utilities.NumDifference<double>(belief, 0.0);
		
		if (numAliveWorlds <= 1){
			Utilities.Normalize(belief);
			return;
		}
		
		// 1. Target drift
		double[] prevBelief;
		prevBelief = (double[]) belief.Clone();
	
		for (i = 0; i < belief.Length; i++) {
			belief[i] = 0;
			if (prevBelief[i] > 0){
				for (j = 0; j < prevBelief.Length; j++) {
					if (j==i)
						belief[i] += agentPersistence * prevBelief[j];
					else 
						belief[i] += (1-agentPersistence) * prevBelief[j] / (numAliveWorlds-1);
				}
			}
//			if (Utilities.DoubleEquals(belief[i], 0)){
//				int blah;
//				blah = 0;
//			}
		}
		
		// 2. Bayesian inference
		double sumProb = 0.0;
		Utilities.NormalizeMatrix(condProbAct, 0);
		//UnityEngine.Debug.Log("condProbAct: "+Utilities.getMatrixStr(condProbAct));
		
		for (i = 0; i < state.mazeProperties.Count; i++) {
			// 0: not terminal
			if (state.mazeProperties[i][1] >= 0){
				// update belief
				belief[i] *= condProbAct[i, agentAction];
			}
			else belief[i] = 0;
			sumProb += belief[i];
		}
	
		for (i = 0; i < belief.Length ; i++) {
			if (sumProb > 0)
				belief[i] /= sumProb;
			else
				belief[i] = 1.0 / belief.Length;
		}
		
	}
	
	/// <summary>
	/// Does nothing.
	/// </summary>
	public void Update(){
		
	}
}
