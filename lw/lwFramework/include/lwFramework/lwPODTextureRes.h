#ifndef __LW_POD_TEXTURE_H__
#define __LW_POD_TEXTURE_H__

#include "lwRes.h"

namespace lw{

	class PODTextureRes : public Res
	{
	public:
		static PODTextureRes* create(const char* fileName, bool reserveData = false);	//reserveData only for PNG
		GLuint getGlId() const{
			return _glId;
		}
		const char* getFileName(){return _fileName.c_str();}
		const PVR_Texture_Header* getHeader() const{
			return &_header;
		}
		const unsigned char* getData() const{
			return _pImgData;
		}

	private:
		~PODTextureRes();
		PODTextureRes(const char* filename, bool reserveData = false);
		void loadPVR(const char* path);
		void loadPNG(const char* path, bool reserveData);

	private:
		GLuint _glId;
		PVR_Texture_Header _header;
		std::string _fileName;
		unsigned char* _pImgData;

		static std::map<std::string, PODTextureRes*> _resMap;
		
	public:
		static void cleanup();
	};

} //namespace lw


#endif //__LW_POD_TEXTURE_H__