/*
 * GameRunner.h
 *
 *	GameRunner extends Simulator with a blackbox style running mode.
 *
 *  Created on: Jun 25, 2011
 *      Author: truonghuy
 */

#ifndef GAMERUNNER_H_
#define GAMERUNNER_H_

#include "Simulator.h"
#include<vector>

class MazeWorld;

class GameRunner: public Simulator {
public:
	GameRunner(MazeWorld& model);
	virtual ~GameRunner();

	/**
	 * This routine runs as a black box, receiving state+belief and return next_state+next_belief
	 * +recommneded_actions.
	 *
	 * @param[in] playerFd the socket to communicate with the frontend. I could consider implementing
	 * a UDP styled server so that each socket is not dedicated for any front end.
	 * @param[in] randSource Source of random numbers
	 *
	 * */
	void runGame_HvABlackBox(int playerIndex, int playerFd, RandSource& randSource);

protected:
	MazeWorld& mazeWorld;

	void receiveStateWBelief(int sock, State& state, std::vector<double>& wBelief,
			long& humanAct, const int playerIndex);

	void sendStateWBelief(int sock, const State& state, const std::vector<double>& wBelief,
			const long p1act, const long p2act, const std::vector<long>& monsterActions);
};

#endif /* GAMERUNNER_H_ */
