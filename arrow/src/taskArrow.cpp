#include "stdafx.h"
#include "taskArrow.h"
#include "batchDraw.h"
#include "exitBtn.h"
#include "arrow.h"
#include "targetFluit.h"
#include "bow.h"

namespace{
	const b2Vec2 GRAVITY(0.0f, -10.f);
	const float LINE_Y = 380.f;
}

void TaskArrow::vBegin(){
	_pExitBtn = new ExitBtn(this);
	_pWorld = new b2World(GRAVITY, true);
	_pEmitter = new FluitEmitter(_targets, _pWorld);

	{
		b2BodyDef bd;
		bd.type = b2_staticBody;

		b2Body* pBody = _pWorld->CreateBody(&bd);
		b2FixtureDef fd;
		fd.restitution = 0.5f;
		fd.friction = 0.0f;
		b2PolygonShape shape;
		fd.shape = &shape;
		float x = 160.f*B2_SCALE;
		float y = 10000.f*B2_SCALE;
		shape.SetAsEdge(b2Vec2(-x, -y), b2Vec2(-x, y));
		pBody->CreateFixture(&fd);
		shape.SetAsEdge(b2Vec2(x, -y), b2Vec2(x, y));
		pBody->CreateFixture(&fd);
	}
	_pBow = new Bow(LINE_Y, _arrows);
	_pLine = lw::Sprite::create("alpha1.png");
	_pLine->setUV(5, 508, 10, 2);
	_pLine->setAnchor(250, 0);
    
    _pSptBG = lw::Sprite::create("newton.png");
    _pSptBG->setUV(0, 0, 640, 960);
    
    _pSptEyeBG = lw::Sprite::create("newton.png");
    _pSptEyeBG->setUV(0, 963, 150, 44);
    
    _pSptEye = lw::Sprite::create("newton.png");
    _pSptEye->setUV(236, 972, 100, 20);
    _lookatX = 140;
    _lookatY = 312;
    
    _pEyelid = lw::Sprite::create("newton.png");
    _pEyelid->setUV(360, 962, 150, 60);
    _blinkFrame = rand()%60+60;
    _openFrame = -1;
    _twice = false;
    
    _upArrow = lw::Sprite::create("newton.png");
    _upArrow->setUV(642, 0, 62, 92);
}

void TaskArrow::vEnd(){
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
	delete _pEmitter;
	delete _pWorld;
	delete _pBow;
	delete _pLine;
    delete _pSptBG;
    delete _pSptEyeBG;
    delete _pSptEye;
    delete _pEyelid;
    delete _upArrow;
}

void TaskArrow::vMain(float dt){
	dt = 1000.f/GAME_FPS;
	_pEmitter->main(dt, WT_ARROW);
	_pWorld->Step(1.f/GAME_FPS, 8, 3);
	{
		std::list<Arrow*>::iterator it = _arrows.begin();
		std::list<Arrow*>::iterator itEnd = _arrows.end();
		for ( ; it != itEnd; ){
			if ( (*it)->main() ){
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
        if ( !_targets.empty() ){
            std::list<Target*>::iterator it = _targets.begin();
            (*it)->getScreenPos(_lookatX, _lookatY);
        }
	}
}

void TaskArrow::vDraw(float dt){
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);
	
    {
        _pSptEyeBG->collect(105, 298, 75, 22);
        float x0 = 118.f;
        float y0 = 306.f;
        float dx = (_lookatX-142.f)*.02f;
        float dy = (_lookatY-312.f)*.02f;
        float lim = 3.f;
        if ( dx < 0 ){
            dx = std::max(dx, -lim);
        }else{
            dx = std::min(dx, lim);
        }
        if ( dy < 0 ){
            dy = std::max(dy, -lim);
        }else{
            dy = std::min(dy, lim);
        }
        float x = x0 + dx;
        float y = y0 + dy;
        
        --_blinkFrame;
        bool blink = false;
        if ( _blinkFrame < 8 ){
            blink = true;
            if ( _blinkFrame == 0 ){
                if ( _twice ){
                    _blinkFrame = rand()%260+30;
                    _twice = false;
                }else{
                    _twice = rand()%10==0;
                    if ( _twice ){
                        _blinkFrame = 12;
                    }else{
                        _blinkFrame = rand()%260+30;
                    }
                }
            }
            if ( _blinkFrame == 7 && y < y0 ){
                _openFrame = 0;
            }
        }
        if ( _openFrame >= 0 ){
            int frameMax = 8;
            float dframe = fabsf(_openFrame-frameMax)/frameMax;
            y = cml::lerp(y0, y, dframe);
            if ( _openFrame >= frameMax ){
                _openFrame = -1;
            }else{
                ++_openFrame;
            }
        }
        
        _pSptEye->collect(x, y, 50, 10);
        _pSptBG->collect(0, 0, 320, 480);
        
        if ( blink ){
            _pEyelid->collect(108, 292, 75, 30);
        }
    }
    
    
	{
		std::list<Arrow*>::iterator it = _arrows.begin();
		std::list<Arrow*>::iterator itEnd = _arrows.end();
		for ( ; it != itEnd; ++it ){
			(*it)->collect();
		}
	}
	{
		std::list<Target*>::iterator it = _targets.begin();
		std::list<Target*>::iterator itEnd = _targets.end();
		for ( ; it != itEnd; ++it ){
			(*it)->collect();
		}
	}
	{
		_pBow->draw();
	}
	{
		//_pLine->collect(160.f, LINE_Y, 500.f, 1.f, 0.f);
	}
    {
        float y = _lookatY-20;
        if ( y < 0 ){
            float alpha = -y/20.f;
            alpha = std::min(alpha, 1.f);
            float scale = pow(0.99f , -y*.1f);
            _upArrow->setAnchor(15*scale, 0);
            _upArrow->collect(_lookatX, 0, 31*scale, 46*scale, 0, false, false, lw::Color(1.f, 1.f, 1.f, alpha));
        }
        
    }
}


bool TaskArrow::vOnTouchEvent(std::vector<lw::TouchEvent>& events){
	_gestureMgr.onTouchEvent(events);
	const std::list<lw::Gesture>& gestures = _gestureMgr.getGestures();
	std::list<lw::Gesture>::const_iterator it = gestures.begin();
	std::list<lw::Gesture>::const_iterator itEnd = gestures.end();
	for ( ; it != itEnd; ++it ){
		if ( it->updated ){
			_pBow->onGestureUpdate(*it);
		}
	}
	_gestureMgr.main();
	return false;
}

TaskArrow taskArrow;