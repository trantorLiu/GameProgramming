#pragma once
//#include "GameControl.h"
#include "ActorStateMachine.h"
#include <vector>

//class AIControl :
	//public GameControl
class AIControl
{
public:
	std::vector<ActorStateMachine *> npcStateMachineList;
	AIControl(void);
	virtual ~AIControl(void);
	int AddNPC(ACTORid ncp, char * ActionFilename);
	void PlayAction(int skip);
};

