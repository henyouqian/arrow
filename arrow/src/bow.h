#ifndef __BOW_H__
#define __BOW_H__

class Arrow;

class Bow{
public:
	Bow(float lineY, std::list<Arrow*>& arrows);
	~Bow();
	void draw();
	bool onGestureUpdate(const lw::Gesture& gesture);	//return true when arrow shoot
	void clear();

private:
	struct ArrowGestureInfo{
		int gestureId;
		int x, y;
		int startX, startY;
		float angle;
	};
	std::list<ArrowGestureInfo> _arrowGestureInfos;
	lw::Sprite* _pSptArrow;
	float _lineY;
	std::list<Arrow*>& _arrows;
};

#endif //__BOW_H__