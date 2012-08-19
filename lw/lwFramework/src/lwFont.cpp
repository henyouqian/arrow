#include "stdafx.h"
#include "lwFont.h"
#include "lwRes.h"
#include "lwFileSys.h"
#include "lwSprite.h"

namespace lw{

	class FontRes: public Res{
	public:
		static FontRes* create(const char* fileName);
		static void cleanup();

		void drawText(const wchar_t* text, float x, float y, const Color& color, float scale, float rotate);
		void drawText(FontComposingData* pComposingData, float x, float y, const Color& color, float rotate);
		void getSize(int& outX, int& outY, const wchar_t* text);
		int getH(){
			return _commonInfo.lineHeight;
		}

		struct CommonInfo{
			unsigned short lineHeight;
			unsigned short base;
			unsigned short texturW;
			unsigned short texturH;
			unsigned short numPages;
			char bitField;
			unsigned char alphaChnl;
			unsigned char redChnl;
			unsigned char greenChnl;
			unsigned char blueChnl;
		};
		struct CharInfo{
			unsigned int id;
			unsigned short x;
			unsigned short y;
			unsigned short w;
			unsigned short h;
			short xoffset;
			short yoffset;
			short xadvance;
			unsigned char page;
			unsigned char chnl;
		};
		std::map<wchar_t, CharInfo>& getCharInfoMap(){
			return _charInfoMap;
		}

	private:
		FontRes(const char* fileName, bool& ok);
		~FontRes();

		std::string _fileName;
		CommonInfo _commonInfo;
		
		std::map<wchar_t, CharInfo> _charInfoMap;
		std::vector<Sprite*> _sprites;

		static std::map<std::string, FontRes*> _sResMap;
	};

	std::map<std::string, FontRes*> FontRes::_sResMap;

	FontRes* FontRes::create(const char* fileName){
		std::map<std::string, FontRes*>::iterator it = _sResMap.find(fileName);
		if ( it != _sResMap.end() ){
			it->second->addRef();
			return it->second;
		}
		bool ok = false;
		FontRes* pRes = new FontRes(fileName, ok);
		lwassert(pRes);
		if ( !ok ){
			lwerror("new FontRes error");
			delete pRes;
			return NULL;
		}
		_sResMap[fileName] = pRes;
		
		return pRes;
	}

	void FontRes::cleanup(){
		
	}

	FontRes::FontRes(const char* fileName, bool& ok){
		ok = false;
		lwassert(fileName);
		_fileName = fileName;

		FILE* pf = fopen(_f(fileName), "rb");
		if ( pf == NULL ){
			lwerror("fopen failed: fileName=" << fileName);
			return;
		}

		//"BMF" and version
		char bmfAndVersion[4];
		fread(bmfAndVersion, sizeof(bmfAndVersion), 1, pf);
		if ( bmfAndVersion[0] != 'B' && bmfAndVersion[1] != 'M' && bmfAndVersion[2] != 'F' ){
			lwerror("not BMF head");
			fclose(pf);
		}
		if ( bmfAndVersion[3] != 3 ){
			lwerror("version 3 needed: this file version=" << (int)(bmfAndVersion[3]));
			fclose(pf);
		}

		//Block type 1: info
		char blockType = 0;
		int blockSize = 0;
		fread(&blockType, sizeof(blockType), 1, pf);
		lwassert(blockType == 1);
		fread(&blockSize, sizeof(blockSize), 1, pf);
		fseek(pf, blockSize, SEEK_CUR);

		//Block type 2: common
		fread(&blockType, sizeof(blockType), 1, pf);
		lwassert(blockType == 2);
		fread(&blockSize, sizeof(blockSize), 1, pf);
		fread(&_commonInfo, blockSize, 1, pf);

		//Block type 3: pages
		fread(&blockType, sizeof(blockType), 1, pf);
		lwassert(blockType == 3);
		fread(&blockSize, sizeof(blockSize), 1, pf);
		char* buf = new char[blockSize];
		fread(buf, blockSize, 1, pf);
		char* p = buf;
		int strSize = 0;
		while ( p < buf + blockSize ){
			strSize = (int)strlen(p);
			p[strSize-3] = 'p';
			p[strSize-2] = 'v';
			p[strSize-1] = 'r';
			Sprite* pSprite = Sprite::create(p);
			if ( pSprite == NULL ){
				lwerror("font create sprite failed: fileName=" << p);
				delete [] buf;
				fclose(pf);
				return;
			}
			_sprites.push_back(pSprite);
			p += strSize + 1;
		}
		delete [] buf;

		//Block type 4: chars
		fread(&blockType, sizeof(blockType), 1, pf);
		lwassert(blockType == 4);
		fread(&blockSize, sizeof(blockSize), 1, pf);
		int numChars = blockSize/20;
		CharInfo cf;
		for ( int i = 0; i < numChars; ++i ){
			fread(&cf, sizeof(cf), 1, pf);
			wchar_t wc = cf.id;
			_charInfoMap[wc] = cf;
		}

		fclose(pf);
		ok = true;
	}

