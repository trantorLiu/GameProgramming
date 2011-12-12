#include "ActorStateMachine.h"

extern char debug[1024];

ActorStateMachine::ActorStateMachine(void)
{
}


ActorStateMachine::~ActorStateMachine(void)
{
}

ActorStateMachine::ActorStateMachine(ACTORid character){
	this->character = character;
	this->state = STATEIDLE;
	this->attackDisable = FALSE;
	this->currentAttackIndex = 0;
	this->lastAttackIndex = 0;
	this->newAttack = FALSE;
}

BOOL ActorStateMachine::CanAttack(){
	if (this->state == STATEIDLE || this->state == STATERUN){ 
		return TRUE;
	}else if (this->state == STATEATTACK && this->attackDisable == FALSE){
		return TRUE;
	}else{
		return FALSE;
	}
}

BOOL ActorStateMachine::CanBeControl(){
	/*
	if (this->state == STATEATTACK || this->state == STATEBEATTACK){
		return FALSE;
	}else{
		return TRUE;
	}*/
	if (this->state == STATEIDLE || this->state == STATERUN){
		return TRUE;
	}else {
		return FALSE;
	}
}

int ActorStateMachine::ChangeState(ActorState s, BOOL forceSet){
	if (forceSet == FALSE && this->state == s){
		return 0;// keep the past action play
	}else{
		this->state = s;
	}

	if (s == STATEIDLE || s == STATERUN || s == STATEDAMAGE){
		FnActor actor;
		actor.Object(this->character);
		ACTIONid actionID;
		if (s == STATEIDLE){
			actionID = actor.GetBodyAction(NULL,"CombatIdle");
		}else if (s == STATERUN){
			actionID = actor.GetBodyAction(NULL,"RUN");
		}else if (s == STATEDAMAGE){
			actionID = actor.GetBodyAction(NULL,"DAMAGEL");
		}
		if (actionID == FAILED_ID){
			sprintf(debug, "%s get action failed\n", debug);
			return -1;
		}
	
		if (actor.MakeCurrentAction(0,NULL,actionID) == FAILED_ID){
			sprintf(debug, "%s make current fail\n", debug);
		}
	
		if (actor.Play(0,START, 0.0, FALSE,TRUE) == FALSE){
			sprintf(debug, "%s play action failed\n", debug);
		}
	}else if (s == STATEATTACK){
		// Serial attack start;
		this->startAttack = TRUE;
	}
	return 0;
}

BOOL ActorStateMachine::CharacterSetIdle(){
	if (this->CanBeControl() == TRUE){
		this->ChangeState(STATEIDLE);
		return TRUE;
	}else{
		return FALSE;
	}
}

BOOL ActorStateMachine::AppendAttackCode(ATTACK_CODE code){
	if (this->CanAttack() == TRUE){
		this->ChangeState(STATEATTACK);
		this->attackKeyQueue[this->lastAttackIndex] = code;
		this->lastAttackIndex++;
		//sprintf(debug, "%s lastAttackIndex:%d\n", debug,lastAttackIndex);
		if (code == HEAVY_ATT || this->lastAttackIndex >= MAXATTACK){
			//sprintf(debug, "attack disable = true lastAttackIndex:%d\n",lastAttackIndex);
			this->attackDisable = TRUE;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL ActorStateMachine::PlayAction(int skip){
	FnActor actor;
	actor.Object(this->character);
	if (this->CanBeControl() == TRUE){
		actor.Play(0,LOOP, (float)skip, FALSE,TRUE);
	}else if (this->state == STATEATTACK){
		this->PlayAttackAction(skip);
	}else if (this->state == STATEDAMAGE){
		BOOL ret = actor.Play(0,ONCE, (float)skip, TRUE,TRUE);
		if (ret == FALSE){
			sprintf(debug, "%s damagel end\n",debug);
			this->ChangeState(STATEIDLE);
		}
	}
	return TRUE;
}

BOOL ActorStateMachine::PlayAttackAction(int skip){
	//this->
	return FALSE;
}