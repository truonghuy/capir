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



#ifndef __GB_AIASSISTANT_H
#define __GB_AIASSISTANT_H
#define private public
#include "Player.h"
/**
 GB_AiAssistant extends Player. In GhostBusters game, there are fiery men who try to rush to an exit door. This GB_AiAssistant plays a conservative strategy if there are still fiery men in the mazeWorld.
 */

class GhostBustersLevel;

class GB_AiAssistant: public Player {

public:
	/**
	 GB_AiAssistant takes in the mazeWorld in order to get access to the world types to choose the right strategy w.r.t Fiery Worlds, in which it plays conservatively.
	 numWorlds must be set later.
	 */
	GB_AiAssistant(long x, long y, GhostBustersLevel* mazeWorld);

	long actionFromChar(const char c);

};

#endif
