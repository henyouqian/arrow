#include "stdafx.h"
#include "lwSprite.h"
#include "lwPODTextureRes.h"
#include "lwPODModel.h"
#include "lwApp.h"
#include "lwFileSys.h"

//#define USEVBO

namespace lw{

	Sprite* Sprite::create(const char* texFileName, bool reserveData){
		lwassert(texFileName);
		bool ok;
		Sprite* p = new Sprite(texFileName, reserveData, ok);
		if ( p && !ok ){
			lwerror("Sprite construct failed");
			delete p;
			p = NULL;
		}
		return p;
	}

	Sprite* Sprite::create(GLuint glId, int w, int h, bool hasAlpha){
		bool ok = false;
		Sprite* p = new Sprite(glId, w, h, hasAlpha, ok);
		if ( p && !ok ){
			lwerror("Sprite construct failed");
			delete p;
			p = NULL;
		}
		return p;
	}

	Sprite::Sprite(const char* texFileName, bool reserveData, bool &ok)
	:_u1(0.f), _v1(0.f), _u2(1.f), _v2(1.f)
	,_ancX(0.f), _ancY(0.f)
	, _width(1), _height(1){
		_pTextureRes = PODTextureRes::create(texFileName, reserveData);
		ok = _pTextureRes != NULL;
		_texW = _pTextureRes->getHeader()->dwWidth;
		_texH = _pTextureRes->getHeader()->dwHeight;
		_glId = _pTextureRes->getGlId();
		_hasAlpha = _pTextureRes->getHeader()->dwAlphaBitMask != 0;
		setUV(0, 0, -1, -1);
	}

	Sprite::Sprite(GLuint glId, int w, int h, bool hasAlpha, bool &ok)
	:_pTextureRes(NULL), _texW(w), _texH(h), _glId(glId), _hasAlpha(hasAlpha),_ancX(0.f), _ancY(0.f){
		setUV(0, 0, -1, -1);
		ok = true;
	}

	Sprite::~Sprite(){
		if ( _pTextureRes ){
			_pTextureRes->release();
		}
	}

	void Sprite::setUV(int u, int v, int w, int h){
		_u1 = (float)u/(float)_texW;
		_v1 = (float)v/(float)_texH;
		if ( w <= 0 ){
			_width = _texW;
		}else{
			_width = w;
		}
		_u2 = (float)(u+_width)/(float)_texW;
		
		if ( h <= 0 ){
			_height = _texH;
		}else{
			_height = h;
		}
		_v2 = (float)(v+_height)/(float)_texH;
	}

	namespace{
		struct BatchInfo{
			int vertexOffset;
			Color color;
			union{
				struct{
					GLuint textureId;
					bool hasAlpha;
				};
				struct{
					PODModel* pModle;
					char nodeIndex;
				};
			};
			std::vector<RanderStateObj*> rsObjs;
		};
		struct ImageStaticData{
			ImageStaticData():currTextureId(-1){
				vbo = -1;
				glGenBuffers(1, &vbo);
			}
			~ImageStaticData(){
				glDeleteBuffers(1, &vbo);
			}
			GLuint vbo;
			std::vector<Sprite::Vertex> vertexVec;
			std::vector<BatchInfo> batchInfoVec;
			GLuint currTextureId;
			Color currColor;
			int currVertexOffset;
		};

		ImageStaticData g_imageStaticData;
		
		const float Z_BEGIN = -1000.f;
		const float Z_END = 1000.f;
		float g_currZ = Z_BEGIN+1.f;

		void stepZ(){
			g_currZ += 0.01f;
		}
	}

