/*==============================================================
This is a demo program for TheFly System

Hello !

(C)2004 Chuan-Chang Wang, All Rights Reserved
Created : 0303, 2004, C. Wang

Last Updated : 1010, 2004, C.Wang
===============================================================*/
#include "TheFlyWin32.h"
#include "KeyboardControl.h"
#include "LyubuStateMachine.h"
#include "AIControl.h"
#include "BattleRoom.h"
#include "FyFx.h"
#include "FyMedia.h"

int oldX, oldY, oldXM, oldYM, oldXMM, oldYMM;

WORLDid gID = FAILED_ID;
VIEWPORTid vID;
SCENEid sID;
OBJECTid nID, cID, lID;
OBJECTid tID;
ACTORid lyubu;
ACTORid donzo, robber;
KeyboardControl *kc;
AIControl *npc;
BattleRoom *bRoom;
LyubuStateMachine * lyubuState;

eF3DFX *fx00 = NULL;

char debug[1024] = "\0";
char loopBuff[1024] = "\0";
void debug_message(char*, char*);

void QuitGame(WORLDid, BYTE, BOOL);
void CleanDebugBuff(WORLDid, BYTE, BOOL);
BOOL initLyubu();
BOOL initNPC();
BOOL initBattleRoom(GameControl *player, AIControl *npc);
BOOL initFX();
void CharacterInit();

void Reset(WORLDid gID, BYTE code, BOOL value);
void PlayAction(int skip);
void GameAI(int);
void Render(int);
void GetPosDetail(char *);
BOOL BlindKeys();
void KeyboardAttackCommand(WORLDid gID, BYTE code, BOOL value);

void InitPivot(WORLDid, int, int);
void PivotCam(WORLDid, int, int);
void InitMove(WORLDid, int, int);
void MoveCam(WORLDid, int, int);
void InitZoom(WORLDid, int, int);
void ZoomCam(WORLDid, int, int);

/*------------------
the main program
C.Wang 0308, 2004
-------------------*/
void main(int argc, char **argv)
{
	// create a new world
	gID = FyWin32CreateWorld("GameProgramming 2011 !", 0, 0, 800, 600, 32, FALSE);

	FnWorld gw;
	gw.Object(gID);
	gw.SetObjectPath("Data\\NTU4\\Scenes"); // for terrain loading
	gw.SetScenePath("Data\\NTU4\\Scenes"); // for scene loading
	gw.SetTexturePath("Data\\NTU4\\Scenes\\Textures");
	gw.SetShaderPath("Data\\NTU4\\Shaders");
	
	// create a viewport
	vID = gw.CreateViewport(0, 0, 800, 600);
	FnViewport vp;
	vp.Object(vID);
	vp.SetBackgroundColor(0.3f, 0.4f, 0.5f);

	// create a 3D scene & the 3D entities
	sID = gw.CreateScene(10);

	FnScene scene;
	scene.Object(sID);
	BOOL flag;
	flag = scene.Load("scene2");
	if (flag == FALSE){
		sprintf(debug, "%s scene load fail\n", debug);
	}
	nID = scene.CreateObject(ROOT);
	cID = scene.CreateCamera(ROOT);
	lID = scene.CreateLight(ROOT);

	// create terrain
	tID = scene.CreateTerrain(ROOT);
	if (tID == FAILED_ID){
		exit(-1);
	}
	FnTerrain terrain;
	terrain.Object(tID);
	flag = terrain.Load("terrain");
	if (flag == FALSE){
		sprintf(debug, "%s ground load fail\n", debug);
	}
	flag = terrain.GenerateTerrainData();
	if (flag == FALSE){
		sprintf(debug, "%s ground generate fail\n", debug);
		//exit(-1);
	}
	
	// translate the camera
	FnCamera camera;
	camera.Object(cID);
	camera.Rotate(X_AXIS, 90.0f, LOCAL);
	camera.Translate(0.0f, 10.0f, 100.0f, LOCAL);
	
	CharacterInit();
	// translate the light
	FnLight light;
	light.Object(lID);
	light.SetName("MainLight");
	light.Translate(-50.0f, -50.0f, 50.0f, GLOBAL);

	// set Hotkeys
	BlindKeys();
	
	/* bind a timer, frame rate = 30 fps */
	FyBindTimer(0, 30.0f, GameAI, TRUE);	
	FyBindTimer(1, 30.0f, Render, TRUE);	

	// invoke the system
	FyInvokeTheFly(TRUE);
}

void CharacterInit(){
	initLyubu();
	initNPC();
	initBattleRoom(kc, npc);
	//initFX();
}

