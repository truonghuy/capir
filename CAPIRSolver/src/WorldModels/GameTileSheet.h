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



#ifndef __GAMETILESHEET_H
#define __GAMETILESHEET_H


/**
  @class GameTileSheet
  @brief Contains information about the game tile sheet.
  @details Currently the only two virtual functions required are to get the id of wall and ground tiles.
  @author Huy Nguyen
  @date Feb 2011

*/

class GameTileSheet
{
public:
  /**
    @return the id of wall tile
  */
  virtual bool isWallID(int tileId) {return (tileId == 5);};
  /**
    @return the id of ground tile
  */
  virtual bool isGroundID(int tileId) {return (tileId == 1);};
};

#endif
