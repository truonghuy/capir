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



#include "Simulator.h"
#include "Distribution.h"
#include <iostream>

using namespace std;

void Simulator::runSingle(long length, double& sumDiscounted,
    double& sumReward, State startState, vector<double>& wBelief, int AI_mode,
    RandSource& randSource) {
	// Variables for computing rewards
	double currReward;
	sumDiscounted = 0;
	sumReward = 0;
	double currDiscount = 1;

	// States
	State currState = startState;
	State nextState;

	long humanAct, aiAct;
	vector<long> monsterActions;

	// Run simulation
	for (long t = 0; t < length; t++) {

		// Check for terminal state
		if (model.isTermState(currState)) {
			break;
		}

		// currReward = model.policy(currState, nextState, randSource);
		if (AI_mode)
			currReward = model.policy(currState, wBelief, humanAct, aiAct, nextState,monsterActions,
			    randSource);
		else
			currReward = model.policy_stupidAI(currState, wBelief, humanAct, aiAct,
			    nextState, monsterActions, randSource, 0);

		sumDiscounted += currDiscount * currReward;
		sumReward += currReward;
		currDiscount *= model.getDiscount();
		currState = nextState;
	}
}
;


void Simulator::runMultiple(long length, long num, vector<double>& rewards,
    vector<double>& discountedRewards, State& startState,
    vector<double>& initBelief, int AI_mode, RandSource& randSource) {
	double sumDiscounted;
	double sumReward;
	double totalDiscounted = 0;
	double totalReward = 0;
	vector<double> wBelief;
	discountedRewards.resize(0);
	rewards.resize(0);

	for (long i = 0; i < num; i++) {
		wBelief = initBelief;
		randSource.startStream(i);
		runSingle(length, sumDiscounted, sumReward, startState, wBelief, AI_mode,
		    randSource);
		discountedRewards.push_back(sumDiscounted);
		rewards.push_back(sumReward);
	}
}
;

void Simulator::runSingleHvHGame(long length, double& sumReward,
    double& sumDiscounted, State startState, RandSource& randSource,
    int connectFd, bool useXML) {
	// Variables for computing rewards
	double currReward;
	sumReward = sumDiscounted = 0;
	double currDiscount = 1;

	// dummy
	vector<int> failArray;
	vector<int> terminalArray;

	// States
	State currState = startState; // initialize
	State nextState;

	char temp;
	long humanAct, aiAct;
	vector<long> monsterActions;
	humanAct = aiAct = -1;
	// Run game

	int result = -2;
	long t;

	for (t = 0; t < length; t++) {

		// 1. send currState
		sendState(connectFd, currState, humanAct, aiAct, monsterActions, length - t, useXML);

		// 2. receive an input for action. This can be either by human/assistant
		Utilities::receiveChar(connectFd, temp);

		// 3. Check for terminal state
		if (model.isTermState(currState)) {
			result = -1;
			break;
		}

		// 4. Retrieve action
		int humanIndex = 0, aiIndex = 1;
		int humanOrAi = -1;
		long act = model.actionFromChar(temp, humanOrAi);

		// human action
		if (humanOrAi == humanIndex) {
			humanAct = act;
			Utilities::sendAck(connectFd, "human action received...");
			// now, ai action
			Utilities::receiveChar(connectFd, temp);
			aiAct = model.actionFromChar(temp, aiIndex);
		}
		// ai action
		else if (humanOrAi == aiIndex) {
			aiAct = act;
			Utilities::sendAck(connectFd, "ai action received...");
			// now, human action
			Utilities::receiveChar(connectFd, temp);
			humanAct = model.actionFromChar(temp, humanIndex);
		}
		// not ai/human action, so this is probably a do nothing act
		else {
			long prevAct = act;
			Utilities::sendAck(connectFd, "Weird action received...");
			Utilities::receiveChar(connectFd, temp);
			act = model.actionFromChar(temp, humanOrAi);

			if (humanOrAi == humanIndex) {
				humanAct = act;
				aiAct = prevAct;
			} else // this means either act is also junk, or it's aiAct
			{
				humanAct = prevAct;
				aiAct = act;
			}
		}

		// run policy given human and ai action
		currReward = model.realDynamics(currState, humanAct, aiAct, nextState,
		    failArray, terminalArray, monsterActions,randSource);

		sumReward += currReward;
		sumDiscounted += currDiscount * currReward;
		currDiscount *= model.getDiscount();
		currState = nextState;
	}

	if (t == length)
		sendFinishMessage(connectFd, result, currState, useXML);

}
;
/**
 * observer* = 1: this is observer
 *
 * */
