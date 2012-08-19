#include "stdafx.h"
#include "lwSound.h"
#include "lwFileSys.h"

#ifdef __APPLE__
#include <pthread.h>
#endif

namespace lw{

	Sound::Sound(SoundBuffer* pBuffer):_pBuffer(pBuffer), _pLoopBuffer(NULL), _source(0), _isLoop(false), _volume(1.f){
	}

	Sound::Sound(SoundBuffer* pAttackBuffer, SoundBuffer* pLoopBuffer)
		:_pBuffer(pAttackBuffer), _pLoopBuffer(pLoopBuffer), _source(0), _isLoop(false){

	}

	Sound::~Sound(){
		stop();
		SoundMgr::s().releaseSoundBuffer(_pBuffer);
		if ( _pLoopBuffer ){
			SoundMgr::s().releaseSoundBuffer(_pLoopBuffer);
		}
	}
	
	bool Sound::play(){
		if ( _source == 0 ){
			_source = SoundMgr::s().getFreeSource();
			if ( _source == 0 ){
				return false;
			}
		}else{
			alSourcef(_source, AL_GAIN, _volume);
			alSourcePlay(_source);
			return true;
		}
		alSourcei(_source, AL_BUFFER, _pBuffer->id);
		if ( _isLoop ){
			alSourcei(_source, AL_LOOPING, 1);
		}else{
			alSourcei(_source, AL_LOOPING, 0);
			std::list<Sound*>& srcs = SoundMgr::s()._noLoopPlayingSounds;
			std::list<Sound*>::iterator it = srcs.begin();
			std::list<Sound*>::iterator itEnd = srcs.end();
			srcs.push_back(this);
		}
		alSourcef(_source, AL_GAIN, _volume);
		alSourcePlay(_source);
		return true;
	}

	void Sound::stop(){
		if ( _source != 0 ){
			alSourceStop(_source);
			alSourcei(_source, AL_BUFFER, NULL);
			if ( !_isLoop ){
				std::list<Sound*>& srcs = SoundMgr::s()._noLoopPlayingSounds;
				std::list<Sound*>::iterator it = srcs.begin();
				std::list<Sound*>::iterator itEnd = srcs.end();
				for ( ; it != itEnd; ++it ){
					if ( *it == this ){
						srcs.erase(it);
						break;
					}
				}
			}
			std::vector<SoundSource>::iterator it = SoundMgr::s()._sources.begin();
			std::vector<SoundSource>::iterator itEnd = SoundMgr::s()._sources.end();
			for ( ; it != itEnd; ++it ){
				if ( it->source == _source ){
					it->used = false;
					break;
				}
			}
			_source = 0;
		}
	}

	void Sound::onPlayEnd(){
		if ( _source != 0 ){
			alSourcei(_source, AL_BUFFER, NULL);
			std::vector<SoundSource>::iterator it = SoundMgr::s()._sources.begin();
			std::vector<SoundSource>::iterator itEnd = SoundMgr::s()._sources.end();
			for ( ; it != itEnd; ++it ){
				if ( it->source == _source ){
					it->used = false;
					break;
				}
			}
			_source = 0;
		}
	}

	Sounds::Sounds(const char* fileName, int instanceNum, bool& ok)
	:_currIdx(0){
		lwassert(fileName);
		for ( int i = 0; i < instanceNum; ++i ){
			Sound* pSound = SoundMgr::s().createSound(fileName);
			if ( !pSound ){
				lwerror("SoundMgr::s().createSound failed: fileName=" << fileName);
				return;
			}else{
				_sounds.push_back(pSound);
			}
		}
		ok = true;
	}

	Sounds::~Sounds(){
		std::vector<Sound*>::iterator it = _sounds.begin();
		std::vector<Sound*>::iterator itEnd = _sounds.end();
		for ( ; it != itEnd; ++it ){
			delete (*it);
		}
	}

	Sound* Sounds::play(){
		Sound* pSound = _sounds[_currIdx++];
		pSound->play();
		if ( _currIdx == (int)_sounds.size() ){
			_currIdx = 0;
		}
		return pSound;
	}

	void Sounds::setVolume(float volume){
		std::vector<Sound*>::iterator it = _sounds.begin();
		std::vector<Sound*>::iterator itEnd = _sounds.end();
		for ( ; it != itEnd; ++it ){
			(*it)->setVolume(volume);
		}
	}

	SoundMgr::SoundMgr(int maxSources){
		alutInit (NULL, NULL);

		SoundSource ss;
		for ( int i = 0; i < maxSources; ++i ){
			alGenSources(1, &(ss.source));
			if ( ss.source == 0 ){
				break;
			}
			ss.used = false;
			_sources.push_back(ss);
		}
	}

