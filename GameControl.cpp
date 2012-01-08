#include "GameControl.h"
#include <math.h>

#define FLOAT_ERROR 0.001
#define MOVE_LENGTH 10.0
#define CAM_MIN_HIGHT 115.0f
#define CAM_MAX_HIGHT 300.0f
extern char debug[1024];
extern OBJECTid tID;

#define DISTANCE 360.0f
#define RADIUS 25.0f
#define RADIUS2 625.0f
//extern ACTORid lyubu;

BOOL firstBlock = TRUE;

GameControl::GameControl(void)
{
}


GameControl::~GameControl(void)
{
}

GameControl::GameControl(ActorStateMachine * character, OBJECTid camera){
	this->camera = camera;
	this->mainChar = character;
	this->CamInTop = FALSE;
}

BOOL GameControl::CharacterMoveForward(DIRECTION_CODE code){
	FnActor actor;
	actor.Object(this->mainChar->character);
	BOOL beBlock;
	int ret = actor.MoveForward(MOVE_LENGTH,TRUE, FALSE, 0.0, TRUE);
	this->mainChar->ChangeState(STATERUN);

	if (ret == 0){
		beBlock = FALSE;
	}else{
		beBlock = TRUE;
	}

	// please check "beBlock" and decide what to do.
	if ((code == MOVE_LEFT || code == MOVE_RIGHT) && beBlock == TRUE){
		GameControl::CamRevolution(code);
	}
	
	if (ret != 0){ // ret will be -1 if the character hit the wall 
		return FALSE;
	}else{
		return TRUE;
	}
}

BOOL GameControl::AppendAttackCode(ATTACK_CODE code){
	return this->mainChar->AppendAttackCode(code);
}

BOOL GameControl::CharacterNormalAttack(){
	/*
	if (this->mainChar->state == STATEDAMAGE){
		return FALSE;
	}
	this->mainChar->ChangeState(STATEATTACK);
	sprintf(debug ,"%d attacking\n",this->mainChar->character);
	*/
	return TRUE;
}

/*
BOOL GameControl::CharacterSetIdle(){
	if (this->mainChar->CanBeControl){
		this->mainChar->ChangeState(STATEIDLE);
		return TRUE;
	}
	return TRUE;
}
*/

int GameControl::GenerateTargetDir(DIRECTION_CODE code){
	FnCamera cam;
	cam.Object(this->camera);
	float camPos[3];
	cam.GetWorldPosition(camPos);

	//FnActor act;
	//act.Object(this->mainChar->character);
	//float actPos[3];
	//act.GetWorldPosition(actPos);
	if (code == MOVE_FORWARD){
		for (int i = 0; i< 2;i++) {
			//this->dir[i] = actPos[i] - camPos[i];
			this->dir[i] = this->cameraTargetPos[i] - camPos[i];
		}
		this->dir[2] = 0.0f;
		this->dir_normalize(this->dir);
	}else if (code == MOVE_BACK){
		for (int i = 0; i< 2;i++) {
			this->dir[i] = camPos[i] - this->cameraTargetPos[i] ;
		}
		this->dir[2] = 0.0f;
		this->dir_normalize(this->dir);
	}else if (code == MOVE_LEFT || code == MOVE_RIGHT){
		this->CalcLeftRight(code);
	}
	return 0;
}

BOOL GameControl::isOnTargetDir(){
	FnActor act;
	act.Object(this->mainChar->character);
	float actFdir[3];
	float actUdir[3];
	act.GetWorldDirection(actFdir,actUdir);
	if ( (abs(this->dir[0] - actFdir[0]) > FLOAT_ERROR) || (abs(this->dir[1] - actFdir[1]) > FLOAT_ERROR) ){
		return FALSE;
	}else{
		return TRUE;
	}
}

