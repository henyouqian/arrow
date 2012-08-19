#include "stdafx.h"
#include "taskBullet.h"
#include "batchDraw.h"
#include "exitBtn.h"

void TaskBullet::vBegin(){
	_pExitBtn = new ExitBtn(this);
}

void TaskBullet::vEnd(){
	delete _pExitBtn;
}

void TaskBullet::vMain(float dt){
	
}

void TaskBullet::vDraw(float dt){
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);
}


bool TaskBullet::vOnTouchEvent(std::vector<lw::TouchEvent>& events){
	return false;
}

TaskBullet taskBullet;