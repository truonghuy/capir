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

/// <summary>
/// This class is meant to store the chosen level to load the Main scene.
/// </summary>
public class MenuCSharp : MonoBehaviour {
	
	/// <summary>
	/// Dummy GameObject that is never destroyed to store the chosen level.
	/// </summary>
	private GameObject levelInfo;
	
	void Start () {
		levelInfo = GameObject.FindGameObjectWithTag("LevelChoice");
	}
	
	void Update () {}
	
	/// <summary>
	/// Updates the Menu in the game.
	/// </summary>
	void OnGUI () {
		//string caption, temp;
		
		//int loadWidth = 60;
		int numLevels = 4;
		int boxDist = 10;
		for (int i=1; i<= numLevels; i++){
		
			if (GUI.Button (new Rect ((i-1)*(Screen.width-boxDist)/numLevels + boxDist, boxDist,
				(Screen.width -boxDist) /numLevels - boxDist, 50), "Level "+ i)){
				//loadAllLevels();
				
				levelInfo.GetComponent<LevelInfoScript>().chosenLevel = i.ToString();
				
				Application.LoadLevel("Main");
			}
		}
	}
}
