#include "stdafx.h"
#include "lwUIMenu.h"
#include "lwFont.h"
#include "lwTouchEvent.h"

namespace lw{

	Menu* Menu::create(const char* fontFile){
		lwassert(fontFile);
		bool ok = false;
		Menu* p = new Menu(fontFile, ok);
		lwassert(p);
		if ( !ok ){
			delete p;
			return NULL;
		}
		return p;
	}

	Menu::Menu(const char* fontFile, bool& ok)
	:_callbackFn(NULL), _trackIndex(-1), _trackIsDown(false){
		ok = false;
		_pFont = Font::create(fontFile);
		if ( _pFont == NULL ){
			lwerror("Font::create failed: fontFile=" << fontFile);
			return;
		}

		ok = true;
	}
	Menu::~Menu(){
		if ( _pFont ){
			delete _pFont;
		}
	}
	void Menu::addItem(const wchar_t* text, int x, int y, void* data){
		lwassert(text);
		Item item;
		item.x = x;
		item.y = y;
		_pFont->getSize(item.w, item.h, text);
		item.text = text;
		item.data = data;

		_items.push_back(item);
	}

	void Menu::collect(){
		std::vector<Item>::iterator it =  _items.begin();
		std::vector<Item>::iterator itEnd =  _items.end();
		lw::Color color1 = lw::COLOR_WHITE;
		lw::Color color2(1.f, 1.f, 0.f, 1.f);
		for ( int i = 0; it != itEnd; ++it, ++i ){
			if ( _trackIndex == i && _trackIsDown ){
				_pFont->drawText(it->text.c_str(), (float)it->x+10, (float)it->y, color2, 1.f);
			}else{
				_pFont->drawText(it->text.c_str(), (float)it->x, (float)it->y, color1, 1.f);
			}
		}
	}

	bool Menu::onTouchEvent(std::vector<lw::TouchEvent>& events){
		lw::TouchEvent& evt = events[0];
		std::vector<Item>::iterator it = _items.begin();
		std::vector<Item>::iterator itEnd = _items.end();
		switch(evt.type)
		{
		case lw::TouchEvent::TOUCH:
			_trackIndex = -1;
			_trackIsDown = false;
			for ( int i = 0; it != itEnd; ++it, ++i ){
				if ( evt.x >= it->x && evt.x < it->x+it->w
				&& evt.y >= it->y && evt.y < it->y+it->h ){
					_trackIndex = i;
					_trackIsDown = true;
					break;
				}
			}
			break;
		case lw::TouchEvent::UNTOUCH:
			if ( _trackIndex >= 0 && _trackIsDown && _callbackFn ){
				_callbackFn(_trackIndex, _items[_trackIndex].data);
			}
			_trackIndex = -1;
			_trackIsDown = false;
			break;
		case lw::TouchEvent::MOVE:
			if ( _trackIndex >= 0 ){
				Item& item = _items[_trackIndex];
				if ( evt.x >= item.x && evt.x < item.x+item.w
				&& evt.y >= item.y && evt.y < item.y+item.h ){
					_trackIsDown = true;
				}else{
					_trackIsDown = false;
				}
			}
			break;
		default:
			break;
		}
		return false;
	}

} //namespace lw