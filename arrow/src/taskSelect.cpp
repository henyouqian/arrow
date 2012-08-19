#include "stdafx.h"
#include "taskSelect.h"
#include "taskMissile.h"
#include "taskArrow.h"
#include "taskBullet.h"
#include "taskDart.h"
#include "taskNetUser.h"
#include "taskBullseye.h"

namespace{
	void menuCallback(int itemIdx, void*){
		taskSelect.stop();
		switch(itemIdx)
		{
		case 0:
			taskMissile.start(0);
			break;
		case 1:
			taskArrow.start(0);
			break;
		case 2:
			taskBullet.start(0);
			break;
		case 3:
			taskDart.start(0);
			break;
		case 4:
			taskNetUser.start(0);
			break;
		case 5:
			taskBullseye.start(0);
			break;
		case 6:
			
			break;
		case 7:
			
			break;
		case 8:
			
			break;
		case 9:
			
			break;
		case 10:
			
			break;
		case 11:
			
			break;
		case 12:
			
			break;
		case 13:
			
			break;
		case 14:
			
			break;
		default:
			lwassert(0);
		    break;
		}
	}
}

void TaskSelect::vBegin(){
	lw::App::setOrient(lw::App::ORIENTATION_UP);

	_pMenu = lw::Menu::create("arial.fnt");
	lwassert(_pMenu);

	const int x0 = 20;
	const int y0 = 20;
	int x = x0;
	int y = y0;
	const int stepY = 50;
	const int stepX = 150;
	_pMenu->addItem(L"*Missile", x, y, NULL);
	y += stepY;
	_pMenu->addItem(L"*Arrow", x, y, NULL);
	y += stepY;
	_pMenu->addItem(L"*Bullet", x, y, NULL);
	y += stepY;
	_pMenu->addItem(L"*Dart", x, y, NULL);
	y += stepY;
	_pMenu->addItem(L"*NetUser", x, y, NULL);
	y += stepY;
	_pMenu->addItem(L"*Bullseye", x, y, NULL);
	y += stepY;
	//y = y0;
	//_pMenu->addItem(L"*HoldOn", x, y, NULL);
	//y += stepY;
	//_pMenu->addItem(L"*Fishing", x, y, NULL);
	//y += stepY;
	//_pMenu->addItem(L"*Volleyball", x, y, NULL);
	//y += stepY;
	//_pMenu->addItem(L"*Throw", x, y, NULL);
	//y += stepY;
	//_pMenu->addItem(L"*Calculate", x, y, NULL);
	//y += stepY;
	//_pMenu->addItem(L"*Spray", x, y, NULL);

	//x += stepX;
	//y = y0;
	//_pMenu->addItem(L"*Mole", x, y, NULL);
	//y += stepY;
	//_pMenu->addItem(L"*Chickens", x, y, NULL);
	//y += stepY;
	//_pMenu->addItem(L"*AppleTree", x, y, NULL);

	_pMenu->callback(menuCallback);
}

void TaskSelect::vEnd(){
	delete _pMenu;
}

void TaskSelect::vMain(float dt){
	
}

void TaskSelect::vDraw(float dt){
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	_pMenu->collect();
}

bool TaskSelect::vOnTouchEvent(std::vector<lw::TouchEvent>& events){
	_pMenu->onTouchEvent(events);

	lw::TouchEvent& evt = events[0];
	//if ( evt.type == lw::TouchEvent::TOUCH && evt.x < 20 && evt.y < 20 ){
	//	static int ort = 3;
	//	ort %= 4;
	//	lw::App::setOrient((lw::App::Orientation)ort);
	//	++ort;
	//}

	return false;
}

TaskSelect taskSelect;