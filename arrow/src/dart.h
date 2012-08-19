#ifndef __DART_H__
#define __DART_H__

extern const float DART_LENGTH; 

class Dart{
public:
	Dart(float x, float y, float angle, float speed, float curve);
	~Dart();

	bool main();
	void collect();
	void die(){
		_isDie = true;
	}
	bool isDie(){
		return _isDie;
	}
	void makeRayCastInput(b2RayCastInput& input);
	float getDist();
	float getAngle(){
		return _angle;
	}
	float getSpeed(){
		return _speed;
	}
	float getSpin(){
		return _spin;
	}

private:
	cml::Vector2 _origin;
	float _dist;
	float _angle;
	lw::Sprite* _pSprite;
	bool _isDie;
	float _speed;
	float _spin;
	float _curve;
};


#endif //__DART_H__