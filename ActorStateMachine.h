#pragma once
#include "TheFlyWin32.h"
#include <map>
#include <string>

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
#define ULTIMATE_ATT 2
typedef int ATTACK_CODE;

#define OUTSHOT_DIS 200.0
#define MAX_LIFE 1000.0

// actor free meaning it can do anything by the controller.
// actor stay meaning that it can't be move beacuse of being attacked.

class ActorStateMachine
{
public:
	ActorState state;
	ACTORid character;
	int attackKeyQueue[MAXATTACK];
	BOOL newAttack;
	BOOL effectiveAttack;
protected:
	int currentAttackIndex;
	int lastAttackIndex;
	BOOL attackDisable;
	BOOL startAttack;
	int life;
	BOOL initActionIDMap(char *ActionFilename);
	std::map<std::string, ACTIONid> ActionIDMap;
	virtual BOOL PlayAttackAction(int skip);
	BOOL SetNewAction(std::string systemName);
	virtual BOOL UpdateEffectiveAttack();
	OBJECTid bloodID;
	virtual BOOL initLife();
public:
	ActorStateMachine(void);
	virtual ~ActorStateMachine(void);
	ActorStateMachine(ACTORid character, char * ActionFilename);
	//int ChangeStateAndAction();
	int ChangeState(ActorState s, BOOL forceCheck = FALSE);
	BOOL CharacterSetIdle();
	BOOL CanBeControl();
	BOOL CanWalk();
	BOOL CanAttack();
	int Damage();
	BOOL AppendAttackCode(ATTACK_CODE code);
	virtual BOOL PlayAction(int skip);
	BOOL CheckEffectiveAttack();
	virtual int AttackEnemy(float enemyPos[3], BOOL *beOutShot = NULL); // beOutShot will be assgin a value after the call
	virtual void TakeDamage(float damage, BOOL beShot, float* attackerPos);
};

