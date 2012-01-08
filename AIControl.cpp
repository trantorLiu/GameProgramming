#include "AIControl.h"
#include "ActorStateMachine.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace std;
extern char debug[1024];

AIControl::AIControl(ACTORid id)
{
	this->lyubuId = id;
}


AIControl::~AIControl(void)
{
	for (unsigned int i = 0;i< this->npcStateMachineList.size(); i++){
		delete this->npcStateMachineList[i];
	}
}

int AIControl::AddNPC(ACTORid npc, char * ActionFilename){
	ActorStateMachine* stm = new ActorStateMachine(npc, ActionFilename);
	this->npcStateMachineList.push_back(stm);
	stm->life = 10;
	return 0;
}

int AIControl::AddBossNPC(ACTORid npc, char * ActionFilename){
	ActorStateMachine* stm = new ActorStateMachine(npc, ActionFilename);
	this->npcStateMachineList.push_back(stm);
	this->bossStateMachine = stm;
	this->bossStateMachine->life = 100;
	return 0;
}

void AIControl::PlayAction(int skip){
	for (unsigned int i = 0;i< this->npcStateMachineList.size(); i++){
		npcStateMachineList[i]->PlayAction(skip);
	}
}
void AIControl::moveTowardLyubu() {
	for (int i = 0;i< this->npcStateMachineList.size(); i++){
		if (this->npcStateMachineList[i]->state == STATEDIE){
		
		}
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
			//+ (npcPos[2] - lyubuPos[2]) * (npcPos[2] - lyubuPos[2]));
		
		//sprintf(debug, "%s x = %f y = %f z = %f\n",debug, lyubuPos[0], lyubuPos[1], lyubuPos[2]);

		if (distance > ATTACK_DISTANCE && distance < KEEP_TRACK_DISTANCE) {
			//turn toward lyubu
			float newFDir[3], normalize, offset;
			srand ( time(NULL) );
			offset = rand() % NPC_MOVE_OFFSET + 1;
			lyubuPos[0] += offset;
			lyubuPos[1] += offset;
			lyubuPos[2] += offset;

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
			//before attack, turn toward lyubu
			float newFDir[3], normalize, offset;

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

			//sprintf(debug, "%s distance = %f\n",debug,distance);
			npcStateMachineList[i]->AppendAttackCode(NORMAL_ATT);
		}
		else {
			npcStateMachineList[i]->CharacterSetIdle();
		}
	}
}
