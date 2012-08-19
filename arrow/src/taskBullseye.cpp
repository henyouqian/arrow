#include "stdafx.h"
#include "taskBullseye.h"
#include "exitBtn.h"
#include "taskSelect.h"
#include "targetBullseye.h"
#include "arrow.h"
#include "target.h"
#include "bow.h"
#include "bullseyeRound.h"

namespace{
	const float LINE_Y = 400.f;
}

void TaskBullseye::vBegin(){
	_pExitBtn = new ExitBtn(this);
	_pBow = new Bow(LINE_Y, _arrows);
	_pLine = lw::Sprite::create("alpha1.png");
	_pLine->setUV(5, 508, 10, 2);
	_pLine->setAnchor(250, 0);

	_pSptHeart = lw::Sprite::create("alpha1.png");

	_pBullseyeRound = NULL;
	_totalScore = 0.f;
	_bestScore = 0.f;
	_roundScore[0] = -1.f;
	_roundScore[1] = -1.f;
	_roundScore[2] = -1.f;
	_arrowRemain = 3;
	_round = 1;

	_pScoreFont = lw::Font::create("calibri20.fnt");
	_pTextFont = lw::Font::create("arial.fnt");

	lw::Color textColor(120, 0, 0, 255);
	_pBtnNext = lw::Button::createButton9("common", "arial.fnt");
	_pBtnNext->setText(L"Next");
	_pBtnNext->setTextColor(textColor);
	_pBtnNext->show(false);
	_pBtnNext->setPos(120, 300);
	_pBtnNext->setSize(80, 50);
	_pBtnNext->setCallback(this);

	_pBtnRestart = lw::Button::createButton9("common", "arial.fnt");
	_pBtnRestart->setText(L"Restart");
	_pBtnRestart->setTextColor(textColor);
	_pBtnRestart->show(false);
	_pBtnRestart->setPos(100, 280);
	_pBtnRestart->setSize(120, 90);
	_pBtnRestart->setCallback(this);
}

void TaskBullseye::vEnd(){
	delete _pExitBtn;
	{
		std::list<Arrow*>::iterator it = _arrows.begin();
		std::list<Arrow*>::iterator itEnd = _arrows.end();
		for ( ; it != itEnd; ++it ){
			delete (*it);
		}
		_arrows.clear();
	}
	{
		std::list<Target*>::iterator it = _targets.begin();
		std::list<Target*>::iterator itEnd = _targets.end();
		for ( ; it != itEnd; ++it){
			delete (*it);
		}
		_targets.clear();
	}
	delete _pBow;
	delete _pLine;
	delete _pSptHeart;
	if ( _pBullseyeRound){
		delete _pBullseyeRound;
	}
	delete _pScoreFont;
	delete _pTextFont;
	delete _pBtnNext;
	delete _pBtnRestart;
}

void TaskBullseye::vMain(float dt){
	dt = 1000.f/GAME_FPS;
	if ( _pBullseyeRound == NULL ){
		_pBullseyeRound = BullseyeRound::create(_round, &_targets);
		if ( _pBullseyeRound == NULL ){
			_round = 1;
			_pBullseyeRound = BullseyeRound::create(_round, &_targets);
		}
		_arrowRemain = 3;
		_roundScore[0] = -1.f;
		_roundScore[1] = -1.f;
		_roundScore[2] = -1.f;
		_scoreIdx = 0;
	}
	_pBullseyeRound->vMain(dt);
	{
		std::list<Arrow*>::iterator it = _arrows.begin();
		std::list<Arrow*>::iterator itEnd = _arrows.end();
		for ( ; it != itEnd; ){
			if ( (*it)->main() ){
				float score = (*it)->getScore()*10.f;
				if ( score < 0.001f ){
					--_life;
					if ( _life == -1 ){
						float roundScore = max(0.f, _roundScore[0]) + max(0.f, _roundScore[1]) + max(0.f, _roundScore[2]);
						_totalScore += roundScore;
						onGameEnd();
					}
				}else if ( score >= 9.f ){
					++_life;
					if ( _life > 3 ){
						_life = 3;
					}
				}
				_roundScore[_scoreIdx] = score;
				++_scoreIdx;
				delete (*it);
				it = _arrows.erase(it);
				itEnd = _arrows.end();
			}else{
				++it;
			}
		}
	}
	{
		std::list<Target*>::iterator it = _targets.begin();
		std::list<Target*>::iterator itEnd = _targets.end();
		for ( ; it != itEnd; ){
			if ( (*it)->main(dt) ){
				delete (*it);
				it = _targets.erase(it);
				itEnd = _targets.end();
			}else{
				std::list<Arrow*>::iterator itAR = _arrows.begin();
				std::list<Arrow*>::iterator itAREnd = _arrows.end();
				for ( ; itAR != itAREnd; ++itAR ){
					if ( !(*itAR)->isDie() ){
						(*it)->checkArrow(*itAR);
					}
				}
				++it;
			}
		}
	}
	if ( _arrowRemain == 0 && _arrows.empty() ){
		if ( _pBullseyeRound && (!_pBtnNext->isShow()) && (!_pBtnRestart->isShow() ) ){
			float roundScore = _roundScore[0] + _roundScore[1] + _roundScore[2];
			_totalScore += roundScore;
			if ( _round == ROUND_NUM ){
				onGameEnd();
			}else{
				_pBtnNext->show(true);
			}
		}
	}
}

