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



#ifndef __SIMULATOR_H
#define __SIMULATOR_H


#include "Model.h"
#include "RandSource.h"
#include "Utilities.h"
#include <vector>

/**
 @class Simulator
 @brief Run a single or multiple simulations (Wee Sun) and single games with human inputs (Huy).
 @details All traces of policy graph were removed. Future versions need to add policy graph handlers.
 @author Wee Sun Lee & Huy Nguyen
 @date December 2010
 */
class Simulator {
public:
  /**
   Constructor.
   @param[in] model MDP model
   */
  Simulator(Model& model) :
    model(model) {
  }
  ;

  /**
   Runs a single simulation from \a startState.
   Does not store simulation trace.
   @param[in] length Simulation length
   @param[out] sumDiscounted Sum of discounted reward
   @param[out] sumReward Sum of reward
   @param[in] startState Initial state of simulation
   @param[in] wBelief Initial world belief
   @param[in] randSource Source of random numbers
   */
  virtual void runSingle(long length, double& sumReward, double& sumDiscounted,
      State startState, std::vector<double>& wBelief, int AI_mode,
      RandSource& randSource);

  /**
   Runs multiple simulations from \a startState.
   @param[in] length Simulation length
   @param[in] num Number of simulation runs. Must be less than \a numStream
   in randSource initialization.
   @param[out] avgReward Average sum of undiscounted reward
   @param[out]  avgDiscounted Average sum of discounted reward
   @param[in] startState Initial state of simulation
   @param[in] randSource Source of random numbers
   */
  void runMultiple(long length, long num, std::vector<double>& rewards, std::vector<
      double>& discountedRewards, State& startState,
      std::vector<double>& initBelief, int AI_mode, RandSource& randSource);

  /**
   Runs a real game with human player from \a startState. States are sent to socket \a connectFd during the process.
   @param[in] length Maximum game turns.
   @param[out] sumReward Sum of undiscounted reward
   @param[out] sumDiscounted Sum of discounted reward
   @param[in] startState Initial game state.
   @param[in] randSource Source of random numbers
   @param[in] connectFd the connected socket to game frontend
   @param[out] humanActionSeq saved sequence of human actions
   @param[in] useXML the messages are formatted in XML
   */
  void runSingleHvAGame(long length, double& sumReward, double& sumDiscounted,
      State startState, std::vector<double>& wBelief, int playerIndex,
      RandSource& randSource, int connectFd, std::vector<char>& humanActionSeq);
  /**
   Runs a real game with human player from \a startState. AI is stupid.
   States are sent to socket \a connectFd during the process.
   @param[in] length Maximum game turns.
   @param[out] sumReward Sum of undiscounted reward
   @param[out] sumDiscounted Sum of discounted reward
   @param[in] startState Initial game state.
   @param[in] randSource Source of random numbers
   @param[in] connectFd the connected socket to game frontend
   @param[out] humanActionSeq saved sequence of human actions
   @param[in] useXML the messages are formatted in XML
   */
  void runSingleHvAGame_stupidAI(long length, double& sumReward,
      double& sumDiscounted, State startState, std::vector<double>& wBelief,
      int playerIndex, RandSource& randSource, int connectFd, bool useXML);
  void runSimAvAGame_stupidAI(long length, double& sumReward,
      double& sumDiscounted, State startState, std::vector<double>& wBelief,
      RandSource& randSource, int connectFd, bool useXML);
  /**
   Runs a simulated game from \a startState. States are sent to socket \a connectFd during the process.
   @param[in] length Maximum game turns.
   @param[out] sumReward Sum of undiscounted reward
   @param[out] sumDiscounted Sum of discounted reward
   @param[in] startState Initial game state.
   @param[in] randSource Source of random numbers
   @param[in] connectFd the connected socket to game frontend
   @param[in] useXML the messages are formatted in XML
   */
  void runSingleSimGame(long length, double& sumReward, double& sumDiscounted,
      State startState, std::vector<double>& wBelief, RandSource& randSource,
      int connectFd, bool useXML);
  /**
   Runs a real game with two human players from \a startState. States are sent to socket \a connectFd during the process.
   TODO: Save human actions' sequence to a file, together with the states. This could help in facilitating Guided AI.
   @param[in] length Maximum game turns.
   @param[out] sumReward Sum of undiscounted reward
   @param[out] sumDiscounted Sum of discounted reward
   @param[in] startState Initial game state.
   @param[in] randSource Source of random numbers
   @param[in] connectFd the connected socket to game frontend
   @param[in] useXML the messages are formatted in XML
   */
  void runSingleHvHGame(long length, double& sumReward, double& sumDiscounted,
      State startState, RandSource& randSource, int connectFd, bool useXML);
  /**
   Runs a real game with two human players from \a startState. States are sent to socket \a connectFd during the process.
   TODO: Save human actions' sequence to a file, together with the states. This could help in facilitating Guided AI.
   @param[in] length Maximum game turns.
   @param[out] sumReward Sum of undiscounted reward
   @param[out] sumDiscounted Sum of discounted reward
   @param[in] startState Initial game state.
   @param[in] wBelief initial world belief.
   @param[in] randSource Source of random numbers
   @param[in] player1Fd the connected socket to player1's game frontend
   @param[in] player2Fd the connected socket to player2's game frontend
   @param[in] useXML the messages are formatted in XML
   @param[in] observer1 indicates player 1 is only observing if set to 1, real playing otherwise.
   @param[in] observer2 indicates player 2 is only observing if set to 1, real playing otherwise.
   */
  void runSingleHvHGameNet(long length, double& sumReward,
      double& sumDiscounted, State startState, std::vector<double>& wBelief,
      RandSource& randSource, int player1Fd, int player2Fd, bool useXML,
      int observer1, int observer2);
  /**
   Runs a simulated game from \a startState with input taken from \a humanActionSeq. States are sent to socket \a connectFd during the process.
   @param[in] length Maximum game turns.
   @param[out] sumReward Sum of undiscounted reward
   @param[out] sumDiscounted Sum of discounted reward
   @param[in] startState Initial game state.
   @param[in] randSource Source of random numbers
   @param[in] connectFd the connected socket to game frontend
   @param[in] humanActionSeq saved sequence of human actions
   @param[in] useXML the messages are formatted in XML
   */
  void runSingleHvASimGame(long length, double& sumReward,
      double& sumDiscounted, State startState, std::vector<double>& wBelief,
      RandSource& randSource, int connectFd, std::vector<char>& humanActionSeq,
      bool useXML);

public:
//private:
  Model& model;

  /**
   This sends the game state to \a sock. If \a useXML is set, the message is formatted as stipulated XML format.
   */
  int sendState(int sock, const State& state, long p1act, long p2act, std::vector<long>& monsterActions,
      long timeLeft, bool useXML, int sock2=0, std::vector<double> *wBelief = 0);

  /**
   Sends the message to indicate the game is over. Without XML set, the message is as follows:
   - 1st number: time left.
   - if 1st number ==0, this means game is over. Then if 2nd number = -2, this means game is over due to time running out. If 2nd number = -1, game is over because the player has reached terminal state (normally this means a win).
   */
  int sendFinishMessage(int sock, int result, const State& state, bool useXML, int sock2=0);
};

#endif //__SIMULATOR_H