int GameControl::TurnToTargetDir(float degree){
	// this function only works in the case that up direction of actor is in positive-z

	// if the gap bigger than 5 degrees, then turn 5 degrees
	// else set the facing dir to the target dir directly
	FnActor act;
	act.Object(this->mainChar->character);
	float actFdir[3];
	float actUdir[3];
	act.GetWorldDirection(actFdir,actUdir);

	if ( abs(actUdir[2] - 1.0f) > FLOAT_ERROR){
		// the actor is not z-up
		sprintf(debug ,"udir=%f\n", actUdir[2]);
		return -1;
	}
	// calculate the gap by arc cosine
	float dot = 0.0f;
	for (int i = 0;i<3;i++){
		dot += actFdir[i] * this->dir[i];
	}
#define PI 3.1415926
	if ( acos(dot) * 180 / PI < degree){
		//sprintf(debug, "%s small than degree(%f)\n", debug,degree);
		act.SetWorldDirection(this->dir, NULL);
		return 0;
	}

	float cross = actFdir[0] * this->dir[1] - actFdir[1] * this->dir[0];
	//sprintf(debug, "%sface:%f %f %f\n", debug,actFdir[0],actFdir[1],actFdir[2]);
	//sprintf(debug, "%straget:%f %f %f\n", debug, this->dir[0],this->dir[1],this->dir[2]);
	//sprintf(debug, "%scross:%f\n", debug, cross);
	if (cross > 0.0f){// up, turn left
		act.TurnRight(-degree);
	}else{
		act.TurnRight(degree);
	}
	return 0;
}

int GameControl::dir_normalize(float pos[3]){
	float sum = 0.0f;
	for (int i = 0; i< 3;i++) {
		sum += pos[i] * pos[i];
	}
	sum = sqrt(sum);
	if (sum == 0.0f){
		return -1;
	}
	for (int i = 0; i< 3;i++) {
		pos[i] = pos[i] / sum;
	}
	return 0;
}

void GameControl::CalcLeftRight(DIRECTION_CODE code){
	FnActor act;
	act.Object(this->mainChar->character);
	float aPos[3];
	act.GetWorldPosition(aPos);

	FnCamera cam;
	cam.Object(this->camera);
	float cPos[3];
	cam.GetWorldPosition(cPos);

	float orig[2];
	float newVector[2];
	float r = 0.0f;
	for (int i = 0;i<2;i++){
		orig[i] = aPos[i] - cPos[i];
		newVector[i] = orig[i];
		r += orig[i] * orig[i];
	}
	r = sqrt(r);

	float theta;
	theta = asin( MOVE_LENGTH / (2 * r) );
	theta *= 2;

	if (code == MOVE_LEFT){
		this->Rotate(theta, newVector);
	}else if(code == MOVE_RIGHT){
		this->Rotate(-theta, newVector);
	}

	for (int i = 0;i<2;i++){
		this->dir[i] = newVector[i] - orig[i];
	}
	this->dir[2] = 0.0f;

	this->dir_normalize(this->dir);
	//sprintf(debug, "%sdir:%f %f %f\n", debug, this->dir[0], this->dir[1], this->dir[2]);
}

int GameControl::Rotate(float theta, float vector[2]){
	// it will re-set vector
	float x;
	float y;
	x = cos(theta) * vector[0] - sin(theta) * vector[1]; 
	y = sin(theta) * vector[0] + cos(theta) * vector[1];
	vector[0] = x;
	vector[1] = y;
	//sprintf(debug, "%stheta:%f v0:%f v1:%f\n", debug, theta,vector[0], vector[1]);
	return 0;
}

void GameControl::CamPointToActor() {
	//sprintf(debug ,"in point to actor\n");
	this->CamUpdate();

	float cPos[3];
	float aFDir[3], aUDir[3];
	float vec[3] = {0};	//vector point to actor from camera

	FnActor act;
	act.Object(this->mainChar->character);
	//act.GetWorldPosition(aPos);
	
	act.GetWorldDirection(aFDir, aUDir);
	
	FnCamera cam;
	cam.Object(this->camera);
	cam.GetWorldPosition(cPos);
	
	/*
	vec[0] = aPos[0] - cPos[0];
	vec[1] = aPos[1] - cPos[1];
	vec[2] = (aPos[2] + 40) - cPos[2];	//the height of actor is about 40
	*/
	// the camera should look at cameraTargetPos, not the actor.
	vec[0] = cameraTargetPos[0] - cPos[0];
	vec[1] = cameraTargetPos[1] - cPos[1];
	vec[2] = (cameraTargetPos[2] + 40) - cPos[2];	//the height of actor is about 40
	dir_normalize(vec);
	//vec[2] = -0.2;

	float cp[3] = {0};	//aUDir cross to vec
	cp[0] = aUDir[1] * vec[2] - aUDir[2] * vec[1];
	cp[1] = aUDir[2] * vec[0] - aUDir[0] * vec[2];
	cp[2] = aUDir[0] * vec[1] - aUDir[1] * vec[0];

	float cUDir[3];	//vec cross to cp
	cUDir[0] = vec[1] * cp[2] - vec[2] * cp[1];
	cUDir[1] = vec[2] * cp[0] - vec[0] * cp[2];
	cUDir[2] = vec[0] * cp[1] - vec[1] * cp[0];

	cam.SetWorldDirection(vec, cUDir);
}

