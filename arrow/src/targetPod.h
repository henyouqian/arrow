#ifndef __TARGET_POD_H__
#define __TARGET_POD_H__

#include "target.h"

class TargetPod : public Target{
public:
	TargetPod(const char* podFile, const char* name, lw::Sounds* pSndHit);
	~TargetPod();

	virtual bool main(float dt);
	virtual void collect();
	virtual void checkArrow(Arrow* pArrow);
	void setScore(float score){
		_score = score;
	}

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

private:
	lw::ModelSprite* _pMdlSpt;
	std::list<cml::Matrix44> _localTrans;
	lw::Sprite* _pSpt;
	float _score;
	b2Vec2 _pos;
	float _angle;
	std::vector<b2PolygonShape> _shapes;
	lw::Sounds* _pSndHit;
};


#endif //__TARGET_POD_H__