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



#ifndef __GB_HUMAN_H
#define __GB_HUMAN_H

#include "Player.h"

/**
  GB_Human has 5 move actions, 1 special action and 0 property.

  The sole special action of human is to Shoot. This could only affect Ghosts.

*/

class GB_Human : public Player
{
public:
	static const double sameMonsterProb = 0.9;
  /**** special actions ************/
  static const long Shoot = 5;

public:
  /**
   * numWorlds is not set here
   * */
  GB_Human(long x, long y, MazeWorld * gbLevel) : Player(x, y, 5, 1, 0, 0, 0, gbLevel)
  {

  };

  /******************* Special act routines ********************/
  /**
   * This routine now returns 'r' as shoot
   * */
  long actionFromChar(const char c);

};

#endif