	FontRes::~FontRes(){
		{
			std::vector<Sprite*>::iterator it = _sprites.begin();
			std::vector<Sprite*>::iterator itEnd = _sprites.end();
			for ( ; it != itEnd; ++it ){
				delete *it;
			}
		}
		{
			std::map<std::string, FontRes*>::iterator it = _sResMap.find(_fileName);
			if ( it != _sResMap.end() ){
				_sResMap.erase(it);
			}
		}
	}

	void FontRes::drawText(const wchar_t* text, float x, float y, const Color& color, float scale, float rotate){
		lwassert(text);
		size_t len = wcslen(text);
		const wchar_t* p = text;
		float currX = x;
		float currY = y;

		while ( p < text+len ){
			if ( *p == '\n' ){
				currY += _commonInfo.lineHeight*scale;
				currX = x;
			}else{
				std::map<wchar_t, CharInfo>::iterator it = _charInfoMap.find(*p);
				if ( it == _charInfoMap.end() ){
					it = _charInfoMap.find(' ');
					if ( it == _charInfoMap.end() ){
						currX += 10;
						++p;
						continue;
					}
				}
				CharInfo& charInfo = it->second;
				lwassert(charInfo.page < _sprites.size());
				Sprite* pSprite = _sprites[charInfo.page];
				pSprite->setUV(charInfo.x, charInfo.y, charInfo.w, charInfo.h);
				if ( rotate == 0 ){
					pSprite->collect((float)currX+charInfo.xoffset, (float)currY+charInfo.yoffset, charInfo.w*scale, charInfo.h*scale, rotate, false, false, color);
				}else{
					cml::Vector2 v2;
					v2[0] = (float)currX+charInfo.xoffset-x;
					v2[1] = (float)currY+charInfo.yoffset-y;
					v2 = cml::rotate_vector_2D(v2, rotate);
					pSprite->collect(x+v2[0], y+v2[1], charInfo.w*scale, charInfo.h*scale, rotate, false, false, color);
				}
				currX += charInfo.xadvance*scale;
			}
			++p;
		}
		
		//for ( size_t i = 0; i < len; ++i ){
		//	unsigned int id = (unsigned int)text[i];
		//	if ( id == '\n' ){}
		//	std::map<wchar_t, CharInfo>::iterator it = _charInfoMap.find(id);
		//	
		//	if ( it == _charInfoMap.end() ){
		//		x += 5;
		//	}
		//}
	}

	void FontRes::drawText(FontComposingData* pComposingData, float x, float y, const Color& color, float rotate){
		lwassert(pComposingData);
		const wchar_t* text = pComposingData->text.c_str();
		const std::vector<int>& linesOffset = pComposingData->linesOffset;
		size_t len = wcslen(text);
		const wchar_t* p = text;
		float currX = x;
		if ( !linesOffset.empty() ){
			currX += linesOffset[0];
		}
		float currY = y;

		int currLine = 1;
		while ( p < text+len ){
			if ( *p == '\n' ){
				currY += _commonInfo.lineHeight*pComposingData->scale;
				currX = x;
				if ( currLine < (int)linesOffset.size() ){
					currX += linesOffset[currLine];
				}
				++currLine;
			}else{
				std::map<wchar_t, CharInfo>::iterator it = _charInfoMap.find(*p);
				if ( it == _charInfoMap.end() ){
					it = _charInfoMap.find(' ');
					if ( it == _charInfoMap.end() ){
						currX += 10;
						++p;
						continue;
					}
				}
				CharInfo& charInfo = it->second;
				lwassert(charInfo.page < _sprites.size());
				Sprite* pSprite = _sprites[charInfo.page];
				pSprite->setUV(charInfo.x, charInfo.y, charInfo.w, charInfo.h);
				if ( rotate == 0 ){
					pSprite->collect((float)currX+charInfo.xoffset, (float)currY+charInfo.yoffset, charInfo.w*pComposingData->scale, charInfo.h*pComposingData->scale, rotate, false, false, color);
				}else{
					cml::Vector2 v2;
					v2[0] = (float)currX+charInfo.xoffset-x;
					v2[1] = (float)currY+charInfo.yoffset-y;
					v2 = cml::rotate_vector_2D(v2, rotate);
					pSprite->collect(x+v2[0], y+v2[1], charInfo.w*pComposingData->scale, charInfo.h*pComposingData->scale, rotate, false, false, color);
				}
				currX += charInfo.xadvance*pComposingData->scale;
			}
			++p;
		}
	}

