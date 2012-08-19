#include "stdafx.h"
#include "bow.h"
#include "arrow.h"
#include "soundManager.h"

namespace{
	float BEHIND_LENGTH = 70.f;
}

Bow::Bow(float lineY, std::list<Arrow*>& arrows):_lineY(lineY), _arrows(arrows){
	_pSptArrow = lw::Sprite::create("alpha1.png");
	_pSptArrow->setUV(506, 0, 6, 66);
	_pSptArrow->setAnchor(3.f, 4.f);
}

Bow::~Bow(){
	delete _pSptArrow;
}

void Bow::draw(){
	std::list<ArrowGestureInfo>::iterator it = _arrowGestureInfos.begin();
	std::list<ArrowGestureInfo>::iterator itEnd = _arrowGestureInfos.end();
	for ( ; it != itEnd; ++it ){
		if ( it->angle != 9999.f ){
			cml::Vector2 v2;
			v2[0] = 0.f; v2[1] = 1.f;
			v2 = cml::rotate_vector_2D(v2, -it->angle);
			v2[0] *= -BEHIND_LENGTH;
			v2[1] *= -BEHIND_LENGTH;
			float x = (float)it->x+v2[0];
			float y = (float)it->y+v2[1];
			_pSptArrow->collect(x, y, 0, 0, -it->angle);
		}
	}
}

bool Bow::onGestureUpdate(const lw::Gesture& gesture){
	bool rt = false;
	const lw::TouchEvent& evt = gesture.evt;
	if ( evt.type == lw::TouchEvent::TOUCH ){
		if ( evt.y > 200 ){
			ArrowGestureInfo info;
			info.gestureId = gesture.id;
			info.startX = evt.x;
			info.startY = evt.y;
			info.angle = 9999.f;
			_arrowGestureInfos.push_back(info);
		}
	}else if (evt.type == lw::TouchEvent::UNTOUCH){
		std::list<ArrowGestureInfo>::iterator ita = _arrowGestureInfos.begin();
		std::list<ArrowGestureInfo>::iterator itaEnd = _arrowGestureInfos.end();
		for ( ; ita != itaEnd; ){
			if ( ita->gestureId == gesture.id ){
				if ( ita->angle != 9999.f ){
					cml::Vector2 v2;
					v2[0] = 0.f; v2[1] = 1.f;
					v2 = cml::rotate_vector_2D(v2, ita->angle);
					v2[0] *= 60.f;
					v2[1] *= 60.f;
					if ( evt.y > _lineY ){
						Arrow* pArrow = new Arrow(evt.x-160.f+v2[0], 480.f-evt.y+v2[1], ita->angle);
						_arrows.push_back(pArrow);
						g_pSndShoot->play();
						rt = true;
					}
				}
				ita = _arrowGestureInfos.erase(ita);
				itaEnd = _arrowGestureInfos.end();
				break;
			}else{
				++ita;
			}
		}
	}else if (evt.type == lw::TouchEvent::MOVE){
		std::list<ArrowGestureInfo>::iterator ita = _arrowGestureInfos.begin();
		std::list<ArrowGestureInfo>::iterator itaEnd = _arrowGestureInfos.end();
		for ( ; ita != itaEnd; ++ita ){
			if ( ita->gestureId == gesture.id ){
				cml::Vector2 v;
				v[0] = (float)(ita->startX - evt.x);
				v[1] = (float)evt.y - ita->startY;
				if ( v.length() < 10.f ){
					ita->angle = 9999.f;
					break;
				}
				cml::Vector2 v1;
				v1[0] = 0.f; v1[1] = 1.f;
				v.normalize();
				ita->angle = cml::signed_angle_2D(v1, v);
				ita->x = evt.x;
				ita->y = evt.y;
				break;
			}
		}
	}
	return rt;
}

void Bow::clear(){
	_arrowGestureInfos.clear();
}