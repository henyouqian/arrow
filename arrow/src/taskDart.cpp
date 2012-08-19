#include "stdafx.h"
#include "taskDart.h"
#include "batchDraw.h"
#include "exitBtn.h"
#include "dart.h"
#include "targetFluit.h"
#include "soundManager.h"

namespace{
	const b2Vec2 GRAVITY(0.0f, -10.f);
	const int DART_GESTURE_SIZE = 4;
	const float LINE_Y = 350.f;
}

void TaskDart::vBegin(){
	_pExitBtn = new ExitBtn(this);

	_pWorld = new b2World(GRAVITY, true);

	_pEmitter = new FluitEmitter(_targets, _pWorld);

	_pLine = lw::Sprite::create("alpha1.png");
	_pLine->setUV(5, 508, 10, 2);
	_pLine->setAnchor(250, 0);

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
		float y = 1000.f*B2_SCALE;
		//shape.SetAsBox(1.f, y, b2Vec2(-x-1.f, 0.f), 0.f);
		shape.SetAsEdge(b2Vec2(-x, -y), b2Vec2(-x, y));
		pBody->CreateFixture(&fd);
		//shape.SetAsBox(1.f, y, b2Vec2(x+1.f, 0.f), 0.f);
		shape.SetAsEdge(b2Vec2(x, -y), b2Vec2(x, y));
		pBody->CreateFixture(&fd);
		//shape.SetAsEdge(b2Vec2(-100.f, 0), b2Vec2(100.f, 0));
		//pBody->CreateFixture(&fd);
	}
}

void TaskDart::vEnd(){
	delete _pExitBtn;
	{
		std::list<Dart*>::iterator it = _darts.begin();
		std::list<Dart*>::iterator itEnd = _darts.end();
		for ( ; it != itEnd; ++it ){
			delete (*it);
		}
		_darts.clear();
	}
	{
		std::list<Target*>::iterator it = _targets.begin();
		std::list<Target*>::iterator itEnd = _targets.end();
		for ( ; it != itEnd; ++it){
			delete (*it);
		}
		_targets.clear();
	}
	_dartGestureInfos.clear();
	delete _pEmitter;
	delete _pWorld;
	delete _pLine;
}

void TaskDart::vMain(float dt){
	dt = 1000.f/GAME_FPS;
	_pEmitter->main(dt, WT_DART);
	_pWorld->Step(1.f/GAME_FPS, 8, 3);
	{
		std::list<Dart*>::iterator it = _darts.begin();
		std::list<Dart*>::iterator itEnd = _darts.end();
		for ( ; it != itEnd; ){
			if ( (*it)->main() ){
				delete (*it);
				it = _darts.erase(it);
				itEnd = _darts.end();
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
				std::list<Dart*>::iterator itAR = _darts.begin();
				std::list<Dart*>::iterator itAREnd = _darts.end();
				for ( ; itAR != itAREnd; ++itAR ){
					if ( !(*itAR)->isDie() ){
						(*it)->checkDart(*itAR);
					}
				}
				++it;
			}
		}
	}
	{
		std::list<DartGestureInfo>::iterator it = _dartGestureInfos.begin();
		std::list<DartGestureInfo>::iterator itEnd = _dartGestureInfos.end();
		for ( ; it != itEnd; ++it ){
			it->datas.back().stayTime += dt;
		}
	}
}

