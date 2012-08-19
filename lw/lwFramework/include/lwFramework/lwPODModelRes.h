#ifndef __LW_POD_MODEL_RES_H__
#define __LW_POD_MODEL_RES_H__

#include "lwRes.h"

namespace lw{

	class PODTextureRes;
	class PODModel;
	class Camera;
	class AnimChannel;

	enum AlphaMode{
		ALPHA_NONE,
		ALPHA_TEST,
		ALPHA_BLEND,
		ALPHA_ADD,
		ALPHA_NOTCARE
	};

	class PODModelRes : public Res
	{
	friend class PODModel;

	public:
		static PODModelRes* create(const char* filename);		

		void setFrame(float frame);
		void draw(int nodeIndex, const cml::Matrix44& worldViewMatrix, const Color& color, const std::map<int, PODTextureRes*>& replaceTexMap);
		AlphaMode getAlphaMode(int nodeIndex);
		void setCamera(Camera& cam, int camIndex);
		int getNodeIndex(const char* nodeName);
		int getCameraIndex(const char* nodeName);
		CPVRTModelPOD& getPOD(){
			return _pod;
		}
		const cml::Vector3& getMeshBoundingBoxCenter(int nodeIndex);
		
	private:
		PODModelRes(const char* filename);
		~PODModelRes();
		bool isValid(){
			return _isValid;
		}
		
	private:
		bool _isValid;
		CPVRTModelPOD _pod;
		GLuint*	_vbos;
		GLuint*	_indexVbos;
		unsigned char** _nodeHierarchyIndices;
		unsigned char* _nodeHierarchy;
		std::vector<PODTextureRes*> _textureReses;
		std::map<std::string, PODModelRes*>::iterator _itResMap;
		std::vector<PVRTBOUNDINGBOX> _meshBoundingBoxes;
		std::vector<cml::Vector3> _meshBoundingBoxCenters;
		float _currFrame;
		std::map<int, AnimChannel*>* _pChannelAlpha;
		std::map<int, AnimChannel*>* _pChannelVisibility;
		std::map<int, AnimChannel*>* _pChannelOffsetU;
		std::map<int, AnimChannel*>* _pChannelOffsetV;

		static std::map<std::string, PODModelRes*> _sResMap;
		
	public:
		static void cleanup();
	};

} //namespace lw


#endif //__LW_POD_MODEL_RES_H__