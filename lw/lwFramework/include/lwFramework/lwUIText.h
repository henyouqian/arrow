#ifndef __LW_UI_TEXT_H__
#define __LW_UI_TEXT_H__

#include "lwUIWidget.h"
#include "lwFont.h"

namespace lw{
	
	class UIText : public Widget{
	public:
		static UIText* create(const char* fontName);
		~UIText();

		void setAlign(FontAlign align);
		void setText(const wchar_t* text);
		void setColor(const Color& color){
			_color = color;
		}

		virtual void vDraw();

	private:
		UIText(const char* fileName, bool& ok);
		Font* _pFont;
		std::wstring _text;
		Color _color;
		FontAlign _align;
		FontComposingData* _pFontCompsingData;
	};

} //namespace lw



#endif //__LW_UI_TEXT_H__