	void Sprite::collect(const cml::Matrix33& m, lw::Color color){
		ImageStaticData& d = g_imageStaticData;

		stepZ();

		float posX1 = -_ancX;
		float posY1 = -_ancY;
		float posX2 = posX1 + (float)_width;
		float posY2 = posY1 + (float)_height;

		std::vector<Vertex>& vertexVec = d.vertexVec;

		if ( d.currVertexOffset == -2 ){
			BatchInfo& batchInfo = d.batchInfoVec.back();
			d.currTextureId = _glId;
			d.currColor = color;
			d.currVertexOffset = (int)vertexVec.size();
			batchInfo.vertexOffset = d.currVertexOffset;
			batchInfo.textureId = d.currTextureId;
			batchInfo.hasAlpha = _hasAlpha;
			batchInfo.color = color;
		}else if ( _glId != d.currTextureId 
			|| color != d.currColor 
			|| d.currVertexOffset == -1 ) {
				d.currTextureId = _glId;
				d.currColor = color;
				d.currVertexOffset = (int)vertexVec.size();
				BatchInfo batchInfo;
				batchInfo.vertexOffset = d.currVertexOffset;
				batchInfo.textureId = d.currTextureId;
				batchInfo.hasAlpha = _hasAlpha;
				batchInfo.color = color; 
				d.batchInfoVec.push_back(batchInfo);
		}

		//
		cml::Vector2 p1;
		p1[0] = posX1; p1[1] = posY1;
		p1 = cml::transform_point_2D(m, p1);
		cml::Vector2 p2;
		p2[0] = posX1; p2[1] = posY2;
		p2 = cml::transform_point_2D(m, p2);
		cml::Vector2 p3;
		p3[0] = posX2; p3[1] = posY1;
		p3 = cml::transform_point_2D(m, p3);
		cml::Vector2 p4;
		p4[0] = posX2; p4[1] = posY2;
		p4 = cml::transform_point_2D(m, p4);

		//
		float minx = min(p1[0], p2[0]);
		minx = min(minx, p3[0]);
		minx = min(minx, p4[0]);
		float miny = min(p1[1], p2[1]);
		miny = min(miny, p3[1]);
		miny = min(miny, p4[1]);
		float maxx = max(p1[0], p2[0]);
		maxx = max(maxx, p3[0]);
		maxx = max(maxx, p4[0]);
		float maxy = max(p1[1], p2[1]);
		maxy = max(maxy, p3[1]);
		maxy = max(maxy, p4[1]);

		const App::Config conf = App::getConfig();
		int clientW = conf.width;
		int clientH = conf.height;
		if ( minx >= clientW || maxx <= 0 
			|| miny >= clientH || maxy <= 0 ){
				return;
		}

		//1
		Vertex v;
		v.x = p1[0];
		v.y = -p1[1];
		v.z = g_currZ;
		v.u = _u1;
		v.v = 1.f -_v1;
		vertexVec.push_back(v);

		//2
		v.x = p2[0];
		v.y = -p2[1];
		v.z = g_currZ;
		v.u = _u1;
		v.v = 1.f -_v2;
		vertexVec.push_back(v);

		//3
		v.x = p3[0];
		v.y = -p3[1];
		v.z = g_currZ;
		v.u = _u2;
		v.v = 1.f -_v1;
		vertexVec.push_back(v);

		//3
		v.x = p3[0];
		v.y = -p3[1];
		v.z = g_currZ;
		v.u = _u2;
		v.v = 1.f -_v1;
		vertexVec.push_back(v);

		//2
		v.x = p2[0];
		v.y = -p2[1];
		v.z = g_currZ;
		v.u = _u1;
		v.v = 1.f -_v2;
		vertexVec.push_back(v);

		//4
		v.x = p4[0];
		v.y = -p4[1];
		v.z = g_currZ;
		v.u = _u2;
		v.v = 1.f -_v2;
		vertexVec.push_back(v);
	}
	
