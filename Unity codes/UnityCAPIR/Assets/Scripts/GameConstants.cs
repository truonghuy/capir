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


/// <summary>
/// This class defines the constants used in SheepSavior game.
/// </summary>
public static class GameConstants {
	
	/// <summary>
	/// Default distance within which NPCs "see" the players.
	/// </summary>
	public const int DefaultVisibility = 3;
	
	/// <summary>
	/// Default hit points of Ghost.
	/// </summary>
	public const int DefaultGhostHP = 2;
	
	/// <summary>
	/// Default distance within which the human-controlled character can shoot ghosts.
	/// </summary>
	public const int DefaultGhostShotDist = 2;
	
	/// <summary>
	/// This multiplicative term increases the difference between 
	/// Q values, because Q values tend to be close to each other.
	/// </summary>
	public const double ActionMult = 10;
	
	/// <summary>
	/// Movement action names. The human/assistant/NPCs have the option not to move.
	/// </summary>
	public enum actions {unchanged, east, south, west, north, shoot}; 
	
	/// <summary>
	/// The relative x movement for each action.
	/// </summary>
	public static readonly int[] RelativeDirX = {0, 1, 0, -1, 0, 0}; 
	/// <summary>
	/// The relative y movement for each action.
	/// </summary>
	public static readonly int[] RelativeDirY = {0, 0, -1, 0, 1, 0}; 
	
	/// <summary>
	/// Number of frames per action animation.
	/// </summary>
	public const int NumAnimsPerStep = 40; // 20;
	
	/* Constants related to the players' actions */
	
	/// <summary>
	/// The number of human actions.
	/// </summary>
	public const int NumAgentActions = 6;
	
	/// <summary>
	/// The number of helper actions.
	/// </summary>
	public const int NumHelperActions = 5;
	
	/* Constants related to map tile */
	public const int Wall = 0;
	public const int Ground = 1;
	public const int SheepPen = 2;
	public const int FieryPen = 3;
	public const int FieryExit = 4;
	
	/**  Constants related to monster types */
	public const int NumNPCTypes = 3;
	
	public const int SheepType = 0;
	public const int GhostType = 1;
	public const int FieryType = 2;
	
	public static int[] numProperties = {2, 3, 2};
	public static string[] NPCTypeStr = {"sheep", "ghost", "fiery"};
		
	/**  Constants related to TMX tile id  */
	public const int GB_GroundId = 1;
	public const int GB_AssistantTileId = 2;
	public const int GB_GhostTileId = 3;
	public const int GB_FieryPenTileId = 4;
	public const int GB_WallId = 5;
	public const int GB_SheepDeterministicTileId = 6;
	public const int GB_FieryTileId = 7;
	public const int GB_FrostyTileId = 8;
	public const int GB_HumanTileId = 9;
	public const int GB_SheepDeterministicPenTileId = 10;
	public const int GB_FieryExitTileId = 11;	
}
