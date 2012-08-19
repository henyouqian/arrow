#ifndef __LW_UI_BUTTON_H__
#define __LW_UI_BUTTON_H__

#include "lwUIWidget.h"

namespace lw{

	struct TouchEvent;
	class Sprite;
	class Font;
	class UIImage9;

	class Button;

	class ButtonCallback{
	public:
		virtual ~ButtonCallback(){}
		virtual void onButtonDown(lw::Button* pButton){}
		virtual void onButtonClick(lw::Button* pButton){}
	};

	class Button : public Widget{
	public:
		static Button* createButton1(const char* style, const char* fontName);
		static Button* createButton1(const char* file, int upU, int upV, int downU, int downV, int w, int h, const char* fontName);
		static Button* createButton1(const char* file, int upU, int upV, int downU, int downV, int disableU, int disableV, int w, int h, const char* fontName);
		static Button* createButton9(const char* style, const char* fontName);
		static Button* createButton9(const char* file, int upU, int upV, int downU, int downV,
										int w1, int w2, int w3, int h1, int h2, int h3, const char* fontName);

		~Button();

		void setCallback(ButtonCallback* pCallback){
			_pCallback = pCallback;
		}

		virtual void vMain(float dt);
		virtual bool vEvent(std::vector<lw::TouchEvent>& events);

		virtual void setColor(const lw::Color& color, const lw::Color& downColor){
			_color = color; _downColor = downColor;
		}
		virtual void setColor(const lw::Color& color){
			_color = color; _downColor = color;
		}

		void setText(const wchar_t* text);
		void setTextColor(const lw::Color& color);
		void setTextColor(const lw::Color& color, const lw::Color& downColor);

		void setExt(int top, int bottom, int left, int right){
			_topExt = top; _bottomExt = bottom; _leftExt = left; _rightExt = right;
		}
		void setScale(float scale){
			_scale = scale;
		}

	protected:
		Button(const char* fontName);

	protected:
		ButtonCallback* _pCallback;

		bool _isTracing;
		bool _isDown;

		lw::Font* _pFont;
		std::wstring _text;
		int _textX, _textY, _textW, _textH;
		lw::Color _color, _downColor, _textColor, _textDownColor;
		int _topExt, _bottomExt, _leftExt, _rightExt;
		float _scale;
	};

	class Button1 : public Button{
	public:
		~Button1();

	private:
		Button1(const char* styleName, const char* fontName, bool& ok);
		Button1(const char* file, int upU, int upV, int downU, int downV, int w, int h, const char* fontName, bool& ok);
		Button1(const char* file, int upU, int upV, int downU, int downV, int disableU, int disableV, int w, int h, const char* fontName, bool& ok);
		virtual void vDraw();

		Sprite* _pSprite;
		int _downU, _downV;
		int _upU, _upV;
		int _disableU, _disableV;

		friend Button* Button::createButton1(const char* style, const char* fontName);
		friend Button* Button::createButton1(const char* file, int upU, int upV, int downU, int downV, int w, int h, const char* fontName);
		friend Button* Button::createButton1(const char* file, int upU, int upV, int downU, int downV, int disableU, int disableV, int w, int h, const char* fontName);
	};

	class Button9 : public Button{
	public:
		~Button9();

	private:
		Button9(const char* styleName, const char* fontName, bool& ok);
		Button9(const char* file, int upU, int upV, int downU, int downV,
			int w1, int w2, int w3, int h1, int h2, int h3, const char* fontName, bool& ok);

		virtual void vMain(float dt);
		virtual void vDraw();
		virtual void vDrawFG();

		UIImage9* _pUpImage;
		UIImage9* _pDownImage;

		friend Button* Button::createButton9(const char* style, const char* fontName);
		friend Button* Button::createButton9(const char* file, int upU, int upV, int downU, int downV,
			int w1, int w2, int w3, int h1, int h2, int h3, const char* fontName);
	};


} //namespace lw



#endif //__LW_UI_BUTTON_H__