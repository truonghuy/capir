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



#ifndef __OBJECTWITHPROPERTIES_H
#define __OBJECTWITHPROPERTIES_H

#include "Utilities.h"

/**
  @class ObjectWithProperties
  @brief This class defines objects with properties and able to react.
  @details Base class of Agent and SpecialLocation.
  
  @author Huy Nguyen
  @date Dec 2010
*/
class ObjectWithProperties
{
public:
  /**
    Names of properties.
  */
  std::vector<std::string> propertyNames;
  /**
    Initial values of properties.
  */
  std::vector<long> properties;
  /**
    Maximum values of properties.
  */
  std::vector<long> maxValues;
public:
  /**
    Constructor.
  */
  ObjectWithProperties(long numProperties = 0)
  {
    properties.resize(numProperties, 0); // by default, initial values for properties are 0
    maxValues.resize(numProperties, 1); // by default there are two values for every properties
    propertyNames.resize(numProperties);
  };
  ~ObjectWithProperties() {};
  
  /**
    Initialize suitable max values of properties. 
  */
  virtual void initialMaxValues() {};
  
  /**
    Gets initial value of properties at \a index.
  */
  inline long getPropertyAt(long index)
  {
    assert( index < properties.size() );
    return properties[index]; 
  };
  /**
    Gets name of properties at \a index.
  */
  inline std::string getPropertyNameAt(long index)
  {
    assert( index < properties.size() );
    return propertyNames[index]; 
  };
  /**
    Sets initial value of properties at \a index to \a value.
  */
  inline long setPropertyAt(long index, long value)
  { 
    assert( index < properties.size() );
    properties[index]=value; 
  };
  /**
    Gets number of properties.
  */
  inline long getNumProperties(){ return properties.size();};
  
  /**
    Enumerate properties, i.e. filling the states in input with all possible values of involved properties.
    @param[in] currProperties the partially filled array of properties.
    @param[out] output the array of filled properties.
  */
  virtual void enumerateProperties(std::vector< long >& currProperties, std::vector< std::vector<long> >& output);
  
  /**
    Enumerate property at index \a pIndex in an in-place manner.
    @param[out] output
  */
  void enumeratePropertyAt(std::vector< std::vector<long> >& output, long pIndex);
  /**
    Reacts at planning stage given players' actions \a humanAct and \a aiAct. This routine is used in Monster and SpecialLocation. By default, does nothing. 
  */
  virtual void absReact(std::vector< std::pair<AbstractState, double> >& input, long humanAct, long aiAct, std::vector< std::pair<AbstractState, double> >& output, const AbstractState& prevAbsState)
  { Utilities::copyAbstractProbArray(input, output); };
  
  /**
    Returns reward given AbtractState.
    @returns NoReward by default.
  */
  virtual double absGetReward(AbstractState& state){ return NoReward;};
  /**
    Returns reward given current State.
    @returns NoReward by default.
  */
  virtual double getReward(const State& state, int worldNum){ return NoReward;};
  
  /************** Raw State related ***************/
  /**
    Amend \a propertiesState with initial property values.
  */
  virtual void getCurrState(std::vector<long>& propertiesState)
  {
    for (unsigned i=0; i<properties.size(); i++)
      propertiesState.push_back(properties[i]);   
  };
 
};

#endif
