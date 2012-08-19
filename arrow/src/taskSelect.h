#ifndef __TASK_SELECT_H__
#define __TASK_SELECT_H__

class TaskSelect : public lw::Task{
public:
	virtual void vBegin();
	virtual void vEnd();
	virtual void vMain(float dt);
	virtual void vDraw(float dt);
	virtual bool vOnTouchEvent(std::vector<lw::TouchEvent>& events);

private:
	lw::Menu* _pMenu;
};

extern TaskSelect taskSelect;


#endif //__TASK_SELECT_H__