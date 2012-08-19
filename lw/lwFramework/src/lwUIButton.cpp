#include "stdafx.h"
#include "lwUIButton.h"
#include "lwTouchEvent.h"
#include "lwDB.h"
#include "lwSprite.h"
#include "lwFont.h"
#include "lwUIImage.h"

namespace lw{

	Button* Button::createButton1(const char* style, const char* fontName){
		lwassert(style);
		bool ok = false;
		Button* p = new Button1(style, fontName, ok);
		lwassert(p);
		if ( !ok ){
			lwerror("new Button1 failed: style="<<style);
			delete p;
			return NULL;
		}
		return p;
	}

	Button* Button::createButton1(const char* file, int upU, int upV, int downU, int downV, int w, int h, const char* fontName){
		lwassert(file);
		bool ok = false;
		Button* p = new Button1(file, upU, upV, downU, downV, w, h, fontName, ok);
		lwassert(p);
		if ( !ok ){
			lwerror("new Button1 failed: file="<<file);
			delete p;
			return NULL;
		}
		return p;
	}

	Button* Button::createButton1(const char* file, int upU, int upV, int downU, int downV, int disableU, int disableV, int w, int h, const char* fontName){
		lwassert(file);
		bool ok = false;
		Button* p = new Button1(file, upU, upV, downU, downV, disableU, disableV, w, h, fontName, ok);
		lwassert(p);
		if ( !ok ){
			lwerror("new Button1 failed: file="<<file);
			delete p;
			return NULL;
		}
		return p;
	}

	Button* Button::createButton9(const char* style, const char* fontName){
		lwassert(style);
		bool ok = false;
		Button* p = new Button9(style, fontName, ok);
		lwassert(p);
		if ( !ok ){
			lwerror("new Button9 failed: style="<<style);
			delete p;
			return NULL;
		}
		return p;
	}

	Button* Button::createButton9(const char* file, int upU, int upV, int downU, int downV,
	int w1, int w2, int w3, int h1, int h2, int h3, const char* fontName){
		lwassert(file);
		bool ok = false;
		Button* p = new Button9(file, upU, upV, downU, downV, w1, w2, w3, h1, h2, h3, fontName, ok);
		lwassert(p);
		if ( !ok ){
			lwerror("new Button9 failed: file="<<file);
			delete p;
			return NULL;
		}
		return p;
	}

	Button::Button(const char* fontName)
	:_pCallback(NULL), _isTracing(false), _isDown(false), _pFont(NULL)
	,_topExt(0), _bottomExt(0), _leftExt(0), _rightExt(0), _scale(1.f){
		if ( fontName ){
			if ( fontName ){
				_pFont = lw::Font::create(fontName);
				if ( _pFont == NULL ){
					lwerror("lw::Font::create failed: fontName=" << fontName);
					return;
				}
			}
		}
	}

	Button::~Button(){
		if ( _pFont ){
			delete _pFont;
		}
	}

	void Button::vMain(float dt){
		_textX = _x - (_textW>>1) + (_w>>1);
		_textY = _y - (_textH>>1) + (_h>>1);
	}

	bool Button::vEvent(std::vector<lw::TouchEvent>& events){
		const lw::TouchEvent& evt = events[0];
		switch (evt.type)
		{
		case lw::TouchEvent::TOUCH:
			if ( evt.x >= _x-_leftExt && evt.x < _x+_w+_rightExt 
			&& evt.y >= _y-_topExt && evt.y < _y+_h+_bottomExt ){
				_isTracing = true;
				_isDown = true;
				if ( _pCallback ){
					_pCallback->onButtonDown(this);
				}
				g_gestrueMgr.deleteGesture(evt.x, evt.y);
				return true;
			}
			break;
		case lw::TouchEvent::UNTOUCH:{
				bool isDown = _isDown;
				_isTracing = false;
				_isDown = false;
				if ( isDown && _pCallback ){
					_pCallback->onButtonClick(this);
					return true;
				}
			}
			break;
		case lw::TouchEvent::MOVE:
			if ( _isTracing ){
				if ( evt.x >= _x-_leftExt && evt.x < _x+_w+_rightExt 
					&& evt.y >= _y-_topExt && evt.y < _y+_h+_bottomExt ){
					_isDown = true;
				}else{
					_isDown = false;
				}
				return true;
			}
			break;
		}
		return false;
	}

