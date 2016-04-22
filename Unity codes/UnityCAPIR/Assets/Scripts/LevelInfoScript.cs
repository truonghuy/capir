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
/// The class stores the chosen level.
/// </summary>
public class LevelInfoScript : MonoBehaviour {
	
	/// <summary>
	/// The chosen level.
	/// </summary>
	public string chosenLevel {get; set;}
	
	/// <summary>
	/// Awake this instance. Set it to non-destroyable.
	/// </summary>
	void Awake() {
        DontDestroyOnLoad(transform.gameObject);
		chosenLevel = "1";
    }
}