	void Sprite::collect(const cml::Matrix44& m, lw::Color color){
		ImageStaticData& d = g_imageStaticData;
		
		stepZ();

		float posX1 = -_ancX;
		float posY1 = -_ancY;
		float posX2 = posX1 + (float)_width;
		float posY2 = posY1 + (float)_height;

		std::vector<Vertex>& vertexVec = d.vertexVec;

		if ( d.currVertexOffset == -2 ){
			BatchInfo& batchInfo = d.batchInfoVec.back();
			d.currTextureId = _glId;
			d.currColor = color;
			d.currVertexOffset = (int)vertexVec.size();
			batchInfo.vertexOffset = d.currVertexOffset;
			batchInfo.textureId = d.currTextureId;
			batchInfo.hasAlpha = _hasAlpha;
			batchInfo.color = color;
		}else if ( _glId != d.currTextureId 
			|| color != d.currColor 
			|| d.currVertexOffset == -1 ) {
			d.currTextureId = _glId;
			d.currColor = color;
			d.currVertexOffset = (int)vertexVec.size();
			BatchInfo batchInfo;
			batchInfo.vertexOffset = d.currVertexOffset;
			batchInfo.textureId = d.currTextureId;
			batchInfo.hasAlpha = _hasAlpha;
			batchInfo.color = color; 
			d.batchInfoVec.push_back(batchInfo);
		}

		//
		cml::Vector3 p1;
		p1[0] = posX1; p1[1] = posY1; p1[2] = 0;
		p1 = cml::transform_point(m, p1);
		cml::Vector3 p2;
		p2[0] = posX1; p2[1] = posY2; p2[2] = 0;
		p2 = cml::transform_point(m, p2);
		cml::Vector3 p3;
		p3[0] = posX2; p3[1] = posY1; p3[2] = 0;
		p3 = cml::transform_point(m, p3);
		cml::Vector3 p4;
		p4[0] = posX2; p4[1] = posY2; p4[2] = 0;
		p4 = cml::transform_point(m, p4);

		//
		float minx = min(p1[0], p2[0]);
		minx = min(minx, p3[0]);
		minx = min(minx, p4[0]);
		float miny = min(p1[1], p2[1]);
		miny = min(miny, p3[1]);
		miny = min(miny, p4[1]);
		float maxx = max(p1[0], p2[0]);
		maxx = max(maxx, p3[0]);
		maxx = max(maxx, p4[0]);
		float maxy = max(p1[1], p2[1]);
		maxy = max(maxy, p3[1]);
		maxy = max(maxy, p4[1]);

		const App::Config conf = App::getConfig();
		int clientW = conf.width;
		int clientH = conf.height;
		if ( minx >= clientW || maxx <= 0 
			|| miny >= clientH || maxy <= 0 ){
				return;
		}
		
		//1
		Vertex v;
		v.x = p1[0];
		v.y = -p1[1];
		v.z = g_currZ;
		v.u = _u1;
		v.v = 1.f -_v1;
		vertexVec.push_back(v);

		//2
		v.x = p2[0];
		v.y = -p2[1];
		v.z = g_currZ;
		v.u = _u1;
		v.v = 1.f -_v2;
		vertexVec.push_back(v);

		//3
		v.x = p3[0];
		v.y = -p3[1];
		v.z = g_currZ;
		v.u = _u2;
		v.v = 1.f -_v1;
		vertexVec.push_back(v);

		//3
		v.x = p3[0];
		v.y = -p3[1];
		v.z = g_currZ;
		v.u = _u2;
		v.v = 1.f -_v1;
		vertexVec.push_back(v);

		//2
		v.x = p2[0];
		v.y = -p2[1];
		v.z = g_currZ;
		v.u = _u1;
		v.v = 1.f -_v2;
		vertexVec.push_back(v);

		//4
		v.x = p4[0];
		v.y = -p4[1];
		v.z = g_currZ;
		v.u = _u2;
		v.v = 1.f -_v2;
		vertexVec.push_back(v);
	}

