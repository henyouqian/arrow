#include "stdafx.h"
#include "lwPODModel.h"
#include "lwPODModelRes.h"
#include "lwPODModelCollector.h"
#include "lwCamera.h"
#include "lwSprite.h"
#include "lwPODTextureRes.h"

namespace lw{

	std::list<PODModel*> PODModel::_sRootChildren;

	PODModel::PODModel(const char* filename, const char* nodeName):_frame(0), _nodeIndex(-1)
	,_pParent(NULL), _parentNodeIndex(-1), _pChildren(NULL){
		_pRes = PODModelRes::create(filename);
		if ( _pRes ){
			_localMatrix.identity();
			_worldMatrix.identity();
		}
		if ( nodeName != NULL ){
			_nodeIndex = _pRes->getNodeIndex(nodeName);
		}
		_color = COLOR_WHITE;
	}

	PODModel::~PODModel(){
		if (_pRes){
			_pRes->release();
		}
		if (_pChildren){
			std::list<PODModel*>::iterator it = _pChildren->begin();
			std::list<PODModel*>::iterator itEnd = _pChildren->end();
			for ( ; it != itEnd; ++it ){
				_sRootChildren.push_back(*it);
				(*it)->_pParent = NULL;
			}
			delete _pChildren;
		}
		std::list<PODModel*>* pParentChildren = NULL;
		if ( _pParent ){
			pParentChildren = _pParent->_pChildren;
		}else{
			pParentChildren = &_sRootChildren;
		}
		lwassert(pParentChildren);
		std::list<PODModel*>::iterator it = pParentChildren->begin();
		std::list<PODModel*>::iterator itEnd = pParentChildren->end();
		for ( ; it != itEnd; ++it ){
			if ( (*it) == this ){
				pParentChildren->erase(it);
				break;
			}
		}
		{
			std::map<int, PODTextureRes*>::iterator it = _replaceTextureMap.begin();
			std::map<int, PODTextureRes*>::iterator itEnd = _replaceTextureMap.end();
			for ( ; it != itEnd; ++it ){
				it->second->release();
			}
		}
	}

	PODModel* PODModel::create(const char* filename, const char* nodeName){
		PODModel* p = new PODModel(filename, nodeName);
		if ( p->_pRes == NULL ){
			delete p;
			return NULL;
		}
		_sRootChildren.push_back(p);
		return p;
	}

// 	void PODModel::collect(){
// 		for ( unsigned int i = 0; i < _pRes->_pod.nNumMeshNode; ++i ){
// 			PODModelCollector::collect(this, i, _pRes->hasAlpha(i));
// 		}
// 	}

	void PODModel::collect(){
		if ( _color.a < 1 ){
			return;
		}
		if ( _nodeIndex == (unsigned char)-1 ){
			AlphaMode am;
			for ( unsigned int i = 0; i < _pRes->_pod.nNumMeshNode; ++i ){
				if ( _color.a < 255 ){
					am = ALPHA_BLEND;
				}else{
					am = _pRes->getAlphaMode(i);
				}
				PODModelCollector::collect(this, i, am);
			}
		}else{
			collect(_nodeIndex);
		}
	}
	void PODModel::collect(unsigned char nodeIndex){
		unsigned char* p = _pRes->_nodeHierarchyIndices[nodeIndex];
		unsigned char sz = *p; 
		for ( unsigned char i = 0; i < sz; ++i ){
			++p;
			collect(*p);
		}
		PODModelCollector::collect(this, nodeIndex, _pRes->getAlphaMode(nodeIndex));
	}

	namespace{
		bool less(const PODModel::SortElem& _1, const PODModel::SortElem& _2){
			return _1.z < _2.z;
		}
	}

	void PODModel::collectAsSprite(bool sortZ){
		if ( _nodeIndex == (unsigned char)-1 ){
			if ( _spriteSortVec.empty() || sortZ ){
				_spriteSortVec.clear();
				cml::Matrix44 mView;
				cml::matrix_look_at_RH(mView, 0.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f, 0.f);
				for ( unsigned int i = 0; i < _pRes->_pod.nNumMeshNode; ++i ){
					cml::Matrix44 m;
					calcNodeWorldViewMatrix(m, i, mView);
					cml::Vector3 v1 = _pRes->getMeshBoundingBoxCenter(i);
					v1 = cml::transform_point(m, v1);
					SortElem se = {i, v1[2]};
					_spriteSortVec.push_back(se);
				}
				std::sort(_spriteSortVec.begin(), _spriteSortVec.end(), less);
			}
			
			for ( size_t i = 0; i < _spriteSortVec.size(); ++i ){
				Sprite::collectModel(this, _spriteSortVec[i].nodeIndex);
			}
		}else{
			collectAsSprite(_nodeIndex);
		}
	}

	void PODModel::collectAsSprite(unsigned char nodeIndex){
		lwassert(0 || "not implemented");
	}

	void PODModel::setParent(PODModel* pParent, unsigned char nodeIndex){
		_parentNodeIndex = nodeIndex;
		if ( pParent != _pParent ){
			if ( _pParent == NULL ){
				std::list<PODModel*>::iterator it = _sRootChildren.begin();
				std::list<PODModel*>::iterator itEnd = _sRootChildren.end();
				for ( ; it != itEnd; ++it ){
					if ( *it == this ){
						_sRootChildren.erase(it);
						break;
					}
				}
			}else{
				std::list<PODModel*>::iterator it = _pParent->_pChildren->begin();
				std::list<PODModel*>::iterator itEnd = _pParent->_pChildren->end();
				for ( ; it != itEnd; ++it ){
					if ( *it == this ){
						_pParent->_pChildren->erase(it);
						break;
					}
				}
			}
			if ( pParent == NULL ){
				_sRootChildren.push_back(this);
			}else{
				if ( pParent->_pChildren == NULL ){
					pParent->_pChildren = new std::list<PODModel*>;
					lwassert(pParent->_pChildren);
				}
				pParent->_pChildren->push_back(this);
			}
			_pParent = pParent;
		}		
	}
	void PODModel::unParent(){
		setParent(NULL, (unsigned char)-1);
	}

