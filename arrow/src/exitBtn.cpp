#include "stdafx.h"
#include "exitBtn.h"
#include "taskSelect.h"

ExitBtn::ExitBtn(lw::Task* pTask):_pTask(pTask){
	_pBtn = lw::Button::createButton9("common", "calibri20.fnt");
	_pBtn->setPos(300, 0);
	_pBtn->setSize(20, 20);
	_pBtn->setCallback(this);
	
}

ExitBtn::~ExitBtn(){
	delete _pBtn;
}

void ExitBtn::onButtonClick(lw::Button* pButton){
	_pTask->stop();
	taskSelect.start(0);
}