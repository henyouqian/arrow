#ifndef __LW_POD_MODEL_H__
#define __LW_POD_MODEL_H__

namespace lw{

	class PODModelRes;
	class PODModelCollector;
	class PODTextureRes;

	class PODModel
	{
	public:
		static PODModel* create(const char* filename, const char* nodeName = NULL);
		~PODModel();
		void collect();
		void collect(unsigned char nodeIndex);
		void collectAsSprite(bool sortZ = false);
		void collectAsSprite(unsigned char nodeIndex);
		void setFrame(float frame){
			_frame = frame;
		}
		cml::Matrix44& getLocalMatrix(){
			return _localMatrix;
		}
		const cml::Matrix44& getWorldMatrix(){
			return _worldMatrix;
		}
		PODModelRes* getRes(){
			return _pRes;
		}
		void setParent(PODModel* pModel, unsigned char nodeIndex);
		void unParent();
		PODModel* getParent(){
			return _pParent;
		}
		int getParentNodeIndex(){
			return (int)_parentNodeIndex;
		}

		void calcNodeWorldViewMatrix(cml::Matrix44& m, int nodeIndex, const cml::Matrix44& mView);

		static void main();
		void drawAsSprite(int nodeIndex);

		bool replaceTexture(int textureIndex, const char* textureName);
		int getTextureIndex(const char* name);

		void setColor(const Color& c){
			_color = c;
		}

		void updateMatrix();

		void draw(const lw::Color& fillColor);

		struct SortElem{
			int nodeIndex;
			float z;
		};

	private:
		PODModel(const char* filename, const char* nodeName);
		void drawNode(int nodeIndex, cml::Matrix44& m);
		//void updateMatrix();
		
	private:
		PODModelRes* _pRes;
		unsigned char _nodeIndex;
		cml::Matrix44 _localMatrix;
		cml::Matrix44 _worldMatrix;
		float _frame;
		PODModel* _pParent;
		unsigned char _parentNodeIndex;
		std::list<PODModel*>* _pChildren;
		Color _color;

		static std::list<PODModel*> _sRootChildren;
		std::map<int, PODTextureRes*> _replaceTextureMap;

		
		std::vector<SortElem> _spriteSortVec;

		friend class PODModelCollector;
	};

} //namespace lw


#endif //__LW_POD_MODEL_H__