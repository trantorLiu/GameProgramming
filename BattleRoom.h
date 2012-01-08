#pragma once
#include "TheFlyWin32.h"
#include "GameControl.h"
#include "AIControl.h"
#include "ActorStateMachine.h"

#define BATTLE_RADIUS 25000.0

#include <map>

class BattleRoom
{
protected:
	ActorStateMachine *playerStateMachine;
	std::vector<ActorStateMachine *> npcStateMachineList; // they are candidates, not always in the arean.
	std::vector<ActorStateMachine *> AreanList;
	std::map<ACTORid, BOOL> playerHitMap;
	std::map<ACTORid, BOOL> npcHitMap;
public:
	BattleRoom(void);
	virtual ~BattleRoom(void);
	BattleRoom(ActorStateMachine *playerStateMachine, std::vector<ActorStateMachine *> npcStateMachineList);
	void RefreshArena();
	void PerformAttack();
	BOOL hurt;
protected:
	BOOL JoinArena(ActorStateMachine * npcStm);
	BOOL CheckDistanceAndState(float playerPos[3], float npcPos[3], ActorState playerState,ActorState npcState );
	BOOL AttackCheck(float attackerDir[3], float attackerPos[3], float vitimPos[3], float attackRange);
};

