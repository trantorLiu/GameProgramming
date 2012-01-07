#include "AIControl.h"
#include "ActorStateMachine.h"
using namespace std;
extern char debug[1024];

AIControl::AIControl(int id)
{
	this->lyubuId = id;
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
void AIControl::moveTowardLyubu() {
	for (int i = 0;i< this->npcStateMachineList.size(); i++){
		if (npcStateMachineList[i]->CanBeControl() == FALSE){
			continue;
		}
		int npcId = npcStateMachineList[i]->character;
		FnActor lyubu;
		FnActor npc;
		lyubu.Object(this->lyubuId);
		npc.Object(npcId);

		float lyubuPos[3];
		float npcPos[3];

		lyubu.GetWorldPosition(lyubuPos);
		npc.GetWorldPosition(npcPos);

		float distance;
		distance = sqrt((npcPos[0] - lyubuPos[0]) * (npcPos[0] - lyubuPos[0]) 
			+ (npcPos[1] - lyubuPos[1])	* (npcPos[1] - lyubuPos[1]));
			//+ (npcPos[0] - lyubuPos[0]) * (npcPos[0] - lyubuPos[0]));
		
		//sprintf(debug, "%s distance = %f\n",debug,distance);

		if (distance > ATTACK_DISTANCE && distance < KEEP_TRACK_DISTANCE) {
			//turn toward lyubu
			float newFDir[3], normalize;
			newFDir[0] = lyubuPos[0] - npcPos[0];
			newFDir[1] = lyubuPos[1] - npcPos[1];
			newFDir[2] = lyubuPos[2] - npcPos[2];
			normalize = sqrt(newFDir[0] * newFDir[0] + newFDir[1] * newFDir[1] + newFDir[2] * newFDir[2]);
			newFDir[0] /= normalize;
			newFDir[1] /= normalize;
			newFDir[2] /= normalize;

			float npcFDir[3], npcUDir[3];
			npc.GetWorldDirection(npcFDir, npcUDir);
			npc.SetWorldDirection(newFDir, npcUDir);

			//move forward
			int block = npc.MoveForward(MOVE_DISTANCE,TRUE, FALSE, 0.0, TRUE);
			if (block) {
				//sprintf(debug, "%s npc is blocked\n",debug);
				while (npc.MoveForward(MOVE_DISTANCE,TRUE, FALSE, 0.0, TRUE)) {
					sprintf(debug, "%s npc turn right\n",debug);
					npc.TurnRight(300);
					npc.MoveForward(MOVE_DISTANCE,TRUE, FALSE, 0.0, TRUE);
				}
			}
			npcStateMachineList[i]->ChangeState(STATERUN, FALSE);
		}
		else if (distance <= ATTACK_DISTANCE) {
			//sprintf(debug, "%s distance = %f\n",debug,distance);
			npcStateMachineList[i]->AppendAttackCode(NORMAL_ATT);
		}
		else {
			npcStateMachineList[i]->ChangeState(STATEIDLE);
		}
	}
}
