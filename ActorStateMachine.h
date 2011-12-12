#pragma once
#include "TheFlyWin32.h"

#define STATEATTACK 0
#define STATEDAMAGE 1
#define STATEIDLE 2
#define STATERUN 3	
#define STATECOMBATIDEL 4
#define STATEDIE 5

typedef int ActorState;

#define MAXATTACK 4

#define NORMAL_ATT 0
#define HEAVY_ATT 1

typedef int ATTACK_CODE;

// actor free meaning it can do anything by the controller.
// actor stay meaning that it can't be move beacuse of being attacked.

class ActorStateMachine
{
public:
	ActorState state;
	ACTORid character;
	int attackKeyQueue[MAXATTACK];
	BOOL newAttack;
protected:
	int currentAttackIndex;
	int lastAttackIndex;
	BOOL attackDisable;
	BOOL startAttack;
	int life;
public:
	ActorStateMachine(void);
	virtual ~ActorStateMachine(void);
	ActorStateMachine(ACTORid character);
	//int ChangeStateAndAction();
	int ChangeState(ActorState s, BOOL forceCheck = FALSE);
	BOOL CharacterSetIdle();
	BOOL CanBeControl();
	BOOL CanWalk();
	BOOL CanAttack();
	int Damage();
	BOOL AppendAttackCode(ATTACK_CODE code);
	virtual BOOL PlayAction(int skip);
	virtual BOOL PlayAttackAction(int skip);
};