int GameControl::CamRevolution(DIRECTION_CODE code){
	if (this->CamInTop == TRUE){
		return -1;
	}
	FnActor act;
	act.Object(this->mainChar->character);
	//float aPos[3];
	//act.GetWorldPosition(aPos);

	FnCamera cam;
	cam.Object(this->camera);
	float cPos[3];
	cam.GetWorldPosition(cPos);

	float orig[2];
	float newVector[2];
	float r = 0.0f;
	for (int i = 0;i<2;i++){
		//orig[i] = cPos[i] - aPos[i];
		orig[i] = cPos[i] - this->cameraTargetPos[i];
		newVector[i] = orig[i];
		r += orig[i] * orig[i];
	}
	r = sqrt(r);

	float theta;
	theta = asin( MOVE_LENGTH / (2 * r) );
	theta *= 2;

	if (code == MOVE_LEFT){
		this->Rotate(theta, newVector);
	}else if(code == MOVE_RIGHT){
		this->Rotate(-theta, newVector);
	}

	float fDir[3];
	float uDir[3];
	for (int i = 0;i<2;i++){
		fDir[i] = newVector[i] - orig[i];
		uDir[i] = 0.0f;
	}
	fDir[2] = 0.0f;
	uDir[2] = 1.0f;

	this->dir_normalize(fDir);
	cam.SetWorldDirection(fDir,uDir);
	cam.MoveForward(MOVE_LENGTH,TRUE,FALSE,0.0f,TRUE);
	cam.PutOnTerrain(tID,FALSE,cPos[2]);
	return 0;
}

void GameControl::PlayAction(int skip){
	this->mainChar->PlayAction(skip);
	/*
	FnActor actor;
	actor.Object(this->mainChar->character);
	if (this->mainChar->CanBeControl() == TRUE){
		actor.Play(0,LOOP, (float)skip, FALSE,TRUE);
	}else if (this->state == STATEATTACK){
		this->
	}*/
	//return TRUE;
}

void GameControl::CamUpdate(){
	float ly_pos[3];
	FnActor actor;
	actor.Object(this->mainChar->character);
	actor.GetWorldPosition(ly_pos);

	float dis = (cameraTargetPos[0] - ly_pos[0]) * (cameraTargetPos[0] - ly_pos[0]) + (cameraTargetPos[1] - ly_pos[1]) * (cameraTargetPos[1] - ly_pos[1]);

	if (dis > RADIUS2){
		//move the cameraTargetPos to reduce the dis. Let ly_pos go back to the cicrle.
		float deltaL = sqrt(dis) - RADIUS;
		float dir[3];
		dir[0] = ly_pos[0] - cameraTargetPos[0];
		dir[1] = ly_pos[1] - cameraTargetPos[1];
		dir[2] = 0.0f;
		this->dir_normalize(dir);

		//then dir[0] will be cosine, dir[1] will be sine
		//update camerTargetPos
		cameraTargetPos[0] += deltaL * dir[0];
		cameraTargetPos[1] += deltaL * dir[1];
	}else if (dis <= RADIUS2){
		return;
	}

	this->CamMove(1.0f);
}

void GameControl::InitCamTargetPos(float pos[3]){
	for (int i = 0;i<3;i++){
		this->cameraTargetPos[i] = pos[i];
	}
}