void Simulator::runSingleHvHGameNet(long length, double& sumReward,
    double& sumDiscounted, State startState, vector<double>& wBelief,
    RandSource& randSource, int player1Fd, int player2Fd, bool useXML,
    int observer1, int observer2) {
	// Variables for computing rewards
	double currReward;
	sumReward = sumDiscounted = 0;
	double currDiscount = 1;

	// dummy
	vector<int> failArray;
	vector<int> terminalArray;

	// States
	State currState = startState; // initialize
	State nextState;

	char temp1, temp2;
	long p1Act, p2Act;
	std::vector<long> monsterActions;
	p1Act = p2Act = -1;

	// Run game

	int result = -2;
	long t;
	for (t = 0; t < length; t++) {

		// 1. send currState
		sendState(player1Fd, currState, p1Act, p2Act, monsterActions, length - t, useXML, player2Fd);

		// 2. receive an input for action. This can be either by human/assistant
		Utilities::receiveChar(player1Fd, temp1);
		Utilities::receiveChar(player2Fd, temp2);

		// 3. Check for terminal state
		if (model.isTermState(currState)) {
			result = -1;
			break;
		}
		int humanInd = 0, aiInd = 1;

		if (!observer1)
			p1Act = model.actionFromChar(temp1, humanInd);
		else
			p1Act = -1;
		if (!observer2)
			p2Act = model.actionFromChar(temp2, aiInd);
		else
			p2Act = -1;

		// run policy given human and ai action
		if (p1Act >= 0) {
			if (p2Act >= 0)
				currReward = model.realDynamics(currState, p1Act, p2Act, nextState,
				    failArray, terminalArray, monsterActions, randSource);
			else
				// player 2 is observing
				currReward = model.policyHuman(currState, wBelief, p1Act, humanIndex,
				    p2Act, nextState, monsterActions, randSource);
		} else {
			if (p2Act >= 0) // player 1 is observing
				currReward = model.policyHuman(currState, wBelief, p2Act, aiIndex,
				    p1Act, nextState, monsterActions, randSource);
			else
				// both players are observing
				currReward = model.policy(currState, wBelief, p1Act, p2Act, nextState,
				    monsterActions, randSource);
		}

		sumReward += currReward;
		sumDiscounted += currDiscount * currReward;
		currDiscount *= model.getDiscount();
		currState = nextState;
	}

	if (t == length) {
		sendFinishMessage(player1Fd, result, currState, useXML, player2Fd);

	}

}
;



