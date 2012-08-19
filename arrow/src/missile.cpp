#include "stdafx.h"
#include "missile.h"

namespace{
	const float PUSH_ACC = 0.05f;
	const float AIR_DRAG_FACTOR = 0.002f;
	const float MISSILE_SCALE = 0.5f;
	const float MISSILE_LENGTH = 35;
}

Missile::Missile(float x, float y, float angle):_bodyAngle(angle), _speedAngle(angle), _speed(0.f), _steer(0.f){
	_pos[0] = x; _pos[1] = y;

	_pSprite = lw::Sprite::create("alpha1.png");
	_pSprite->setUV(0, 0, 11, 35);
	_pSprite->setAnchor(5*MISSILE_SCALE, 14*MISSILE_SCALE);
}

Missile::~Missile(){
	delete _pSprite;
}

void Missile::steer(float steer){
	//_steer = cml::clamp(steer, -1.f, 1.f);
}

bool Missile::main(){
	float acc = PUSH_ACC - _speed*_speed*AIR_DRAG_FACTOR;
	_speed += acc;

	float ds = _speed;
	float dx = ds*sin(_steer);
	cml::Vector2 v1, v2;
	v1[0] = 0.f; v1[1] = 1.f;
	v2[0] = dx; v2[1] = MISSILE_LENGTH;
	v2.normalize();
	float angle = cml::signed_angle_2D(v2, v1);
	_bodyAngle -= angle * fabs(cos(_steer));

	//_bodyAngle -= _steer*0.05f;
	_speedAngle = cml::lerp(_speedAngle, _bodyAngle, 0.05f);

	cml::Vector2 vForward;
	vForward[0] = 0.f;
	vForward[1] = 1.f;
	vForward = cml::rotate_vector_2D(vForward, _speedAngle);
	vForward *= _speed;
	_pos += vForward;

	if ( _pos[0] < -320 || _pos[0] > 320 || _pos[1] > 800 || _pos[1] < -200 ){
		return true;
	}
	return false;
}

void Missile::collect(){
	float screenX = _pos[0]+160.f;
	float screenY = -_pos[1]+480.f;
	
	_pSprite->collect(screenX, screenY, _pSprite->getW()*MISSILE_SCALE, _pSprite->getH()*MISSILE_SCALE, -_bodyAngle);
}