BOOL initLyubu(){ // init Lyubu and Camera
	FnWorld gw;
	// create actor:lyubu
	gw.Object(gID);
	gw.SetTexturePath("Data\\NTU4\\Characters");
	gw.SetCharacterPath("Data\\NTU4\\Characters");

	FnScene scene;
	scene.Object(sID);
	lyubu = scene.LoadActor("Lyubu");
	if (lyubu == FAILED_ID){
		return FALSE;
	}
	FnActor actor;
	actor.Object(lyubu);
	float pos[3];
	pos[0] = 3569.0;
	pos[1] = -3210;
	pos[2] = 100;
	actor.SetPosition(pos);
	
	//flag = actor.PutOnTerrain(tID,FALSE,0.0,10.0,10.0,30.0,100.0);
	BOOL flag = actor.PutOnTerrain(tID,FALSE,0.0);

	if (flag == FALSE){
		sprintf(debug, "%s put on fail\n", debug);
		return FALSE;
	}
	// set lyubu idle action
	ACTIONid idleID = actor.GetBodyAction(NULL,"CombatIdle");
	//ACTIONid idleID = actor.GetBodyAction(NULL,"WALK");
	if (idleID == FAILED_ID){
		sprintf(debug, "%s get action failed\n", debug);
		return FALSE;
	}else{
		sprintf(debug, "%s get action success\n", debug);
	}
	
	if (actor.MakeCurrentAction(0,NULL,idleID) == FAILED_ID){
		sprintf(debug, "%s make current fail\n", debug);
	}else{
		sprintf(debug, "%s make action success\n", debug);
	}
	
	if (actor.Play(0,START, 0.0, FALSE,TRUE) == FALSE){
		sprintf(debug, "%s play action failed\n", debug);
	}

	FnCamera camera;
	camera.Object(cID);
	pos[0] = 3569.0;
	pos[1] = -3570;
	pos[2] = 115.0;
	camera.SetWorldPosition(pos);

	float fDir[3];
	float uDir[3];
	fDir[0] = 0.0;
	fDir[1] = 1.0;
	fDir[2] = -0.2;
	uDir[0] = 0.0;
	uDir[1] = 0.2;
	uDir[2] = 1.0;
	
	camera.SetWorldDirection(fDir,uDir);
	sprintf(debug, "%sface:%f %f %f\n", debug,fDir[0],fDir[1],fDir[2]);
	sprintf(debug, "%sup:%f %f %f\n", debug, uDir[0],uDir[1],uDir[2]);


	fDir[2] = 0.0f;
	uDir[1] = 0.0f;
	actor.SetWorldDirection(fDir,uDir);


	actor.GetWorldPosition(pos);
	lyubuState = new LyubuStateMachine(lyubu, "Data\\LyubuAction.txt");
	kc = new KeyboardControl(lyubuState, cID);
	kc->InitCamTargetPos(pos);
	sprintf(debug, "%sactorID=%d cID=%d",debug, lyubu, cID);
	
	return TRUE;
}

BOOL initNPC(){
	FnWorld gw;
	gw.Object(gID);
	gw.SetTexturePath("Data\\NTU4\\Characters");
	gw.SetCharacterPath("Data\\NTU4\\Characters");

	FnScene scene;
	scene.Object(sID);
	donzo = scene.LoadActor("Donzo");
	if (donzo == FAILED_ID){
		return FALSE;
	}
	FnActor actor;
	actor.Object(donzo);
	float pos[3];
	pos[0] = 3569.0;
	pos[1] = -3010;
	pos[2] = 100;
	actor.SetPosition(pos);

	BOOL flag = actor.PutOnTerrain(tID,FALSE,0.0);

	if (flag == FALSE){
		sprintf(debug, "%s put on fail\n", debug);
		return FALSE;
	}
	// set donzo idle action
	ACTIONid idleID = actor.GetBodyAction(NULL,"CombatIdle");
	
	//actor.MakeCurrentAction(0,NULL,idleID,0.0,TRUE);
	//if (actor.MakeCurrentAction(0,NULL,FAILED_ID) == FAILED_ID){
	if (actor.MakeCurrentAction(0,NULL,idleID) == FAILED_ID){
		sprintf(debug, "%s make current fail\n", debug);
	}else{
		sprintf(debug, "%s make action success\n", debug);
	}
	
	if (actor.Play(0,START, 0.0, FALSE,TRUE) == FALSE){
		sprintf(debug, "%s play action failed\n", debug);
	}
	
	robber = scene.LoadActor("Robber02");
	if (robber == FAILED_ID){
		sprintf(debug, "%s Robber02 load fail\n", debug);
		return FALSE;
	}
	FnActor actor_robber;
	actor_robber.Object(robber);
	float pos_robber[3];
	pos_robber[0] = 3569.0;
	pos_robber[1] = -3010;
	pos_robber[2] = 100;
	actor_robber.SetPosition(pos_robber);

	flag = actor_robber.PutOnTerrain(tID,FALSE,0.0);

	if (flag == FALSE){
		return FALSE;
	}
	// set donzo idle action
	ACTIONid idleID_robber = actor_robber.GetBodyAction(NULL,"CombatIdle");
	
	//actor.MakeCurrentAction(0,NULL,idleID,0.0,TRUE);
	//if (actor.MakeCurrentAction(0,NULL,FAILED_ID) == FAILED_ID){
	if (actor_robber.MakeCurrentAction(0,NULL,idleID_robber) == FAILED_ID){
		sprintf(debug, "%s make current fail\n", debug);
	}else{
		sprintf(debug, "%s make action success\n", debug);
	}
	
	if (actor_robber.Play(0,START, 0.0, FALSE,TRUE) == FALSE){
		sprintf(debug, "%s play action failed\n", debug);
	}


	npc = new AIControl();
	npc->AddNPC(donzo,"Data\\DozonAction.txt");
	npc->AddNPC(robber,"Data\\Robber02Action.txt");
	return TRUE;
}