void Simulator::runSingleHvAGame(long length, double& sumReward,
    double& sumDiscounted, State startState, vector<double>& wBelief,
    int playerIndex, RandSource& randSource, int connectFd,
    vector<char>& humanActionSeq) {
	// Variables for computing rewards
	double currReward;
	sumReward = sumDiscounted = 0;
	double currDiscount = 1;

	humanActionSeq.resize(0);

	// States
	State currState = startState; // initialize
	State nextState;

	char temp;
	long humanAct, aiAct;
	std::vector<long> monsterActions;
	// Run game

	int result = -2;
	long t;
	humanAct = aiAct = -1;
	for (t = 0; t < length; t++) {

		// send currState
		if (playerIndex == humanIndex)
			sendState(connectFd, currState, humanAct, aiAct, monsterActions, length - t, true, 0, &wBelief);
		else
			sendState(connectFd, currState, aiAct, humanAct, monsterActions, length - t, true, 0, &wBelief);

		//std::cin >> temp;
		Utilities::receiveChar(connectFd, temp);

		// Check for terminal state
		if (model.isTermState(currState)) {
			result = -1;
			break;
		}
		humanActionSeq.push_back(temp);

		humanAct = model.actionFromChar(temp, playerIndex);
		// run policy given human action
		// this modifies randSource, so could be thread-unsafe
		// wBelief is updated in-place
		currReward = model.policyHuman(currState, wBelief, humanAct, playerIndex,
		    aiAct, nextState, monsterActions, randSource);

		sumReward += currReward;
		sumDiscounted += currDiscount * currReward;
		currDiscount *= model.getDiscount();
		currState = nextState;
	}

	if (t == length)
		sendFinishMessage(connectFd, result, currState, true);
}
;

void Simulator::runSingleHvAGame_stupidAI(long length, double& sumReward,
    double& sumDiscounted, State startState, std::vector<double>& wBelief,
    int playerIndex, RandSource& randSource, int connectFd, bool useXML) {

	double currReward;
	sumReward = sumDiscounted = 0;
	double currDiscount = 1;

	// States
	State currState = startState; // initialize
	State nextState;

	char temp;
	long humanAct, aiAct;
	std::vector<long> monsterActions;
	// Run game

	int result = -2;
	long t;
	humanAct = aiAct = -1;
	for (t = 0; t < length; t++) {

		// send currState
		if (playerIndex == humanIndex)
			sendState(connectFd, currState, humanAct, aiAct, monsterActions, length - t, useXML);
		else
			sendState(connectFd, currState, aiAct, humanAct, monsterActions, length - t, useXML);

		//std::cin >> temp;
		Utilities::receiveChar(connectFd, temp);

		// Check for terminal state
		if (model.isTermState(currState)) {
			result = -1;
			break;
		}

		humanAct = model.actionFromChar(temp, playerIndex);
		// run policy given human action
		// this modifies randSource, so could be thread-unsafe
		// wBelief is updated in-place
		currReward = model.policyHuman_stupidAI(currState, wBelief, humanAct,
		    playerIndex, aiAct, nextState, monsterActions, randSource);

		sumReward += currReward;
		sumDiscounted += currDiscount * currReward;
		currDiscount *= model.getDiscount();
		currState = nextState;
	}

	if (t == length)
		sendFinishMessage(connectFd, result, currState, useXML);
}
;

void Simulator::runSimAvAGame_stupidAI(long length, double& sumReward,
    double& sumDiscounted, State startState, std::vector<double>& wBelief,
    RandSource& randSource, int connectFd, bool useXML) {

	double currReward;
	sumReward = sumDiscounted = 0;
	double currDiscount = 1;

	// States
	State currState = startState; // initialize
	State nextState;

	char temp;
	long humanAct, aiAct;
	std::vector<long> monsterActions;
	// Run game

	int result = -2;
	long t;
	humanAct = aiAct = -1;
	for (t = 0; t < length; t++) {

		// send currState
		sendState(connectFd, currState, humanAct, aiAct, monsterActions, length - t, useXML);

		//std::cin >> temp;
		Utilities::receiveChar(connectFd, temp);

		// Check for terminal state
		if (model.isTermState(currState)) {
			result = -1;
			break;
		}
		// run policy
		// wBelief is updated in-place
		currReward = model.policy_stupidAI(currState, wBelief, humanAct, aiAct,
		    nextState, monsterActions, randSource, 0);

		sumReward += currReward;
		sumDiscounted += currDiscount * currReward;
		currDiscount *= model.getDiscount();
		currState = nextState;
	}

	if (t == length)
		sendFinishMessage(connectFd, result, currState, useXML);
}
;

