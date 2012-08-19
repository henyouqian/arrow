#ifndef __TASK_ARROW_H__
#define __TASK_ARROW_H__

class ExitBtn;
class Arrow;
class Target;
class FluitEmitter;
class Bow;

class TaskArrow : public lw::Task{
public:
	virtual void vBegin();
	virtual void vEnd();
	virtual void vMain(float dt);
	virtual void vDraw(float dt);
	virtual bool vOnTouchEvent(std::vector<lw::TouchEvent>& events);

private:
	ExitBtn* _pExitBtn;
	std::list<Arrow*> _arrows;
	std::list<Target*> _targets;
	lw::GestureMgr _gestureMgr;

	b2World* _pWorld;
	FluitEmitter* _pEmitter;
	Bow* _pBow;
	lw::Sprite* _pLine;
    lw::Sprite* _pSptBG;
};

extern TaskArrow taskArrow;


#endif //__TASK_ARROW_H__