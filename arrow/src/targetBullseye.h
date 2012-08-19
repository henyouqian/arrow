#ifndef __BULLSEYE_H__
#define __BULLSEYE_H__

#include "target.h"

class Arrow;

class TargetBullseye : public Target{
public:
	TargetBullseye(float scale);
	~TargetBullseye();

	void setPos(float x, float y){
		_pos.Set(x, y);
	}
	const b2Vec2& getPos(){
		return _pos;
	}
	void setAngle(float angle){
		_angle = angle;
	}
	float getAngle(){
		return _angle;
	}
	virtual bool main(float dt);
	virtual void collect();
	virtual void checkArrow(Arrow* pArrow);
	
private:
	lw::Sprite* _pSptTop;
	lw::Sprite* _pSptBottom;
	lw::Sprite* _pSptArrow;
	lw::Sprite* _pShadow;
	b2Vec2 _pos;
	float _scale;
	float _angle;
	b2PolygonShape _shape;
	std::list<cml::Matrix44> _localTrans;
};

#endif //__BULLSEYE_H__