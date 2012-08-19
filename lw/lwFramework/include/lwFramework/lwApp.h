#ifndef __LW_APP_H__
#define __LW_APP_H__

namespace lw{

	class App{
	public:
		
		enum Orientation{
			ORIENTATION_LEFT,
			ORIENTATION_UP,
			ORIENTATION_RIGHT,
			ORIENTATION_DOWN,
		};
		
		struct Config
		{
			Config()
			:title(L"no title")
			,width(320)
			,height(480)
			,isWindowMode(true)
			,orientation(ORIENTATION_UP)
			,maxFPS(0)
			,maxSoundSources(8){
			}
			std::wstring title;
			unsigned short width; 
			unsigned short height;
			unsigned short width0; 
			unsigned short height0;
			bool isWindowMode;
			Orientation orientation;
			Orientation orientation0;
			int maxFPS;
			int maxSoundSources;
		};
		static void create(const Config& config);
		static void destroy();
		static const Config& getConfig();
		static void setOrient(Orientation orient);

		static float getAspect();
		static float getCurrFPS();

		static void main(float ms);
		static void draw(float ms);

#ifdef WIN32
		static int run();
		static void initEgl(NativeWindowType hwnd);
		static void flMain();
		static HWND getHwnd();
#endif

	private:
		static void init();
		static void quit();
	
	};

	float time2Frame(float ms, float fps);
	

} //namespace lw



#endif //__LW_APP_H__