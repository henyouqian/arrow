#include "stdafx.h"
#include "lwApp.h"
#include "lwTask.h"
#include "lwPODModel.h"
#include "lwPODModelCollector.h"
#include "lwPODModelRes.h"
#include "lwPODTextureRes.h"
#include "lwSprite.h"
#include "lwSound.h"
#include "lwUIWidget.h"
#include "lwTouchEvent.h"

namespace lw{
	
	namespace{
		struct Data{
			App::Config config;
			float currFPS;
#ifdef WIN32
			EGLDisplay eglDisplay;
			EGLSurface eglSurface;
#endif
		};
		Data* g_pData = NULL;
#ifdef WIN32
		void drawEnd(){
			Data*& d = g_pData;
			glFinish();
			eglWaitGL();
			eglSwapBuffers(d->eglDisplay, d->eglSurface);
		}
#endif
	}
	
	
	void App::create(const Config& config){
		Data*& d = g_pData;
		lwassert(d == NULL);
		d = new Data;
		lwassert(d);
		d->config = config;
#ifdef __APPLE__
		d->config.width = 320;
		d->config.height = 480;
#endif
		switch (config.orientation){
		case ORIENTATION_LEFT:
		case ORIENTATION_RIGHT:
			{
				unsigned short temp = d->config.height;
				d->config.height = d->config.width;
				d->config.width = temp;
			}
			break;
		default:
			break;
		}

		d->config.orientation0 = d->config.orientation;
		d->config.width0 = d->config.width;
		d->config.height0 = d->config.height;

		init();
	}

	void App::destroy(){
		quit();
		Data*& d = g_pData;
		lwassert(d);
		delete d;
		d = NULL;
	}
	
#ifdef WIN32
	namespace{
		int done = 0;
		HWND g_hwnd;

		void transXY(short& x, short& y){
			int dOrient = g_pData->config.orientation - g_pData->config.orientation0;
			if ( dOrient < 0 ){
				dOrient += 4;
			}
			if ( abs(dOrient) == 2 ){
				x = g_pData->config.width0-x;
				y = g_pData->config.height0-y;
			}else if (dOrient == 3){
				short temp = x;
				x = g_pData->config.height0-y;
				y = temp;
			}else if (dOrient == 1){
				short temp = x;
				x = y;
				y = g_pData->config.width0-temp;
			}
		}
	}
	
	
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
		static short prevX = 0;
		static short prevY = 0;
		static bool touched = false;
		static bool lDown = false;
		TouchEvent evt;
		std::vector<TouchEvent> evts;
		switch (message){
			case WM_CLOSE:
				done=1;
				PostQuitMessage(0);
				return 1;
			case WM_MOUSEMOVE:
				if ( lDown ){
					const App::Config& config = App::getConfig();
					evt.x = LOWORD(lParam);
					evt.y = HIWORD(lParam);
					transXY(evt.x, evt.y);
					if ( touched ){
						evt.prevX = prevX;
						evt.prevY = prevY;
						if ( evt.x < 0 || evt.x >= config.width || evt.y < 0 || evt.y >= config.height ){
							touched = false;
							evt.type = TouchEvent::UNTOUCH;
						}else{
							evt.type = TouchEvent::MOVE;
							prevX = evt.x;
							prevY = evt.y;
						}
						evts.push_back(evt);
						g_gestrueMgr.onTouchEvent(evts);
						if ( uiEvent(evts) == false ){
							TaskMgr::onTouchEvent(evts);
						}
						g_gestrueMgr.main();
					}else{
						if ( evt.x >= 0 && evt.x < config.width && evt.y >= 0 && evt.y < config.height ){
							touched = true;
							evt.type = TouchEvent::TOUCH;
							prevX = evt.x;
							prevY = evt.y;
							evts.push_back(evt);
							g_gestrueMgr.onTouchEvent(evts);
							if ( uiEvent(evts) == false ){
								TaskMgr::onTouchEvent(evts);
							}
							g_gestrueMgr.main();
						}
					}
				}
				break;
			case WM_LBUTTONDOWN:
				touched = true;
				lDown = true;
				evt.type = TouchEvent::TOUCH;
				evt.x = LOWORD(lParam);
				evt.y = HIWORD(lParam);
				transXY(evt.x, evt.y);
				evt.prevX = evt.x;
				evt.prevY = evt.y;
				prevX = evt.x;
				prevY = evt.y;
				evts.push_back(evt);
				g_gestrueMgr.onTouchEvent(evts);
				if ( uiEvent(evts) == false ){
					TaskMgr::onTouchEvent(evts);
				}
				g_gestrueMgr.main();
				::SetCapture(hWnd);
				break;
			case WM_LBUTTONUP:
				touched = false;
				lDown = false;
				evt.type = TouchEvent::UNTOUCH;
				evt.x = LOWORD(lParam);
				evt.y = HIWORD(lParam);
				transXY(evt.x, evt.y);
				evt.prevX = prevX;
				evt.prevY = prevY;
				evts.push_back(evt);
				g_gestrueMgr.onTouchEvent(evts);
				if ( uiEvent(evts) == false ){
					TaskMgr::onTouchEvent(evts);
				}
				g_gestrueMgr.main();
				::ReleaseCapture();
				break;
			default:
				break;
		}
		
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	
	int TestEGLError(HWND hWnd, char* pszLocation)
	{
		/*
		 eglGetError returns the last error that has happened using egl,
		 not the status of the last called function. The user has to
		 check after every single egl call or at least once every frame.
		 */
		
		EGLint iErr=eglGetError();
		if (iErr!=EGL_SUCCESS)
		{
			TCHAR pszStr[256];
			swprintf_s(pszStr, 256, _T("%s failed (%d).\n"), pszLocation, iErr);
			MessageBox(hWnd, pszStr, _T("Error"), MB_OK|MB_ICONEXCLAMATION);
			return 0;
		}
		
		return 1;
	}

