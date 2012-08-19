#include "stdafx.h"
#include "lwUIText.h"
#include "lwFont.h"

namespace lw{

	UIText* UIText::create(const char* fontName){
		lwassert(fontName);
		bool ok = false;
		UIText* pUIText = new UIText(fontName, ok);
		lwassert(pUIText);
		if ( !ok ){
			delete pUIText;
			return NULL;
		}
		return pUIText;
	}

	UIText::UIText(const char* fontName, bool& ok):_align(ALIGN_LEFT), _pFontCompsingData(NULL), _pFont(NULL){
		_pFont = Font::create(fontName);
		if ( _pFont == NULL ){
			lwerror("Font::create failed: file=" <<fontName);
			return;
		}
		_pFontCompsingData = new FontComposingData;
		ok = true;
	}

	UIText::~UIText(){
		if ( _pFont ){
			delete _pFont;
		}
		if ( _pFontCompsingData ){
			delete _pFontCompsingData;
		}
	}

	void UIText::setAlign(FontAlign align){
		_align = align;
		_pFontCompsingData->set(_pFont, _text.c_str(), _align, (float)_w, 1.f);
	}

	void UIText::setText(const wchar_t* text){
		lwassert(text);
		_text = text;
		_pFontCompsingData->set(_pFont, _text.c_str(), _align, (float)_w, 1.f);
		//_pFont->getSize(_w, _h, text);
	}

	void UIText::vDraw(){
		_pFont->drawText(_pFontCompsingData, (float)_x, (float)_y, _color);
	}
	

} //namespace lw