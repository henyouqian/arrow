#include "stdafx.h"
#include "lwUIWidget.h"

namespace lw{

	namespace{
		Widget* g_pRoot = NULL;
		bool g_uiAutoDraw = true;
	}
	
	void initUI(){
		lwassert(g_pRoot == NULL);
		g_pRoot = new Widget(1);
		lwassert(g_pRoot);
	}

	void destroyUI(){
		Widget* p = g_pRoot;
		if ( !g_pRoot->getChildren().empty() ){
			lwerror("widgets leak");
		}
		delete g_pRoot;
		g_pRoot = NULL;
	}

	bool checkUILeak(){
		if ( !g_pRoot->getChildren().empty() ){
			lwerror("widgets leak");
			return true;
		}
		return false;
	}

	void clearUI(){
		while ( !g_pRoot->_children.empty() ){
			delete g_pRoot->_children.back();
		}
	}
	void uiMain(float dt){
		g_pRoot->uiMain(dt);
	}

	void uiDraw(){
		if ( g_uiAutoDraw ){
			g_pRoot->uiDraw();
		}
	}
	void uiDrawForce(){
		g_pRoot->uiDraw();
	}

	bool uiEvent(std::vector<lw::TouchEvent>& events){
		return g_pRoot->uiEvent(events);
	}

	void uiSetAutoDraw(bool autoDraw){
		g_uiAutoDraw = autoDraw;
	}

	Widget::Widget():_pParent(NULL), _show(true), _enable(true){
		_rx = _ry = _x = _y = _w = _h = 0;
		setParent(g_pRoot);
	}

	Widget::Widget(int constructForRoot): _pParent(NULL), _show(true), _enable(true){
		_rx = _ry = _x = _y = _w = _h = 0;
	}

	Widget::~Widget(){
		if ( _pParent ){
			_pParent->removeChild(this);
		}
		std::list<Widget*>::iterator it = _children.begin();
		std::list<Widget*>::iterator itEnd = _children.end();
		while ( !_children.empty() ){
			delete _children.back();
		}
	}

	void Widget::setParent(Widget* pParent){
		if ( _pParent == pParent ){
			return;
		}
		if ( _pParent ){
			_pParent->removeChild(this);
		}
		_pParent = pParent;
		if ( _pParent ){
			_pParent->_children.push_back(this);
		}
	}

	void Widget::removeChild(Widget* pWidget){
		std::list<Widget*>::iterator it = _children.begin();
		std::list<Widget*>::iterator itEnd = _children.end();
		for ( ; it != itEnd; ++it ){
			if ( *it == pWidget ){
				_children.erase(it);
				return;
			}
		}
	}

	void Widget::uiMain(float dt){
		//if ( !_show ){
		//	return;
		//}
		if ( _pParent ){
			_x = _pParent->_x + _rx;
			_y = _pParent->_y + _ry;
		}
		vMain(dt);
		std::list<Widget*>::iterator it = _children.begin();
		std::list<Widget*>::iterator itEnd = _children.end();
		for ( ; it != itEnd; ++it ){
			(*it)->uiMain(dt);
		}
	}
	void Widget::uiDraw(){
		if ( !_show ){
			return;
		}
		vDraw();
		std::list<Widget*>::iterator it = _children.begin();
		std::list<Widget*>::iterator itEnd = _children.end();
		for ( ; it != itEnd; ++it ){
			(*it)->uiDraw();
		}
		vDrawFG();
	}
	bool Widget::uiEvent(std::vector<lw::TouchEvent>& events){
		if ( !_enable || !_show ){
			return false;
		}
		std::list<Widget*>::reverse_iterator it = _children.rbegin();
		std::list<Widget*>::reverse_iterator itEnd = _children.rend();
		for ( ; it != itEnd; ++it ){
			if ( (*it)->uiEvent(events) ){
				return true;
			}
		}
		return vEvent(events);
	}

	

} //namespace lw