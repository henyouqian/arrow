#include "stdafx.h"
#include "lwClip.h"
#include "lwSprite.h"
#include "lwFileSys.h"
#include "lwPODTextureRes.h"

namespace lw{
	Clip* Clip::create(const char* fileName, int borderLeft, int borderRight, 
	int u0, int v0, int w, int h, float interval, int frameNum, bool loop){
		lwassert(fileName);
		lwassert(borderLeft < borderRight);
		lwassert( u0 >= borderLeft && u0+w <= borderRight );
		lwassert(frameNum > 0);
		bool ok = false;
		Clip* pClip = new Clip(fileName, borderLeft, borderRight, u0, v0, w, h, interval, frameNum, loop, ok);
		lwassert(pClip);
		if ( !ok ){
			delete pClip;
			return NULL;
		}
		return pClip;
	}

	Clip::Clip(const char* fileName, int borderLeft, int borderRight, 
	int u0, int v0, int w, int h, float interval, int frameNum, bool loop, bool& ok)
	:_pSprite(NULL), _bl(borderLeft), _br(borderRight), _u0(u0), _v0(v0), _w(w), _h(h), _interval(interval), _frameNum(frameNum), _loop(loop), _t(0){
		_pSprite = Sprite::create(fileName);
		if ( _pSprite == NULL ){
			lwerror("Sprite::create failed: fileName=" << fileName);
			return;
		}

		ok = true;
	}

	Clip::~Clip(){
		if ( _pSprite ){
			delete _pSprite;
		}
	}

	void Clip::main(float dt){
		if ( _interval >= 0 ){
			_t += dt;
		}else{
			_t -= dt;
		}
		
		float period = (_frameNum)*fabs(_interval);
		if ( _loop ){
			_t = fmod(_t, period);
		}else{
			_t = max(min(_t, period-0.001f), 0.f);
		}
		int currFrame = (int)(_t/fabs(_interval));

		int framesRow1 = (_br-_u0)/_w;
		unsigned short u = 0;
		unsigned short v = 0;
		if ( currFrame < framesRow1 ){
			u = _u0 + currFrame*_w;
			v = _v0;
		}else{
			int framesPerRow = (_br-_bl)/_w;
			int currFrameFromSecondRow = currFrame - framesRow1;
			int row = currFrameFromSecondRow/framesPerRow;
			int	col = currFrameFromSecondRow-row*framesPerRow;
			u = (col)*_w;
			v = _v0 + (row+1)*_h;
		}
		_pSprite->setUV(u, v, _w, _h);
	}

	void Clip::setFrame(int frame){
		lwassert(frame >= 0);
		_t = fabs(_interval)* frame;
	}

	ClipRes::ClipRes(const char* fileName, unsigned short x, unsigned short y, unsigned short w, unsigned short h, short ancX, short ancY, short numFrame, unsigned char fps, bool loop, bool& ok)
	:_x(x), _y(y), _w(w), _h(h), _numFrame(numFrame), _fps(fps), _loop(loop){
		lwassert(fileName);
		ok = false;
		_pSprite = Sprite::create(fileName);
		if ( _pSprite == NULL ){
			lwerror("Sprite::create failed: fileName=" << fileName);
			return;
		}
		_pSprite->setAnchor(ancX, ancY);
		const PODTextureRes* pTexRes = _pSprite->getTextureRes();
		const PVR_Texture_Header* pHeader = pTexRes->getHeader();
		_texW = pHeader->dwWidth;
		_texH = pHeader->dwHeight;
		if ( _w > _texW || _h > _texH ){
			lwerror("invalid w or h(w, h, texW, texH):"<< w << ", " << h << ", " << _texW << ", " << _texH);
			return;
		}
		ok = true;
	}

	ClipRes::~ClipRes(){
		if ( _pSprite ){
			delete _pSprite;
		}
	}

