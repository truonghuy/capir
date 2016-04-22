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
using System;

/// <summary>
/// State class. Stores the game state.
/// </summary>
[System.Serializable]
public class State {
	
	/// <summary>
	/// Two players, each having only (x,y) coordinates.
	/// 0: human, 1: helper.
	/// </summary>
	public int[,] playerProperties {get; set;}
	
	/// <summary>
	/// List of NPC properties. At least 3 properties for mazeProperties[i]:
	/// type, coordX, coordY.
	/// </summary>
	public List<List<int>> mazeProperties {get; set;}
	
	public State(){
		playerProperties = new int[2,2]; // 2 players, 2 properties each
		mazeProperties = new List<List<int>>();
	}
	
	public override string ToString(){
		string result = "State: ";
		result += "[" + playerProperties[0,0] + ", " + playerProperties[0,1] + ", " + 
			playerProperties[1,0] + ", " + playerProperties[1,1] +  "]";
		
		foreach(List<int> npc in mazeProperties){
			result += ", [";
			
			foreach(int prob in npc){
				result += prob.ToString() + ", ";
			}
			
			result += "]";
		}
		return result;
	}
	
}

/// <summary>
/// Virtual state, extracted from the game <see cref="State"/>.
/// </summary>
/// <remarks>
/// Each VirtualState object only has data about one NPC.
/// Note that State and VirtualState are currently fixed to contain only
/// the coordinates of players, thus playerProperties are of size [2,2].
/// </remarks>
[System.Serializable]
public class VirtualState{
	/// <summary>
	/// Two players, each having only (x,y) coordinates.
	/// 0: human, 1: helper.
	/// </summary>
	public int[,] playerProperties {get; set;}
	
	/// <summary>
	/// The properties of the NPC.
	/// </summary>
	public int[] monsterProperty;
	public VirtualState(int monsterSize){
		playerProperties = new int[2,2]; // 2 players, 2 properties each
		monsterProperty = new int[monsterSize];
	}
	
	
	public override string ToString(){
		string result = "VirtualState: ";
		result += "[" + playerProperties[0,0] + ", " + playerProperties[0,1] + ", " + 
			playerProperties[1,0] + ", " + playerProperties[1,1] +  "], [";
		
		
		for (int i =0; i<monsterProperty.Length; i++){
			result += monsterProperty[i].ToString() + ", ";
		}
		result += "]";
		
		return result;
	}
	
	// For searching in Dictionary
	public override int GetHashCode() {
        
		int sum = 0;
		
		// sum all and gethastcode
		for (int i=0; i< 2; i++)
			for (int j=0; j<2; j++)
				sum += playerProperties[i, j];
		
		for (int i=0; i< monsterProperty.Length; i++)
			sum += monsterProperty[i];
		
		return sum.GetHashCode();
    }
	
	public override bool Equals(object obj) {
        var otherState = obj as VirtualState;
		
		if (otherState == null)
             return false;
		
		for (int i=0; i< 2; i++){
			for (int j=0; j<2; j++)
				if (playerProperties[i,j] != otherState.playerProperties[i,j])
					return false;
		}
		
		if (monsterProperty.Length != otherState.monsterProperty.Length)
			return false;
		
		for (int i=0; i< monsterProperty.Length; i++)
			if (monsterProperty[i] != otherState.monsterProperty[i])
				return false;
		
		return true;
    }
	
}

/// <summary>
/// Utilities class that has utility routines.
/// </summary>
public static class Utilities{
	
	/// <summary>
	/// The precision used for equality comparison.
	/// </summary>
	public static double precision = 0.0000001;
	
	/// <summary>
	/// Compares two doubles.
	/// </summary>
	/// <returns>
	/// True if equal, false otherwise.
	/// </returns>
	public static bool DoubleEquals(double x1, double x2){
		return (Math.Abs(x1 - x2) <= precision);
	}
	
	/// <summary>
	/// Populates uni-dimensional array <typeparamref name="arr"/> with init value <typeparamref name="value"/>.
	/// </summary>
	public static void Populate<T>(T[] arr, T value ){
		for ( int i = 0; i < arr.Length;i++ ) {
			arr[i] = value;
		}
	}
	
	/// <summary>
	/// Populates two-dimensional array <typeparamref name="arr"/> with init value <typeparamref name="value"/>.
	/// </summary>
	public static void PopulateMatrix<T>(T[,] arr, T value ){
		for ( int i = 0; i < arr.GetLength(0) ;i++ ) {
			for (int j=0; j< arr.GetLength(1); j++)
				arr[i,j] = value;
		}
	}
	