void Simulator::runSingleSimGame(long length, double& sumReward,
    double& sumDiscounted, State startState, vector<double>& wBelief,
    RandSource& randSource, int connectFd, bool useXML) {
	// Variables for computing rewards
	double currReward;
	sumReward = sumDiscounted = 0;
	double currDiscount = 1;

	// States
	State currState = startState; // initialize
	State nextState;

	char temp;
	int result = -2;
	long t;
	long p1act, p2act;
	std::vector<long> monsterActions;

	p1act = p2act = -1;
	for (t = 0; t < length; t++) {
		// send currState
		sendState(connectFd, currState, p1act, p2act, monsterActions, length - t, useXML);

		//std::cin >> temp;
		Utilities::receiveChar(connectFd, temp);

		// Check for terminal state
		if (model.isTermState(currState)) {
			result = -1;
			break;
		}

		currReward = model.policy(currState, wBelief, p1act, p2act, nextState, monsterActions,
		    randSource);

		sumReward += currReward;
		sumDiscounted += currDiscount * currReward;
		currDiscount *= model.getDiscount();
		currState = nextState;
	}

	if (t == length)
		sendFinishMessage(connectFd, result, currState, useXML);

}
;

void Simulator::runSingleHvASimGame(long length, double& sumReward,
    double& sumDiscounted, State startState, vector<double>& wBelief,
    RandSource& randSource, int connectFd, vector<char>& humanActionSeq,
    bool useXML) {
	// Variables for computing rewards
	double currReward;
	sumReward = sumDiscounted = 0;
	double currDiscount = 1;

	// States
	State currState = startState; // initialize
	State nextState;

	char temp;
	long humanAct, aiAct;
	std::vector<long> monsterActions;
	humanAct = aiAct = -1;
	// Run game

	int result = -2;
	long t;
	for (t = 0; t < length; t++) {

		// send currState
		sendState(connectFd, currState, humanAct, aiAct, monsterActions, length - t, useXML);

		//std::cin >> temp;
		Utilities::receiveChar(connectFd, temp);

		// Check for terminal state
		if (model.isTermState(currState)) {
			result = -1;
			break;
		}
		int humanOrAi = humanIndex;

		// if the supplied actions are not enough to finish the game, the game is ended as a loss
		if (t >= humanActionSeq.size()) {
			break;
		}
		humanAct = model.actionFromChar(humanActionSeq[t], humanOrAi);
		// run policy given human action
		currReward = model.policyHuman(currState, wBelief, humanAct, humanIndex,
		    aiAct, nextState, monsterActions, randSource);

		sumReward += currReward;
		sumDiscounted += currDiscount * currReward;
		currDiscount *= model.getDiscount();
		currState = nextState;
	}

	if (t == length)
		sendFinishMessage(connectFd, result, currState, useXML);
}
;


/**
 This sends the positions of human+agent+all sheep.
 Ignores result of sending to sock2.
 */

int Simulator::sendState(int sock, const State& state, long p1act, long p2act,
		std::vector<long>& monsterActions, long timeLeft, bool useXML,
		int sock2, vector<double> *wBelief) {
	char *posArray;
	AugmentedState augState;
	Utilities::convertState2AugState(state, augState, p1act, p2act, monsterActions, wBelief);
	posArray = Utilities::stateToChar(timeLeft, model, augState, useXML);
	if (wBelief){
		// print world belief
		cout << "World belief: ";
		Distribution::printDistrib(*wBelief);
		cout << endl;
	}

	int result = Utilities::sendCharArray(sock, posArray);
	if (sock2)
		Utilities::sendCharArray(sock2, posArray);
	delete posArray;
	return result;

}
;

/**
 * Ignores result of sending to sock2.
 * */
int Simulator::sendFinishMessage(int sock, int result, const State& state,
    bool useXML, int sock2) {
	char* msg;
	if (useXML)
		msg = model.getFinishMessageXML(result, state);
	else
		msg = model.getFinishMessage(result, state);

	int sendResult = Utilities::sendCharArray(sock, msg);
	if (sock2)
		Utilities::sendCharArray(sock2, msg);
	delete msg;
	return sendResult;
}
;
