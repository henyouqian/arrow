#ifndef __LW_UI_WIDGET_H__
#define __LW_UI_WIDGET_H__

namespace lw{

	struct TouchEvent;

	void initUI();
	void destroyUI();
	void clearUI();

	void uiMain(float dt);
	void uiDraw();
	bool uiEvent(std::vector<lw::TouchEvent>& events);
	void uiSetAutoDraw(bool autoDraw);
	void uiDrawForce();

	class Widget{
	public:
		Widget();
		virtual ~Widget();

		void uiMain(float dt);
		void uiDraw();
		bool uiEvent(std::vector<lw::TouchEvent>& events);

		void setParent(Widget* pParent);
		void setPos(int x, int y){
			_rx = x; _ry = y;
		}
		void getPos(int& x, int& y){
			x = _rx;
			y = _ry;
		}
		void setScreenPos(int x, int y){
			_x = x; _y = y;
		}
		virtual void setSize(int w, int h){
			_w = w; _h = h;
		}
		void getSize(int& w, int& h){
			w = _w; h = _h;
		}
		void show(bool s){
			_show = s;
		}
		bool isShow(){
			return _show;
		}
		void enable(bool e){
			_enable = e;
		}

		void removeChild(Widget* pWidget);
		std::list<Widget*>& getChildren(){
			return _children;
		}

		void forceDraw(){
			vDraw();
		}

		virtual void setColor(const lw::Color& color){}

	private:
		virtual void vMain(float dt){}
		virtual void vDraw(){}
		virtual void vDrawFG(){}
		virtual bool vEvent(std::vector<lw::TouchEvent>& events){return false;}
		Widget(int constructForRoot);

	protected:
		Widget* _pParent;
		std::list<Widget*> _children;
		int _rx, _ry, _x, _y, _w, _h;
		bool _show;
		bool _enable;

		friend void initUI();
		friend void clearUI();
	};


} //namespace lw



#endif //__LW_UI_WIDGET_H__