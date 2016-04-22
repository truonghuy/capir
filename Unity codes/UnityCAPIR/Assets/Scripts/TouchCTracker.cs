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

/**
 * 
 * 
 * */

/// <summary>
/// This class is used when the game is played on mobile devices with touch interface.
/// </summary>
[System.Serializable]
public class TouchCTracker {
	
	// expose our finger id for convenience
	public int fingerId {get; set;}
	
	// our last data from Unity
	public Touch touch {get; set;}
	
	// have we been updated this frame?
	public bool isDirty {get; set;}
	
	// track how long we've been active
	public float totalTime {get; set;}
	
	// where we started
	public Touch firstTouch {get; set;}
	
	/// <summary>
	/// Initializes a new instance of the <see cref="TouchCTracker"/> class.
	/// </summary>
	/// <param name='firstTouch'>
	/// First touch.
	/// </param>
	public TouchCTracker(Touch firstTouch)
	{
		this.totalTime = 0.0f;
		this.isDirty =  false;
		this.fingerId = firstTouch.fingerId;
		this.firstTouch = firstTouch;
		this.touch = firstTouch; 
		
		Begin();
		Update(firstTouch);
	}
	
	/// <summary>
	/// Update the specified touch.
	/// </summary>
	/// <param name='touch'>
	/// Touch.
	/// </param>
	public void Update(Touch touch)
	{
		this.touch = touch;
		isDirty = true;
		
		totalTime += Time.deltaTime;
	}
	
	/// <summary>
	/// Cleans this instance. Called before our manager updates everything
	/// </summary>
	public void Clean()
	{
		isDirty = false;
	}
	
	/// <summary>
	/// Callback when we start.
	/// </summary>
	public void Begin()
	{
		Debug.Log("started tracking finger " + fingerId.ToString());
		
	}
	
	/// <summary>
	/// Ends this instance. Cleans things up.
	/// </summary>
	public void End()
	{
		Debug.Log("ended tracking finger " + fingerId.ToString());
				
		// how far did we move?
		Vector2 movement = touch.position - firstTouch.position;
		movement /= totalTime;
		
		Debug.Log("moved " + movement.ToString());
		
		// test for *very* simple swipe detection
		if(movement.x > 200 && Mathf.Abs(movement.y) < 200)
			Debug.Log("swipe right");
		if(movement.x < 200 && Mathf.Abs(movement.y) < 200)
			Debug.Log("swipe left");

	}
}