	void Sprite::collect(float x, float y, float w, float h, float rotate, bool flipH, bool flipV, lw::Color color){
		ImageStaticData& d = g_imageStaticData;

		stepZ();

		float posX1 = -_ancX;
		float posY1 = -_ancY;
		float posX2 = 0;
		float posY2 = 0;
		if ( w <= 0 ){
			posX2 = posX1 + (float)_width;
		}else{
			posX2 = posX1 + w;
		}
		if ( h <= 0 ){
			posY2 = posY1 + (float)_height;
		}else{
			posY2 = posY1 + h;
		}

		cml::Vector3 p1;
		p1[0] = posX1; p1[1] = posY1; p1[2] = 0;
		cml::Vector3 p2;
		p2[0] = posX1; p2[1] = posY2; p2[2] = 0;
		cml::Vector3 p3;
		p3[0] = posX2; p3[1] = posY1; p3[2] = 0;
		cml::Vector3 p4;
		p4[0] = posX2; p4[1] = posY2; p4[2] = 0;

		if ( rotate != 0.f ){
			cml::Matrix44 m;
			cml::matrix_rotation_world_z(m, rotate);
			p1 = cml::transform_point(m, p1);
			p2 = cml::transform_point(m, p2);
			p3 = cml::transform_point(m, p3);
			p4 = cml::transform_point(m, p4);
		}
		p1[0]+=x; p1[1]+=y;
		p2[0]+=x; p2[1]+=y;
		p3[0]+=x; p3[1]+=y;
		p4[0]+=x; p4[1]+=y;

		float minx = min(p1[0], p2[0]);
		minx = min(minx, p3[0]);
		minx = min(minx, p4[0]);
		float miny = min(p1[1], p2[1]);
		miny = min(miny, p3[1]);
		miny = min(miny, p4[1]);
		float maxx = max(p1[0], p2[0]);
		maxx = max(maxx, p3[0]);
		maxx = max(maxx, p4[0]);
		float maxy = max(p1[1], p2[1]);
		maxy = max(maxy, p3[1]);
		maxy = max(maxy, p4[1]);

		const App::Config conf = App::getConfig();
		int clientW = conf.width;
		int clientH = conf.height;
		if ( minx >= clientW || maxx <= 0 
			|| miny >= clientH || maxy <= 0 ){
				return;
		}
		std::vector<Vertex>& vertexVec = d.vertexVec;

		if ( d.currVertexOffset == -2 ){
			BatchInfo& batchInfo = d.batchInfoVec.back();
			d.currTextureId = _glId;
			d.currColor = color;
			d.currVertexOffset = (int)vertexVec.size();
			batchInfo.vertexOffset = d.currVertexOffset;
			batchInfo.textureId = d.currTextureId;
			batchInfo.hasAlpha = _hasAlpha;
			batchInfo.color = color;
		}
		else if ( _glId != d.currTextureId 
			|| color != d.currColor 
			|| d.currVertexOffset == -1) {
				d.currTextureId = _glId;
				d.currColor = color;
				d.currVertexOffset = (int)vertexVec.size();
				BatchInfo batchInfo;
				batchInfo.vertexOffset = d.currVertexOffset;
				batchInfo.textureId = d.currTextureId;
				batchInfo.hasAlpha = _hasAlpha;
				batchInfo.color = color; 
				d.batchInfoVec.push_back(batchInfo);
		}
		

		//1
		Vertex v;
		v.x = p1[0];
		v.y = -p1[1];
		v.z = g_currZ;
		if ( flipH ){
			v.u = _u2;
		}else{
			v.u = _u1;
		}
		if ( flipV ){
			v.v = 1.f - _v2;
		}else{
			v.v = 1.f - _v1;
		}
		vertexVec.push_back(v);

		//2
		v.x = p2[0];
		v.y = -p2[1];
		v.z = g_currZ;
		if ( flipH ){
			v.u = _u2;
		}else{
			v.u = _u1;
		}
		if ( flipV ){
			v.v = 1.f - _v1;
		}else{
			v.v = 1.f - _v2;
		}
		vertexVec.push_back(v);

		//3
		v.x = p3[0];
		v.y = -p3[1];
		v.z = g_currZ;
		if ( flipH ){
			v.u = _u1;
		}else{
			v.u = _u2;
		}
		if ( flipV ){
			v.v = 1.f - _v2;
		}else{
			v.v = 1.f - _v1;
		}
		vertexVec.push_back(v);

		//3
		v.x = p3[0];
		v.y = -p3[1];
		v.z = g_currZ;
		if ( flipH ){
			v.u = _u1;
		}else{
			v.u = _u2;
		}
		if ( flipV ){
			v.v = 1.f - _v2;
		}else{
			v.v = 1.f - _v1;
		}
		vertexVec.push_back(v);

		//2
		v.x = p2[0];
		v.y = -p2[1];
		v.z = g_currZ;

		if ( flipH ){
			v.u = _u2;
		}else{
			v.u = _u1;
		}
		if ( flipV ){
			v.v = 1.f - _v1;
		}else{
			v.v = 1.f - _v2;
		}
		vertexVec.push_back(v);

		//4
		v.x = p4[0];
		v.y = -p4[1];
		v.z = g_currZ;
		if ( flipH ){
			v.u = _u1;
		}else{
			v.u = _u2;
		}
		if ( flipV ){
			v.v = 1.f - _v1;
		}else{
			v.v = 1.f - _v2;
		}

		vertexVec.push_back(v);
	}

