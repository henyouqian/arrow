#ifndef __LW_CLIP_H__
#define __LW_CLIP_H__

namespace lw{

	class Sprite;

	class Clip{
	public:
		static Clip* create(const char* fileName, int borderLeft, int borderRight, 
			int u0, int v0, int w, int h, float interval, int frameNum, bool loop);
		~Clip();
		Sprite* getSprite(){
			return _pSprite;
		}
		void main(float dt);
		void setFrame(int frame);

	private:
		Clip(const char* fileName, int borderLeft, int borderRight, 
			int u0, int v0, int w, int h, float interval, int frameNum, bool loop, bool& ok);
		Sprite* _pSprite;
		int _bl, _br;
		int _u0, _v0, _w, _h;
		float _interval;
		int _frameNum;
		bool _loop;
		float _t;
	};

	class ClipRes
	{
	public:
		ClipRes(const char* fileName, unsigned short x, unsigned short y, unsigned short w, unsigned short h, short ancX, short ancY, 
			short numFrame, unsigned char fps, bool loop, bool& ok);
		~ClipRes();
		void collect(const cml::Matrix44& m, float time) const;
		

		unsigned short _x, _y, _w, _h;
		short _numFrame;
		unsigned char _fps;
		bool _loop;
		short _texW, _texH;
		Sprite* _pSprite;
	};
	
	class ClipResSet{
	public:
		~ClipResSet() {
			clear();
		}
		bool addFromFile(const char* fileName);
		ClipRes* get(const char* clipName);
		void clear();

	private:
		std::map<std::string, ClipRes*> _clips;
	};

} //namespace lw

#endif //__LW_CLIP_H__