	void ClipRes::collect(const cml::Matrix44& m, float time) const{
		float msPerFrame = 1000.0f/_fps;
		int currFrame = (int)((float)time/msPerFrame);
		if ( _loop ){
			currFrame %= _numFrame;
		}else{
			currFrame = min(_numFrame-1, max(0, currFrame));
		}

		int framesRow1 = (_texW-_x)/_w;
		unsigned short u = 0;
		unsigned short v = 0;
		if ( currFrame < framesRow1 ){
			u = _x + currFrame*_w;
			v = _y;
		}else{
			int framesPerRow = _texW/_w;
			int currFrameFromSecondRow = currFrame - framesRow1;
			int row = currFrameFromSecondRow/framesPerRow;
			int	col = currFrameFromSecondRow-row*framesPerRow;
			u = (col)*_w;
			v = _y + (row+1)*_h;
		}
		_pSprite->setUV(u, v, _w, _h);
		_pSprite->collect(m);
	}

	bool ClipResSet::addFromFile(const char* fileName){
		lwassert(fileName);
		_f filePath = _f(fileName);
		TiXmlDocument doc;
		if ( !doc.LoadFile(filePath) ){
			lwerror("doc.LoadFile() failed: filePath=" <<filePath);
			return false;
		}
		TiXmlElement *pElem = doc.FirstChildElement();
		if ( !pElem ){
			lwerror("doc.FirstChildElement() failed: filePath=" <<filePath);
			return false;
		}
		pElem = pElem->FirstChildElement("clip");
		ClipRes* pClip = NULL;
		const char* attr = NULL;
		const char* name = NULL;
		const char* clipFileName = NULL;
		int x, y, w, h, ancX, ancY, numFrame, fps, loop = 0;
		while ( pElem ){
			name = pElem->Attribute("name");
			if ( name == NULL ){
				lwerror("attr name is not exist: filePath=" << filePath);
				return false;
			}
			if ( _clips.find(name) != _clips.end() ){
				lwerror("clip name collide, may be add same file twice: filePath=" << filePath << " name=" << name);
				return false;
			}
			clipFileName = pElem->Attribute("file");
			if ( clipFileName == NULL ){
				lwerror("attr file is not exist: filePath=" << filePath);
				return false;
			}
			attr = pElem->Attribute("x", &x);
			if ( attr == NULL ){
				lwerror("attr x is not exist: filePath=" << filePath << " name=" << name);
				return false;
			}
			attr = pElem->Attribute("y", &y);
			if ( attr == NULL ){
				lwerror("attr y is not exist: filePath=" << filePath << " name=" << name);
				return false;
			}
			attr = pElem->Attribute("w", &w);
			if ( attr == NULL ){
				lwerror("attr w is not exist: filePath=" << filePath << " name=" << name);
				return false;
			}
			attr = pElem->Attribute("h", &h);
			if ( attr == NULL ){
				lwerror("attr h is not exist: filePath=" << filePath << " name=" << name);
				return false;
			}
			attr = pElem->Attribute("anchorX", &ancX);
			if ( attr == NULL ){
				ancX = 0;
			}
			attr = pElem->Attribute("anchorY", &ancY);
			if ( attr == NULL ){
				ancY = 0;
			}
			attr = pElem->Attribute("frame", &numFrame);
			if ( attr == NULL ){
				lwerror("attr frame is not exist: filePath=" << filePath << " name=" << name);
				return false;
			}
			attr = pElem->Attribute("fps", &fps);
			if ( attr == NULL ){
				lwerror("attr fps is not exist: filePath=" << filePath << " name=" << name);
				return false;
			}
			attr = pElem->Attribute("loop", &loop);
			if ( attr == NULL ){
				loop = false;
			}

			bool ok;
			pClip = new ClipRes(clipFileName, (short)x, (short)y, (short)w, (short)h, (short)ancX, (short)ancY,
				(short)numFrame, (unsigned char)fps, loop!=0, ok);
			lwassert(pClip);
			if ( !ok ){
				delete pClip;
				return false;
			}
			_clips[name] = pClip;
			pElem = pElem->NextSiblingElement("clip");
		}
		
		return true;
	}

	ClipRes* ClipResSet::get(const char* clipName){
		lwassert(clipName);
		std::map<std::string, ClipRes*>::iterator it = _clips.find(clipName);
		if ( it == _clips.end() ){
			return NULL;
		}else{
			return it->second;
		}
	}

	void ClipResSet::clear(){
		std::map<std::string, ClipRes*>::iterator it = _clips.begin();
		std::map<std::string, ClipRes*>::iterator itEnd = _clips.end();
		for ( ; it != itEnd; ++it ){
			delete it->second;
		}
		_clips.clear();
	}

} //namespace lw