	void Sprite::collect(float x, float y, lw::Color color){
		collect(x, y, 0, 0, 0.0f, false, false, color);
	}

	void Sprite::collectModel(PODModel* pModel, int nodeIndex, lw::Color color){
		lwassert(pModel);
		stepZ();
		ImageStaticData& d = g_imageStaticData;

		if ( d.currVertexOffset == -2 ){
			BatchInfo& batchInfo = d.batchInfoVec.back();
			batchInfo.vertexOffset = -1;
			batchInfo.pModle = pModel;
			batchInfo.nodeIndex = (char)nodeIndex;
			batchInfo.color = color;
			d.currVertexOffset = -1;
		}else{
			BatchInfo batchInfo;
			batchInfo.vertexOffset = -1;
			batchInfo.pModle = pModel;
			batchInfo.nodeIndex = (char)nodeIndex;
			batchInfo.color = color;
			d.currVertexOffset = -1;
			d.batchInfoVec.push_back(batchInfo);
		}
	}

	void Sprite::collectModel(const cml::Matrix44& m, const std::vector<Vertex>& vertexVec, const lw::Color& color, GLuint glId, bool hasAlpha){
		ImageStaticData& d = g_imageStaticData;
		hasAlpha = hasAlpha || color.a < 255;

		std::vector<Vertex>& v = d.vertexVec;

		if ( d.currVertexOffset == -2 ){
			BatchInfo& batchInfo = d.batchInfoVec.back();
			d.currTextureId = glId;
			d.currColor = color;
			d.currVertexOffset = (int)v.size();
			batchInfo.vertexOffset = d.currVertexOffset;
			batchInfo.textureId = d.currTextureId;
			batchInfo.hasAlpha = hasAlpha;
			batchInfo.color = color;
		}
		else if ( glId != d.currTextureId 
			|| color != d.currColor 
			|| d.currVertexOffset == -1) {
				d.currTextureId = glId;
				d.currColor = color;
				d.currVertexOffset = (int)v.size();
				BatchInfo batchInfo;
				batchInfo.vertexOffset = d.currVertexOffset;
				batchInfo.textureId = d.currTextureId;
				batchInfo.hasAlpha = hasAlpha;
				batchInfo.color = color; 
				d.batchInfoVec.push_back(batchInfo);
		}

		std::vector<Vertex>::const_iterator it = vertexVec.begin();
		std::vector<Vertex>::const_iterator itEnd = vertexVec.end();
		for ( ; it != itEnd; ++it ){
			cml::Vector3 p1;
			p1[0] = it->x; p1[1] = it->y; p1[2] = 0;
			p1 = cml::transform_point(m, p1);
			Vertex vtx;
			vtx = *it;
			vtx.x = p1[0]; vtx.y = p1[1];
			v.push_back(vtx);
		}
		stepZ();
	}

