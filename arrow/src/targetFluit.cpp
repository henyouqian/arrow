#include "stdafx.h"
#include "targetFluit.h"
#include "arrow.h"
#include "dart.h"
#include "soundManager.h"

TargetFluit::TargetFluit(const char* podFile, const char* name, b2World* pWorld, const b2Vec2& pos, const b2Vec2& lv, float av, WEAPON_TYPE wt)
:_pWorld(pWorld), _wt(wt), _score(0.f){
	_pMdlSpt = lw::ModelSprite::create(podFile, name);
	lwassert(_pMdlSpt);

	const std::vector<lw::Sprite::Vertex>& vec = _pMdlSpt->getVertexVec();
	lwassert(vec.size()%3 == 0);

	b2BodyDef bd;
	bd.type = b2_dynamicBody;
	bd.allowSleep = false;
	bd.linearDamping = 0.3f;
	bd.angularDamping = 2.2f;
	bd.bullet = true;
	bd.position.Set(pos.x*B2_SCALE, pos.y*B2_SCALE);
	bd.linearVelocity.Set(lv.x*B2_SCALE, lv.y*B2_SCALE);
	bd.angularVelocity = av;
	_pBody = _pWorld->CreateBody(&bd);

	b2PolygonShape polygonShape;
	b2Vec2 b2v[3];
	for ( int i = 0; i < (int)vec.size(); ){
		b2v[0].Set(vec[i].x*B2_SCALE*0.9f, vec[i].y*B2_SCALE*0.9f);
		++i;
		b2v[1].Set(vec[i].x*B2_SCALE*0.9f, vec[i].y*B2_SCALE*0.9f);
		++i;
		b2v[2].Set(vec[i].x*B2_SCALE*0.9f, vec[i].y*B2_SCALE*0.9f);
		++i;
		polygonShape.Set(b2v, 3);
		b2FixtureDef fd;
		fd.density = 1.f;
		fd.shape = &polygonShape;
		fd.filter.groupIndex = -1;
		fd.friction = 0.2f;
		fd.restitution = 0.5f;
		_pBody->CreateFixture(&fd);
	}

	_pSpt = lw::Sprite::create("alpha1.png");
	if ( wt == WT_ARROW ){
		_pSpt->setUV(506, 0, 6, 66);
		_pSpt->setAnchor(3.f, 4.f);
	}else if ( wt == WT_DART ){
		_pSpt->setUV(0, 165, 24, 24);
		_pSpt->setAnchor(12.f, 12.f);
	}
}

TargetFluit::~TargetFluit(){
	delete _pMdlSpt;
	delete _pSpt;
	_pWorld->DestroyBody(_pBody);
}

bool TargetFluit::main(float dt){
	const b2Vec2& pos = _pBody->GetPosition();
	if ( pos.y < -2.f ){
		return true;
	}
	return false;
}

void TargetFluit::collect(){
	{
		std::list<cml::Matrix44>::iterator it = _localTrans.begin();
		std::list<cml::Matrix44>::iterator itEnd = _localTrans.end();
		const b2Vec2& pos = _pBody->GetPosition();
		float angle = _pBody->GetAngle();
		cml::Matrix44 m, m1;
		cml::matrix_translation(m, pos.x/B2_SCALE+160.f, -pos.y/B2_SCALE+480.f);
		cml::matrix_rotation_world_z(m1, -angle);
		m *= m1;
		for ( ; it != itEnd; ++it ){
			m1 = m * (*it);
			//m1 = m;
			
			_pSpt->collect(m1);
		}
	}
	{
		const b2Vec2& pos = _pBody->GetPosition();
		float screenX = pos.x/B2_SCALE+160.f;
		float screenY = -pos.y/B2_SCALE+480.f;
		float angle = _pBody->GetAngle();
		cml::Matrix44 m, m1;
		cml::matrix_translation(m, screenX, screenY, 0.f);
		cml::matrix_rotation_world_z(m1, angle);
		m *= m1;
		_pMdlSpt->collect(m);
	}
}

