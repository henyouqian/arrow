#include "stdafx.h"
#include <limits>
#include "lwPODModelRes.h"
#include "lwPODTextureRes.h"
#include "lwFileSys.h"
#include "lwCamera.h"
#include "lwApp.h"
#include "lwGLExtension.h"
#include "lwFileSys.h"
#include "lwPODModelExt.h"


namespace lw{


	std::map<std::string, PODModelRes*> PODModelRes::_sResMap;

	PODModelRes::PODModelRes(const char* filename)
	:_isValid(true), _vbos(NULL), _indexVbos(NULL)
	,_nodeHierarchyIndices(NULL), _nodeHierarchy(NULL)
	,_currFrame((std::numeric_limits<float>::max)())
	,_pChannelAlpha(NULL), _pChannelVisibility(NULL), _pChannelOffsetU(NULL), _pChannelOffsetV(NULL){
		lwassert(filename);
		if(_pod.ReadFromFile(_f(filename)) != PVR_SUCCESS)
		{
			lwerror("Failed to load scene. file=" << filename);
			_isValid = false;
			return;
		}
		
		//create vbos
		if (_pod.nNumMesh > 0){
			_vbos = new GLuint[_pod.nNumMesh];
			_indexVbos = new GLuint[_pod.nNumMesh];

			glGenBuffers(_pod.nNumMesh, _vbos);

			for(unsigned int i = 0; i < _pod.nNumMesh; ++i)
			{
				SPODMesh& Mesh = _pod.pMesh[i];
				_indexVbos[i] = 0;

				unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
				if ( Mesh.pInterleaved ){
					glBindBuffer(GL_ARRAY_BUFFER, _vbos[i]);
					glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);
				}
				if(Mesh.sFaces.pData)
				{
					glGenBuffers(1, &(_indexVbos[i]));
					uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexVbos[i]);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
				}
			}
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		//textures
		PODTextureRes* pTexRes = NULL;
		for(int i = 0; i < (int) _pod.nNumTexture; ++i){
			const char* textureName = _pod.pTexture[i].pszName;
			if ( textureName == NULL ){
				lwerror("textureName == NULL");
				continue;
			}
			const char* p = strrchr(textureName, '.');
			std::string str = textureName;
			if ( p ){
				str.resize(p-textureName);
			}
			str.append(".pvr");
			pTexRes = PODTextureRes::create(str.c_str());
			_textureReses.push_back(pTexRes);
		}

		//hierarchy
		std::vector<std::vector<unsigned char> >* pChildrenVec = new std::vector<std::vector<unsigned char> >;
		lwassert(pChildrenVec);

		for ( unsigned int i = 0; i < _pod.nNumNode; ++i ){
			std::vector<unsigned char> v;
			pChildrenVec->push_back(v);
		}
		int numChildren = 0;
		for ( unsigned int i = 0; i < _pod.nNumNode; ++i ){
			if ( _pod.pNode[i].nIdxParent != -1 ){
				(*pChildrenVec)[_pod.pNode[i].nIdxParent].push_back(i);
				++numChildren;
			}
		}

		if ( _pod.nNumNode > 0 ){
			_nodeHierarchyIndices = new unsigned char*[_pod.nNumNode];
			lwassert(_nodeHierarchyIndices);
		}

		//_nodeHierarchy = (nNumChildren + nNumChildren * pChildNodeIndex) * nNumNode
		if ( _pod.nNumNode+numChildren > 0 ){
			_nodeHierarchy = new unsigned char[_pod.nNumNode+numChildren];
			lwassert(_nodeHierarchy);
			unsigned char* p = _nodeHierarchy;
			unsigned char numChildren = 0;
			for ( size_t i = 0; i < pChildrenVec->size(); ++i ){
				_nodeHierarchyIndices[i] = p;
				numChildren = (unsigned char)(*pChildrenVec)[i].size();
				*p = numChildren;
				++p;
				for ( int j = 0; j < numChildren; ++j ){
					*p = (unsigned char)((*pChildrenVec)[i][j]);
					++p;
				}
			}
		}
		delete pChildrenVec;

		//bounding boxes
		PVRTBOUNDINGBOX bbox;
		cml::Vector3 vCenter;
		for ( unsigned int i = 0; i < _pod.nNumMesh; ++i ){
			SPODMesh& mesh = _pod.pMesh[i];
			lwassert(mesh.pInterleaved == NULL);
			PVRTBoundingBoxCompute(&bbox, (PVRTVECTOR3*)(mesh.sVertex.pData), mesh.nNumVertex);
			_meshBoundingBoxes.push_back(bbox);
			//center
			vCenter.zero();
			for ( int j = 0; j < 8; ++j ){
				vCenter[0] += bbox.Point[j].x;
				vCenter[1] += bbox.Point[j].y;
				vCenter[2] += bbox.Point[j].z;
			}
			vCenter /= 8.f;
			_meshBoundingBoxCenters.push_back(vCenter);
		}

