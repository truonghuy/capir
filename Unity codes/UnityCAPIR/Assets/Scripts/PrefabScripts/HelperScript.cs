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
/// This class defines the behavior of the helper.
/// </summary>
/// <remarks>
/// It periodically queries <see cref="CapirBox"/> for the best collaborative action.
/// </remarks>
public class HelperScript : MonoBehaviour {
	
	/// <summary>
	/// This defines the assumption of the helper on the persistence of the 
	/// human-controlled character.
	/// </summary>
	/// <remarks>
	/// The persistence is defined as the probability
	/// of pursuing the same target in the next time step.
	/// </remarks>
	public float agentPersistence;
	
	/// <summary>
	/// Indicates whether this character is in the middle of some action.
	/// </summary>
	private bool isActing;
	
	/// <summary>
	/// The number of animation steps so far in completing the movement.
	/// </summary>
	private int movingSteps;
	
	/// <summary>
	/// The executing action.
	/// </summary>
	private int action;
	
	/// <summary>
	/// The parent game script.
	/// </summary>
	/// <remarks>
	/// From there, we can access the current state.
	/// </remarks>
	private MainCSharp levelGenerator;
	
	/// <summary>
	/// The AI query box.
	/// </summary>
	private CapirBox capirBox;
	
	// 
	/// <summary>
	/// Start this instance, used for initialization.
	/// </summary>
	void Start () {
		// if agentPersistence is not set, set it to 0.8
		if (agentPersistence == 0)
			agentPersistence = 0.8f;
		
		isActing = false;
		movingSteps = 0;
		
		levelGenerator = GameObject.FindWithTag("LevelGenerator").GetComponent<MainCSharp>();
		
		capirBox = levelGenerator.capirBox;
		
	}
	
	public void Restart(){
		Start();
	}
	
	/// <summary>
	/// Update this instance.
	/// </summary>
	/// <remarks>
	/// If the helper is executing some action, continues doing it. Otherwise,
	/// query <see cref="CapirBox"/> for the best helper action to execute.
	/// </remarks>
	void Update () {
		if (!levelGenerator.gameEnded){
			if (isActing){
				Utilities.animateMovement(action, this.gameObject);
				movingSteps++;
				isActing = !(movingSteps==GameConstants.NumAnimsPerStep);
				
				if (!isActing){
					levelGenerator.state.playerProperties[1,0] = (int)Mathf.Round(this.transform.position.x);
					levelGenerator.state.playerProperties[1,1] = (int)Mathf.Round(this.transform.position.z);
				}
			}
			// Not currently acting
			else {
				// levelGenenerator.state is storing the current state.
				// query capirBox for action
				action = capirBox.getBestHelperAction();
				
				// even when this is a no movement, the helper still waits for 1 time unit
				movingSteps = 0;
				isActing = true;
			}
		}
	}
	
	
}
