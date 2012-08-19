#ifndef __ARROW_H__
#define __ARROW_H__

extern const float ARROW_LENGTH; 
extern const float ARROW_SPEED;

class Arrow{
public:
	Arrow(float x, float y, float angle);
	~Arrow();

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
	void setScore(float score){
		_score = score;
	}
	float getScore(){
		return _score;
	}

private:
	cml::Vector2 _origin;
	float _dist;
	float _angle;
	lw::Sprite* _pSprite;
	bool _isDie;
	float _score;
};


#endif //__ARROW_H__