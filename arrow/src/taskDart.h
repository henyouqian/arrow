#ifndef __TASK_DART_H__
#define __TASK_DART_H__

class ExitBtn;
class Dart;
class Target;
class FluitEmitter;

class TaskDart : public lw::Task{
public:
	virtual void vBegin();
	virtual void vEnd();
	virtual void vMain(float dt);
	virtual void vDraw(float dt);
	virtual bool vOnTouchEvent(std::vector<lw::TouchEvent>& events);

private:
	ExitBtn* _pExitBtn;
	std::list<Dart*> _darts;
	std::list<Target*> _targets;
	lw::GestureMgr _gestureMgr;

	struct GestureData 
	{
		int x, y;
		float stayTime;
	};
	
	struct DartGestureInfo{
		int gestureId;
		std::list<GestureData> datas;
	};
	std::list<DartGestureInfo> _dartGestureInfos;

	b2World* _pWorld;
	FluitEmitter* _pEmitter;

	lw::Sprite* _pLine;
};

extern TaskDart taskDart;


#endif //__TASK_DART_H__