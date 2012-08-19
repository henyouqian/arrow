#include "stdafx.h"
#include "taskMissile.h"
#include "taskSelect.h"
#include "batchDraw.h"
#include "missile.h"
#include "lwFramework/lwAccelerometer.h"
#include "exitBtn.h"

namespace{
	float ANGLE_SCALE = 1.5f;
}


void TaskMissile::vBegin(){
	_pBtnExit = new ExitBtn(this);

	lw::initAccelerometer(1.f/60.f);

	_pMdlSprite = lw::ModelSprite::create("fluit.pod", "apple");
	_pArrow = lw::Sprite::create("alpha1.png");
	_pArrow->setUV(0, 512-65, 21, 65);
	_pArrow->setAnchor(10, 60);

	_pLine = lw::Sprite::create("alpha1.png");
	_pLine->setUV(5, 5, 2, 5);
	_pLine->setAnchor(1, 500);
	_lineRot = 0.f;

	_isTouching = false;

	_pClip = lw::Clip::create("boom.png", 0, 1024, 0, 0, 64, 64, 50, 12, false);
}

void TaskMissile::vEnd(){
	delete _pBtnExit;

	lw::destroyAccelerometer();

	std::list<Missile*>::iterator it = _pMissiles.begin();
	std::list<Missile*>::iterator itEnd = _pMissiles.end();
	for ( ; it != itEnd; ++it ){
		delete *it;
	}
	_pMissiles.clear();
	delete _pMdlSprite;
	delete _pArrow;
	delete _pClip;
	delete _pLine;
}

void TaskMissile::vMain(float dt){
	std::list<Missile*>::iterator it = _pMissiles.begin();
	std::list<Missile*>::iterator itEnd = _pMissiles.end();
	for ( ; it != itEnd; ){
		if ( (*it)->main() ){
			delete (*it);
			it = _pMissiles.erase(it);
			itEnd = _pMissiles.end();
		}else{
			++it;
		}
	}

	if ( false && !_pMissiles.empty() ){
		const cml::Vector3& v3 = lw::getAccelerometer();
		Missile* pMissile = _pMissiles.back();
		pMissile->steer(v3[0]*1.5f);
	}

	if ( false && !_pMissiles.empty() ){
		const cml::Vector3& v3 = lw::getAccelerometer();
		cml::Vector2 v2;
		v2[0] = v3[0];
		v2[1] = v3[1];
		if ( fabs(v2[0]) < 0.001f && fabs(v2[1]) < 0.001f ){
			return;
		}
		v2.normalize();

		Missile* pMissile = _pMissiles.back();
		cml::Vector2 vForward;
		vForward[0] = 0.f;
		vForward[1] = 1.f;
		vForward = cml::rotate_vector_2D(vForward, pMissile->getBodyAngle());
		vForward.normalize();

		float dAngle = cml::signed_angle_2D(v2, vForward);
		pMissile->steer(dAngle/(float)M_PI);
	}

	if ( false && !_pMissiles.empty() && _isTouching ){
		const cml::Vector3& v3 = lw::getAccelerometer();
		cml::Vector2 v2;
		v2[0] = 0.f;
		v2[1] = 1.f;
		v2 = cml::rotate_vector_2D(v2, -v3[0]*0.5f*(float)M_PI);
		if ( fabs(v2[0]) < 0.001f && fabs(v2[1]) < 0.001f ){
			return;
		}
		v2.normalize();

		Missile* pMissile = _pMissiles.back();
		cml::Vector2 vForward;
		vForward[0] = 0.f;
		vForward[1] = 1.f;
		vForward = cml::rotate_vector_2D(vForward, pMissile->getBodyAngle());
		vForward.normalize();

		float dAngle = cml::signed_angle_2D(v2, vForward);
		pMissile->steer(dAngle/(float)M_PI*2.f);
	}

	if ( !_pMissiles.empty() && _isTouching ){
		const cml::Vector3& v3 = lw::getAccelerometer();
		float angle = -v3[0]*0.5f*(float)M_PI*ANGLE_SCALE;

		Missile* pMissile = _pMissiles.back();
		float missileAngle = pMissile->getBodyAngle();
		missileAngle = cml::lerp(missileAngle, angle, 0.4f);
		pMissile->setBodyAngle(missileAngle);
	}

	_pClip->main(dt);

}

void TaskMissile::vDraw(float dt){
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);

	cml::Matrix44 m;
	m.identity();
	cml::matrix_set_translation(m, 160.f, -240.f, 0.f);
	_pMdlSprite->collect(m);

	{
		const cml::Vector3& v3 = lw::getAccelerometer();
		cml::Matrix44 m, mTrans, mRot;
		m.identity();
		mTrans.identity();
		mRot.identity();
		cml::matrix_translation(mTrans, 160.f, 480.f, 0.f);
		float rot = v3[0]*0.5f*(float)M_PI*ANGLE_SCALE;
		cml::matrix_rotation_world_z(mRot, rot);
		m = mTrans * mRot;
		_pArrow->collect(m);

		rot = cml::lerp(_lineRot, rot, 0.2f);
		_pLine->collect(160.f, 480.f, 1.f, 500.f, rot);
		_lineRot = rot;
	}

	std::list<Missile*>::iterator it = _pMissiles.begin();
	std::list<Missile*>::iterator itEnd = _pMissiles.end();
	for ( ; it != itEnd; ++it ){
		(*it)->collect();
	}

	_pClip->getSprite()->collect(100.f, 200.f);
}


bool TaskMissile::vOnTouchEvent(std::vector<lw::TouchEvent>& events){
	lw::TouchEvent& evt = events[0];
	
#ifdef WIN32
	if ( evt.type == lw::TouchEvent::TOUCH || evt.type == lw::TouchEvent::MOVE ){
		cml::Vector3 v3;
		v3[0] = ((float)evt.x-160.f)/160.f;
		v3[1] = -((float)evt.y-240.f)/240.f;
		v3[2] = -1.f;
		lw::setAccelerometer(v3);
	}
#endif

	if ( evt.type == lw::TouchEvent::TOUCH ){
		_isTouching = true;
		if ( !_pMissiles.empty() ){
			_pMissiles.back()->steer(0.f);
		}

		const cml::Vector3 v3 = lw::getAccelerometer();
		Missile* pMissile = new Missile(0.f, 0.f, -v3[0]*0.5f*(float)M_PI);
		_pMissiles.push_back(pMissile);
		_pClip->setFrame(0);
	}else if ( evt.type == lw::TouchEvent::UNTOUCH ){
		_isTouching = false;
		if ( !_pMissiles.empty() ){
			_pMissiles.back()->steer(0.f);
		}
	}

	return false;
}

TaskMissile taskMissile;