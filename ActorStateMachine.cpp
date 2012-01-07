#include "ActorStateMachine.h"

extern char debug[1024];
extern SCENEid sID;
using namespace std;
#define MOVE_LENGTH 20.0

ActorStateMachine::ActorStateMachine(void)
{
}


ActorStateMachine::~ActorStateMachine(void)
{
	FnScene scene;
	scene.Object(sID);
	scene.DeleteObject(this->bloodID);
}

ActorStateMachine::ActorStateMachine(ACTORid character, char * ActionFilename){
	this->character = character;
	this->state = STATEIDLE;
	this->attackDisable = FALSE;
	this->currentAttackIndex = 0;
	this->lastAttackIndex = 0;
	this->newAttack = FALSE;
	this->effectiveAttack = FALSE;
	this->initActionIDMap(ActionFilename);
	this->initLife();
}

BOOL ActorStateMachine::initLife(){
	this->life = (int) MAX_LIFE;

	FnScene scene;
	scene.Object(sID);
	bloodID = scene.CreateObject(ROOT);
	FnObject blood;
	blood.Object(bloodID);
	//FnBillBoard blood;
	//blood.Object(bloodID, 0);
	FnActor actor;
	actor.Object(this->character);
	OBJECTid baseID = actor.GetBaseObject();

	float pos[3], size[2], color[3];
	pos[0] = 0.0f;
	pos[1] = 0.0f;
	pos[2] = 80.0f;
	size[0] = 50.0f;
	size[1] = 2.0f;
	color[0] = 1.0f; color[1] = color[2] = 0.0f;
	blood.Billboard(pos, size, NULL, 0, color);
	blood.SetParent(baseID);
	return TRUE;
}
BOOL ActorStateMachine::initActionIDMap(char *ActionFilename){
	FILE *fp = fopen(ActionFilename,"r");
	if (fp == NULL){
		sprintf(debug, "%s ActionFilename failed: %s\n", debug, ActionFilename);
		return FALSE;
	}
	char systemName[100];
	char designName[100];
	int ret;

	FnActor actor;
	actor.Object(this->character);
	ACTIONid aID;
	while (!feof(fp)){
		ret = fscanf(fp, "%s %s", systemName, designName);
		if (ret != 2){
			sprintf(debug, "%s fscanf actionID failed\n", debug, systemName, designName);
			return FALSE;
		}
		aID = actor.GetBodyAction(NULL,designName);
		if (aID == FAILED_ID){
			sprintf(debug, "%s init actionID %s %s failed\n", debug, systemName, designName);
			continue;
		}
		string actionName(systemName);
		this->ActionIDMap[actionName] = aID;
	}
	return TRUE;
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

	if (s == STATEIDLE || s == STATERUN || s == STATEDAMAGE || s == STATEDIE){
		if (s == STATEIDLE){
			this->SetNewAction("CombatIdle");
		}else if (s == STATERUN){
			this->SetNewAction("Run");
		}else if (s == STATEDAMAGE){
			FnActor actor;
			actor.Object(this->character);
			actor.MoveForward(-MOVE_LENGTH,TRUE, FALSE, 0.0, TRUE);
			this->SetNewAction("LightDamage");
			this->currentAttackIndex = 0;
			this->lastAttackIndex = 0;
			//this->SetNewAction("HeavyDamage");
		}else if (s == STATEDIE){
			this->SetNewAction("Die");
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
		if (code == ULTIMATE_ATT){
			this->ChangeState(STATEATTACK, TRUE);
			this->attackDisable = TRUE;
			this->currentAttackIndex = 0;
			this->lastAttackIndex = 0;
			//sprintf(debug, "%s get ultimate\n", debug);
		}else{
			this->ChangeState(STATEATTACK);
		}
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
			sprintf(debug, "%s damage end\n",debug);
			this->ChangeState(STATEIDLE);
		}
	}else if (this->state == STATEDIE){
		BOOL ret = actor.Play(0,ONCE, (float)skip, TRUE,TRUE);
		/*
		if (ret == FALSE){
			sprintf(debug, "%s character die\n",debug);
		}*/
	}
	return TRUE;
}

BOOL ActorStateMachine::PlayAttackAction(int skip){
	FnActor actor;
	actor.Object(this->character);
	//ACTIONid actionID;

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

BOOL ActorStateMachine::SetNewAction(string systemName){
	ACTIONid actionID = this->ActionIDMap[systemName];
	
	FnActor actor;
	actor.Object(this->character);
	if (actor.MakeCurrentAction(0,NULL,actionID) == FAILED_ID){
		sprintf(debug, "%s make current action %s fail\n", debug, systemName.c_str());
		return FALSE;
	}else{
		//sprintf(debug, "%s %s make successful\n", debug, systemName.c_str());
	}
	
	if (actor.Play(0,START, 0.0, FALSE,TRUE) == FALSE){
		sprintf(debug, "%s %s play action failed\n", debug, systemName.c_str());
		return FALSE;
	}else{
		//sprintf(debug, "%s %s play successful\n", debug, systemName.c_str());
	}
	return TRUE;
}

BOOL ActorStateMachine::CheckEffectiveAttack(){
	return this->effectiveAttack;
}
BOOL ActorStateMachine::UpdateEffectiveAttack(){
	return FALSE;
}

int ActorStateMachine::AttackEnemy(float enemyPos[3], BOOL *beOutShot){
	// beOutShot is a parameter
	if (beOutShot != NULL){
		*beOutShot = FALSE;
	}
	// the return value is the attack power
	return FALSE;
}

void ActorStateMachine::TakeDamage(float damage, BOOL beShot, float *attackerPos ){
	FnActor actor;
	actor.Object(character);
	float pos[3];
	float dir[3];
	actor.GetWorldPosition(pos);
	actor.GetWorldDirection(dir, NULL);
	if (beShot == TRUE && attackerPos !=NULL){
		float newDir[3];
		newDir[0] = attackerPos[0] - pos[0];
		newDir[1] = attackerPos[1] - pos[1];
		newDir[2] = 0.0f;
		actor.SetWorldDirection(newDir,NULL);
		//if (beShot == TRUE){
			//actor.MoveForward(-OUTSHOT_DIS,TRUE, FALSE, 0.0, TRUE);
		//}
		actor.SetWorldDirection(dir,NULL);
	}
	this->life -= damage;
	sprintf(debug, "%s life=%d\n", debug, this->life);
	if (this->life <= 0) {
		this->ChangeState(STATEDIE, TRUE);
	}else {
		this->ChangeState(STATEDAMAGE, TRUE);
	}

	FnBillBoard blood;
	blood.Object(bloodID, 0);
	float size[2];
	blood.GetSize(size);
	size[0] = life / MAX_LIFE * 50.0f;
	blood.SetSize(size);
}