	void FontRes::getSize(int& outX, int& outY, const wchar_t* text){
		lwassert(text);
		size_t len = wcslen(text);
		const wchar_t* p = text;
		outX = 0;
		outY = _commonInfo.lineHeight;
		int maxX = 0;
		int x = 0;
		while ( p < text+len ){
			if ( *p == '\n' ){
				outY += _commonInfo.lineHeight;
				maxX = max(maxX, x);
				x = 0;
			}else{
				std::map<wchar_t, CharInfo>::iterator it = _charInfoMap.find(*p);
				if ( it == _charInfoMap.end() ){
					it = _charInfoMap.find(' ');
					if ( it == _charInfoMap.end() ){
						x += 10;
						++p;
						continue;
					}
				}
				CharInfo& charInfo = it->second;
				lwassert(charInfo.page < _sprites.size());
				x += charInfo.xadvance;
			}
			++p;
		}
		outX = max(maxX, x);
	}

	void FontComposingData::set(Font* pFont, const wchar_t* t, FontAlign al, float w, float sc){
		lwassert(pFont && t);
		linesOffset.clear();
		scale = sc;

		text = t;
		align = al;
		if ( align == ALIGN_LEFT ){
			return;
		}

		std::map<wchar_t, FontRes::CharInfo>& charInfoMap = pFont->getFontRes()->getCharInfoMap();

		size_t len = text.length();
		const wchar_t* p = text.c_str();

		int x = 0;
		while ( 1 ){
			if ( *p == '\n' || p == text.c_str()+len ){
				x = (int)(x*scale);
				switch (align)
				{
				case ALIGN_LEFT:
					linesOffset.push_back(0);
					break;
				case ALIGN_CENTER:
					linesOffset.push_back((int)((w-x)*.5f));
					break;
				case ALIGN_RIGHT:
					linesOffset.push_back((int)(w-x));
					break;
				}
				x = 0;
				if ( p == text.c_str()+len ){
					break;
				}
			}else{
				std::map<wchar_t, FontRes::CharInfo>::iterator it = charInfoMap.find(*p);
				if ( it == charInfoMap.end() ){
					it = charInfoMap.find(' ');
					if ( it == charInfoMap.end() ){
						x += 10;
						++p;
						continue;
					}
				}
				FontRes::CharInfo& charInfo = it->second;
				x += charInfo.xadvance;
			}
			++p;
		}
	}

	Font* Font::create(const char* fileName){
		FontRes* pRes = FontRes::create(fileName);
		if ( pRes == NULL ){
			return NULL;
		}
		Font* pFont = new Font(pRes);
		lwassert(pFont);
		return pFont;
	}

	Font::Font(FontRes* pRes):_pRes(pRes){

	}

	Font::~Font(){
		_pRes->release();
	}

	void Font::drawText(const wchar_t* text, float x, float y, const Color& color, float scale, float rotate){
		if ( text ){
			_pRes->drawText(text, x, y, color, scale, rotate);
		}
	}
	void Font::drawText(FontComposingData* pComposingData, float x, float y, const Color& color, float rotate){
		if ( !pComposingData->text.empty() ){
			_pRes->drawText(pComposingData, x, y, color, rotate);
		}
	}

	void Font::cleanup(){
		FontRes::cleanup();
	}

	void Font::getSize(int& outX, int& outY, const wchar_t* text){
		_pRes->getSize(outX, outY, text);
	}

	int Font::getH(){
		return _pRes->getH();
	}

} //namespace lw