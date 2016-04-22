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



#include "Utilities.h"
#include "Model.h"
#include <sstream>

long Utilities::getMilsecDiff(struct timeval &startTime, struct timeval &endTime)
{
	long mtime, useconds, seconds;

	seconds = endTime.tv_sec - startTime.tv_sec;
	useconds = endTime.tv_usec - startTime.tv_usec;

	mtime = ((seconds) * 1000 + useconds / 1000.0) + 0.5;

	return mtime;
};

char* Utilities::stateToChar(long timeLeft, Model& model,
    const AugmentedState& currState, bool useXML) {

	/**
	 This gets the positions of human+agent+all monsters
	 */
	char *charArray;
	if (useXML)
		charArray = model.stateToPosCharXML(currState, timeLeft);
	else {
		std::string tempS;
		std::stringstream out;

		charArray = model.stateToPosChar(currState);

		out << timeLeft << " ";
		out << charArray;

		tempS = out.str();
		charArray = new char[tempS.size() + 1];
		charArray[tempS.size()] = 0;
		memcpy(charArray, tempS.c_str(), tempS.size());
	}
	return charArray;
}
;


int Utilities::sendAck(int sock, std::string msg) {
	char cArray[] = "R";

	std::cout << "sending acknowledgement: " << msg << std::endl;
	return send(sock, cArray, strlen(cArray), 0);

}
;

int Utilities::sendCharArray(int sock, char *cArray) {

	//std::cout << "sending: " << cArray << std::endl;
	return send(sock, cArray, strlen(cArray), 0);
}
;

int Utilities::receiveChar(int sock, char& c) {
	char charArray[10];
	int result = recv(sock, charArray, 10, 0);
	c = charArray[0];
	return result;
}
;

int Utilities::receiveCharArray(int sock, char *buffer) {
	return recv(sock, buffer, BUFF_SIZE - 1, 0);
}
;
// TODO --------------------- State Utility functions
/******************* State Utility functions ***************************/
void Utilities::convertState2AugState(const State& state, AugmentedState& augState,
    const long p1act, const long p2act, const std::vector<long>& monsterActions,
    const std::vector<double>* wBelief) {

	// copy state
	augState.playerProperties[humanIndex] = state.playerProperties[humanIndex];
	augState.playerProperties[aiIndex] = state.playerProperties[aiIndex];
	augState.mazeProperties = state.mazeProperties;

	// add actions into the state
	augState.longData.resize(0);
	augState.longData.push_back(p1act);
	augState.longData.push_back(p2act);

	for (unsigned i=0; i< monsterActions.size(); i++){
		augState.longData.push_back(monsterActions[i]);
	}

	if (wBelief)
		for (unsigned i=0; i< wBelief->size(); i++)
			augState.doubleData.push_back((*wBelief)[i]);

}
;
void Utilities::copyState(const State& currState, State& nextState) {

	nextState.playerProperties[humanIndex]
	    = currState.playerProperties[humanIndex];

	nextState.playerProperties[aiIndex] = currState.playerProperties[aiIndex];

	nextState.mazeProperties = currState.mazeProperties;
}
;

void Utilities::printState(const State& state) {

	std::cout << "State: ";
	// players
	for (long i = 0; i < 2; i++) {
		for (unsigned j = 0; j < state.playerProperties[i].size(); j++)
			std::cout << state.playerProperties[i][j] << " ";
	}
	// mazes
	for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
		for (unsigned j = 0; j < state.mazeProperties[i].size(); j++)
			std::cout << state.mazeProperties[i][j] << " ";
	}
	std::cout << std::endl;
}
;

void Utilities::AugStateToOutStream(const AugmentedState& state, std::ostream& outStr){

	// 1. state
	outStr << "State: ";
	for (long i = 0; i < 2; i++) {
		for (unsigned j = 0; j < state.playerProperties[i].size(); j++)
			outStr << state.playerProperties[i][j] << " ";
	}
	// mazes
	for (unsigned i = 0; i < state.mazeProperties.size(); i++) {
		for (unsigned j = 0; j < state.mazeProperties[i].size(); j++)
			outStr << state.mazeProperties[i][j] << " ";
	}
	outStr << "\n";

	// 2. actions
	outStr << "Actions: ";
	for (unsigned i=0; i< state.longData.size(); i++)
		outStr << state.longData[i] << " ";
	outStr << "\n";

	// 3. Belief if any
	outStr << "Belief: ";
	for (unsigned i=0; i< state.doubleData.size(); i++)
		outStr << state.doubleData[i] << " ";
	outStr << "\n";

};

void Utilities::AbstractStateToOutStream(const AbstractState& state, std::ostream& outStr)
{
	// playerProperties
	LongVectorToOutStream(state.playerProperties[0], outStr);
	LongVectorToOutStream(state.playerProperties[1], outStr);

	// monsterProperties
	LongVectorToOutStream(state.monsterProperties, outStr);
	//specialLocation
	LongVectorToOutStream(state.specialLocationProperties, outStr);

};

void Utilities::LongVectorToOutStream(const std::vector<long>& longVector, std::ostream& outStr)
{
	for (unsigned k = 0; k< longVector.size(); k++)
		outStr << longVector[k] << " ";
};

