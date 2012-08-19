#ifndef __LW_UI_IMAGE_H__
#define __LW_UI_IMAGE_H__

#include "lwUIWidget.h"

namespace lw{

	class Sprite;
	class UIImage;

	class UIImageCallback{
	public:
		virtual ~UIImageCallback(){}
		virtual void onImageTouch(lw::UIImage* pUIImage){}
	};

	class UIImage : public Widget{
	public:
		static UIImage* create(const char* fileName, int uvX, int uvY, int uvW, int uvH, int w, int h);
		~UIImage();

		void setCallback(UIImageCallback* p){
			_pCallback = p;
		}

		virtual void vDraw();
		virtual bool vEvent(std::vector<lw::TouchEvent>& events);
		void setColor(lw::Color& color){
			_color = color;
		}

	private:
		UIImage(const char* fileName, int uvX, int uvY, int uvW, int uvH, int w, int h, bool& ok);
		Sprite* _pSprite;
		UIImageCallback* _pCallback;
		lw::Color _color;
	};

	class UIImage9: public Widget{
	public:
		static UIImage9* create(const char* fileName, int uvX, int uvY, int uvW1, int uvW2, int uvW3, int uvH1, int uvH2, int uvH3);
		~UIImage9();

		virtual void vDraw();
		void setColor(const lw::Color& color){
			_color = color;
		}

	private:
		UIImage9(const char* fileName, int uvX, int uvY, int uvW1, int uvW2, int uvW3, int uvH1, int uvH2, int uvH3, bool& ok);

		int _uvX[3];
		int _uvY[3];
		int _uvW[3];
		int _uvH[3];
		Sprite* _pSprite;
		lw::Color _color;
	};


} //namespace lw



#endif //__LW_UI_IMAGE_H__