	void Sprite::draw(){
		ImageStaticData& d = g_imageStaticData;
		if ( d.vertexVec.empty() && d.batchInfoVec.empty() ){
			return;
		}
#ifdef USEVBO		
		if ( d.vbo == -1 ){
			glGenBuffers(1, &d.vbo);
		}
#endif

		glDisable(GL_LIGHTING);
		glDepthMask(GL_FALSE);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		const App::Config conf = App::getConfig();
		float w = (float)conf.width;
		float h = (float)conf.height;
#ifdef __APPLE__
		float rotDegree = 0;
		const App::Orientation orient = App::getConfig().orientation;
		switch (orient) {
			case App::ORIENTATION_LEFT:
				rotDegree = 90;
				break;
			case App::ORIENTATION_RIGHT:
				rotDegree = -90;
				break;
			default:
				break;
		}
		glRotatef(rotDegree, 0, 0, 1);
#endif
#ifdef WIN32
		int dOrient = (App::getConfig().orientation-App::getConfig().orientation0);
		float rotDegree = -dOrient * 90.f;
		glRotatef(rotDegree, 0, 0, 1);
		//if ( abs(dOrient) == 1 ){
		//	float temp = h;
		//	h = w;
		//	w = temp;
		//}
#endif
		cml::Matrix44 m;
		cml::matrix_orthographic_RH(m, 0.f, w, -h, 0.f, Z_BEGIN, Z_END, cml::z_clip_neg_one);
		glMultMatrixf(m.data());

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_TEXTURE_2D);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

#ifdef USEVBO
		glBindBuffer(GL_ARRAY_BUFFER, d.vbo);
		glBufferData(GL_ARRAY_BUFFER, (int)d.vertexVec.size()*sizeof(Vertex), &(d.vertexVec[0]), GL_DYNAMIC_DRAW);
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (GLvoid*)0);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (GLvoid*)(sizeof(float)*3));
#else
		if ( !d.vertexVec.empty() ){
			glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &(d.vertexVec[0]));
			char* p = (char*)&(d.vertexVec[0]);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), p+(sizeof(float)*3));
		}
