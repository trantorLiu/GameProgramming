#include "AIControl.h"


AIControl::AIControl(void)
{
}


AIControl::~AIControl(void)
{
	for (int i = 0;i< this->npcStateMachineList.size(); i++){
		delete this->npcStateMachineList[i];
	}
}

int AIControl::AddNPC(ACTORid npc, char * ActionFilename){
	ActorStateMachine* stm = new ActorStateMachine(npc, ActionFilename);
	this->npcStateMachineList.push_back(stm);
	return 0;
}

void AIControl::PlayAction(int skip){
	for (int i = 0;i< this->npcStateMachineList.size(); i++){
		npcStateMachineList[i]->PlayAction(skip);
	}
}