	/// <summary>
	/// Returns the number of elements of <typeparamref name="arr"/> that 
	/// are different from <typeparamref name="value"/>.
	/// </summary>
	/// <returns>
	/// The number of different numbers.
	/// </returns>
	public static int NumDifference<T>(T[] arr, T value){
		int result = 0;
		for ( int i = 0; i < arr.Length;i++ ) {
			if (!arr[i].Equals(value))
				result++;
		}
		return result;
	}
	
	/// <summary>
	/// Normalizes the distribution <typeparamref name="arr"/>.
	/// </summary>
	public static void Normalize(double[] arr){
		double sum = SumDouble(arr);
		
		if (sum != 0){
			for ( int i = 0; i < arr.Length;i++ ) {
				arr[i] /= sum;
			}
		}
	}
	
	
	/// <summary>
	/// Sums the double array <typeparamref name="arr"/>.
	/// </summary>
	/// <returns>
	/// The sum.
	/// </returns>
	public static double SumDouble(double[] arr){
		double result = 0.0;
		for ( int i = 0; i < arr.Length;i++ ) {
			result += arr[i];
		}
		return result;
	}
	
	/// <summary>
	/// Normalizes the matrix <typeparamref name="arr"/> with 
	/// respect to dimension <typeparamref name="dim"/>.
	/// </summary>
	public static void NormalizeMatrix(double[,] arr, int dim){
		if (dim == 0)
			for (int i=0; i<arr.GetLength(0); i++)
				NormalizeDim1(arr, i);
		else
			for (int i=0; i<arr.GetLength(1); i++)
				NormalizeDim0(arr, i);
	}
	
	/// <summary>
	/// Normalizes the matrix <typeparamref name="arr"/> at row/column
	/// <typeparamref name="dim1Index"/> of dimension 0.
	/// </summary>
	public static void NormalizeDim0(double[,] arr, int dim1Index) {
		double sum = 0.0;
		for ( int i = 0; i < arr.GetLength(0);i++ ) {
			sum += arr[i, dim1Index];
		}
		if (sum != 0) {
			for ( int i = 0; i < arr.GetLength(0);i++ ) {
				arr[i, dim1Index] /= sum;
			}
		}
	}
	
	/// <summary>
	/// Normalizes the matrix <typeparamref name="arr"/> at row/column
	/// <typeparamref name="dim0Index"/> of dimension 1.
	/// </summary>
	public static void NormalizeDim1(double[,] arr, int dim0Index) {
		double sum = 0.0;
		for ( int i = 0; i < arr.GetLength(1);i++ ) {
			sum += arr[dim0Index, i];
		}
		if (sum != 0){
			for ( int i = 0; i < arr.GetLength(1);i++ ) {
				arr[dim0Index, i] /= sum;
			}
		}
	}
	
	
	/// <summary>
	/// Animates the movement of GameObject <typeparamref name="go"/>
	/// in direction <typeparamref name="direction"/>.
	/// </summary>
	/// <param name='direction'>
	/// The movement direction.
	/// </param>
	/// <param name='go'>
	/// The targeted GameObject.
	/// </param>
	public static void animateMovement(int direction, GameObject go) {

		switch(direction){
			case (int)GameConstants.actions.east: // 1
				go.transform.Translate(Vector3.right * (1/ (float)GameConstants.NumAnimsPerStep));
				break;
			case (int)GameConstants.actions.south: // 2
				go.transform.Translate(Vector3.forward * (-1/ (float)GameConstants.NumAnimsPerStep));
				break;
			case (int)GameConstants.actions.west: // 3
				go.transform.Translate(Vector3.right * (-1/ (float)GameConstants.NumAnimsPerStep));
				break;
			case (int)GameConstants.actions.north: // 4
				go.transform.Translate(Vector3.forward * (1/ (float)GameConstants.NumAnimsPerStep));
				break;
		}
	}
	
	
	/// <summary>
	/// Returns the string representation of array <typeparamref name=" distrib"/>.
	/// </summary>
	public static string getDistribStr(double[] distrib){
		string result = "";
		foreach (double d in distrib){
			result += d + " ";
		}
		return result;
	}
	
	/// <summary>
	/// Returns the string representation of matrix <typeparamref name=" distrib"/>.
	/// </summary>
	public static string getMatrixStr(double[,] distrib){
		string result = "";
		foreach (double d in distrib){
			result += d + " ";
		}
		return result;
	}
}