#endif
		
		bool blendEnabled = false;
		for ( size_t i = 0; i < d.batchInfoVec.size(); ++i ){
			BatchInfo& batchInfo = d.batchInfoVec[i];
			glColor4f(batchInfo.color.r/255.f, batchInfo.color.g/255.f, batchInfo.color.b/255.f, batchInfo.color.a/255.f);
			if ( !batchInfo.rsObjs.empty() ){
				std::vector<RanderStateObj*>::iterator it = batchInfo.rsObjs.begin();
				std::vector<RanderStateObj*>::iterator itEnd = batchInfo.rsObjs.end();
				for ( ; it != itEnd; ++it ){
					(*it)->set();
				}
			}
			if ( batchInfo.vertexOffset == -2 ){
				if ( !batchInfo.rsObjs.empty() ){
					std::vector<RanderStateObj*>::iterator it = batchInfo.rsObjs.begin();
					std::vector<RanderStateObj*>::iterator itEnd = batchInfo.rsObjs.end();
					for ( ; it != itEnd; ++it ){
						delete (*it);
					}
				}
				batchInfo.rsObjs.clear();
				continue;
			}
			//model
			if ( batchInfo.vertexOffset == -1 ){
				batchInfo.pModle->drawAsSprite(batchInfo.nodeIndex);
				if ( blendEnabled ){
					glEnable(GL_BLEND);
					//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}else{
					glDisable(GL_BLEND);
				}
				if ( !d.vertexVec.empty() && i < d.batchInfoVec.size()-1 ){
					glEnable(GL_TEXTURE_2D);
					glEnableClientState(GL_VERTEX_ARRAY);
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &(d.vertexVec[0]));
					char* p = (char*)&(d.vertexVec[0]);
					glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), p+(sizeof(float)*3));
				}

				if ( !batchInfo.rsObjs.empty() ){
					std::vector<RanderStateObj*>::iterator it = batchInfo.rsObjs.begin();
					std::vector<RanderStateObj*>::iterator itEnd = batchInfo.rsObjs.end();
					for ( ; it != itEnd; ++it ){
						delete (*it);
					}
				}
				batchInfo.rsObjs.clear();
				continue;
			}

			bool shouldBlend = batchInfo.hasAlpha || batchInfo.color.a != 255;
			if ( shouldBlend != blendEnabled ){
				blendEnabled = shouldBlend;
				if ( blendEnabled ){
					glEnable(GL_BLEND);
					//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}else{
					glDisable(GL_BLEND);
				}
			}
			
			glBindTexture(GL_TEXTURE_2D, batchInfo.textureId);
			lwassert(glGetError()==GL_NO_ERROR);
			//find next sprite batch
			int sz = 0;
			bool find = false;
			for ( size_t idx = i+1; idx < d.batchInfoVec.size(); ++idx ){
				if ( d.batchInfoVec[idx].vertexOffset != -1 ){
					sz = d.batchInfoVec[idx].vertexOffset - batchInfo.vertexOffset;
					find = true;
					break;
				}
			}
			if ( !find ){
				sz = (int)d.vertexVec.size()-batchInfo.vertexOffset;
			}
			//if ( i == d.batchInfoVec.size() - 1 ){
			//	sz = (int)d.vertexVec.size()-batchInfo.vertexOffset;
			//}else{
			//	sz = d.batchInfoVec[i+1].vertexOffset - batchInfo.vertexOffset;
			//}
			
			glDrawArrays(GL_TRIANGLES, batchInfo.vertexOffset, sz);

			if ( !batchInfo.rsObjs.empty() ){
				std::vector<RanderStateObj*>::iterator it = batchInfo.rsObjs.begin();
				std::vector<RanderStateObj*>::iterator itEnd = batchInfo.rsObjs.end();
				for ( ; it != itEnd; ++it ){
					delete (*it);
				}
				batchInfo.rsObjs.clear();
			}
		}

		glDepthMask(GL_TRUE);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		if ( blendEnabled ){
			glDisable(GL_BLEND);
		}
		d.vertexVec.clear();
		d.batchInfoVec.clear();
		d.currTextureId = -1;
		//float z = g_currZ;
		g_currZ = Z_BEGIN+1.f;
	}

	RanderStateObj::RanderStateObj(){
		ImageStaticData& d = g_imageStaticData;
		if ( d.currVertexOffset != -2 ){
			BatchInfo batchInfo;
			batchInfo.vertexOffset = -2;
			d.batchInfoVec.push_back(batchInfo);
			d.currVertexOffset = -2;
		}
		BatchInfo& bInfo = d.batchInfoVec.back();
		bInfo.rsObjs.push_back(this);
	}

	void RSTexEnv::set(){
		if ( _dataType == X ){
			glTexEnvx(GL_TEXTURE_ENV, _pname, _xParam);
		}else{
			glTexEnvf(GL_TEXTURE_ENV, _pname, _fParam);
		}
	}

	RSTexEnvV::RSTexEnvV(GLenum pname, GLfixed* param):_pname(pname), _dataType(X){
		memcpy(_xParam, param, sizeof(_xParam));
	}

	RSTexEnvV::RSTexEnvV(GLenum pname, GLfloat* param):_pname(pname), _dataType(F){
		memcpy(_fParam, param, sizeof(_fParam));
	}

	void RSTexEnvV::set(){
		if ( _dataType == X ){
			glTexEnvxv(GL_TEXTURE_ENV, _pname, _xParam);
		}else{
			glTexEnvfv(GL_TEXTURE_ENV, _pname, _fParam);
		}
	}

	ModelSprite* ModelSprite::create(const char* podFileName, const char* nodeName){
		bool ok = false;
		ModelSprite* p = new ModelSprite(podFileName, nodeName, ok);
		lwassert(p);
		if ( p && !ok ){
			lwerror("new ModelSprite failed");
			delete p;
			return NULL;
		}
		return p;
	}

	ModelSprite::ModelSprite(const char* podFileName, const char* nodeName, bool& ok){
		ok = false;
		lwassert(podFileName && nodeName);
		CPVRTModelPOD pod;
		if(pod.ReadFromFile(_f(podFileName)) != PVR_SUCCESS){
			lwerror("Failed to load pod file. file=" << podFileName);
			return;
		}

		//find node
		SPODNode* pNode = NULL;
		for ( int i = 0; i < (int)pod.nNumNode; ++i ){
			if ( strcmp(pod.pNode[i].pszName, nodeName) == 0 ){
				pNode = pod.pNode + i;
			}
		}
		if ( pNode == NULL ){
			lwerror("node is not found: nodeName=" << nodeName);
			return;
		}
		if ( pNode->nIdx >= (int)pod.nNumMesh ){
			lwerror("invalid mesh index");
			return;
		}
		SPODMesh* pMesh = pod.pMesh+pNode->nIdx;
		
		char* bp = (char*)pMesh->sVertex.pData;
		unsigned int stride = pMesh->sVertex.nStride;
		unsigned short* pIndex = (unsigned short*)pMesh->sFaces.pData;
		char* bpUV = (char*)pMesh->psUVW[0].pData;
		unsigned int strideUV = pMesh->psUVW[0].nStride;
		float* pv = NULL;

		Sprite::Vertex vtx;
		for ( unsigned int j = 0; j < pMesh->nNumFaces; ++j ){
			for ( int ii = 0; ii < 3; ++ii ){
				pv = (float*)(bp + (*pIndex)*stride);
				vtx.x = *pv;
				vtx.y = *(pv+1);
				vtx.z = 0;
				pv = (float*)(bpUV + (*pIndex)*strideUV);
				vtx.u = *pv;
				vtx.v = *(pv+1);
				_vertexVec.push_back(vtx);
				++pIndex;
			}
		}
		
		SPODMaterial* pMaterial = &pod.pMaterial[pNode->nIdxMaterial];
		int textureIndex = pMaterial->nIdxTexDiffuse;
		if ( textureIndex == -1 ){
			_glId = 0;
			_hasAlpha = false;
		}else{
			const char* textureName = pod.pTexture[textureIndex].pszName;
			if ( textureName == NULL ){
				lwerror("textureName == NULL");
				return;
			}
			const char* p = strrchr(textureName, '.');
			std::string str = textureName;
			if ( p ){
				str.resize(p-textureName);
			}
			str.append(".png");
			_pTextureRes = PODTextureRes::create(str.c_str());
			if ( _pTextureRes == NULL ){
				lwerror("PODTextureRes::create failed");
				return;
			}
			_glId = _pTextureRes->getGlId();
			_hasAlpha = _pTextureRes->getHeader()->dwAlphaBitMask != 0;
		}

		ok = true;
	}

	ModelSprite::~ModelSprite(){
		_pTextureRes->release();
	}

	void ModelSprite::collect(const cml::Matrix44& m, const lw::Color& color){
		cml::Matrix44 mm = m;
		mm.data()[13] = -m.data()[13];
		Sprite::collectModel(mm, _vertexVec, color, _glId, _hasAlpha);
	}


} //namespace lw