BOOL initBattleRoom(GameControl *player, AIControl *npc){
	bRoom = new BattleRoom(player->mainChar, npc->npcStateMachineList);
	return TRUE;
}

BOOL initFX(){
	FnWorld gw;
	gw.Object(gID);
	gw.SetTexturePath("Data\\FXs\\Textures");
	gw.SetObjectPath("Data\\FXs\\Models");

	fx00 = new eF3DFX(sID);
	fx00->SetWorkPath("Data\\FXs");
	BOOL beOK = fx00->Load("NoPigeon");

	if (beOK == FALSE){
		sprintf(debug, "%s fx load failed\n", debug);
		return FALSE;
	}else{
		sprintf(debug, "%s fx load successful\n", debug);
	}
	float pos[3];
	pos[0] = 3569.0;
	pos[1] = -3210.0;
	pos[2] = 0.0;

	eF3DBaseFX *fx;
	int i, numFX = fx00->NumberFXs();
	for (i = 0; i < numFX; i++) {
		fx = fx00->GetFX(i);
		fx->InitPosition(pos);
	}
	return TRUE;
}

void Reset(WORLDid gID, BYTE code, BOOL value){
	if (code == FY_F1) {
		if (value) {
			FnScene scene;
			scene.Object(sID);
			scene.DeleteActor(lyubu);
			scene.DeleteActor(donzo);
			scene.DeleteActor(robber);
			debug[0] = '\0';
			ActorStateMachine * lyubuState = kc->mainChar;
			delete lyubuState;
			delete kc;
			delete npc;
			delete bRoom;
			if (fx00 != NULL){
				delete fx00;
			}
			fx00 = NULL;
			CharacterInit();
		}
	}	
}

void KeyboardAttackCommand(WORLDid gID, BYTE code, BOOL value){
	if (code == FY_J && FyCheckHotKeyStatus(FY_J) == TRUE){
		kc->AppendAttackCode(NORMAL_ATT);
	}else if(code == FY_K && FyCheckHotKeyStatus(FY_K) == TRUE){
		kc->AppendAttackCode(HEAVY_ATT);
	}else if(code == FY_L && FyCheckHotKeyStatus(FY_L) == TRUE){
		kc->AppendAttackCode(ULTIMATE_ATT);
	}
}

void PlayAction(int skip){
	kc->PlayAction(skip);
	npc->PlayAction(skip);
	if (fx00 != NULL) {
      BOOL beOK = fx00->Play((float) skip);
      if (!beOK) {
         fx00->Reset();  // make it from the starting position and play it again

         // if you just play the FX once, here you need to delete the FX
         // delete fx00;
         // fx00 = NULL;
      }
   }
	/*
	if (lyubuState->isNowAttackState()) {
		kc->CamFallow();
		lyubuState->resetAttackState();
	}
	*/
	//bRoom->RefreshArena();
}

void GetPosDetail(char *buffer){
	FnCamera camera;
	camera.Object(cID);
	float fDir[3];
	float uDir[3];
	float pos[3];
	camera.GetWorldPosition(pos);
	camera.GetWorldDirection(fDir, uDir);
	sprintf(buffer, "carmer pos[0] = %f,pos[1] = %f,pos[2] = %f \n", pos[0],pos[1],pos[2]);
	sprintf(buffer, "%scarmer fDir[0] = %f,fDir[1] = %f,fDir[2] = %f \n", buffer, fDir[0],fDir[1],fDir[2]);
	sprintf(buffer, "%scarmer uDir[0] = %f,uDir[1] = %f,uDir[2] = %f \n", buffer, uDir[0],uDir[1],uDir[2]);
	
	FnActor actor;
	actor.Object(lyubu);
	actor.GetWorldPosition(pos);
	sprintf(buffer, "%slyubu pos[0] = %f,pos[1] = %f,pos[2] = %f \n", buffer, pos[0],pos[1],pos[2]);	
	
}

