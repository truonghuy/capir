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
/// Camera controller. 
/// </summary>
/// <remarks>The camera does not move. It tracks the human-controlled character.</remarks>
public class CameraControllerCSharp : MonoBehaviour {
	
	private int zoom;

	private int normal;
	private int curzoom;

	private float smooth;
	
	private int nTouch;

	private float minDist;
	private float maxDist;

	private float moveFactor;
	private Vector2 curDist;
	private Vector2 prevDist;

	private Touch touch2; //iPhoneTouch;
	private Touch touch; //iPhoneTouch;
	private float slide;

	// Use this for initialization
	void Start () {
		zoom = 10;
		normal = 60;
		curzoom = 60;
		smooth = 4;
		nTouch = 0;
		
		moveFactor = 0.1f;
	}
	
	// Update is called once per frame
	void Update () {
		
		nTouch = Input.touchCount;
		
		if (nTouch == 2) {
			//pinch = false;
			touch = Input.GetTouch(0);
	 		touch2 = Input.GetTouch(1);
	
			if (touch.phase == TouchPhase.Moved &&
			    touch2.phase == TouchPhase.Moved) {
	
				curDist = touch.position - touch2.position; 
	
				prevDist = (touch.position - touch.deltaPosition) -
				                  (touch2.position - touch2.deltaPosition); 
	
				slide = moveFactor * (prevDist.magnitude - curDist.magnitude);
	
				curzoom = curzoom + (int)slide;
				
				curzoom = Mathf.Min(curzoom, normal);
				curzoom = Mathf.Max(curzoom, zoom);
			}
		}
		
		camera.fieldOfView = Mathf.Lerp(camera.fieldOfView, curzoom, Time.deltaTime*smooth);
		
		Transform target=GameObject.FindWithTag("Player").transform;
		
		float angleToReach = Mathf.LerpAngle(transform.eulerAngles.y,target.eulerAngles.y,4*Time.deltaTime);
		
		Quaternion currentRotation = Quaternion.Euler(0,angleToReach,0);
		transform.position = target.position + new Vector3(0,15,0); //(0,9,0)
		transform.position -= currentRotation* Vector3.forward*4;
		
		transform.LookAt (target.position+ new Vector3(0,3,0));
	}
}
