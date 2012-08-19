#ifndef __LW_SPRITE_H__
#define __LW_SPRITE_H__

namespace lw{

	class PODTextureRes;
	class PODModel;

	class Sprite{
	public:
		static Sprite* create(const char* texFileName, bool reserveData = false);
		static Sprite* create(GLuint glId, int w, int h, bool hasAlpha);

		~Sprite();
		void setUV(int u, int v, int w, int h);
		void setAnchor(float x, float y){
			_ancX = x; _ancY = y;
		}
		void getAnchor(float& x, float& y){
			x = _ancX;
			y = _ancY;
		}
		void collect(const cml::Matrix44& m, lw::Color color = lw::COLOR_WHITE);
		void collect(const cml::Matrix33& m, lw::Color color = lw::COLOR_WHITE);
		void collect(float x, float y, lw::Color color);
		void collect(float x, float y, float w=0, float h=0, float rotate = 0, bool flipH = false, bool flipV = false, lw::Color color = lw::COLOR_WHITE); //if w or h == 0, use uv's w or h;
		static void collectModel(PODModel* pModel, int nodeIndex, lw::Color color = lw::COLOR_WHITE);

		struct Vertex{
			float x, y, z;
			float u, v;
		};
		static void collectModel(const cml::Matrix44& m, const std::vector<Vertex>& vertexVec, const lw::Color& color, GLuint glId, bool hasAlpha);

		const PODTextureRes* getTextureRes(){
			return _pTextureRes;
		}
		unsigned short getW(){
			return _width;
		}
		unsigned short getH(){
			return _height;
		}
		
	private:
		Sprite(const char* texFileName, bool reserveData, bool &ok);
		Sprite(GLuint glId, int w, int h, bool hasAlpha, bool &ok);

	private:
		PODTextureRes* _pTextureRes;
		float _u1, _v1, _u2, _v2;
		float _ancX, _ancY;
		int _width, _height;
		int _texW, _texH;
		GLuint _glId;
		bool _hasAlpha;
	public:
		static void draw();
	};

	class ModelSprite{
	public:
		static ModelSprite* create(const char* podFileName, const char* nodeName);
		~ModelSprite();
		void collect(const cml::Matrix44& m, const lw::Color& color = lw::COLOR_WHITE);
		const std::vector<Sprite::Vertex>& getVertexVec(){
			return _vertexVec;
		}
	private:
		ModelSprite(const char* podFileName, const char* nodeName, bool& ok);
		std::vector<Sprite::Vertex> _vertexVec;
		PODTextureRes* _pTextureRes;
		bool _hasAlpha;
		GLuint _glId;
	};

	class RanderStateObj{
	public:
		RanderStateObj();
		virtual ~RanderStateObj(){}
		virtual void set() = 0;
	};

	class RSTexEnv : public RanderStateObj{
	public:
		RSTexEnv(GLenum pname, GLfixed param):_pname(pname), _xParam(param), _dataType(X){}
		RSTexEnv(GLenum pname, GLfloat param):_pname(pname), _fParam(param), _dataType(F){}
		virtual void set();
		enum DataType{
			X,
			F,
		};
		DataType _dataType;

	private:
		GLenum _pname;
		union{
			struct{
				GLfixed _xParam;
			};
			struct{
				GLfloat _fParam;
			};
		};
		
	};

	class RSTexEnvV : public RanderStateObj{
	public:
		RSTexEnvV(GLenum pname, GLfixed* param);
		RSTexEnvV(GLenum pname, GLfloat* param);
		virtual void set();
		enum DataType{
			X,
			F,
		};
		DataType _dataType;

	private:
		GLenum _pname;
		union{
			struct{
				GLfixed _xParam[4];
			};
			struct{
				GLfloat _fParam[4];
			};
		};

	};

} //namespace lw

#endif //__LW_SPRITE_H__