void TaskDart::vDraw(float dt){
	glClearColor(78.f/255.f, 75.f/255.f, 74.f/255.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);

	{
		_pLine->collect(160.f, LINE_Y, 500.f, 1.f, 0.f);
	}
	
	{
		std::list<Dart*>::iterator it = _darts.begin();
		std::list<Dart*>::iterator itEnd = _darts.end();
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
	cml::Matrix44 m;
	m.identity();
	cml::matrix_translation(m, 160.f, 240.f, 0.f);
}


bool TaskDart::vOnTouchEvent(std::vector<lw::TouchEvent>& events){
	_gestureMgr.onTouchEvent(events);
	const std::list<lw::Gesture>& gestures = _gestureMgr.getGestures();
	std::list<lw::Gesture>::const_iterator it = gestures.begin();
	std::list<lw::Gesture>::const_iterator itEnd = gestures.end();
	for ( ; it != itEnd; ++it ){
		if ( it->updated ){
			bool release = false;
			const lw::TouchEvent& evt = it->evt;
			if ( evt.type == lw::TouchEvent::TOUCH ){
				DartGestureInfo info;
				info.gestureId = it->id;
				GestureData data;
				data.stayTime = 0.f;
				data.x = evt.x;
				data.y = evt.y;
				info.datas.push_back(data);
				_dartGestureInfos.push_back(info);
			}else if (evt.type == lw::TouchEvent::UNTOUCH){
				release = true;
			}else if (evt.type == lw::TouchEvent::MOVE){
				std::list<DartGestureInfo>::iterator ita = _dartGestureInfos.begin();
				std::list<DartGestureInfo>::iterator itaEnd = _dartGestureInfos.end();
				for ( ; ita != itaEnd; ++ita ){
					if ( ita->gestureId == it->id ){
						GestureData data;
						data.x = evt.x;
						data.y = evt.y;
						data.stayTime = 0.f;
						if ( ita->datas.size() == DART_GESTURE_SIZE ){
							ita->datas.pop_front();
						}
						ita->datas.push_back(data);
						if ( data.y <= LINE_Y ){
							release = true;
						}
						break;
					}
				}
			}
			if ( release ){
				std::list<DartGestureInfo>::iterator ita = _dartGestureInfos.begin();
				std::list<DartGestureInfo>::iterator itaEnd = _dartGestureInfos.end();
				for ( ; ita != itaEnd; ){
					if ( ita->gestureId == it->id ){
						std::list<GestureData>::reverse_iterator itData = ita->datas.rbegin();
						std::list<GestureData>::reverse_iterator itDataEnd = ita->datas.rend();
						float timeSum = 0.f;
						for ( int i = 0; itData != itDataEnd && i < 4; ++itData, ++i ){
							if ( itData->stayTime < 50.f ){
								timeSum += itData->stayTime;
							}
						}
						float dx = (float)ita->datas.begin()->x - ita->datas.back().x;
						float dy = (float)ita->datas.begin()->y - ita->datas.back().y;
						if ( itData != itDataEnd ){
							dx = (float)itData->x - ita->datas.back().x;
							dy = (float)itData->y - ita->datas.back().y;
						}
						float ds = sqrtf(dx*dx+dy*dy);
						float speed = min(ds/timeSum*10.f, 15.f);
						
						{
							std::list<GestureData>::iterator itData = ita->datas.begin();
							std::list<GestureData>::iterator itDataEnd = ita->datas.end();
							float timeSum = 0.f;
							for ( ; itData != itDataEnd; ++itData ){
								
							}
						}

						cml::Vector2 v;
						v[0] = (float)ita->datas.back().x - ita->datas.begin()->x;
						v[1] = (float)ita->datas.back().y - ita->datas.begin()->y;
						v[1] = -v[1];
						if ( v.length() < 10.f ){
							break;
						}
						cml::Vector2 v1;
						v1[0] = 0.f; v1[1] = 1.f;
						v.normalize();
						float angle = cml::signed_angle_2D(v1, v);
						if ( speed > 5.f ){
							float x, y;
							if ( ita->datas.back().y <= LINE_Y ){
								std::list<GestureData>::reverse_iterator itD = ita->datas.rbegin();
								++itD;
								float x1 = (float)itD->x; float y1 = (float)itD->y;
								float x2 = it->evt.x; float y2 = it->evt.y;
								float f = (LINE_Y-y1)/(y2-y1);
								x = (x1 + (x2-x1)*f)-160.f;
								y = 480.f - LINE_Y;
							}else{
								x = it->evt.x-160.f;
								y = 480.f-it->evt.y;
							}
							Dart* pDart = new Dart(x, y, angle, speed, 0.f);
							_darts.push_back(pDart);
							g_pSndShoot->play();
						}
						ita = _dartGestureInfos.erase(ita);
						itaEnd = _dartGestureInfos.end();
					}else{
						++ita;
					}
				}
			}
		}
	}
	_gestureMgr.main();
	return false;
}

TaskDart taskDart;