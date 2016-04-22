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



#include "ObjectWithProperties.h"


void ObjectWithProperties::enumerateProperties(std::vector< long >& currProperties, std::vector< std::vector<long> >& output)
{ 
  output.resize(0);
  output.push_back(currProperties);
  // in-place enumeration
  for (unsigned i=0; i<properties.size(); i++){
    enumeratePropertyAt(output, i);
  }
};

void ObjectWithProperties::enumeratePropertyAt(std::vector< std::vector<long> >& output, long pIndex)
{
  std::vector< std::vector<long> > tempOutput(0);
  std::vector<long> currProp;
  
  while(!output.empty()){
    currProp = output.back();
    output.pop_back();
 
    // Enumerate value of property[pIndex] from 0 to maxValues[pIndex]
    for (long j=0; j<=maxValues[pIndex]; j++){
      //currProp[currProp.size()-properties.size()+pIndex] = j;
      currProp.push_back(j);
      tempOutput.push_back(currProp);
      currProp.pop_back();
    }
  }
  
  for (unsigned i=0; i<tempOutput.size(); i++){
    output.push_back(tempOutput[i]);
  }
  
};
