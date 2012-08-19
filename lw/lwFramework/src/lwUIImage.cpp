#include "stdafx.h"
#include "lwUIImage.h"
#include "lwSprite.h"
#include "lwTouchEvent.h"

namespace lw{

	UIImage* UIImage::create(const char* fileName, int uvX, int uvY, int uvW, int uvH, int w, int h){
		lwassert(fileName);
		bool ok = false;
		UIImage* pUIImage = new UIImage(fileName, uvX, uvY, uvW, uvH, w, h, ok);
		lwassert(pUIImage);
		if ( !ok ){
			delete pUIImage;
			return NULL;
		}
		return pUIImage;
	}

	UIImage::UIImage(const char* fileName, int uvX, int uvY, int uvW, int uvH, int w, int h, bool& ok):_pCallback(NULL){
		_pSprite = Sprite::create(fileName);
		if ( _pSprite == NULL ){
			lwerror("Sprite::create failed: file=" <<fileName);
			return;
		}
		_pSprite->setUV(uvX, uvY, uvW, uvH);
		_w = w;
		_h = h;

		ok = true;
	}

	UIImage::~UIImage(){
		if ( _pSprite ){
			delete _pSprite;
		}
	}

	void UIImage::vDraw(){
		_pSprite->collect((float)_x, (float)_y, (float)_w, (float)_h, 0, false, false, _color);
	}

	bool UIImage::vEvent(std::vector<lw::TouchEvent>& events){
		const lw::TouchEvent& evt = events[0];
		switch (evt.type)
		{
		case lw::TouchEvent::TOUCH:
			if ( evt.x >= _x && evt.x < _x + _w && evt.y >= _y && evt.y < _y +_h ){
				if ( _pCallback ){
					_pCallback->onImageTouch(this);
				}
				return true;
			}
			break;
		}
		return false;
	}

	UIImage9* UIImage9::create(const char* fileName, int uvX, int uvY, int uvW1, int uvW2, int uvW3, int uvH1, int uvH2, int uvH3){
		lwassert(fileName);
		bool ok = false;
		UIImage9* pUIImage9 = new UIImage9(fileName, uvX, uvY, uvW1, uvW2, uvW3, uvH1, uvH2, uvH3, ok);
		lwassert(pUIImage9);
		if ( !ok ){
			delete pUIImage9;
			return NULL;
		}
		return pUIImage9;
	}
	
	UIImage9::UIImage9(const char* fileName, int uvX, int uvY, int uvW1, int uvW2, int uvW3, int uvH1, int uvH2, int uvH3, bool& ok){
		_pSprite = Sprite::create(fileName);
		if ( _pSprite == NULL ){
			lwerror("Sprite::create failed: file=" <<fileName);
			return;
		}

		_uvW[0] = uvW1;
		_uvW[1] = uvW2;
		_uvW[2] = uvW3;

		_uvH[0] = uvH1;
		_uvH[1] = uvH2;
		_uvH[2] = uvH3;

		_uvX[0] = uvX;
		_uvX[1] = _uvX[0] + uvW1;
		_uvX[2] = _uvX[1] + uvW2;

		_uvY[0] = uvY;
		_uvY[1] = _uvY[0] + uvH1;
		_uvY[2] = _uvY[1] + uvH2;


		_w = _uvW[0] + _uvW[1] + _uvW[2];
		_h = _uvH[0] + _uvH[1] + _uvH[2];

		ok = true;
	}

	UIImage9::~UIImage9(){
		if ( _pSprite ){
			delete _pSprite;
		}
	}

	void UIImage9::vDraw(){
		int midW = max(0, _w - _uvW[0] - _uvW[2]);
		int midH = max(0, _h - _uvH[0] - _uvH[2]);
		int x[3];
		x[0] = _x;
		x[1] = x[0] + _uvW[0];
		x[2] = x[1] + midW;
		int y[3];
		y[0] = _y;
		y[1] = y[0] + _uvH[0];
		y[2] = y[1] + midH;

		int w[3];
		w[0] = _uvW[0];
		w[1] = midW;
		w[2] = _uvW[2];
		int h[3];
		h[0] = _uvH[0];
		h[1] = midH;
		h[2] = _uvH[2];
		
		for ( int i = 0; i < 3; ++i ){
			for ( int j = 0; j < 3; ++j ){
				_pSprite->setUV(_uvX[i], _uvY[j], _uvW[i], _uvH[j]);
				_pSprite->collect((float)x[i], (float)y[j], (float)w[i], (float)h[j], 0, false, false, _color);
			}
		}
	}
	

} //namespace lw