#include "stdafx.h"
#include "lwUICheckBox.h"
#include "lwTouchEvent.h"
#include "lwDB.h"
#include "lwSprite.h"
#include "lwFont.h"
#include "lwUIImage.h"

namespace lw{

	CheckBox* CheckBox::create(const char* style, const char* fontName){
		lwassert(style);
		bool ok = false;
		CheckBox* p = new CheckBox1(style, fontName, ok);
		lwassert(p);
		if ( !ok ){
			lwerror("new CheckBox failed: style="<<style);
			delete p;
			return NULL;
		}
		return p;
	}

	CheckBox* CheckBox::create(const char* file, int upU, int upV, int downU, int downV, int w, int h, const char* fontName){
		lwassert(file);
		bool ok = false;
		CheckBox* p = new CheckBox1(file, upU, upV, downU, downV, w, h, fontName, ok);
		lwassert(p);
		if ( !ok ){
			lwerror("new CheckBox failed: file="<<file);
			delete p;
			return NULL;
		}
		return p;
	}

	CheckBox* CheckBox::create9(const char* style, const char* fontName){
		lwassert(style);
		bool ok = false;
		CheckBox* p = new CheckBox9(style, fontName, ok);
		lwassert(p);
		if ( !ok ){
			lwerror("new CheckBox9 failed: style="<<style);
			delete p;
			return NULL;
		}
		return p;
	}

	CheckBox* CheckBox::create9(const char* file, int upU, int upV, int downU, int downV,
	int w1, int w2, int w3, int h1, int h2, int h3, const char* fontName){
		lwassert(file);
		bool ok = false;
		CheckBox* p = new CheckBox9(file, upU, upV, downU, downV, w1, w2, w3, h1, h2, h3, fontName, ok);
		lwassert(p);
		if ( !ok ){
			lwerror("new CheckBox9 failed: file="<<file);
			delete p;
			return NULL;
		}
		return p;
	}

	CheckBox::CheckBox(const char* fontName)
	:_pCallback(NULL), _isChecked(false), _pFont(NULL)
	,_topExt(0), _bottomExt(0), _leftExt(0), _rightExt(0){
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

	CheckBox::~CheckBox(){
		if ( _pFont ){
			delete _pFont;
		}
	}

	void CheckBox::vMain(float dt){
		_textX = _x - (_textW>>1) + (_w>>1);
		_textY = _y - (_textH>>1) + (_h>>1);
	}

	bool CheckBox::vEvent(std::vector<lw::TouchEvent>& events){
		const lw::TouchEvent& evt = events[0];
		switch (evt.type)
		{
		case lw::TouchEvent::TOUCH:
			if ( evt.x >= _x-_leftExt && evt.x < _x+_w+_rightExt 
			&& evt.y >= _y-_topExt && evt.y < _y+_h+_bottomExt ){
				static int i = 0;
				++i;
				if ( i == 2 ){
					i = 1;
				}
				_isChecked = !_isChecked;
				if ( _pCallback ){
					_pCallback->onCheckBoxCheck(this);
					
				}
				return true;
			}
			break;
		case lw::TouchEvent::UNTOUCH:
			break;
		default:
			break;
		}

		return false;
	}

	void CheckBox::setText(const wchar_t* text){
		lwassert(text);
		if ( !_pFont ){
			lwerror("no font");
			return;
		}
		_text = text;
		_pFont->getSize(_textW, _textH, text);
	}

	void CheckBox::setTextColor(const lw::Color& color){
		_textColor = color;
	}

	CheckBox1::CheckBox1(const char* styleName, const char* fontName, bool& ok)
		:CheckBox(fontName), _pSprite(NULL){
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

	CheckBox1::CheckBox1(const char* file, int upU, int upV, int downU, int downV, int w, int h, const char* fontName, bool& ok)
		:CheckBox(fontName), _pSprite(NULL), _upU(upU), _upV(upV), _downU(downU), _downV(downV){
			_w = w;
			_h = h;
			_pSprite = lw::Sprite::create(file);
			if ( _pSprite == NULL ){
				lwerror("lw::Sprite::create file: style=" << file);
				return;
			}
			ok = true;
	}

	CheckBox1::~CheckBox1(){
		if ( _pSprite ){
			delete _pSprite;
		}
	}

	void CheckBox1::vDraw(){
		if ( _isChecked ){
			_pSprite->setUV(_downU, _downV, _w, _h);
		}else{
			_pSprite->setUV(_upU, _upV, _w, _h);
		}
		_pSprite->collect((float)_x, (float)_y, 0, 0, 0);
		if ( _pFont ){
			_pFont->drawText(_text.c_str(), (float)_textX, (float)_textY, _textColor, 1.f);
		}
	}

	CheckBox9::CheckBox9(const char* styleName, const char* fontName, bool& ok)
		:CheckBox(fontName){
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

	CheckBox9::CheckBox9(const char* file, int upU, int upV, int downU, int downV,
		int w1, int w2, int w3, int h1, int h2, int h3, const char* fontName, bool& ok)
		:CheckBox(fontName){
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

	CheckBox9::~CheckBox9(){
		if ( _pUpImage ){
			delete _pUpImage;
		}
		if ( _pDownImage ){
			delete _pDownImage;
		}
	}

	void CheckBox9::vMain(float dt){
		CheckBox::vMain(dt);
		_pUpImage->setSize(_w, _h);
		_pDownImage->setSize(_w, _h);
	}

	void CheckBox9::vDraw(){
		if ( _isChecked ){
			_pUpImage->show(false);
			_pDownImage->show(true);
		}else{
			_pUpImage->show(true);
			_pDownImage->show(false);
		}
	}

	void CheckBox9::vDrawFG(){
		_pFont->drawText(_text.c_str(), (float)_textX, (float)_textY, _textColor, 1.f);
	}

} //namespace lw