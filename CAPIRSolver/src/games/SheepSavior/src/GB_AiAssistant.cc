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



#include "GB_AiAssistant.h"
#include "GhostBustersLevel.h"

GB_AiAssistant::GB_AiAssistant(long x, long y, GhostBustersLevel* mazeWorld) :
	Player(x, y, 5, 0, 0, 1, 0, mazeWorld) {
	// num move acts = 5, num special acts = 0
};


long GB_AiAssistant::actionFromChar(const char c)
{
  switch(c)
  {
    case 'i':
      return north;
    case 'k':
      return south;
    case 'j':
      return west;
    case 'l':
      return east;
    default:
      return unchanged;
  }
};