	void Button::setText(const wchar_t* text){
		lwassert(text);
		if ( !_pFont ){
			lwerror("no font");
			return;
		}
		_text = text;
		_pFont->getSize(_textW, _textH, text);
	}

	void Button::setTextColor(const lw::Color& color){
		_textColor = color;
		_textDownColor = color;
	}

	void Button::setTextColor(const lw::Color& color, const lw::Color& downColor){
		_textColor = color;
		_textDownColor = downColor;
	}

	Button1::Button1(const char* styleName, const char* fontName, bool& ok)
	:Button(fontName), _pSprite(NULL), _disableU(-1), _disableV(-1){
		ok = false;
		sqlite3_stmt* pStmt = NULL;
		std::string str = "select imageFile, w, h, upU, upV, downU, downV from uiButton where style='";
		str.append(styleName);
		str.append("'");
		int r = sqlite3_prepare_v2(g_lwDB, str.c_str(), -1, &pStmt, NULL);
		lwassert(r == SQLITE_OK);
		bool found = false;
		std::string file;
		while ( 1 ){
			r = sqlite3_step(pStmt);
			if ( r == SQLITE_ROW ){
				found = true;
				file = (const char*)sqlite3_column_text(pStmt, 0);
				_w = sqlite3_column_int(pStmt, 1);
				_h = sqlite3_column_int(pStmt, 2);
				_upU = sqlite3_column_int(pStmt, 3);
				_upV = sqlite3_column_int(pStmt, 4);
				_downU = sqlite3_column_int(pStmt, 5);
				_downV = sqlite3_column_int(pStmt, 6);
			}else if ( r == SQLITE_DONE ){
				break;
			}else{
				lwassert(0);
				break;
			}
		}
		sqlite3_finalize(pStmt);
		if ( !found ){
			lwerror("button style not found: " << styleName);
			return;
		}

		_pSprite = lw::Sprite::create(file.c_str());
		if ( _pSprite == NULL ){
			lwerror("lw::Sprite::create failed: style=" << styleName);
			return;
		}

		ok = true;
	}

	Button1::Button1(const char* file, int upU, int upV, int downU, int downV, int w, int h, const char* fontName, bool& ok)
	:Button(fontName), _pSprite(NULL), _upU(upU), _upV(upV), _downU(downU), _downV(downV), _disableU(-1), _disableV(-1){
		_w = w;
		_h = h;
		_pSprite = lw::Sprite::create(file);
		if ( _pSprite == NULL ){
			lwerror("lw::Sprite::create file: style=" << file);
			return;
		}
		ok = true;
	}

	Button1::Button1(const char* file, int upU, int upV, int downU, int downV, int disableU, int disableV, int w, int h, const char* fontName, bool& ok)
	:Button(fontName), _pSprite(NULL), _upU(upU), _upV(upV), _downU(downU), _downV(downV), _disableU(disableU), _disableV(disableV){
		_w = w;
		_h = h;
		_pSprite = lw::Sprite::create(file);
		if ( _pSprite == NULL ){
			lwerror("lw::Sprite::create file: style=" << file);
			return;
		}
		ok = true;
	}

	Button1::~Button1(){
		if ( _pSprite ){
			delete _pSprite;
		}
	}

	void Button1::vDraw(){
		if ( !_enable && _disableU >= 0 ){
			_pSprite->setUV(_disableU, _disableV, _w, _h);
		}else{
			if ( _isDown ){
				_pSprite->setUV(_downU, _downV, _w, _h);
			}else{
				_pSprite->setUV(_upU, _upV, _w, _h);
			}
		}

		lw::Color* pColor = &_color;
		lw::Color* pFontColor = &_textColor;
		if ( _isDown ){
			pColor = &_downColor;
			pFontColor = &_textDownColor;
		}
		
		if ( _scale != 1.f ){
			_pSprite->collect((float)_x-(_w*.5f*(_scale-1.f)), (float)_y-(_h*.5f*(_scale-1.f)), (float)_w*_scale, (float)_h*_scale, 0, false, false, *pColor);
		}else{
			_pSprite->collect((float)_x, (float)_y, *pColor);
		}
		if ( _pFont ){
			_pFont->drawText(_text.c_str(), (float)_textX, (float)_textY, *pFontColor, 1.f);
		}
	}

