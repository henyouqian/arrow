#ifndef __LW_UI_CHECKBOX_H__
#define __LW_UI_CHECKBOX_H__

#include "lwUIWidget.h"

namespace lw{

	struct TouchEvent;
	class Sprite;
	class CheckBox;
	class Font;
	class UIImage9;

	class CheckBoxCallback{
	public:
		virtual ~CheckBoxCallback(){}
		virtual void onCheckBoxCheck(lw::CheckBox* pCheckBox){}
	};

	class CheckBox : public Widget{
	public:
		static CheckBox* create(const char* style, const char* fontName);
		static CheckBox* create(const char* file, int upU, int upV, int downU, int downV, int w, int h, const char* fontName);
		static CheckBox* create9(const char* style, const char* fontName);
		static CheckBox* create9(const char* file, int upU, int upV, int downU, int downV,
			int w1, int w2, int w3, int h1, int h2, int h3, const char* fontName);
		~CheckBox();

		typedef void (*CallbackFn) (void*);
		void setCallback(CheckBoxCallback* pCB){
			_pCallback = pCB;
		}

		void check(bool b){
			_isChecked = b;
		}
		bool isChecked(){
			return _isChecked;
		}

		virtual void vMain(float dt);
		virtual bool vEvent(std::vector<lw::TouchEvent>& events);

		void setText(const wchar_t* text);
		void setTextColor(const lw::Color& color);

		void setExt(int top, int bottom, int left, int right){
			_topExt = top; _bottomExt = bottom; _leftExt = left; _rightExt = right;
		}

	protected:
		CheckBox(const char* fontName);

	protected:
		CheckBoxCallback* _pCallback;
		void* _callbackData;

		bool _isChecked;

		lw::Font* _pFont;
		std::wstring _text;
		int _textX, _textY, _textW, _textH;
		lw::Color _textColor;
		int _topExt, _bottomExt, _leftExt, _rightExt;
	};

	class CheckBox1 : public CheckBox{
	public:
		~CheckBox1();

	private:
		CheckBox1(const char* styleName, const char* fontName, bool& ok);
		CheckBox1(const char* file, int upU, int upV, int downU, int downV, int w, int h, const char* fontName, bool& ok);
		virtual void vDraw();

		Sprite* _pSprite;
		int _downU, _downV;
		int _upU, _upV;

		friend CheckBox* CheckBox::create(const char* style, const char* fontName);
		friend CheckBox* CheckBox::create(const char* file, int upU, int upV, int downU, int downV, int w, int h, const char* fontName);
	};

	class CheckBox9 : public CheckBox{
	public:
		~CheckBox9();

	private:
		CheckBox9(const char* styleName, const char* fontName, bool& ok);
		CheckBox9(const char* file, int upU, int upV, int downU, int downV,
			int w1, int w2, int w3, int h1, int h2, int h3, const char* fontName, bool& ok);

		virtual void vMain(float dt);
		virtual void vDraw();
		virtual void vDrawFG();

		UIImage9* _pUpImage;
		UIImage9* _pDownImage;

		friend CheckBox* CheckBox::create9(const char* style, const char* fontName);
		friend CheckBox* CheckBox::create9(const char* file, int upU, int upV, int downU, int downV,
			int w1, int w2, int w3, int h1, int h2, int h3, const char* fontName);
	};

} //namespace lw



#endif //__LW_UI_CHECKBOX_H__