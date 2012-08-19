#include "stdafx.h"
#include "taskEmpty.h"
#include "taskSelect.h"

void TaskEmpty::vBegin(){
	
}

void TaskEmpty::vEnd(){
	
}

void TaskEmpty::vMain(float dt){
	
}

void TaskEmpty::vDraw(float dt){
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);
}

bool TaskEmpty::vOnTouchEvent(std::vector<lw::TouchEvent>& events){
	lw::TouchEvent& evt0 = events[0];
	if ( evt0.type == lw::TouchEvent::TOUCH ){
		if ( evt0.x > 460 && evt0.y < 20 ){
			stop();
			taskSelect.start(0);
			return true;
		}
	}
	return false;
}


TaskEmpty taskEmpty;