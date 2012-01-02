#include "LyubuStateMachine.h"

extern char debug[1024];
using namespace std;
extern WORLDid gID;
extern SCENEid sID;

LyubuStateMachine::LyubuStateMachine(void)
{
}


LyubuStateMachine::~LyubuStateMachine(void){
	FnWorld gw;
	gw.Object(gID);
	gw.DeleteAudio(audioN1);
	FnScene scene;
	scene.Object(sID);
	scene.DeleteObject(this->bloodID);
}

LyubuStateMachine::LyubuStateMachine(ACTORid character, char *ActionFilename):ActorStateMachine(character,ActionFilename){
	attackState = TRUE;
	FnWorld gw;
	gw.Object(gID);
	gw.SetAudioPath("Data\\Audio");
	audioN1 = gw.CreateAudio();
	FnAudio audio;
	audio.Object(audioN1);
	BOOL beA = audio.Load("lyubuN01");   // au_bullet.hit1.wav
	if (beA == FALSE){
		sprintf(debug, "%s audioN1 load failed\n", debug);
	}
	audioN2 = gw.CreateAudio();
	audio.Object(audioN2);
	beA = audio.Load("lyubuN02");   // au_bullet.hit1.wav
	if (beA == FALSE){
		sprintf(debug, "%s audioN2 load failed\n", debug);
	}
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
	//duplicate with parrent's PlayAttackAction
	//this function can be removed (in future);
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
			FnAudio audio;
			audio.Object(audioN1);
			audio.Play(ONCE);
			// attackName should be "N1"
		}else if (this->attackKeyQueue[currentAttackIndex] == HEAVY_ATT){
			sprintf(attackName, "H%d", currentAttackIndex + 1); 
			FnAudio audio;
			audio.Object(audioN2);
			audio.Play(ONCE);
			// attackName should be "H1"
		}else if (this->attackKeyQueue[currentAttackIndex] == ULTIMATE_ATT){
			sprintf(attackName, "UAttack"); 
		}
		//sprintf(debug, "%s %s\n", debug, attackName);
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
			FnAudio audio;
			audio.Object(audioN2);
			audio.Play(ONCE);
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
	if (this->attackKeyQueue[currentAttackIndex] == HEAVY_ATT){
		if (frame > 20.0){
			this->effectiveAttack =	TRUE;
		}
	}else if (this->currentAttackIndex > 0){
		if (frame > 10.0){
			this->effectiveAttack =	TRUE;
		}
	}else{
		this->effectiveAttack =	TRUE;
	}
	return FALSE;
}

int LyubuStateMachine::AttackEnemy(float enemyPos[3]){
	// the return value is the attack power
	FnActor actor;
	actor.Object(this->character);
	float attackerPos[3], attackerDir[3];
	actor.GetWorldPosition(attackerPos);
	actor.GetWorldDirection(attackerDir,NULL);

	float dist = 0.0;
	for (int i = 0;i< 3;i++){
		dist += (attackerPos[i] - enemyPos[i]) * (attackerPos[i] - enemyPos[i]);
	}
	//sprintf(debug, "%s dist = %lf\n",debug,dist);
	if ( dist >= LYUBU_ATTACKRANGE ){
		return 0; // no attack power
	}
	float cosine,dotProduct;
	//float v[3];
	dotProduct = 0.0;
	for (int i = 0;i< 3;i++){
		dotProduct += (enemyPos[i] - attackerPos[i]) * attackerDir[i];
	}
	float length = 0.0;
	for (int i = 0;i< 3;i++){
		length += (enemyPos[i] - attackerPos[i])* (enemyPos[i] - attackerPos[i]);
	}
	cosine = dotProduct / sqrt(length);
	//sprintf(debug, "%s cosine = %lf\n",debug,cosine);
	if (this->currentAttackIndex == 0){
		if (this->attackKeyQueue[currentAttackIndex] == ULTIMATE_ATT){
			sprintf(debug, "%s attack power = %d\n",debug,10);
			return 10;
		}else if (cosine > 0.8){
			sprintf(debug, "%s attack power = %d\n",debug,1);
			return 1;
		}
	}else if (this->currentAttackIndex <= 2){
		if (cosine >= 0.0){
			sprintf(debug, "%s attack power = %d\n",debug,2);
			return 2;
		}
	}else if (this->currentAttackIndex <= 3){
		if (cosine >= -1.0){
			sprintf(debug, "%s attack power = %d\n",debug,2);
			return 3;
		}
	}
	return 0;
	
}