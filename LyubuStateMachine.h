#pragma once
#include "ActorStateMachine.h"
#include "KeyboardControl.h"
#include "FyMedia.h"
#define LYUBU_ATTACKRANGE 20000.0


class LyubuStateMachine :
	public ActorStateMachine
{
public:
	LyubuStateMachine(void);
	virtual ~LyubuStateMachine(void);
	LyubuStateMachine(ACTORid character, char *ActionFilename);
	virtual BOOL PlayAttackAction(int skip);
	int AttackEnemy(float enemyPos[3]);
protected:
	virtual BOOL UpdateEffectiveAttack();
	AUDIOid audioN1;
	AUDIOid audioN2;
	/*
	BOOL isNowAttackState(void);	//if now is attack state(i.e. var attackState == true), return true
	void resetAttackState(void);	//reset var attackState
	void setAttackState(void);	//set var attackState
	*/
private:
	BOOL attackState;
};