	void PODModel::calcNodeWorldViewMatrix(cml::Matrix44& m, int nodeIndex, const cml::Matrix44& mView){
		//Camera* pCam = Camera::getCurrent();

		_pRes->setFrame(_frame);

		CPVRTModelPOD& pod = _pRes->getPOD();
		SPODNode* pNode = &pod.pNode[nodeIndex];
		int meshIdx = pNode->nIdx;
		SPODMesh* pMesh = &pod.pMesh[meshIdx];
		if ( pMesh->sBoneWeight.pData != NULL ){
			m = mView * _worldMatrix;
		}else{
			PVRTMat4 pvrWorld;
			pod.GetWorldMatrix(pvrWorld, *pNode);

			memcpy(m.data(), pvrWorld.f, sizeof(pvrWorld.f));
			m = mView * _worldMatrix * m;
		}		
	}

	void PODModel::main(){
		std::list<PODModel*>::iterator it = _sRootChildren.begin();
		std::list<PODModel*>::iterator itEnd = _sRootChildren.end();
		for ( ; it != itEnd; ++it ){
			(*it)->updateMatrix();
		}
	}

	void PODModel::drawAsSprite(int nodeIndex){
		cml::Matrix44 mView;
		cml::matrix_look_at_RH(mView, 0.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f, 0.f);

		cml::Matrix44 m;
		calcNodeWorldViewMatrix(m, nodeIndex, mView);
		glLoadMatrixf(m.data());

		switch (_pRes->getAlphaMode(nodeIndex))
		{
		case ALPHA_NONE:
			glDisable(GL_BLEND);
			break;
		case ALPHA_ADD:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		default:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
		_pRes->draw(nodeIndex, mView * _worldMatrix, _color, _replaceTextureMap);

		glLoadIdentity();	
	}

	void PODModel::drawNode(int nodeIndex, cml::Matrix44& m){
		//cml::Matrix44 wv;
		//wv = Camera::getCurrent()->getView() * _worldMatrix;
		//_pRes->draw(nodeIndex, wv, _color, _replaceTextureMap);

		_pRes->draw(nodeIndex, m, _color, _replaceTextureMap);
	}

	void PODModel::updateMatrix(){
		if ( _pParent == NULL ){
			_worldMatrix = _localMatrix;
		}else{
			if ( _parentNodeIndex == (unsigned char)-1 ){
				_worldMatrix = _pParent->_worldMatrix * _localMatrix;
			}else{
				PVRTMat4 matPOD;
				const SPODNode& node = _pParent->_pRes->_pod.pNode[_parentNodeIndex];
				_pParent->_pRes->setFrame(_pParent->_frame);
				_pParent->_pRes->_pod.GetWorldMatrix(matPOD, node);
				cml::Matrix44 m;
				memcpy(m.data(), matPOD.f, sizeof(matPOD.f));
				_worldMatrix = _pParent->_worldMatrix * m;
				_worldMatrix *= _localMatrix;
			}
		}
		PVRTMat4 matPOD;
		if ( _nodeIndex != (unsigned char)-1 ){
			const SPODNode& node = _pRes->_pod.pNode[_nodeIndex];
			_pRes->setFrame(_frame);
			_pRes->_pod.GetWorldMatrix(matPOD, node);
			cml::Matrix44 m;
			memcpy(m.data(), matPOD.f, sizeof(matPOD.f));
			_worldMatrix *= m;
		}
		
		if ( _pChildren ){
			std::list<PODModel*>::iterator it = _pChildren->begin();
			std::list<PODModel*>::iterator itEnd = _pChildren->end();
			for ( ; it != itEnd; ++it ){
				(*it)->updateMatrix();
			}
		}
	}

	bool PODModel::replaceTexture(int textureIndex, const char* textureName){
		lwassert(textureIndex >= 0 && textureIndex < (int)_pRes->getPOD().nNumTexture);
		std::map<int, PODTextureRes*>::iterator it = _replaceTextureMap.find(textureIndex);
		if ( it != _replaceTextureMap.end() ){
			if ( strcmp(it->second->getFileName(), textureName) == 0 ){
				return true;
			}
		}
		PODTextureRes* pTextrueRes = PODTextureRes::create(textureName);
		if ( pTextrueRes == NULL ){
			lwerror("PODTextureRes::create failed: textureName=" << textureName);
			return false;
		}

		if ( it != _replaceTextureMap.end() ){
			it->second->release();
			_replaceTextureMap.erase(it);
		}
		
		_replaceTextureMap[textureIndex] = pTextrueRes;
		return true;
	}

	int PODModel::getTextureIndex(const char* name){
		lwassert(name);
		CPVRTModelPOD& pod = _pRes->getPOD();
		for ( int i = 0; i < (int)pod.nNumTexture; ++i ){
			if ( strcmp(pod.pTexture[i].pszName, name) == 0 ){
				return i;
			}
		}
		return -1;
	}

	void PODModel::draw(const lw::Color& fillColor){
		cml::Matrix44 wv;
		for ( unsigned int i = 0; i < _pRes->_pod.nNumMeshNode; ++i ){
			calcNodeWorldViewMatrix(wv, i, Camera::getCurrent()->getView());
			glLoadMatrixf(wv.data());
			_pRes->draw(i, wv, fillColor, _replaceTextureMap);
		}
	}

} //namespace lw