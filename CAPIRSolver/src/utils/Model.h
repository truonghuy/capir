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



#ifndef __MODEL_H
#define __MODEL_H

#include <vector>
#include "RandSource.h"
#include "Utilities.h"
#include <cfloat>
#include <climits>
#include <string.h>

// Constants and types used in the implementation


/**
 @class Model
 @brief Base class for MDP models.
 @details Extend this class to implement a new model.
 See examples in the problems subdirectory.
 States are represented as a vector of doubles.
 Observations are represented as a vector of integers.

 The following needs to be provided or implemented:
 - Discount factor
 - Number of actions, number of macro actions and number of initial policies.
 - Number of state variables, observation variables.
 - A sampler for actions: implements the basic dynamics of the model
 - A sampler for macro actions: uses basic actions to create abstract actions
 - A sampler for initial policies: initializes the solver and provides
 robust fallback.
 - Maximum, minimum of reward in any state
 - Function to indicate whether a state is a terminal state
 - A function restricting allowable actions given an observation: allows
 search to be restricted based on observations

 Take note of the following global definitions that are defined in this file.
 - typedef std::vector<double> State; // shorthand for defining state

 @author Wee Sun Lee
 @date 8 July 2009,
 updated 25 October 2009

 */
class Model {
public:

  /**
   Provides initialization for all parameters.
   @param[in] discount Discount factor for the MDP problem.
   */
  Model(double discount) :
    discount(discount) {
  }
  ;

  virtual void getCurrState(State& state) = 0;

  virtual double realDynamics(const State& currState, long humanAct,
      long aiAct, State& nextState, std::vector<int>& failArray, std::vector<
          int>& terminalArray, std::vector<long>& monsterActions, RandSource& randSource) = 0;

  /**
   Action sampler.
   @return Reward of doing \a act in \a currState.
   @param[in] currState Current state
   @param[out] humanAct the sampled human action
   @param[in] act Index of AI action to perform
   @param[out] nextState Sampled next state. This is random and changes from
   invocation to invocation.
   @param[in] randSource Source of random numbers
   */
  virtual double sample(const State& currState, std::vector<double>& wBelief,
      long& humanAct, long& aiAct, State& nextState, std::vector<long>& monsterActions,
      RandSource& randSource) = 0;

  /**
   Initial policy sampler. These are the initial policies when the solver
   starts solving. They should be somewhat robust as the controller will
   default to initial policies eventually. Also, when an the controller
   sees an observation it has never seen before from the particular
   controller state, it defaults to the initial policy with
   \a policyIndex 0, so that particular policy should be particularly
   robust. As in macro actions, initial policies may have internal states,
   maintained internally or passed to the next time the routine is called
   using \a nextControllerState. Unlike macro actions, initial actions
   keeps running forever, so there is no observation to indicate that it
   has terminated.

   @return Reward of doing policy \a policyIndex in \a currState
   and \a controllerState
   @param[in] currState Current state.
   @param[out] nextState Sampled next state.
   @param[in] randSource Source of random numbers
   */
  virtual double policy(const State& currState, std::vector<double>& wBelief,
      long& p1act, long& p2act, State& nextState, std::vector<long>& monsterActions,
      RandSource& randSource) = 0;

  virtual double policy_stupidAI(const State& currState,
      std::vector<double>& wBelief, long& p1act, long& p2act, State& nextState,
      std::vector<long>& monsterActions, RandSource& randSource, int scriptMode=0)=0;

  virtual double policyHuman(const State& currState,
      std::vector<double>& wBelief, long playerAct, int playerIndex,
      long& aiAct, State& nextState, std::vector<long>& monsterActions, RandSource& randSource) = 0;

  virtual double policyHuman_stupidAI(const State& currState, std::vector<
      double>& wBelief, long playerAct, int playerIndex, long& aiAct,
      State& nextState, std::vector<long>& monsterActions, RandSource& randSource,
      int scriptMode=0) = 0;

  /**
   @return Whether this state a terminal state
   */
  virtual bool isTermState(const State& state) = 0;

  /**
   @return Discount value for the problem.
   */
  inline double getDiscount() {
    return discount;
  }
  ;

  /**
   Output an ascii visual display of the state.
   Implement this to display game on the screen.
   */
  virtual void displayState(const State& state, long type = 0) {
    return;
  }
  ;

  /************** For Simulator ***************/

  /**
   Convert state to a position char array
   */
  virtual char* stateToPosChar(const AugmentedState& state) = 0;
  virtual char* stateToPosCharXML(const AugmentedState& state,
      const int timeLeft = -2) = 0;

  virtual long actionFromChar(const char c, int& humanOrAi) = 0;

  /**
   Render a finish message to send to the GUI.
   Super class should extend this routine and check state and result
   to create a more meaningful finishing message (how does the end state look?)
   Currently, this routine return "0 -2" if the user loses due to time running out, and "0 -1" if he reaches end state before time running out (usually means a win).
   */
  virtual char* getFinishMessage(int result, const State& state) {
    char* msg = new char[5];
    if (result == -2)
      strcpy(msg, "0 -2");
    else
      strcpy(msg, "0 -1");
    msg[4] = '\0';
    return msg;
  }
  ;

  /**
   Render a finish message to send to the GUI.
   Super class should extend this routine and check state and result
   to create a more meaningful finishing message (how does the end state look?)
	0: win, -1: lost, -2: time_up
   */
  virtual char* getFinishMessageXML(int result, const State& state) {
    std::string tempS;
    std::stringstream out;
    char* msg;

    // 1. formality
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

    out << "<state time=\"0\" ended=\"";

    if (result == -2)
      out << "time_up";
    else if (result == -1)
      out << "lost";
    else
    	out << "won";
    out << "\"/>";

    tempS = out.str();
    msg = new char[tempS.size() + 1];
    msg[tempS.size()] = 0;
    memcpy(msg, tempS.c_str(), tempS.size());
    return msg;
  }
  ;

  /**
   Destructor
   */
  virtual ~Model() {
  }
  ;

  //protected:
  double discount; // discount factor for MDP

};

#endif // __MODEL_H