void TargetFluit::checkArrow(Arrow* pArrow){
	b2RayCastOutput output;
	b2RayCastInput input;
	pArrow->makeRayCastInput(input);
	float dist = pArrow->getDist();
	b2Fixture* pFixture = _pBody->GetFixtureList();
	float maxDist = 0.f;
	while ( pFixture ){
		bool b = pFixture->GetShape()->RayCast(&output, input, _pBody->GetTransform());
		if ( b ){
			maxDist = max(maxDist, dist*(1.f-output.fraction));
		}
		pFixture = pFixture->GetNext();
	}
	float hitDist = dist - maxDist;
	//const float OVER_DIST = 0.45f;
    const float OVER_DIST = cml::random_float(0.3f, 0.45f);
	if ( maxDist > OVER_DIST && maxDist < (ARROW_LENGTH+ARROW_SPEED)*B2_SCALE ){
		pArrow->die();

		b2Vec2 v = input.p2 - input.p1;
		v.Normalize();
		b2Vec2 vForward = v;
		float d = hitDist+OVER_DIST;
		v.x *= d;
		v.y *= d;
		v = input.p1 + v;

		b2Vec2 pos = _pBody->GetLocalPoint(v);
		float dAngle = pArrow->getAngle() - _pBody->GetAngle();
		cml::Matrix44 m, m1;
		cml::matrix_translation(m, pos.x/B2_SCALE, -pos.y/B2_SCALE);
		cml::matrix_rotation_world_z(m1, -dAngle);
		m *= m1;
		_localTrans.push_back(m);

		//vForward.x *= 2.5f; vForward.y *= 2.5f;
		v = _pBody->GetLocalPoint(v);
		v.x *= 0.5f; v.y *= 0.5f;   //rotate slower
		v = _pBody->GetWorldPoint(v);
        
        cml::Vector2 v2;
        pArrow->getSpeed(v2);
        b2Vec2 velArrow(v2[0], v2[1]);
        b2Vec2 velSelf(_pBody->GetLinearVelocity());
        b2Vec2 dVel = velArrow - velSelf;
        b2Vec2 imp = dVel;
        dVel.Normalize();
        velArrow.Normalize();
        float f = b2Dot(dVel, velArrow);
        //lwinfo(velArrow.y << "  " << velSelf.y);
        f *= 0.07f;
        imp.x *= f;
        imp.y *= f;
        lwinfo(imp.y);
		_pBody->ApplyLinearImpulse(imp, v);

		g_pSndHit->play();
	}
}

void TargetFluit::checkDart(Dart* pDart){
	b2RayCastOutput output;
	b2RayCastInput input;
	pDart->makeRayCastInput(input);
	float dist = pDart->getDist();
	b2Fixture* pFixture = _pBody->GetFixtureList();
	float maxDist = -1000.f;
	while ( pFixture ){
		bool b = pFixture->GetShape()->RayCast(&output, input, _pBody->GetTransform());
		if ( b ){
			maxDist = max(maxDist, dist*(1.f-output.fraction));
		}
		pFixture = pFixture->GetNext();
	}
	float hitDist = dist - maxDist;
	const float OVER_DIST = 0.05f;
	if ( maxDist > OVER_DIST && maxDist < (DART_LENGTH+pDart->getSpeed())*B2_SCALE ){
		pDart->die();

		b2Vec2 v = input.p2 - input.p1;
		v.Normalize();
		b2Vec2 vForward = v;
		float d = hitDist+OVER_DIST;
		v.x *= d;
		v.y *= d;
		v = input.p1 + v;

		b2Vec2 pos = _pBody->GetLocalPoint(v);
		float dAngle = pDart->getAngle() - _pBody->GetAngle();
		cml::Matrix44 m, m1;
		cml::matrix_translation(m, pos.x/B2_SCALE, -pos.y/B2_SCALE);
		cml::matrix_rotation_world_z(m1, -dAngle+pDart->getSpin());
		m *= m1;
		_localTrans.push_back(m);

		float dartSpd = pDart->getSpeed();
		vForward.x *= dartSpd; vForward.y *= dartSpd;
		b2Vec2 vBodyVel = _pBody->GetLinearVelocity();
		vForward = vForward - vBodyVel;
		vForward.x *= 0.03f;
		vForward.y *= 0.03f;

		v = _pBody->GetLocalPoint(v);
		v.x *= 0.1f; v.y *= 0.1f;
		v = _pBody->GetWorldPoint(v);
		_pBody->ApplyLinearImpulse(vForward, v);

		g_pSndHit->play();
	}
}

void TargetFluit::getScreenPos(float& x, float& y){
    const b2Vec2& pos = _pBody->GetPosition();
    x = pos.x/B2_SCALE+160.f;
    y = -pos.y/B2_SCALE+480.f;
}

FluitEmitter::FluitEmitter(std::list<Target*>& targets, b2World* pWorld):_targets(targets), _pWorld(pWorld){
	reset();
}

void FluitEmitter::reset(){
	_t = 0.f;
}

void FluitEmitter::main(float dt, WEAPON_TYPE wt){
	_t -= dt;
	if ( _targets.empty() ){
		_t = cml::random_float(1500.f, 2500.f);
		float posX = cml::random_float(-100.f, 100.f);
		float lvX = cml::random_float(-400.f, 400.f);
		float lvY = cml::random_float(1000.f, 1200.f);
		float av = cml::random_float(-20.f, 20.f);
		int type = cml::random_integer(0, 1);
		Target* pTarget = NULL;
		if ( type == 0 ){
			pTarget = new TargetFluit("fluit.pod", "apple", _pWorld, b2Vec2(posX, -40.f), b2Vec2(lvX, lvY), av, wt);
		}else{
			pTarget = new TargetFluit("fluit.pod", "banana", _pWorld, b2Vec2(posX, -40.f), b2Vec2(lvX, lvY), av, wt);
		}
		
		_targets.push_back(pTarget);
	}
}