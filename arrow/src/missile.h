#ifndef __MISSILE_H__
#define __MISSILE_H__

class Missile{
public:
	Missile(float x, float y, float angle);
	~Missile();

	bool main();
	void collect();

	void steer(float steer);
	float getSpeedAngle(){
		return _speedAngle;
	}
	float getBodyAngle(){
		return _bodyAngle;
	}
	void setBodyAngle(float angle){
		_bodyAngle = angle;
	}

private:
	cml::Vector2 _pos;
	float _bodyAngle;
	float _speedAngle;
	float _speed;
	float _steer;
	lw::Sprite* _pSprite;
};


#endif //__MISSILE_H__