void TaskBullseye::vDraw(float dt){
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);

	_pLine->collect(160.f, LINE_Y, 500.f, 1.f, 0.f);
	{
		std::list<Target*>::iterator it = _targets.begin();
		std::list<Target*>::iterator itEnd = _targets.end();
		for ( ; it != itEnd; ++it ){
			(*it)->collect();
		}
	}
	{
		std::list<Arrow*>::iterator it = _arrows.begin();
		std::list<Arrow*>::iterator itEnd = _arrows.end();
		for ( ; it != itEnd; ++it ){
			(*it)->collect();
		}
	}
	_pBow->draw();

	const int SZ = 16;
	wchar_t buf[SZ];
	{
		swprintf(buf, SZ, L"%.2f", _roundScore[0]);
		_pScoreFont->drawText(buf, 10, 460);
	}
	{
		swprintf(buf, SZ, L"%.2f", _roundScore[1]);
		_pScoreFont->drawText(buf, 50, 460);
	}
	{
		swprintf(buf, SZ, L"%.2f", _roundScore[2]);
		_pScoreFont->drawText(buf, 90, 460);
	}
	{
		swprintf(buf, SZ, L"score:%.3f", _totalScore);
		_pScoreFont->drawText(buf, 140, 460);
	}
	{
		swprintf(buf, SZ, L"best:%.3f", _bestScore);
		_pScoreFont->drawText(buf, 235, 460);
	}

	{
		float x = 10.f;
		for ( int i = 0; i < 3; ++i ){
			if ( i < _life ){
				_pSptHeart->setUV(0, 190, 22, 20);
			}else{
				_pSptHeart->setUV(23, 190, 22, 20);
			}
			_pSptHeart->collect(x, 5.f);
			x += 25.f;
		}
	}
	
	if ( _life == -1 ){
		_pTextFont->drawText(L"Game over", 90, 160);
	}
}

bool TaskBullseye::vOnTouchEvent(std::vector<lw::TouchEvent>& events){
	_gestureMgr.onTouchEvent(events);
	if ( _arrowRemain > 0 ){
		const std::list<lw::Gesture>& gestures = _gestureMgr.getGestures();
		std::list<lw::Gesture>::const_iterator it = gestures.begin();
		std::list<lw::Gesture>::const_iterator itEnd = gestures.end();
		for ( ; it != itEnd; ++it ){
			if ( it->updated ){
				if ( _pBow->onGestureUpdate(*it) ){
					--_arrowRemain;
				}
			}
		}
	}
	_gestureMgr.main();
	return false;
}

void TaskBullseye::onButtonClick(lw::Button* pButton){
	if ( pButton == _pBtnNext ){
		onBtnNext();
	}else if ( pButton == _pBtnRestart ){
		onBtnRestart();
	}
}

void TaskBullseye::onBtnNext(){
	delete _pBullseyeRound;
	_pBullseyeRound = NULL;
	++_round;

	{
		std::list<Arrow*>::iterator it = _arrows.begin();
		std::list<Arrow*>::iterator itEnd = _arrows.end();
		for ( ; it != itEnd; ++it ){
			delete (*it);
		}
		_arrows.clear();
	}
	{
		std::list<Target*>::iterator it = _targets.begin();
		std::list<Target*>::iterator itEnd = _targets.end();
		for ( ; it != itEnd; ++it){
			delete (*it);
		}
		_targets.clear();
	}
	_pBtnNext->show(false);
}

void TaskBullseye::onBtnRestart(){
	onBtnNext();
	_round = 1;
	_totalScore = 0.f;
	_pBtnRestart->show(false);
	_life = 0;
}

void TaskBullseye::onGameEnd(){
	_pBtnRestart->show(true);
	_bestScore = max(_bestScore, _totalScore);
	_arrowRemain = 0;
	_pBow->clear();
}

TaskBullseye taskBullseye;