#pragma once
#include "TheFlyWin32.h"
#include "ActorStateMachine.h"

#define MOVE_FORWARD 0
#define MOVE_BACK 1
#define MOVE_LEFT 2
#define MOVE_RIGHT 3

typedef int DIRECTION_CODE;

class GameControl
{
//protected:
public:
	//ACTORid mainChar;
	ActorStateMachine *mainChar;
	OBJECTid camera;
	float dir[3];
	//float actorOldPosition[3];
	float cameraTargetPos[3];
public:
	GameControl(void);
	virtual ~GameControl(void);
	GameControl(ActorStateMachine * mainChar, OBJECTid camera);

	virtual int DirControl() = 0;
	BOOL CharacterMoveForward(DIRECTION_CODE code);
	BOOL CharacterNormalAttack();
	BOOL CharacterSetIdle();
	int TurnToTargetDir(float degree = 30.0f);
	BOOL isOnTargetDir();
	int GenerateTargetDir(DIRECTION_CODE code);	//return the angle from CalcLeftRight()
	int Rotate(float theta, float vector[2]);
	void CalcLeftRight(DIRECTION_CODE code);	//return the angle the object should turn
	//void CamFallow();
	//void CamBackOff();
	void CamPointToActor();
	int CamRevolution(DIRECTION_CODE code);
	void CamUpdate();
	void InitCamTargetPos(float pos[3]);
	BOOL AppendAttackCode(ATTACK_CODE code);
	void PlayAction(int skip);
private:
	int dir_normalize(float pos[3]);
	BOOL CamInTop;
};