void GameControl::ResetCamView(){
	FnActor actor;
	actor.Object(this->mainChar->character);
	float pos[3];
	float fDir[3];
	float uDir[3];
	actor.GetWorldDirection(fDir,uDir);
	actor.GetWorldPosition(pos);
	this->InitCamTargetPos(pos);

	FnObject cam;
	cam.Object(this->camera);
	float camPos[3];
	camPos[0] = pos[0] - fDir[0] * 100;
	camPos[1] = pos[1] - fDir[1] * 100;
	camPos[2] = CAM_MIN_HIGHT; //no effect
	cam.SetWorldPosition(camPos);
	this->CamInTop = FALSE;
	BOOL flag = cam.PutOnTerrain(tID,FALSE, CAM_MIN_HIGHT);
	if (flag == FALSE) {
		sprintf(debug, "%s reset cam view put on fail\n", debug);
	}
	
	// the camera should look at cameraTargetPos, not the actor.
	float vec[3];
	vec[0] = cameraTargetPos[0] - camPos[0];
	vec[1] = cameraTargetPos[1] - camPos[1];
	vec[2] = (cameraTargetPos[2] + 40) - camPos[2];	//the height of actor is about 40
	dir_normalize(vec);
	//vec[2] = -0.2;

	float cp[3];	//aUDir cross to vec
	cp[0] = uDir[1] * vec[2] - uDir[2] * vec[1];
	cp[1] = uDir[2] * vec[0] - uDir[0] * vec[2];
	cp[2] = uDir[0] * vec[1] - uDir[1] * vec[0];

	float cUDir[3];	//vec cross to cp
	cUDir[0] = vec[1] * cp[2] - vec[2] * cp[1];
	cUDir[1] = vec[2] * cp[0] - vec[0] * cp[2];
	cUDir[2] = vec[0] * cp[1] - vec[1] * cp[0];

	cam.SetWorldDirection(vec, cUDir);
	for (int i= 0;i< 10;i++){
		this->CamMove(2.0f);
	}
}

void GameControl::CamMove(float skip){
	FnObject cam;
	cam.Object(this->camera);
	float cam_pos[3];
	float fDir[3], uDir[3];
	float tempFDir[3], tempUDir[3];
	
	cam.GetWorldDirection(fDir,uDir);

	tempFDir[0] = fDir[0];
	tempFDir[1] = fDir[1];
	tempFDir[2] = 0.0;
	tempUDir[0] = 0.0;
	tempUDir[1] = 0.0;
	tempUDir[2] = uDir[2];

	cam.SetWorldDirection(tempFDir,tempUDir);
	cam.GetWorldPosition(cam_pos);
	
	float dis = (cam_pos[0] - cameraTargetPos[0]) * (cam_pos[0] - cameraTargetPos[0]) + (cam_pos[1] - cameraTargetPos[1]) * (cam_pos[1] - cameraTargetPos[1]);

	if (dis > 129600) {
		// move forward
		cam.MoveForward(MOVE_LENGTH * skip,FALSE, FALSE, 0.0, FALSE);
		BOOL flag;
		if (cam_pos[2] - MOVE_LENGTH / 1.2 > CAM_MIN_HIGHT){
			flag = cam.PutOnTerrain(tID,FALSE, cam_pos[2] - MOVE_LENGTH / 1.2);
		}else{
			this->CamInTop = FALSE;
			flag = cam.PutOnTerrain(tID,FALSE, CAM_MIN_HIGHT);
		}

		if (flag == FALSE) {
			sprintf(debug, "%s forward put on fail\n", debug);
		}
		firstBlock = TRUE;
	}else if (dis < 122500){
		/*
		if (skip > 1.0){
			sprintf(debug, "%s skip = %lf move = %lf\n", debug, skip,-MOVE_LENGTH * skip);
		}*/
		int ret = cam.MoveForward(-MOVE_LENGTH * skip,TRUE, FALSE, 0.0, TRUE);
		BOOL flag;
		if (ret == 0){
			flag = cam.PutOnTerrain(tID,FALSE, CAM_MIN_HIGHT);
		}else{
			if (firstBlock == TRUE){
				cam.MoveForward(-0.1, FALSE, FALSE, 0.0, TRUE);	//prevent the actor be under the camera, or it can't go out there
				firstBlock = FALSE;
			}
			//sprintf(debug, "%s Camera is blocked\n", debug);
			float pos[3];
			cam.GetWorldPosition(pos);
			if (pos[2] > CAM_MAX_HIGHT) {
				flag = cam.PutOnTerrain(tID,FALSE,CAM_MAX_HIGHT);
				cam.GetWorldPosition(pos);
			}
			else {
				this->CamInTop = TRUE;
				flag = cam.PutOnTerrain(tID,FALSE,pos[2] + MOVE_LENGTH / 2);
			}
		}

		if (flag == FALSE) {
			sprintf(debug, "%s backoff put on fail\n", debug);
		}
	}
}