		//.ext file
		std::string extFileName = filename;
		extFileName += ".ext";
		_f extFilePath(extFileName.c_str());
		if ( extFilePath.isValid() ){
			FILE* pf = fopen(_f(extFileName.c_str()), "rb");
			if ( pf ){
				int frameCount = 0;
				fread(&frameCount, sizeof(frameCount), 1, pf);
				int strLen = 0;
				char buf[128];
				//double v = 0;
				int nodeIdx = 0;
				int channelType = 0;
				const int LW_ALPHA_CHANNEL = 0;
				const int LW_VISIBILITY_CHANNEL = 1;
				const int LW_OFFSET_U_CHANNEL = 2;
				const int LW_OFFSET_V_CHANNEL = 3;
				//for each node
				while ( 1 ){
					fread(&strLen, sizeof(strLen), 1, pf);
					if (feof(pf) != 0){
						break;
					}
					fread(buf, sizeof(char), strLen+1, pf);
					nodeIdx = getNodeIndex(buf);
					lwassert(nodeIdx != -1);
					//for each channel
					while (1){
						fread(&channelType, sizeof(channelType), 1, pf);
						if ( channelType == -1 ){
							break;
						}
						AnimChannel* pChannel = AnimChannel::create(pf, frameCount);
						if ( pChannel ){
							switch (channelType)
							{
							case LW_ALPHA_CHANNEL:
								if ( _pChannelAlpha == NULL ){
									_pChannelAlpha = new std::map<int, AnimChannel*>;
									lwassert(_pChannelAlpha);
								}
								(*_pChannelAlpha)[nodeIdx] = pChannel;
								break;
							case LW_VISIBILITY_CHANNEL:
								if ( _pChannelVisibility == NULL ){
									_pChannelVisibility = new std::map<int, AnimChannel*>;
									lwassert(_pChannelVisibility);
								}
								(*_pChannelVisibility)[nodeIdx] = pChannel;
								break;
							case LW_OFFSET_U_CHANNEL:
								if ( _pChannelOffsetU == NULL ){
									_pChannelOffsetU = new std::map<int, AnimChannel*>;
									lwassert(_pChannelOffsetU);
								}
								(*_pChannelOffsetU)[nodeIdx] = pChannel;
								break;
							case LW_OFFSET_V_CHANNEL:
								if ( _pChannelOffsetV == NULL ){
									_pChannelOffsetV = new std::map<int, AnimChannel*>;
									lwassert(_pChannelOffsetV);
								}
								(*_pChannelOffsetV)[nodeIdx] = pChannel;
								break;
							}
						}
					}
				}
				fclose(pf);
			}
		}
	}

	PODModelRes::~PODModelRes(){
		if ( _itResMap != _sResMap.end() ){
			_sResMap.erase(_itResMap);
		}
		_pod.Destroy();
		if ( _vbos )
			delete[] _vbos;
		if ( _indexVbos )
			delete[] _indexVbos;
		std::vector<PODTextureRes*>::iterator it = _textureReses.begin();
		std::vector<PODTextureRes*>::iterator itEnd = _textureReses.end();
		for ( ; it != itEnd; ++it ){
			if (*it) (*it)->release();
		}
		if ( _nodeHierarchy ){
			delete [] _nodeHierarchy;
		}
		if ( _nodeHierarchyIndices ){
			delete [] _nodeHierarchyIndices;
		}
		if ( _pChannelAlpha ){
			std::map<int, AnimChannel*>::iterator it = _pChannelAlpha->begin();
			std::map<int, AnimChannel*>::iterator itEnd = _pChannelAlpha->end();
			for ( ; it != itEnd; ++it ){
				delete it->second;
			}
			delete _pChannelAlpha;
		}
		if ( _pChannelVisibility ){
			std::map<int, AnimChannel*>::iterator it = _pChannelVisibility->begin();
			std::map<int, AnimChannel*>::iterator itEnd = _pChannelVisibility->end();
			for ( ; it != itEnd; ++it ){
				delete it->second;
			}
			delete _pChannelVisibility;
		}
		if ( _pChannelOffsetU ){
			std::map<int, AnimChannel*>::iterator it = _pChannelOffsetU->begin();
			std::map<int, AnimChannel*>::iterator itEnd = _pChannelOffsetU->end();
			for ( ; it != itEnd; ++it ){
				delete it->second;
			}
			delete _pChannelOffsetU;
		}
		if ( _pChannelOffsetV ){
			std::map<int, AnimChannel*>::iterator it = _pChannelOffsetV->begin();
			std::map<int, AnimChannel*>::iterator itEnd = _pChannelOffsetV->end();
			for ( ; it != itEnd; ++it ){
				delete it->second;
			}
			delete _pChannelOffsetV;
		}
	}

	PODModelRes* PODModelRes::create(const char* filename){
		lwassert(filename);
		std::map<std::string, PODModelRes*>::iterator it = _sResMap.find(filename);
		if ( it == _sResMap.end() ){
			PODModelRes* p = new PODModelRes(filename);
			p->_itResMap = _sResMap.end();
			if ( p && !p->isValid() ){
				delete p;
				return NULL;
			}else{
				_sResMap[filename] = p;
				p->_itResMap = _sResMap.find(filename);

				return p;
			}
		}else{
			it->second->addRef();
			return it->second;
		}
	}

	const cml::Vector3& PODModelRes::getMeshBoundingBoxCenter(int nodeIndex){
		SPODNode* pNode = &_pod.pNode[nodeIndex];
		return _meshBoundingBoxCenters[pNode->nIdx];
	}

	void PODModelRes::draw(int iNode, const cml::Matrix44& worldViewMatrix, const Color& color, const std::map<int, PODTextureRes*>& replaceTexMap){
		SPODNode* pNode = &_pod.pNode[iNode];
		int meshIdx = pNode->nIdx;
		SPODMesh* pMesh = &_pod.pMesh[meshIdx];

		//material
		bool bUseTexture = false;
		if ( _pChannelVisibility ){
			std::map<int, AnimChannel*>::iterator it = _pChannelVisibility->find(iNode);
			if ( it != _pChannelVisibility->end() ){
				if ( it->second->value(_currFrame) < 0.5f ){
					return;
				}
			}
		}
		if ( _pChannelAlpha ){
			std::map<int, AnimChannel*>::iterator it = _pChannelAlpha->find(iNode);
			if ( it != _pChannelAlpha->end() ){
				float alpha = it->second->value(_currFrame);
				if ( alpha < 0.003f ){
					return;
				}
				glColor4f(color.r/255.f, color.g/255.f, color.b/255.f, color.a*alpha/255.f);
			}
		}else{
			glColor4f(color.r/255.f, color.g/255.f, color.b/255.f, color.a/255.f);
		}

		float offsetU = 0;
		float offsetV = 0;
		if ( _pChannelOffsetU ){
			std::map<int, AnimChannel*>::iterator it = _pChannelOffsetU->find(iNode);
			if ( it != _pChannelOffsetU->end() ){
				offsetU = it->second->value(_currFrame);
			}
		}
		if ( _pChannelOffsetV ){
			std::map<int, AnimChannel*>::iterator it = _pChannelOffsetV->find(iNode);
			if ( it != _pChannelOffsetV->end() ){
				offsetV = it->second->value(_currFrame);
			}
		}
		if ( offsetU != 0 || offsetV != 0 ){
			glMatrixMode(GL_TEXTURE);
			cml::Matrix44 m;
			cml::matrix_translation(m, offsetU, offsetV, 0.0f);
			glLoadMatrixf(m.data());
			glMatrixMode(GL_MODELVIEW);
		}

		if ( pNode->nIdxMaterial != -1 ){
			SPODMaterial* pMaterial = &_pod.pMaterial[pNode->nIdxMaterial];
			int textureIndex = pMaterial->nIdxTexDiffuse;
			std::map<int, PODTextureRes*>::const_iterator it = replaceTexMap.find(textureIndex);
			if ( it != replaceTexMap.end() ){
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, it->second->getGlId());
				bUseTexture = true;
			}
			else if ( textureIndex >= 0 && textureIndex < (int)_textureReses.size() && _textureReses[textureIndex] != NULL ){
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, _textureReses[textureIndex]->getGlId());
				bUseTexture = true;
			}
			
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, PVRTVec4(pMaterial->pfMatAmbient,  f2vt(1.0f)).ptr());
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, PVRTVec4(pMaterial->pfMatDiffuse,  f2vt(1.0f)).ptr());
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, PVRTVec4(pMaterial->pfMatSpecular,  f2vt(1.0f)).ptr());

			//glColor4f(1.f, 1.f, 1.f, pMaterial->fMatOpacity);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, pMaterial->fMatOpacity);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pMaterial->fMatShininess);
		}else{
			GLfloat black[4] = {0,0,0,0};
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, pMaterial->fMatOpacity);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0);
		}

		//vertex
		if ( pMesh->pInterleaved ){
			glBindBuffer(GL_ARRAY_BUFFER, _vbos[meshIdx]);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexVbos[meshIdx]);

		glVertexPointer(pMesh->sVertex.n, VERTTYPEENUM, pMesh->sVertex.nStride, pMesh->sVertex.pData);

		if(pMesh->nNumUVW){
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(pMesh->psUVW[0].n, VERTTYPEENUM, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);
		}
		if(pMesh->sNormals.n){
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(VERTTYPEENUM, pMesh->sNormals.nStride, pMesh->sNormals.pData);
		}
		if(pMesh->sVtxColours.n){
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(pMesh->sVtxColours.n * PVRTModelPODDataTypeComponentCount(pMesh->sVtxColours.eType), GL_UNSIGNED_BYTE, pMesh->sVtxColours.nStride, pMesh->sVtxColours.pData);
		}

		bool bSkinning = pMesh->sBoneWeight.pData != 0;
		//skinning
		if(bSkinning)
		{
			glEnableClientState(GL_MATRIX_INDEX_ARRAY_OES);
			glEnableClientState(GL_WEIGHT_ARRAY_OES);

#ifdef WIN32
			GLExtension::get().glMatrixIndexPointerOES(pMesh->sBoneIdx.n, GL_UNSIGNED_BYTE, pMesh->sBoneIdx.nStride, pMesh->sBoneIdx.pData);
			GLExtension::get().glWeightPointerOES(pMesh->sBoneWeight.n, VERTTYPEENUM, pMesh->sBoneWeight.nStride, pMesh->sBoneWeight.pData);
#else
			glMatrixIndexPointerOES(pMesh->sBoneIdx.n, GL_UNSIGNED_BYTE, pMesh->sBoneIdx.nStride, pMesh->sBoneIdx.pData);
			glWeightPointerOES(pMesh->sBoneWeight.n, VERTTYPEENUM, pMesh->sBoneWeight.nStride, pMesh->sBoneWeight.pData);
#endif

			glEnable(GL_MATRIX_PALETTE_OES);
			glMatrixMode(GL_MATRIX_PALETTE_OES);

			PVRTMat4	mBoneWorld;
			int			i32NodeID;

			for(int i32Batch = 0; i32Batch < pMesh->sBoneBatches.nBatchCnt; ++i32Batch){
				for(int j = 0; j < pMesh->sBoneBatches.pnBatchBoneCnt[i32Batch]; ++j){
#ifdef WIN32
					GLExtension::get().glCurrentPaletteMatrixOES(j);
#else
					glCurrentPaletteMatrixOES(j);
#endif
					i32NodeID = pMesh->sBoneBatches.pnBatches[i32Batch * pMesh->sBoneBatches.nBatchBoneMax + j];
					_pod.GetBoneWorldMatrix(mBoneWorld, *pNode, _pod.pNode[i32NodeID]);

					PVRTMat4 mWorldView(worldViewMatrix.data());
					mBoneWorld = mWorldView * mBoneWorld;
					
					glLoadMatrixf(mBoneWorld.f);
				}

				int i32Tris;
				if(i32Batch + 1 < pMesh->sBoneBatches.nBatchCnt)
					i32Tris = pMesh->sBoneBatches.pnBatchOffset[i32Batch+1] - pMesh->sBoneBatches.pnBatchOffset[i32Batch];
				else
					i32Tris = pMesh->nNumFaces - pMesh->sBoneBatches.pnBatchOffset[i32Batch];

				glDrawElements(GL_TRIANGLES, i32Tris * 3, GL_UNSIGNED_SHORT, &((unsigned short*)0)[3 * pMesh->sBoneBatches.pnBatchOffset[i32Batch]]);
			}
			glMatrixMode(GL_MODELVIEW);
			glDisable(GL_MATRIX_PALETTE_OES);
			glDisableClientState(GL_MATRIX_INDEX_ARRAY_OES);
			glDisableClientState(GL_WEIGHT_ARRAY_OES);

		}else{
			if(pMesh->nNumStrips == 0){
				if(_indexVbos[meshIdx]){
					glDrawElements(GL_TRIANGLES, pMesh->nNumFaces * 3, GL_UNSIGNED_SHORT, 0);
				}else{
					glDrawArrays(GL_TRIANGLES, 0, pMesh->nNumFaces * 3);
				}
			}else{
				int offset = 0;
				for(int i = 0; i < (int) pMesh->nNumStrips; ++i){
					if(_indexVbos[meshIdx]){
						glDrawElements(GL_TRIANGLE_STRIP, pMesh->pnStripLength[i]+2, GL_UNSIGNED_SHORT, &((GLshort*)0)[offset]);
					}else{
						glDrawArrays(GL_TRIANGLE_STRIP, offset, pMesh->pnStripLength[i]+2);
					}
					offset += pMesh->pnStripLength[i]+2;
				}
			}
		}
		if ( bUseTexture ){
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		if(pMesh->nNumUVW){
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		if(pMesh->sNormals.n){
			glDisableClientState(GL_NORMAL_ARRAY);
		}
		if(pMesh->sVtxColours.n){
			glDisableClientState(GL_COLOR_ARRAY);
		}
		if ( pMesh->pInterleaved ){
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		if ( offsetU != 0 || offsetV != 0 ){
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void PODModelRes::setFrame(float frame){
		if ( frame == _currFrame ){
			return;
		}
		float frameMax = (float)(_pod.nNumFrame-1);
		if ( frame > frameMax ){
			float n = floor(frame/frameMax);
			frame -= n*frameMax;
		}else if ( frame < 0 ){
			float n = floor((-frame)/frameMax) + 1;
			frame += n*frameMax;
		}
		_pod.SetFrame(frame);
		_currFrame = frame;
	}

	AlphaMode PODModelRes::getAlphaMode(int nodeIndex){
		SPODNode* pNode = &_pod.pNode[nodeIndex];
		const char* pName = pNode->pszName;
		if ( pName[0] == '_' ){
			if ( pName[1] == 'A' && pName[2] == 'T' ){
				return ALPHA_TEST;
			}
			if ( pName[1] == 'A' && pName[2] == 'B' ){
				return ALPHA_BLEND;
			}
			if ( pName[1] == 'A' && pName[2] == 'N' ){
				return ALPHA_NONE;
			}
			if ( pName[1] == 'A' && pName[2] == 'A' ){
				return ALPHA_ADD;
			}
		}
		
		if ( pNode->nIdxMaterial == -1 ){
			return ALPHA_NONE;
		}
		SPODMaterial* pMaterial = &_pod.pMaterial[pNode->nIdxMaterial];

		if ( pMaterial->nIdxTexDiffuse == -1 ){
			return ALPHA_NONE;
		}
		PODTextureRes* pTextureRes = _textureReses[pMaterial->nIdxTexDiffuse];
		if ( pTextureRes && pTextureRes->getHeader()->dwAlphaBitMask != 0 ){
			return ALPHA_BLEND;
		}
		if ( _pChannelAlpha ){
			std::map<int, AnimChannel*>::iterator it = _pChannelAlpha->find(nodeIndex);
			if ( it != _pChannelAlpha->end() ){
				return ALPHA_BLEND;
			}
		}
		return ALPHA_NONE;
	}

	void PODModelRes::setCamera(Camera& cam, int camIndex){
		SPODCamera* p = _pod.pCamera + camIndex;
		float aspect = App::getAspect();
		cam.perspective(p->fFOV*180.f/(float)M_PI, aspect, p->fNear, p->fFar);

		PVRTVec3 from, to, up;
		_pod.GetCamera(from, to, up, camIndex);
		cam.lookat(from.x, from.y, from.z, to.x, to.y, to.z, up.x, up.y, up.z);
	}

	int PODModelRes::getNodeIndex(const char* nodeName){
		lwassert(nodeName);
		for ( unsigned int i = 0; i < _pod.nNumNode; ++i ){
			if ( strcmp(nodeName, _pod.pNode[i].pszName) == 0 ){
				return i;
			}
		}
		return -1;
	}

	int PODModelRes::getCameraIndex(const char* nodeName){
		lwassert(nodeName);
		unsigned baseIdx = _pod.nNumMeshNode+_pod.nNumLight;
		for ( unsigned int i = 0; i < _pod.nNumCamera; ++i ){
			if ( strcmp(nodeName, _pod.pNode[baseIdx+i].pszName) == 0 ){
				return i;
			}
		}
		return -1;
	}

	void PODModelRes::cleanup(){
		std::map<std::string, PODModelRes*>::iterator it = _sResMap.begin();
		std::map<std::string, PODModelRes*>::iterator itEnd = _sResMap.end();
		while( !_sResMap.empty() ){
			lwwarning("model resource leak: file=" << _sResMap.begin()->first.c_str());
			delete (_sResMap.begin()->second);
		}
	}

} //namespace lw