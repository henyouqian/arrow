#include "stdafx.h"
#include "lwPODModelCollector.h"
#include "lwPODModel.h"
#include "lwPODModelRes.h"
#include "lwCamera.h"

namespace lw{

	namespace {
		struct CollectorElem{
			CollectorElem(PODModel* pM, int idx, cml::Matrix44& mWV, Camera* pC):pModel(pM), nodeIdx(idx), worldViewMatrix(mWV), pCam(pC){};
			PODModel* pModel;
			int nodeIdx;
			cml::Matrix44 worldViewMatrix;
			Camera* pCam;
		};
		struct PODModelCollectorData{
			PODModelCollectorData():_currentCamera(NULL){};
			std::vector<CollectorElem> _opaque;
			std::vector<CollectorElem> _alphaBlend;
			std::vector<CollectorElem> _alphaTest;
			Camera* _currentCamera;
		};
		PODModelCollectorData g_PODModelCollectorData;

		bool less(const CollectorElem& _1, const CollectorElem& _2){
			cml::Vector3 v1 = _1.pModel->getRes()->getMeshBoundingBoxCenter(_1.nodeIdx);
			v1 = cml::transform_point(_1.worldViewMatrix, v1);
			cml::Vector3 v2 = _2.pModel->getRes()->getMeshBoundingBoxCenter(_2.nodeIdx);
			v2 = cml::transform_point(_2.worldViewMatrix, v2);
			return v1[2] < v2[2];
		}
		bool greater(const CollectorElem& _1, const CollectorElem& _2){
			cml::Vector3 v1 = _1.pModel->getRes()->getMeshBoundingBoxCenter(_1.nodeIdx);
			cml::transform_point(_1.worldViewMatrix, v1);
			cml::Vector3 v2 = _2.pModel->getRes()->getMeshBoundingBoxCenter(_2.nodeIdx);
			cml::transform_point(_2.worldViewMatrix, v2);
			return v1[2] > v2[2];
		}
	}

	void PODModelCollector::collect(PODModel* pModel, int nodeIndex, AlphaMode alphaMode){
		PODModelCollectorData* pd = &g_PODModelCollectorData;
		
		cml::Matrix44 m;
		pModel->calcNodeWorldViewMatrix(m, nodeIndex, Camera::getCurrent()->getView());
		
		Camera* pCam = Camera::getCurrent();
		CollectorElem elem(pModel, nodeIndex, m, pCam);
		switch( alphaMode )
		{
		case ALPHA_BLEND:
			pd->_alphaBlend.push_back(elem);
			break;
		case ALPHA_TEST:
			pd->_alphaTest.push_back(elem);
		    break;
		default:
			pd->_opaque.push_back(elem);
		    break;
		}
	}

	void PODModelCollector::draw(){
		PODModelCollectorData* pd = &g_PODModelCollectorData;
		glEnableClientState(GL_VERTEX_ARRAY);

		//opaque
		std::vector<CollectorElem>::iterator it = pd->_opaque.begin();
		std::vector<CollectorElem>::iterator itEnd = pd->_opaque.end();
		std::sort(it, itEnd, greater);
		it = pd->_opaque.begin();
		itEnd = pd->_opaque.end();
		for ( it; it != itEnd; ++it ){
			if ( it->pCam != pd->_currentCamera ){
				pd->_currentCamera = it->pCam;
				glMatrixMode(GL_PROJECTION);
				glLoadMatrixf(it->pCam->getProj().data());
				glMatrixMode(GL_MODELVIEW);
				it->pCam->setCurrent();
			}
			glLoadMatrixf(it->worldViewMatrix.data());
			it->pModel->drawNode(it->nodeIdx, it->worldViewMatrix);
		}

		//alpha test
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER,0.9f);
		it = pd->_alphaTest.begin();
		itEnd = pd->_alphaTest.end();
		std::sort(it, itEnd, greater);
		it = pd->_alphaTest.begin();
		itEnd = pd->_alphaTest.end();
		for ( it; it != itEnd; ++it ){
			if ( it->pCam != pd->_currentCamera ){
				pd->_currentCamera = it->pCam;
				glMatrixMode(GL_PROJECTION);
				glLoadMatrixf(it->pCam->getProj().data());
				glMatrixMode(GL_MODELVIEW);
				it->pCam->setCurrent();
			}
			glLoadMatrixf(it->worldViewMatrix.data());
			it->pModel->drawNode(it->nodeIdx, it->worldViewMatrix);
		}
		glDisable(GL_ALPHA_TEST);

		//alpha blend
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
		glLightModelx(GL_LIGHT_MODEL_TWO_SIDE, 1);
		it = pd->_alphaBlend.begin();
		itEnd = pd->_alphaBlend.end();
		std::sort(it, itEnd, less);
		it = pd->_alphaBlend.begin();
		itEnd = pd->_alphaBlend.end();
		for ( it; it != itEnd; ++it ){
			if ( it->pCam != pd->_currentCamera ){
				pd->_currentCamera = it->pCam;
				glMatrixMode(GL_PROJECTION);
				glLoadMatrixf(it->pCam->getProj().data());
				glMatrixMode(GL_MODELVIEW);
				it->pCam->setCurrent();
			}
			glLoadMatrixf(it->worldViewMatrix.data());
			it->pModel->drawNode(it->nodeIdx, it->worldViewMatrix);
		}
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glLightModelx(GL_LIGHT_MODEL_TWO_SIDE, 0);

		pd->_opaque.clear();
		pd->_alphaTest.clear();
		pd->_alphaBlend.clear();

		glDisableClientState(GL_VERTEX_ARRAY);
		pd->_currentCamera = NULL;
	}
} //namespace lw