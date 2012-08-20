#ifndef __TARGET_H__
#define __TARGET_H__

extern const float B2_SCALE;

class Arrow;
class Dart;

class Target{
public:
	Target();
	virtual ~Target();

	virtual bool main(float dt) = 0;
	virtual void collect() = 0;
	virtual void checkArrow(Arrow* pArrow){}
	virtual void checkDart(Dart* pDart){}
    virtual void getScreenPos(float& x, float& y){x = 0; y = 0;}

protected:
	
};

#endif //__TARGET_H__