// TODO --------------------- Abstract State Utility routines
/********************* Abstract State Utility routines ************************/
void Utilities::printAbsState(const AbstractState& state) {
	std::cout << "State: " << std::endl;
	// players
	for (long i = 0; i < 2; i++) {
		std::cout << "player" << (i + 1) << ": ";
		for (unsigned j = 0; j < state.playerProperties[i].size(); j++)
			std::cout << state.playerProperties[i][j] << " ";
		std::cout << std::endl;
	}
	// mazes
	std::cout << "monster" << " size " << state.monsterProperties.size() << ":";
	for (unsigned i = 0; i < state.monsterProperties.size(); i++) {
		std::cout << state.monsterProperties[i] << " ";
	}
	for (unsigned i = 0; i < state.specialLocationProperties.size(); i++) {
		std::cout << state.specialLocationProperties[i] << " ";
	}

	std::cout << std::endl;
}
;
void Utilities::cloneAbsState(AbstractState& inState, AbstractState& outState) {
	outState.playerProperties[humanIndex].resize(0);
	outState.playerProperties[aiIndex].resize(0);
	outState.monsterProperties.resize(0);
	outState.specialLocationProperties.resize(0);

	for (unsigned i = 0; i < inState.playerProperties[humanIndex].size(); i++)
		outState.playerProperties[humanIndex].push_back(
		    inState.playerProperties[humanIndex][i]);

	for (unsigned i = 0; i < inState.playerProperties[aiIndex].size(); i++)
		outState.playerProperties[aiIndex].push_back(
		    inState.playerProperties[aiIndex][i]);

	for (unsigned i = 0; i < inState.monsterProperties.size(); i++)
		outState.monsterProperties.push_back(inState.monsterProperties[i]);

	for (unsigned i = 0; i < inState.specialLocationProperties.size(); i++)
		outState.specialLocationProperties.push_back(
		    inState.specialLocationProperties[i]);

}
;

void Utilities::addAbsStateToVector(AbstractState& absState, double prob,
    std::vector<std::pair<AbstractState, double> >& output) {
	long curr = -1;
	for (unsigned l = 0; l < output.size(); l++) {
		if (isEqualAbsState(output[l].first, absState))
			curr = l;
		break;
	}

	// if absState is not in the output vector yet, add it
	if (curr == -1)
		output.push_back(std::pair<AbstractState, double>(absState, prob));

	// else, increase its probability by the amount
	else
		output[curr].second += prob;
}
;

void Utilities::addLongAbsStateToVector(long absState, double prob,
    sparseStateBelief& output) {
	long curr = -1;
	for (unsigned l = 0; l < output.size(); l++) {
		if (output[l].first == absState)
			curr = l;
		break;
	}

	// if absState is not in the output vector yet, add it
	if (curr == -1)
		output.push_back(std::pair<long, double>(absState, prob));

	// else, increase its probability by the amount
	else
		output[curr].second += prob;
}
;

void Utilities::copyAbstractStateArray(std::vector<AbstractState>& input,
    std::vector<AbstractState>& output) {
	output.resize(0);

	for (unsigned k = 0; k < input.size(); k++) {
		output.push_back(input[k]);
	}
}
;

void Utilities::copyAbstractProbArray(
    std::vector<std::pair<AbstractState, double> >& input,
    std::vector<std::pair<AbstractState, double> >& output) {
	output.resize(0);

	for (unsigned k = 0; k < input.size(); k++) {
		output.push_back(input[k]);
	}
}
;

void Utilities::copyAbstractProbArray(
    std::vector<std::pair<AbstractState, double> >& input, double prob,
    std::vector<std::pair<AbstractState, double> >& output) {
	output.resize(0);

	for (unsigned k = 0; k < input.size(); k++) {
		output.push_back(
		    std::pair<AbstractState, double>(input[k].first, input[k].second * prob));
	}
}
;

bool Utilities::isEqualAbsState(const AbstractState& p1,
    const AbstractState& p2) {
	/*
	 assert( (p1.playerProperties[humanIndex].size() == p2.playerProperties[humanIndex].size()) );
	 assert( (p1.playerProperties[aiIndex].size() == p2.playerProperties[aiIndex].size()) );
	 assert( (p1.monsterProperties.size() == p2.monsterProperties.size()) );
	 assert( (p1.specialLocationProperties.size() == p2.specialLocationProperties.size()) );
	 */

	for (unsigned i = 0; i < p1.playerProperties[humanIndex].size(); i++) {
		if (p1.playerProperties[humanIndex][i]
		    != p2.playerProperties[humanIndex][i])
			return false;
	}

	for (unsigned i = 0; i < p1.playerProperties[aiIndex].size(); i++) {
		if (p1.playerProperties[aiIndex][i] != p2.playerProperties[aiIndex][i])
			return false;
	}

	for (unsigned i = 0; i < p1.monsterProperties.size(); i++) {
		if (p1.monsterProperties[i] != p2.monsterProperties[i])
			return false;
	}

	for (unsigned i = 0; i < p1.specialLocationProperties.size(); i++) {
		if (p1.specialLocationProperties[i] != p2.specialLocationProperties[i])
			return false;
	}

	return true;
}
;

/**

 @param reward is the reward already multiplied by the prob.
 */

void Utilities::addAbsStateRewardToVector(AbstractState& absState, double prob,
    std::vector<std::pair<AbstractState, double> >& output, double reward,
    std::vector<double>& rewards) {
	long curr = -1;
	for (unsigned l = 0; l < output.size(); l++) {
		if (isEqualAbsState(output[l].first, absState))
			curr = l;
		break;
	}

	// if absState is not in the output vector yet, add it
	if (curr == -1) {
		output.push_back(std::pair<AbstractState, double>(absState, prob));
		rewards.push_back(reward);
	}
	// else, increase its probability by the amount
	else {
		output[curr].second += prob;
		rewards[curr] += reward;
	}
}
;

void Utilities::absDoNothing(const AbstractState& currAbsState, double prob,
    std::vector<std::pair<AbstractState, double> >& output) {
	output.resize(0);
	output.push_back(std::pair<AbstractState, double>(currAbsState, prob));
}
;