	SoundMgr::~SoundMgr(){
		alutExit();
	}

	void SoundMgr::main(float dt){
		static float checkTime = 0;
		checkTime += dt;
		if ( checkTime > 50 ){
			checkTime -= 50;
			std::list<Sound*>::iterator it = _noLoopPlayingSounds.begin();
			std::list<Sound*>::iterator itEnd = _noLoopPlayingSounds.end();
			while ( it != itEnd){
				ALint state;
				alGetSourcei((*it)->_source, AL_SOURCE_STATE, &state);
				if ( state == AL_STOPPED ){
					(*it)->onPlayEnd();
					it = _noLoopPlayingSounds.erase(it);
					itEnd = _noLoopPlayingSounds.end();
				}else{
					++it;
				}
			}
		}
	}

	Sound* SoundMgr::createSound(const char* fileName){
		lwassert(fileName);
		SoundBuffer *pBuffer = getBuffer(fileName);
		if ( pBuffer == NULL ){
			lwerror("getBuffer failed: fileName=" <<fileName );
			return NULL;
		}
		Sound* pSound = new Sound(pBuffer);
		lwassert(pSound);
		return pSound;
	}

	Sound* SoundMgr::createSound(const char* attackFileName, const char* loopFileName){
		lwassert(attackFileName && loopFileName);
		SoundBuffer *pAttackBuffer = getBuffer(attackFileName);
		SoundBuffer *pLoopBuffer = getBuffer(loopFileName);
		if ( pAttackBuffer == NULL || pLoopBuffer == NULL ){
			if ( pAttackBuffer ){
				releaseSoundBuffer(pAttackBuffer);
			}
			if ( pLoopBuffer ){
				releaseSoundBuffer(pAttackBuffer);
			}
			return NULL;
		}

		Sound* pSound = new Sound(pAttackBuffer, pLoopBuffer);
		lwassert(pSound);
		return pSound;
	}

	Sounds* SoundMgr::createSounds(const char* fileName, int instanceNum){
		lwassert(fileName);
		lwassert(instanceNum > 0);
		bool ok = false;
		Sounds* pSounds = new Sounds(fileName, instanceNum, ok);
		lwassert(pSounds);
		if ( !ok ){
			lwerror("new Sounds failed: fileName=" << fileName );
			delete pSounds;
			pSounds = NULL;
		}
		return pSounds;
	}

	SoundBuffer* SoundMgr::getBuffer(const char* fileName){
		lwassert(fileName);
		SoundBuffer *pBuffer = NULL;
		std::map<std::string, SoundBuffer*>::iterator it = _bufferMap.find(fileName);
		if ( it != _bufferMap.end() ){
			++(it->second->refCount);
			pBuffer = it->second;
		}else{
			pBuffer = new SoundBuffer;
			lwassert(pBuffer);
			pBuffer->refCount = 1;
			pBuffer->id = alutCreateBufferFromFile (_f(fileName));
			int e = alGetError();
			int ee = alutGetError();
			if ( e != 0 || ee != 0 ){
				lwerror("al error: " << e << " alut error:" << alutGetErrorString(ee));
				lwassert(e == 0);
				return NULL;
			}
			if (pBuffer->id == AL_NONE){
				delete pBuffer;
				lwerror("alutCreateBufferFromFile failed");
				return NULL;
			}
			it = _bufferMap.insert(_bufferMap.begin(), std::make_pair(fileName, pBuffer));
			pBuffer->fileName = it->first.c_str();
		}
		return pBuffer;
	}

	void SoundMgr::releaseSoundBuffer(SoundBuffer* pBuffer){
		lwassert(pBuffer);
		--pBuffer->refCount;
		if ( pBuffer->refCount == 0 ){
			alDeleteBuffers(1, &pBuffer->id);
			alGetError();
			std::map<std::string, SoundBuffer*>::iterator it = _bufferMap.find(pBuffer->fileName);
			lwassert(it != _bufferMap.end());
			_bufferMap.erase(it);
			delete pBuffer;
		}
	}

	unsigned int SoundMgr::getFreeSource(){
		std::vector<SoundSource>::iterator it = _sources.begin();
		std::vector<SoundSource>::iterator itEnd = _sources.end();
		for ( ; it != itEnd; ++it ){
			if ( !it->used ){
				it->used = true;
				return it->source;
			}
		}
		return 0;
	}

} //namespace lw
