#include "LyubuStateMachine.h"

extern char debug[1024];
using namespace std;

LyubuStateMachine::LyubuStateMachine(void)
{
}


LyubuStateMachine::~LyubuStateMachine(void)
{
}

LyubuStateMachine::LyubuStateMachine(ACTORid character, char *ActionFilename):ActorStateMachine(character,ActionFilename){
	attackState = TRUE;
}
/*
BOOL LyubuStateMachine::isNowAttackState(void) {	//if now is attack state(i.e. var attackState == true), return true
	return attackState;
}
void LyubuStateMachine::resetAttackState(void) {	//reset var attackState
	attackState = FALSE;
}
void LyubuStateMachine::setAttackState(void) {	//set var attackState
	attackState = TRUE;
}
*/
BOOL LyubuStateMachine::PlayAttackAction(int skip){
	FnActor actor;
	actor.Object(this->character);
	ACTIONid actionID;

	//this->setAttackState();

	char attackName[20] = "\0";
	if (this->startAttack == TRUE){// first attack
		this->startAttack = FALSE; // reset
		//sprintf(debug, "%sstart attack\n", debug);
		if (this->attackKeyQueue[currentAttackIndex] == NORMAL_ATT ){
			sprintf(attackName, "N%d", currentAttackIndex + 1); 
			// attackName should be "N1"
		}else if (this->attackKeyQueue[currentAttackIndex] == HEAVY_ATT){
			sprintf(attackName, "H%d", currentAttackIndex + 1); 
			// attackName should be "H1"
		}
		string systemName(attackName);
		this->SetNewAction(systemName);
		// it performs the new attack
		// set the flag here and BattleRoom will check it.
		this->newAttack = TRUE;	
	}else{
		// the attack name should be refine from reading the file
		BOOL ret = actor.Play(0,ONCE, (float)skip, TRUE,TRUE);
		this->UpdateEffectiveAttack();
		if (ret == FALSE){
			// play the next one
			this->effectiveAttack = FALSE;
			currentAttackIndex++;
			if (currentAttackIndex >= lastAttackIndex){
				// finish attacking
				this->ChangeState(STATEIDLE);// should be change into combatidle;
				this->attackDisable = FALSE;
				currentAttackIndex = 0;
				lastAttackIndex = 0;
				return FALSE;
			}else if (this->attackKeyQueue[currentAttackIndex] == NORMAL_ATT){
				// get the next one attacking pos
				sprintf(attackName, "N%d", currentAttackIndex + 1);
			}else if (this->attackKeyQueue[currentAttackIndex] == HEAVY_ATT){
				// get the next one attacking pos
				sprintf(attackName, "H%d", currentAttackIndex);
			}else{
				sprintf(debug, "%s next Attack fail condition\n", debug);
				return FALSE;
			}
			string systemName(attackName);
			this->SetNewAction(systemName);
			// it performs the new attack
			// set the flag here and BattleRoom will check it and set the flag to FALSE after checking.
			this->newAttack = TRUE;	
		}
	}
	//sprintf(debug, "%s lastAttack%d\n", debug, lastAttackIndex );
	return TRUE;
}

BOOL LyubuStateMachine::UpdateEffectiveAttack(){
	FnActor actor;
	actor.Object(this->character);
	float frame = actor.QueryCurrentFrame(0);
	//sprintf(debug, "%s frame:%lf\n", debug, frame );
	if (this->currentAttackIndex > 0 || this->attackKeyQueue[currentAttackIndex] == HEAVY_ATT){
		if (frame > 10.0){
			this->effectiveAttack =	TRUE;
		}else{
			this->effectiveAttack = FALSE;
		}
	}else{
		this->effectiveAttack =	TRUE;
	}
	return FALSE;
}