BOOL BlindKeys(){
	FyDefineHotKey(FY_ESCAPE, QuitGame, FALSE);
	FyDefineHotKey(FY_F1, Reset, FALSE);
	FyDefineHotKey(FY_F2, CleanDebugBuff, FALSE);

	// define some mouse functions
	FyBindMouseFunction(LEFT_MOUSE, InitPivot, PivotCam, NULL, NULL);
	FyBindMouseFunction(MIDDLE_MOUSE, InitZoom, ZoomCam, NULL, NULL);
	FyBindMouseFunction(RIGHT_MOUSE, InitMove, MoveCam, NULL, NULL);

	FyDefineHotKey(FY_J, KeyboardAttackCommand, FALSE);
	FyDefineHotKey(FY_K, KeyboardAttackCommand, FALSE);
	FyDefineHotKey(FY_L, KeyboardAttackCommand, FALSE);
	return TRUE;
}

void CleanDebugBuff(WORLDid gID, BYTE code, BOOL value){
	debug[0] = '\0';
}
//------------------------------------------------------------------------------------
// timer callback function which will be invoked by TheFly3D System every 1/30 second
// C.Wang 0308, 2004
//------------------------------------------------------------------------------------
void GameAI(int skip)
{
	kc->Command();
	bRoom->RefreshArena();
	kc->CamPointToActor();
}

void Render(int skip){
	// render the scene
	FnViewport vp;
	vp.Object(vID);
	vp.Render(cID, TRUE, TRUE);
	PlayAction(skip);
	GetPosDetail(loopBuff);
	debug_message(debug,loopBuff);
	FnWorld gw;
	gw.Object(gID);
	gw.SwapBuffers();	
}
//-------------------
// quit the demo
// C.Wang 0304, 2004
//-------------------
void QuitGame(WORLDid gID, BYTE code, BOOL value)
{
	if (code == FY_ESCAPE) {
		if (value) {
			FyWin32EndWorld(gID);
			delete kc;
		}
	}
}


/*-----------------------------------
initialize the pivot of the camera
C.Wang 0329, 2004
------------------------------------*/
void InitPivot(WORLDid g, int x, int y)
{
	oldX = x;
	oldY = y;
}


/*------------------
pivot the camera
C.Wang 0329, 2004
-------------------*/
void PivotCam(WORLDid g, int x, int y)
{
	FnObject model;

	if (x != oldX) {
		model.Object(cID);
		model.Rotate(Z_AXIS, (float) (x - oldX), GLOBAL);
		oldX = x;
	}

	if (y != oldY) {
		model.Object(cID);
		model.Rotate(X_AXIS, (float) (y - oldY), GLOBAL);
		oldY = y;
	}
}


/*----------------------------------
initialize the move of the camera
C.Wang 0329, 2004
-----------------------------------*/
void InitMove(WORLDid g, int x, int y)
{
	oldXM = x;
	oldYM = y;
}


/*------------------
move the camera
C.Wang 0329, 2004
-------------------*/
void MoveCam(WORLDid g, int x, int y)
{
	if (x != oldXM) {
		FnObject model;

		model.Object(cID);
		model.Translate((float)(x - oldXM)*0.1f, 0.0f, 0.0f, LOCAL);
		oldXM = x;
	}
	if (y != oldYM) {
		FnObject model;

		model.Object(cID);
		model.Translate(0.0f, (float)(oldYM - y)*0.1f, 0.0f, LOCAL);
		oldYM = y;
	}
}


/*----------------------------------
initialize the zoom of the camera
C.Wang 0329, 2004
-----------------------------------*/
void InitZoom(WORLDid g, int x, int y)
{
	oldXMM = x;
	oldYMM = y;
}


/*------------------
zoom the camera
C.Wang 0329, 2004
-------------------*/
void ZoomCam(WORLDid g, int x, int y)
{
	if (x != oldXMM || y != oldYMM) {
		FnObject model;

		model.Object(cID);
		model.Translate(0.0f, 0.0f, (float)(x - oldXMM), LOCAL);
		oldXMM = x;
		oldYMM = y;
	}
}

void debug_message(char *string, char *string2){
	FnWorld gw;
	gw.Object(gID);
	gw.StartMessageDisplay();
	gw.MessageOnScreen(20,20,string2,255,0,0);
	gw.MessageOnScreen(20,200,string,255,0,0);
	gw.FinishMessageDisplay();
}