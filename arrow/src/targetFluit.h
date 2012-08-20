#ifndef __TARGET_FLUIT_H__
#define __TARGET_FLUIT_H__

#include "target.h"

enum WEAPON_TYPE{
	WT_ARROW,
	WT_DART,
};

class TargetFluit : public Target{
public:
	TargetFluit(const char* podFile, const char* name, b2World* pWorld, const b2Vec2& pos, const b2Vec2& lv, float av, WEAPON_TYPE wt);
	~TargetFluit();

	virtual bool main(float dt);
	virtual void collect();
	virtual void checkArrow(Arrow* pArrow);
	virtual void checkDart(Dart* pDart);
    virtual void getScreenPos(float& x, float& y);
	void setScore(float score){
		_score = score;
	}

private:
	lw::ModelSprite* _pMdlSpt;
	std::list<cml::Matrix44> _localTrans;
	lw::Sprite* _pSpt;
	WEAPON_TYPE _wt;
	float _score;
	b2World* _pWorld;
	b2Body* _pBody;
};

class FluitEmitter{
public:
	FluitEmitter(std::list<Target*>& targets, b2World* pWorld);
	void reset();
	void main(float dt, WEAPON_TYPE wt);

private:
	float _t;
	std::list<Target*>& _targets;
	b2World* _pWorld;
};


#endif //__TARGET_FLUIT_H__