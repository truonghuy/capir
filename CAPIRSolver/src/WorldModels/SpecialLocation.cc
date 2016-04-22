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



#include "SpecialLocation.h"

void SpecialLocation::enumerateProperties(std::vector< AbstractState >& input, std::vector< AbstractState >& output)
{
  if (properties.empty()){
    Utilities::copyAbstractStateArray(input, output);
    return;
  }
  
  output.resize(0);
  
  AbstractState state;
  
  std::vector< std::vector<long> > tempProperties;
  long currSize;
  
  while (!input.empty()){
    state = input.back();
    input.pop_back();
    
    // 1. Allocate space
    currSize = state.specialLocationProperties.size();
    state.specialLocationProperties.resize(currSize + properties.size(), 0);
    
    // 2. Enumerate properties and place in tempProperties
    ObjectWithProperties::enumerateProperties(state.specialLocationProperties, tempProperties);
    
    // 3. Add to output
    for (unsigned j=0; j<tempProperties.size(); j++){
      state.specialLocationProperties = tempProperties[j];
      output.push_back(state);
    }   
  }
};
