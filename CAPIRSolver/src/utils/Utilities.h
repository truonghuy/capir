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



#ifndef __UTILITIES_H
#define __UTILITIES_H

#include <sstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <cassert>
#include <string>
#include <climits>
#include <typeinfo>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
  @class Utilities
  @brief Contains utility functions pertinent to manipulation of AbstractState and State.
  @author Huy Nguyen
  @date Dec 2010

*/

/*** Forward declarations***/
class Model;


/************** Constants **********************/

#define __epsilon 0.00001

const int humanIndex = 0;
const int aiIndex = 1;
/**
  Terminal state's value.
*/
const long TermState = INT_MIN;
/**
  Terminal state's index. Used for mappings between terminal state and its index
*/
const long longTermState = 0; 
/**
  Region type
*/
enum regionType{junction, horizon, vertical}; 

const double NoReward = 0;

const double HumanRewardBias = 0.5;

const int BUFF_SIZE = 2000;

/************** Structs/typedefs ***************/


template<class T>
class Stats {
  static long instance_count;
public:
  Stats() {
    instance_count++;
  }
  ~Stats() {
    instance_count--;
  }
  static void print() {
    std::cout << instance_count << " instances of " << typeid(T).name() <<
        ", " << sizeof(T) << " bytes each." << std::endl;
  }
};

template<class T>
long Stats<T>::instance_count = 0;

struct State : Stats<State>
{
  // humanProperties: (coordX, coordY, <additional properties>)
  std::vector<long> playerProperties[2];
  
  // (coordX, coordY, <additional properties>)
  std::vector< std::vector<long> > mazeProperties;

  inline bool operator == (const State &o) const {
	// playerProperties
	for (long i=0; i<2; i++){
		for (unsigned j=0; j< playerProperties[i].size(); j++){
		  if (playerProperties[i][j] != o.playerProperties[i][j])
			return false;
		}
	}

	// mazeProperties
	for (unsigned i=0; i< mazeProperties.size(); i++){
		for (unsigned j=0; j< mazeProperties[i].size(); j++){
		  if (mazeProperties[i][j] != o.mazeProperties[i][j])
			return false;
		}
	}

	return true;
  }

};

/**
 * This struct augments \a State with added data such as previous players' and npcs' actions.
 * */
struct AugmentedState : State
{
  std::vector<long> longData; // used for prev actions
  std::vector<double> doubleData; // used for belief
};

struct AbstractState : Stats<AbstractState>
{
  // humanProperties: (regionId, coordX, coordY, <additional properties>)
  std::vector<long> playerProperties[2];
  
  // if monsterProperties is not empty: 
  // (coordX, coordY, seesHuman, seesAssistant, <additional properties>)
  std::vector<long> monsterProperties;
  
  // if specialLocationProperties is not empty: 
  // (<additional properties>)
  std::vector<long> specialLocationProperties;

};
 
/**
  Shorthand for sparse belief representation. long is the state index, double is the probability
*/ 
typedef std::vector<std::pair<long, double> > sparseStateBelief; 

/***************** Class ***********************/

class Utilities
{
public:

  /*********************************
      Constants
  
  **********************************/
  /**
    The goal of the game
    - or: Reached if one of the virtual worlds is solved. 
    - and: Reached only when all virtual worlds reach terminal.
  */
  enum goalType{orType, andType};


  /****************** Comparator ***********************/
  class StateComparator {
    public:
      // return false if p1 < p2, true otherwise
      bool operator()(const State& p1, const State& p2) const
      {
        
        // playerProperties
        for (long i=0; i<2; i++){
          for (unsigned j=0; j< p1.playerProperties[i].size(); j++){
            if (p1.playerProperties[i][j] < p2.playerProperties[i][j])
              return true;
            if (p1.playerProperties[i][j] > p2.playerProperties[i][j])
              return false;
          }
        }
        
        // mazeProperties
        for (unsigned i=0; i< p1.mazeProperties.size(); i++){
          for (unsigned j=0; j< p1.mazeProperties[i].size(); j++){
            if (p1.mazeProperties[i][j] < p2.mazeProperties[i][j])
              return true;
            if (p1.mazeProperties[i][j] > p2.mazeProperties[i][j])
              return false;
          }
        }
        
        return false;
      };
  };
  
  
  class AbstractStateComparator {
    public:
      // return false if p1 < p2, true otherwise
      bool operator()(const AbstractState& p1, const AbstractState& p2) const
      {
        
        // playerProperties
        for (long i=0; i<2; i++){
          for (unsigned j=0; j< p1.playerProperties[i].size(); j++){
            if (p1.playerProperties[i][j] < p2.playerProperties[i][j])
              return true;
            if (p1.playerProperties[i][j] > p2.playerProperties[i][j])
              return false;
          }
        }
        
        // monsterProperties
        for (unsigned j=0; j< p1.monsterProperties.size(); j++){
          if (p1.monsterProperties[j] < p2.monsterProperties[j])
            return true;
          if (p1.monsterProperties[j] > p2.monsterProperties[j])
            return false;
        }
        
        // specialLocation 
        for (unsigned j=0; j< p1.specialLocationProperties.size(); j++){
          if (p1.specialLocationProperties[j] < p2.specialLocationProperties[j])
            return true;
          if (p1.specialLocationProperties[j] > p2.specialLocationProperties[j])
            return false;
        }
        
        return false;
      };
  };
  
  
public:

