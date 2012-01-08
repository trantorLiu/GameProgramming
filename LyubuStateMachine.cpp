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

BOOL LyubuStateMachine::PlayAttackAction(int skip){
	// the difference from parent function is that this function contain the sound fx
	FnActor actor;
	actor.Object(this->character);

	//this->setAttackState();

	char attackName[20] = "\0";
	if (this->startAttack == TRUE){// first attack
		this->lastAttackFrame = 0.0f;
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
			this->lastAttackFrame = 0.0f;
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
	if (this->attackKeyQueue[currentAttackIndex] == ULTIMATE_ATT){
		if (frame > 20.0){
			if (frame - this->lastAttackFrame > 10.0f){
				this->lastAttackFrame = frame;
				this->newAttack = TRUE;
			}
			this->effectiveAttack =	TRUE;
		}
	}else if (this->attackKeyQueue[currentAttackIndex] == HEAVY_ATT){
		if (frame > 60.0f){
			this->effectiveAttack = FALSE;
		}else if  (frame > 35.0f && frame <= 60.0f){
			if (frame - this->lastAttackFrame > 30.0f){// trick
				this->lastAttackFrame = frame;
				this->newAttack = TRUE; // another attack
			}
			this->effectiveAttack =	TRUE;
		}else if (frame > 20.0 && frame <= 30.0f){
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

int LyubuStateMachine::AttackEnemy(float enemyPos[3], SHOT_CODE *shot_code){
	// the return value is the attack power
	FnActor actor;
	actor.Object(this->character);
	float attackerPos[3], attackerDir[3];
	actor.GetWorldPosition(attackerPos);
	actor.GetWorldDirection(attackerDir,NULL);
	float frame = actor.QueryCurrentFrame(0);

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

	if (this->attackKeyQueue[currentAttackIndex] == HEAVY_ATT || currentAttackIndex == MAXATTACK -1){
		*shot_code = BIG_SHOT;
	}else {
		*shot_code = SMALL_SHOT;
	}

	if (this->currentAttackIndex == 0){
		if (this->attackKeyQueue[currentAttackIndex] == ULTIMATE_ATT){
			if (frame > 100){ // almost finish attack
				*shot_code = BIG_SHOT;
				sprintf(debug, "%s attack power = %d\n",debug,10);
				return 10;
			}
			*shot_code = STUCK_SHOT;
			//sprintf(debug, "%s attack power = %d\n",debug,1);
			return 1;
		}else if (cosine > 0.8){ // normal or heavy attack, only attack the front side enemy
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