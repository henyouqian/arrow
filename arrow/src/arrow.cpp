#include "stdafx.h"
#include "arrow.h"
#include "target.h"

const float ARROW_LENGTH = 66.0f;
const float ARROW_SPEED = 15.f;

Arrow::Arrow(float x, float y, float angle):_angle(angle), _dist(0.f), _isDie(false), _score(0){
	_origin[0] = x; _origin[1] = y;

	_pSprite = lw::Sprite::create("alpha1.png");
	_pSprite->setUV(506, 0, 6, (int)ARROW_LENGTH);
	_pSprite->setAnchor(3.f, 4.f);
}

Arrow::~Arrow(){
	delete _pSprite;
}

bool Arrow::main(){
	if ( _isDie ){
		return true;
	}
	_dist += ARROW_SPEED;

	if ( _dist > 600 ){
		return true;
	}
	return false;
}

void Arrow::collect(){
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
	
	_pSprite->collect(screenX, screenY, 0, 0, -_angle);
}

void Arrow::makeRayCastInput(b2RayCastInput& input){
	input.p1.x = _origin[0];
	input.p1.y = _origin[1];
	cml::Vector2 vForward;
	vForward[0] = 0.f;
	vForward[1] = 1.f;
	vForward = cml::rotate_vector_2D(vForward, _angle);
	vForward.normalize();
	input.p2.x = _origin[0]+vForward[0]*_dist;
	input.p2.y = _origin[1]+vForward[1]*_dist;
	
	input.p1.x *= B2_SCALE;
	input.p1.y *= B2_SCALE;
	input.p2.x *= B2_SCALE;
	input.p2.y *= B2_SCALE;
	input.maxFraction = 1.f;
}

float Arrow::getDist(){
	return _dist*B2_SCALE;
}