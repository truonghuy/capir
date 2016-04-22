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



#ifndef __SPECIALLOCATION_H
#define __SPECIALLOCATION_H

#include "ObjectWithProperties.h"

class SpecialLocation : public ObjectWithProperties
{
public:
  long x, y;
public:
  SpecialLocation(long x, long y, long numProperties) : ObjectWithProperties(numProperties), x(x), y(y) {};
  
  /**
    Enumerate properties, i.e. filling the states in input with all possible values of \a agentIndex's properties.
  */
  virtual void enumerateProperties(std::vector< AbstractState >& input, std::vector< AbstractState >& output);
  virtual double absGetReward(AbstractState& state){return NoReward;};
};

#endif
