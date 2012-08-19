#include "stdafx.h"
#include "targetPod.h"
#include "arrow.h"
#include "taskArrow.h"

TargetPod::TargetPod(const char* podFile, const char* name, lw::Sounds* pSndHit)
:_score(0.f), _angle(0.f), _pSndHit(pSndHit){
	_pMdlSpt = lw::ModelSprite::create(podFile, name);
	lwassert(_pMdlSpt);

	const std::vector<lw::Sprite::Vertex>& vec = _pMdlSpt->getVertexVec();
	lwassert(vec.size()%3 == 0);

	b2Vec2 b2v[3];
	float scale = B2_SCALE*.9f;
	
	for ( int i = 0; i < (int)vec.size(); ){
		b2v[0].Set(vec[i].x*scale, vec[i].y*scale);
		++i;
		b2v[1].Set(vec[i].x*scale, vec[i].y*scale);
		++i;
		b2v[2].Set(vec[i].x*scale, vec[i].y*scale);
		++i;
		b2PolygonShape shape;
		shape.Set(b2v, 3);
		_shapes.push_back(shape);
	}

	_pSpt = lw::Sprite::create("alpha1.png");
	_pSpt->setUV(506, 0, 6, 66);
	_pSpt->setAnchor(3.f, 4.f);

	_pos.x = _pos.y = 0.f;
}

TargetPod::~TargetPod(){
	delete _pMdlSpt;
	delete _pSpt;
}

bool TargetPod::main(float dt){
	return false;
}

void TargetPod::collect(){
	{
		std::list<cml::Matrix44>::iterator it = _localTrans.begin();
		std::list<cml::Matrix44>::iterator itEnd = _localTrans.end();
		cml::Matrix44 m, m1;
		cml::matrix_translation(m, _pos.x+160.f, -_pos.y+480.f, 0.f);
		cml::matrix_rotation_world_z(m1, -_angle);
		m *= m1;
		for ( ; it != itEnd; ++it ){
			m1 = m * (*it);
			_pSpt->collect(m1);
		}
	}
	{
		cml::Matrix44 m, m1;
		cml::matrix_translation(m, _pos.x+160.f, -_pos.y+480.f, 0.f);
		cml::matrix_rotation_world_z(m1, _angle);
		m *= m1;
		_pMdlSpt->collect(m);
	}
}

void TargetPod::checkArrow(Arrow* pArrow){
	float score = 0.f;
	b2RayCastOutput output;
	b2RayCastInput input;
	pArrow->makeRayCastInput(input);
	float dist = pArrow->getDist();
	b2Transform trans;
	b2Vec2 adjPos = _pos;
	adjPos.Set(adjPos.x*B2_SCALE, adjPos.y*B2_SCALE);
	trans.Set(adjPos, _angle);

	bool hit = false;
	{
		std::vector<b2PolygonShape>::iterator it = _shapes.begin();
		std::vector<b2PolygonShape>::iterator itEnd = _shapes.end();
		for ( ; it != itEnd; ++it ){
			hit = it->RayCast(&output, input, trans);
			if ( hit ){
				break;
			}
		}
	}
	if ( !hit ){
		return;
	}

	float overDist = dist*(1.f-output.fraction);
	float hitDist = dist - overDist;
	const float OVER_DIST = cml::random_float(.1f, .14f);
	if ( overDist > OVER_DIST && overDist < (ARROW_LENGTH+ARROW_SPEED)*B2_SCALE ){
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
			pArrow->setScore(_score);
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

		_pSndHit->play();
	}
}