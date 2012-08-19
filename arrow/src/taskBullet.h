#ifndef __TASK_BULLET_H__
#define __TASK_BULLET_H__

class ExitBtn;

class TaskBullet : public lw::Task{
public:
	virtual void vBegin();
	virtual void vEnd();
	virtual void vMain(float dt);
	virtual void vDraw(float dt);
	virtual bool vOnTouchEvent(std::vector<lw::TouchEvent>& events);

private:
	ExitBtn* _pExitBtn;
};

extern TaskBullet taskBullet;


#endif //__TASK_BULLET_H__