/*
 * GameRunner.cc
 *
 *  Created on: Jun 25, 2011
 *      Author: truonghuy
 */

#include "GameRunner.h"
#include "MazeWorld.h"

GameRunner::GameRunner(MazeWorld& model) : mazeWorld(model), Simulator((Model&) model) {
	// TODO Auto-generated constructor stub

}

GameRunner::~GameRunner() {
	// TODO Auto-generated destructor stub
}


/**
 * This routine always uses XML to communicate.
 *
 * */
void GameRunner::runGame_HvABlackBox(int playerIndex, int playerFd, RandSource& randSource)
{

	State nextState, currState;
	vector<double> wBelief;

	long p1Act, p2Act, humanAct, aiAct;
	std::vector<long> monsterActions;

	p1Act = p2Act = -1;

	// get init belief
	mazeWorld.player[1-playerIndex]->getInitBelief(wBelief);

	// get init state
	mazeWorld.getCurrState(nextState);

	// start
	while(true){

		// 1. send next state+next belief+actions to playerFd
		sendStateWBelief(playerFd, nextState, wBelief, p1Act, p2Act, monsterActions);

		// 2. receive current state and current belief. p1Act and p2Act
		// are optional, just in case front end want to know what the best
		// collab action w.r.t. p1Act OR p2Act. Set to -1 if not given.
		receiveStateWBelief(playerFd, currState, wBelief, humanAct, playerIndex);

		// 3. update next state and belief together with actions. Ignore
		// returned value, the reward.
		model.policyHuman(currState, wBelief, humanAct, playerIndex,
				    aiAct, nextState, monsterActions, randSource);

		// update action
		if (playerIndex == 0){
			p1Act = humanAct;
			p2Act = aiAct;
		}else{
			p2Act = humanAct;
			p1Act = aiAct;
		}

	}

};

/**
 * Assumption: Frontend never sends terminal states to back end
 * (seriously, for what?)
 * Note that state can have different world numbers.
 *
 * First, I just assume that the number of worlds never changes, and the world types
 * stay the same. Later, I'll change this so that the number of worlds and world
 * order is extracted from here. Then the game would be really dynamic.
 *
 * */
void GameRunner::receiveStateWBelief(int sock, State& state, vector<double>& wBelief,
			long& humanAct, const int playerIndex){

	char buffer[BUFF_SIZE];
	// 1. receive message from socket
	Utilities::receiveCharArray(sock, buffer);

	// 2. parse the XML document, i.e. construct state and belief. Get humanAction.
	pugi::xml_document doc;

	pugi::xml_parse_result result = doc.load(buffer);

	if (result){
		// parsing succeeded
		pugi::xml_attribute attr;
		pugi::xml_node node;

		// 2a. player1
		node = doc.child("state").child("player1");

		state.playerProperties[0].resize(0);
		state.playerProperties[0].push_back(node.attribute("x").as_int());
		state.playerProperties[0].push_back(node.attribute("y").as_int());

		// extract action if this is the human
		if (playerIndex == 0)
			humanAct = node.attribute("prev_act").as_int();

		// properties
		for (pugi::xml_node property = node.first_child(); property; property = property.next_sibling())
			state.playerProperties[0].push_back(property.attribute("value").as_int());

		// 2b. player2
		node = doc.child("state").child("player2");

		state.playerProperties[1].resize(0);
		state.playerProperties[1].push_back(node.attribute("x").as_int());
		state.playerProperties[1].push_back(node.attribute("y").as_int());

		// extract action if this is the human
		if (playerIndex == 1)
			humanAct = node.attribute("prev_act").as_int();

		// properties
		for (pugi::xml_node property = node.first_child(); property; property = property.next_sibling())
			state.playerProperties[1].push_back(property.attribute("value").as_int());

		// 2c. worlds. Because I assume the subworld order and types never change, properties can just be
		// pushed to the state.
		pugi::xml_node worlds = doc.child("state").child("worlds");
		wBelief.resize(0);
		state.mazeProperties.resize(0);
		std::vector<long> stateProperties;
		for(pugi::xml_node world = worlds.first_child(); world; world = world.next_sibling()){
			// belief
			wBelief.push_back(world.attribute("belief").as_double());

			// populate stateProperties
			stateProperties.resize(0);
			node = world.child("npc");
			if (node){
				stateProperties.push_back(node.attribute("x").as_int());
				stateProperties.push_back(node.attribute("y").as_int());

				for (pugi::xml_node property = node.first_child(); property; property = property.next_sibling())
					stateProperties.push_back(property.attribute("value").as_int());

			}
			node = world.child("special_location");
			if (node){ // special location does not have x, y

				for (pugi::xml_node property = node.first_child(); property; property = property.next_sibling())
					stateProperties.push_back(property.attribute("value").as_int());
			}

			// push to mazeProperties
			state.mazeProperties.push_back(stateProperties);
		}


	}
	else{
		// failed
		std::cout << "Error description: " << result.description() << "\n";
		std::cout << "Error offset: " << result.offset << " (error at [..." << (buffer + result.offset) << "]\n\n";
	}

};

void GameRunner::sendStateWBelief(int sock, const State& state, const vector<double>& wBelief,
		const long p1act, const long p2act, const std::vector<long>& monsterActions) {

	AugmentedState augState;
	Utilities::convertState2AugState(state, augState, p1act, p2act, monsterActions, &wBelief);

	pugi::xml_document doc;

	// 1. convert AugmentedState to xml_document
	mazeWorld.stateToXMLDoc(doc, augState);

	// 2. convert doc to character array
	char *posChar;
	std::string tempS;
	std::stringstream out;

	// discard all spaces
	doc.save(out, "\t", pugi::format_raw);

	tempS = out.str();
	posChar = new char[tempS.size() + 1];
	posChar[tempS.size()] = 0;
	memcpy(posChar, tempS.c_str(), tempS.size());

	// posArray = Utilities::stateToChar(timeLeft, model, augState, true);
	Utilities::sendCharArray(sock, posChar);
	delete posChar;
}
;
