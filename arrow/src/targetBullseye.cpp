#include "stdafx.h"
#include "targetBullseye.h"
#include "arrow.h"
#include "target.h"
#include "soundManager.h"

TargetBullseye::TargetBullseye(float scale)
:_scale(scale), _angle(0.f){
	_pSptTop = lw::Sprite::create("alpha1.png");
	_pSptTop->setUV(123, 0, 128, 38);
	_pSptTop->setAnchor(64*scale, 38*scale);

	_pSptBottom = lw::Sprite::create("alpha1.png");
	_pSptBottom->setUV(123, 38, 128, 29);
	_pSptBottom->setAnchor(64*scale, 0*scale);

	_pSptArrow = lw::Sprite::create("alpha1.png");
	_pSptArrow->setUV(506, 0, 6, 66);
	_pSptArrow->setAnchor(3.f, 4.f);

	_pShadow = lw::Sprite::create("alpha1.png");
	_pShadow->setUV(9, 77, 9, 9);
	_pShadow->setAnchor(4.f, 4.f);

	_pos.x = _pos.y = 0.f;

	float x = 64.f*scale*B2_SCALE;
	_shape.SetAsEdge(b2Vec2(-x, 0), b2Vec2(x, 0));
}

TargetBullseye::~TargetBullseye(){
	delete _pSptTop;
	delete _pSptBottom;
	delete _pSptArrow;
	delete _pShadow;
}

bool TargetBullseye::main(float dt){
	return false;
}

void TargetBullseye::checkArrow(Arrow* pArrow){
	b2RayCastOutput output;
	b2RayCastInput input;
	pArrow->makeRayCastInput(input);
	float dist = pArrow->getDist();
	b2Transform trans;
	b2Vec2 adjPos = _pos;
	adjPos.Set(adjPos.x*B2_SCALE, adjPos.y*B2_SCALE);
	trans.Set(adjPos, _angle);
	bool b = _shape.RayCast(&output, input, trans);
	if ( !b ){
		return;
	}

	float overDist = dist*(1.f-output.fraction);
	float hitDist = dist - overDist;
	const float OVER_DIST = cml::random_float(.1f, .14f);
	if ( overDist > OVER_DIST && overDist < (ARROW_LENGTH+ARROW_SPEED)*B2_SCALE-OVER_DIST ){
		pArrow->die();

		b2Vec2 v = input.p2 - input.p1;
		v.Normalize();
		b2Vec2 vForward = v;
		{
			b2Vec2 v2 = v;
			v2.x *= hitDist;
			v2.y *= hitDist;
			v2 = input.p1 + v2;
			v2.Set(v2.x/B2_SCALE, v2.y/B2_SCALE);
			v2 = v2 - _pos;
			float score = (64.f*_scale-v2.Length())/(64.f*_scale);
			pArrow->setScore(score);
		}
		float d = hitDist+OVER_DIST;
		v.x *= d;
		v.y *= d;
		v = input.p1 + v;

		v.Set(v.x/B2_SCALE, v.y/B2_SCALE);
		b2Vec2 pos = v - _pos;
		pos.y = -pos.y;
		b2Transform rot;
		rot.Set(b2Vec2_zero, _angle);
		pos = b2Mul(rot, pos);
		float dAngle = pArrow->getAngle() - _angle;
		cml::Matrix44 m, m1;
		cml::matrix_translation(m, pos.x, pos.y);
		cml::matrix_rotation_world_z(m1, -dAngle);
		m *= m1;
		_localTrans.push_back(m);

		g_pSndHit->play();
	}
}

void TargetBullseye::collect(){
	_pSptBottom->collect(_pos.x+160.f, 480.f-_pos.y, 128.f*_scale, 29.f*_scale, -_angle);

	{
		std::list<cml::Matrix44>::iterator it = _localTrans.begin();
		std::list<cml::Matrix44>::iterator itEnd = _localTrans.end();
		cml::Matrix44 m, m1;
		cml::matrix_translation(m, _pos.x+160.f, -_pos.y+480.f);
		cml::matrix_rotation_world_z(m1, -_angle);
		m *= m1;
		for ( ; it != itEnd; ++it ){
			m1 = m * (*it);
			_pSptArrow->collect(m1);
			//_pShadow->collect(m1);
		}
	}
	_pSptTop->collect(_pos.x+160.f, 480.f-_pos.y, 128.f*_scale, 38.f*_scale, -_angle);
}