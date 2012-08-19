#ifndef __TASK_MISSILE_H__
#define __TASK_MISSILE_H__

class Missile;
class ExitBtn;

class TaskMissile : public lw::Task{
public:
	virtual void vBegin();
	virtual void vEnd();
	virtual void vMain(float dt);
	virtual void vDraw(float dt);
	virtual bool vOnTouchEvent(std::vector<lw::TouchEvent>& events);

private:
	ExitBtn* _pBtnExit;
	std::list<Missile*> _pMissiles;
	lw::ModelSprite* _pMdlSprite;
	lw::Sprite* _pArrow;
	lw::Sprite* _pLine;
	bool _isTouching;
	
	lw::Clip* _pClip;
	float _lineRot;
};

extern TaskMissile taskMissile;


#endif //__TASK_MISSILE_H__