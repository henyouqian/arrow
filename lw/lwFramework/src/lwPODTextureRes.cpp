#include "stdafx.h"
#include "lwPODTextureRes.h"
#include "lwFileSys.h"
#include "soil/SOIL.h"

namespace lw{

	std::map<std::string, PODTextureRes*> PODTextureRes::_resMap;

	PODTextureRes::PODTextureRes(const char* fileName, bool reserveData) : _glId(-1), _pImgData(NULL){
		lwassert(fileName);
		_fileName = fileName;
		_fileName.resize(_fileName.size()-4);

		size_t len = strlen(fileName);
		if ( len < 4 ){
			lwerror("texture file name too short: filepath = " << _f(fileName));
			_glId = -1;
			return;
		}
		_f fpath(fileName);
		if ( fileName[len-4] == '.' && fileName[len-3] == 'p' 
		&& fileName[len-2] == 'v' && fileName[len-1] == 'r'){
			if ( fpath.isValid() ){
				loadPVR(fpath);
			}else{
				std::string str = fileName;
				str[len-4] = '.';
				str[len-3] = 'p'; 
				str[len-2] = 'n';
				str[len-1] = 'g';
				_f fpath(str.c_str());
				if ( fpath.isValid() ){
					loadPNG(fpath, reserveData);
				}else{
					lwerror("texture is not exist: " << fileName << " or " << str.c_str());
					return;
				}
			}
		}else{
			if ( fpath.isValid() ){
				loadPNG(_f(fileName), reserveData);
			}else{
				lwerror("texture is not exist: " << fileName);
				return;
			}
		}
	}

	void PODTextureRes::loadPVR(const char* path){
		if(PVRTTextureLoadFromPVR(path, &_glId, &_header) != PVR_SUCCESS)
		{
			lwerror("Failed to load texture: path=" << path);
			_glId = -1;
		}
	}

	//void PODTextureRes::loadPNG(const char* path){
	//	int numChannels = 0;
	//	_glId = SOIL_load_OGL_texture(
	//		path,
	//		SOIL_LOAD_AUTO,
	//		SOIL_CREATE_NEW_ID,
	//		SOIL_FLAG_INVERT_Y,
	//		&(_header.dwWidth),
	//		&(_header.dwHeight),
	//		&numChannels
	//		);
	//	_header.dwAlphaBitMask = numChannels == 4 ? 1 : 0;
	//	if ( _glId == 0 ){
	//		lwerror("Failed to load texture: path=" << path);
	//		_glId = -1;
	//	}
	//}

	void PODTextureRes::loadPNG(const char* path, bool reserveData){
		int numChannels;
		int w, h;
		_pImgData = SOIL_load_image(path, &w, &h, &numChannels, SOIL_LOAD_AUTO);
		_header.dwWidth = w;
		_header.dwHeight = h;
		_header.dwAlphaBitMask = numChannels == 4 ? 1 : 0;
		//int format = _header.dwAlphaBitMask == 1 ? GL_RGBA:GL_RGB;
		_glId = SOIL_internal_create_OGL_texture(_pImgData, w, h, numChannels,
			SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y,
			GL_TEXTURE_2D, GL_TEXTURE_2D,
			GL_MAX_TEXTURE_SIZE);
		
		if ( !reserveData ){
			SOIL_free_image_data(_pImgData);
			_pImgData = NULL;
		}
		
		if ( _glId == 0 ){
			lwerror("Failed to load texture: path=" << path);
			_glId = -1;
		}
	}

	PODTextureRes::~PODTextureRes(){
		if ( _glId != -1 ){
			glDeleteTextures(1, &_glId);
			std::map<std::string, PODTextureRes*>::iterator it = _resMap.find(_fileName.c_str());
			if ( it != _resMap.end() ){
				_resMap.erase(it);
			}else{
				lwerror("texture is not found: filename=" << _fileName.c_str());
			}
			if ( _pImgData ){
				SOIL_free_image_data(_pImgData);
			}
		}
	}
	PODTextureRes* PODTextureRes::create(const char* fileName, bool reserveData){
		lwassert(fileName);
		//size_t len = strlen(fileName);
		//if ( len < 4 ){
		//	lwerror("texture file name too short: filepath = " << _f(fileName));
		//	return NULL;
		//}
		//if ( fileName[len-4] != '.' || fileName[len-3] != 'p' 
		//|| fileName[len-2] != 'v' || fileName[len-1] != 'r'){
		//	lwerror("texture file must be pvr file");
		//	return NULL;
		//}

		size_t len = strlen(fileName);
		lwassert( fileName[len-4] == '.' );
		std::string strFileName = fileName;
		strFileName.resize(len-4);

		std::map<std::string, PODTextureRes*>::iterator it = _resMap.find(strFileName);
		if ( it == _resMap.end() ){
			PODTextureRes* p = new PODTextureRes(fileName, reserveData);
			if ( p && p->_glId == -1 ){
				delete p;
				return NULL;
			}else{
				_resMap[strFileName] = p;
				return p;
			}
		}else{
			it->second->addRef();
			return it->second;
		}
	}

	void PODTextureRes::cleanup(){
		while( !_resMap.empty() ){
			lwwarning("Texture resource leak: file=" << _resMap.begin()->first.c_str());
			delete (_resMap.begin()->second);
		}
	}

} //namespace lw