	Button9::Button9(const char* styleName, const char* fontName, bool& ok)
	:Button(fontName){
		ok = false;
		sqlite3_stmt* pStmt = NULL;
		std::string str = "select imageFile, upU, upV, downU, downV, w1, w2, w3, h1, h2, h3 from uiButton9 where style='";
		str.append(styleName);
		str.append("'");
		int r = sqlite3_prepare_v2(g_lwDB, str.c_str(), -1, &pStmt, NULL);
		lwassert(r == SQLITE_OK);
		bool found = false;
		std::string file;
		int upU, upV, downU, downV, w1, w2, w3, h1, h2, h3;
		while ( 1 ){
			r = sqlite3_step(pStmt);
			if ( r == SQLITE_ROW ){
				found = true;
				file = (const char*)sqlite3_column_text(pStmt, 0);
				upU = sqlite3_column_int(pStmt, 1);
				upV = sqlite3_column_int(pStmt, 2);
				downU = sqlite3_column_int(pStmt, 3);
				downV = sqlite3_column_int(pStmt, 4);
				w1 = sqlite3_column_int(pStmt, 5);
				w2 = sqlite3_column_int(pStmt, 6);
				w3 = sqlite3_column_int(pStmt, 7);
				h1 = sqlite3_column_int(pStmt, 8);
				h2 = sqlite3_column_int(pStmt, 9);
				h3 = sqlite3_column_int(pStmt, 10);
				break;
			}else if ( r == SQLITE_DONE ){
				break;
			}else{
				lwassert(0);
				break;
			}
		}
		sqlite3_finalize(pStmt);
		if ( !found ){
			lwerror("button style not found: " << styleName);
			return;
		}

		_pUpImage = lw::UIImage9::create(file.c_str(), upU, upV, w1, w2, w3, h1, h2, h3);
		if ( _pUpImage == NULL ){
			lwerror("lw::UIImage9::create failed: style=" << styleName);
			return;
		}
		_pUpImage->setParent(this);

		_pDownImage = lw::UIImage9::create(file.c_str(), downU, downV, w1, w2, w3, h1, h2, h3);
		if ( _pDownImage == NULL ){
			lwerror("lw::UIImage9::create failed: style=" << styleName);
			return;
		}
		_pDownImage->setParent(this);
		_pDownImage->show(false);

		ok = true;
	}

	Button9::Button9(const char* file, int upU, int upV, int downU, int downV,
		int w1, int w2, int w3, int h1, int h2, int h3, const char* fontName, bool& ok)
	:Button(fontName){
		ok = false;

		_pUpImage = lw::UIImage9::create(file, upU, upV, w1, w2, w3, h1, h2, h3);
		if ( _pUpImage == NULL ){
			lwerror("lw::UIImage9::create failed: file=" << file);
			return;
		}
		_pUpImage->setParent(this);

		_pDownImage = lw::UIImage9::create(file, downU, downV, w1, w2, w3, h1, h2, h3);
		if ( _pDownImage == NULL ){
			lwerror("lw::UIImage9::create failed: file=" << file);
			return;
		}
		_pDownImage->setParent(this);
		_pDownImage->show(false);

		ok = true;
	}

	Button9::~Button9(){
		if ( _pUpImage ){
			delete _pUpImage;
		}
		if ( _pDownImage ){
			delete _pDownImage;
		}
	}

	void Button9::vMain(float dt){
		Button::vMain(dt);
		_pUpImage->setSize(_w, _h);
		_pDownImage->setSize(_w, _h);
	}

	void Button9::vDraw(){
		if ( _isDown ){
			_pDownImage->setColor(_downColor);
			_pUpImage->show(false);
			_pDownImage->show(true);
		}else{
			_pUpImage->setColor(_color);
			_pUpImage->show(true);
			_pDownImage->show(false);
		}
	}

	void Button9::vDrawFG(){
		lw::Color* pTextColor;
		if ( _isDown ){
			pTextColor = &_textDownColor;
		}else{
			pTextColor = &_textColor;
		}
		_pFont->drawText(_text.c_str(), (float)_textX, (float)_textY, *pTextColor);
	}

} //namespace lw