	int App::run(){		
		Data*& d = g_pData;
		Config& config = d->config;
		
		// Windows variables
		HDC  hDC=0;
		
		// Register the windows class
		WNDCLASS sWC;
		
		sWC.style=CS_HREDRAW | CS_VREDRAW;
		sWC.lpfnWndProc=WndProc;
		sWC.cbClsExtra=0;
		sWC.cbWndExtra=0;
		sWC.hInstance=GetModuleHandle(NULL);
		sWC.hIcon=0;
		sWC.hCursor=0;
		sWC.lpszMenuName=0;
		sWC.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
		sWC.lpszClassName=L"LWJJ";
		
		int nWidth=config.width;
		int nHeight=config.height;
		
		ATOM registerClass=RegisterClass(&sWC);
		if (!registerClass)
		{
			MessageBox(0, _T("Failed to register the window class"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		}
		
		// Create the eglWindow
		RECT sRect;
		SetRect(&sRect, 0, 0, nWidth, nHeight);
		AdjustWindowRectEx(&sRect, WS_CAPTION | WS_SYSMENU, 0, 0);
		int w = sRect.right - sRect.left;
		int h = sRect.bottom - sRect.top;
		g_hwnd=CreateWindow(sWC.lpszClassName, config.title.c_str(), WS_VISIBLE | WS_SYSMENU, 0, 0, w, h, NULL, NULL, GetModuleHandle(NULL), NULL);
		hDC = GetDC(g_hwnd);

		initEgl(g_hwnd);
		
		DWORD t1 = ::timeGetTime();
		DWORD t2 = ::timeGetTime();
		/* Render stuff */
		do {
			//main
			int dt = t2-t1;
			if (dt == 0)
				dt = 1;
			if ( dt > 100 ){
				dt = 100;
			}
			t1 = t2;

			if ( config.maxFPS > 0 ){
				float frameDuration = 1000.f/config.maxFPS;
				main(frameDuration);
			}else{
				main((float)dt);
			}
			//std::wstringstream ss;
			//ss << config.title << L" FPS:" << g_pData->currFPS; 
			//::SetWindowText(hWnd, ss.str().c_str());

			//draw
			//glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			draw((float)dt);
			drawEnd();
			
			if (done){
				break;
			}

			MSG msg;
			while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			} // while (PeekMessage())

			t2 = ::timeGetTime();
			if ( config.maxFPS > 0 ){
				float frameDuration = 1000.f/config.maxFPS;
				dt = t2 - t1;
				int sleepT = (int)(frameDuration-dt);
				if ( dt < frameDuration ){
					Sleep(sleepT);
				}
				t2 += sleepT;
			}
		} while(1);
		
		eglMakeCurrent(d->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglTerminate(d->eglDisplay);
		
		if (hDC){
			ReleaseDC(g_hwnd, hDC);
		}
		if (g_hwnd)
		{
			DestroyWindow(g_hwnd);
		}
		
		return 0;
	}
	
	void App::initEgl(NativeWindowType eglWindow){
		Data*& d = g_pData;
		
		// Windows variables
		HDC  hDC=0;
		
		// EGL variables
		EGLConfig  eglConfig=0;
		EGLContext eglContext=0;
		EGLint pi32ConfigAttribs[128];
		EGLint iMajorVersion, iMinorVersion;
		int iConfigs;
		
		// Get the associated device context
		hDC=GetDC(eglWindow);
		lwassert(hDC);
		
		d->eglDisplay=eglGetDisplay((NativeDisplayType)hDC);
		if(d->eglDisplay==EGL_NO_DISPLAY){
			d->eglDisplay=eglGetDisplay((NativeDisplayType)EGL_DEFAULT_DISPLAY);
		}
		lwassert(d->eglDisplay!=EGL_NO_DISPLAY);
		
		if (!eglInitialize(d->eglDisplay, &iMajorVersion, &iMinorVersion)){
			lwassert(0);
		}
		
		int i=0;
		pi32ConfigAttribs[i++]=EGL_RED_SIZE;
		pi32ConfigAttribs[i++]=5;
		pi32ConfigAttribs[i++]=EGL_GREEN_SIZE;
		pi32ConfigAttribs[i++]=6;
		pi32ConfigAttribs[i++]=EGL_BLUE_SIZE;
		pi32ConfigAttribs[i++]=5;
		pi32ConfigAttribs[i++]=EGL_ALPHA_SIZE;
		pi32ConfigAttribs[i++]=0;
		pi32ConfigAttribs[i++]=EGL_DEPTH_SIZE;
		pi32ConfigAttribs[i++]=16;
		pi32ConfigAttribs[i++]=EGL_SURFACE_TYPE;
		pi32ConfigAttribs[i++]=EGL_WINDOW_BIT;
		pi32ConfigAttribs[i++]=EGL_NONE;
		
		if (!eglChooseConfig(d->eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs) || (iConfigs != 1)){
			lwassert(0);
		}
		
		d->eglSurface=eglCreateWindowSurface(d->eglDisplay, eglConfig, eglWindow, NULL);
		if (d->eglSurface==EGL_NO_SURFACE){
			eglGetError(); // Clear error
			d->eglSurface=eglCreateWindowSurface(d->eglDisplay, eglConfig, NULL, NULL);
		}
		lwassert(d->eglSurface!=EGL_NO_SURFACE);
		
		if (!TestEGLError(eglWindow, "eglCreateWindowSurface")){
			lwassert(0);
		}
		
		eglContext=eglCreateContext(d->eglDisplay, eglConfig, NULL, NULL);
		if (!TestEGLError(eglWindow, "eglCreateContext")){
			lwassert(0);
		}
		
		eglMakeCurrent(d->eglDisplay, d->eglSurface, d->eglSurface, eglContext);
		if (!TestEGLError(eglWindow, "eglMakeCurrent"))
		{
			lwassert(0);
		}
		
		//glViewport(0, 0, d->config.height, d->config.width);
		glViewport(0, 0, d->config.width, d->config.height);
	}

	void App::flMain(){
		static DWORD t1 = ::timeGetTime();
		static DWORD t2 = 0;
		t2 = ::timeGetTime();
		int dt = t2-t1;
		if (dt == 0){
			Sleep(1);
			return;
		}
		if ( dt > 100 ){
			dt = 100;
		}
		t1 = t2;

		main((float)dt);
		draw((float)dt);
		drawEnd();
		Sleep(1);
	}

	HWND App::getHwnd(){
		return g_hwnd;
	}
	
#endif //WIN32
	
	const App::Config& App::getConfig(){
		return g_pData->config;
	}

	void App::setOrient(App::Orientation orient){
		Config& conf = g_pData->config;
		g_pData->config.orientation = orient;
		if ( abs(orient-g_pData->config.orientation0) == 1 ){
			conf.width = conf.height0;
			conf.height = conf.width0;
		}else{
			conf.width = conf.width0;
			conf.height = conf.height0;
		}
	}
	
	float App::getAspect(){
		Config& conf = g_pData->config;
		return (float)conf.width/(float)conf.height;
	}

	float App::getCurrFPS(){
		return g_pData->currFPS;
	}
	
	void App::init(){
		//TaskMgr::init();
#ifndef SOUND_DISABLE
		Config& conf = g_pData->config;
		new SoundMgr(conf.maxSoundSources);
#endif
		initUI();
	}
	
	void App::quit(){
		TaskMgr::quit();
		destroyUI();
		PODModelRes::cleanup();
		PODTextureRes::cleanup();
#ifndef SOUND_DISABLE
		delete SoundMgr::ps();
#endif
	}
	
	void App::main(float dt){
		if ( g_pData == NULL ){
			return;
		}
#ifndef SOUND_DISABLE
		SoundMgr::s().main(dt);
#endif
		TaskMgr::processPending();
		uiMain(dt);
		TaskMgr::main(dt);
		PODModel::main();
		
		static float t = 0;
		static int frm = 0;
		t += dt;
		frm += 1;
		if ( t > 200 ){
			g_pData->currFPS = frm/t*1000.f;
			t = 0;
			frm = 0;
		}
	}
	
	void App::draw(float dt){
		if ( g_pData == NULL ){
			return;
		}
		TaskMgr::draw(dt);
		uiDraw();
		PODModelCollector::draw();
		Sprite::draw();
	}


	float time2Frame(float ms, float fps){
		return ms * 0.001f * fps;
	}
	
	
} //namespace lw