  static long getMilsecDiff(struct timeval &startTime, struct timeval &endTime);

  /**
    Returns a char array containing the time left for this level + current state
    @param[in] timeLeft time left in the game.
    @param[in] model the game's model/level.
    @param[in] currState current game State.
    @param[in] useXML if set to true, returns the XML message instead of raw string.
  */
  static char* stateToChar( long timeLeft, Model& model, const AugmentedState& currState, bool useXML);

  /**
      Sends acknowledgement \a msg to socket \a sock.
  */
  static int sendAck(int sock, std::string msg);

  /**
    Sends char array \a cArray to socket \a sock.
  */
  static int sendCharArray(int sock, char *cArray);
  /**
    Receives one character \a c from socket \a sock.
  */
  static int receiveChar(int sock, char& c);

  /**
   * Receive char array.
   * */
  static int receiveCharArray(int sock, char *buffer);
  

  /******************* State Utility functions ***************************/
  static void convertState2AugState(const State& state, AugmentedState& augState,
		  const long p1act, const long p2act, const std::vector<long>& monsterActions,
		  const std::vector<double>* wBelief = NULL);
  /**
    Copies from \a currState to \a nextState.
  */
  static void copyState(const State& currState, State& nextState);
  /**
    Prints State \a state in the command terminal.
  */
  static void printState(const State& state);
  
  static void AugStateToOutStream(const AugmentedState& state, std::ostream& outStr);
  static void AbstractStateToOutStream(const AbstractState& state, std::ostream& outStr);
  static void LongVectorToOutStream(const std::vector<long>& state, std::ostream& outStr);

  /************ Abstract State Utility routines ************************/
  /**
    Prints AbstractState \a state in the command terminal.
  */
  static void printAbsState(const AbstractState& state);
  /**
    Copies from AbstractState \a inState to \a outState.
  */
  static void cloneAbsState(AbstractState& inState, AbstractState& outState);
  /**
    Adds AbstractState \a absState with probability \a prob to vector \a output. If \a absState already exists in \a output, its probability wil be merged.
  */
  static void addAbsStateToVector(AbstractState& absState, double prob, std::vector< std::pair<AbstractState, double> >& output);
  /**
    Adds long form of AbstractState \a absState with probability \a prob to the state belief \a output. If \a absState already exists in \a output, its probability wil be merged.
  */
  static void addLongAbsStateToVector(long absState, double prob, sparseStateBelief& output);
  /**
    Copies AbstractState vector \a input to \a output.
  */
  static void copyAbstractStateArray(std::vector< AbstractState >& input, std::vector< AbstractState >& output);  
  /**
    Copies vectors of AbstractState with probability from \a input to \a output.
  */
  static void copyAbstractProbArray(std::vector< std::pair<AbstractState, double> >& input, std::vector< std::pair<AbstractState, double> >& output);  
  
  /**
    Similar to copyAbstractProbArray but multiplies the probabilities in input by \a prob to produce \a output.
  */
  static void copyAbstractProbArray(std::vector< std::pair<AbstractState, double> >& input, double prob, std::vector< std::pair<AbstractState, double> >& output);  
  /**
    Compares AbstractState \a p1 and \a p2.
  */
  static bool isEqualAbsState(const AbstractState& p1, const AbstractState& p2);
  
  /**
    Adds AbstractState \a absState with probability \a prob to vector \a output and respective \a reward to vector \a rewards. If \a absState already exists in \a output, its probability and reward wil be merged in corresponding vectors.
  */
  static void addAbsStateRewardToVector(AbstractState& absState, double prob, std::vector< std::pair<AbstractState, double> >& output, double reward, std::vector <double>& rewards);

  /**
    When planning, adds \a currAbsState with probability \a prob to empty vector \a output.
  */
  static void absDoNothing(const AbstractState& currAbsState, double prob, std::vector< std::pair<AbstractState, double> >& output);

};
#endif
