#ifndef __TASK_EMPTY_H__
#define __TASK_EMPTY_H__

class TaskEmpty : public lw::Task{
public:
	virtual void vBegin();
	virtual void vEnd();
	virtual void vMain(float dt);
	virtual void vDraw(float dt);
	virtual bool vOnTouchEvent(std::vector<lw::TouchEvent>& events);

private:

};

extern TaskEmpty taskEmpty;


#endif //__TASK_EMPTY_H__