#ifndef __TASK_BULLSEYE_H__
#define __TASK_BULLSEYE_H__

class ExitBtn;
class BullseyeRound;
class Arrow;
class Target;
class Bow;

class TaskBullseye : public lw::Task, public lw::ButtonCallback{
public:
	virtual void vBegin();
	virtual void vEnd();
	virtual void vMain(float dt);
	virtual void vDraw(float dt);
	virtual bool vOnTouchEvent(std::vector<lw::TouchEvent>& events);
	virtual void onButtonClick(lw::Button* pButton);

private:
	void onBtnNext();
	void onBtnRestart();
	void onGameEnd();

	ExitBtn* _pExitBtn;
	std::list<Arrow*> _arrows;
	std::list<Target*> _targets;
	lw::GestureMgr _gestureMgr;
	Bow* _pBow;
	lw::Sprite* _pLine;
	BullseyeRound* _pBullseyeRound;

	int _round;
	int _life;
	float _roundScore[3];
	int _scoreIdx;
	float _totalScore;
	int _arrowRemain;
	lw::Font* _pScoreFont;
	lw::Font* _pTextFont;
	lw::Button* _pBtnNext;
	lw::Button* _pBtnRestart;
	float _bestScore;
	lw::Sprite* _pSptHeart;
};

extern TaskBullseye taskBullseye;


#endif //__TASK_BULLSEYE_H__