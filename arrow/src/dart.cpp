#include "stdafx.h"
#include "dart.h"
#include "target.h"

const float DART_LENGTH = 24.0f;

Dart::Dart(float x, float y, float angle, float speed, float curve):_angle(angle), _dist(0.f), _isDie(false), _speed(speed), _curve(curve){
	_origin[0] = x; _origin[1] = y;

	_pSprite = lw::Sprite::create("alpha1.png");
	_pSprite->setUV(0, 165, 24, 24);
	_pSprite->setAnchor(12.f, 12.f);
	_spin = 0.f;
}

Dart::~Dart(){
	delete _pSprite;
}

bool Dart::main(){
	if ( _isDie ){
		return true;
	}
	_dist += _speed;
	_angle += _curve;

	if ( _dist > 600 ){
		return true;
	}

	_spin += 0.2f;
	return false;
}

void Dart::collect(){
	if ( _isDie ){
		return;
	}
	cml::Vector2 vForward;
	vForward[0] = 0.f;
	vForward[1] = 1.f;
	vForward = cml::rotate_vector_2D(vForward, _angle);
	vForward.normalize();
	cml::Vector2 pos;
	pos = _origin + (vForward * _dist);
	float screenX = pos[0]+160.f;
	float screenY = -pos[1]+480.f;
	
	_pSprite->collect(screenX, screenY, 0, 0, -_angle+_spin);
}

void Dart::makeRayCastInput(b2RayCastInput& input){
	input.p1.x = _origin[0];
	input.p1.y = _origin[1];
	cml::Vector2 vForward;
	vForward[0] = 0.f;
	vForward[1] = 1.f;
	vForward = cml::rotate_vector_2D(vForward, _angle);
	vForward.normalize();
	input.p2.x = _origin[0]+vForward[0]*(_dist+14.f);
	input.p2.y = _origin[1]+vForward[1]*(_dist+14.f);
	
	input.p1.x *= B2_SCALE;
	input.p1.y *= B2_SCALE;
	input.p2.x *= B2_SCALE;
	input.p2.y *= B2_SCALE;
	input.maxFraction = 1.f;
}

float Dart::getDist(){
	return _dist*B2_SCALE;
}