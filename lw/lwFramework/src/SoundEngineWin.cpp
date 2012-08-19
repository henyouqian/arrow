#include "stdafx.h"
#include "SoundEngine.h"


namespace{
	struct SoundBuffer{
		ALuint id;
		int refCount;
		std::string filePath;
	};
	struct SoundEffectLoop{
		SoundBuffer* pLoopBuffer;
		SoundBuffer* pAttackBuffer;
		SoundBuffer* pDecayBuffer;
		ALuint source;
		enum State{
			S_THREAD_NONE,
			S_THREAD_RUN,
			S_THREAD_KILL,
			S_THREAD_DONE,
		};
		State state;
	};
	struct SoundEngineData 
	{
		std::map<std::string, SoundBuffer*> bufferMap;
		std::map<ALuint, SoundBuffer*> soundEffectMap;
		std::map<ALuint, SoundEffectLoop*> soundEffectLoopMap;
	};
	SoundEngineData g_soundData;
	ALuint g_BGMSource = 0;
}

OSStatus  SoundEngine_Initialize(Float32 inMixerOutputRate){
	alutInit (NULL, NULL);
	return kSoundEngineOK;
}

void releaseBuffer(SoundBuffer* pBuffer){
	lwassert(pBuffer);
	SoundEngineData& d = g_soundData;
	--pBuffer->refCount;
	if ( pBuffer->refCount == 0 ){
		alDeleteBuffers(1, &pBuffer->id);
		std::map<std::string, SoundBuffer*>::iterator it = d.bufferMap.find(pBuffer->filePath.c_str());
		lwassert(it != d.bufferMap.end());
		d.bufferMap.erase(it);
		delete pBuffer;
	}
}

OSStatus SoundEngine_Teardown(){
	SoundEngineData& d = g_soundData;
	{
		std::map<ALuint, SoundBuffer*>::iterator it = d.soundEffectMap.begin();
		std::map<ALuint, SoundBuffer*>::iterator itEnd = d.soundEffectMap.end();
		while( it != itEnd ){
			lwerror("sound effect leak: filePath=" << it->second->filePath.c_str());
			SoundEngine_UnloadEffect(it->first);
			it = d.soundEffectMap.begin();
		}
	}
	{
		std::map<ALuint, SoundEffectLoop*>::iterator it = d.soundEffectLoopMap.begin();
		std::map<ALuint, SoundEffectLoop*>::iterator itEnd = d.soundEffectLoopMap.end();
		while( it != itEnd ){
			lwerror("sound loop effect leak: loopFilePath=" << it->second->pLoopBuffer->filePath.c_str());
			SoundEngine_UnloadEffect(it->first);
			it = d.soundEffectLoopMap.begin();
		}
	}
	{
		std::map<std::string, SoundBuffer*>::iterator it = d.bufferMap.begin();
		std::map<std::string, SoundBuffer*>::iterator itEnd = d.bufferMap.end();
		while( it != itEnd ){
			lwerror("sound buffer leak: filePath=" << it->second->filePath.c_str());
			releaseBuffer(it->second);
			it = d.bufferMap.begin();
		}
	}
	
	alutExit();
	return kSoundEngineOK;
}

OSStatus  SoundEngine_SetMasterVolume(Float32 inValue){
	return kSoundEngineOK;
}

OSStatus  SoundEngine_SetListenerPosition(Float32 inX, Float32 inY, Float32 inZ){
	return kSoundEngineOK;
}

OSStatus  SoundEngine_SetListenerGain(Float32 inValue){
	return kSoundEngineOK;
}

OSStatus  SoundEngine_LoadBackgroundMusicTrack(const char* inPath, Boolean inAddToQueue, Boolean inLoadAtOnce){
	lwassert(inPath);
	if ( g_BGMSource != 0 ){
		SoundEngine_UnloadBackgroundMusicTrack();
	}
	return SoundEngine_LoadLoopingEffect(inPath, NULL, NULL, &g_BGMSource);
}

OSStatus  SoundEngine_UnloadBackgroundMusicTrack(){
	OSStatus r = SoundEngine_UnloadEffect(g_BGMSource);
	g_BGMSource = 0;
	return r;
}

OSStatus  SoundEngine_StartBackgroundMusic(){
	return SoundEngine_StartEffect(g_BGMSource);
}

OSStatus  SoundEngine_StopBackgroundMusic(Boolean inStopAtEnd){
	return SoundEngine_StopEffect(g_BGMSource, false);
}

OSStatus  SoundEngine_SetBackgroundMusicVolume(Float32 inValue){
	alSourcef(g_BGMSource, AL_GAIN, inValue);
	return kSoundEngineOK;
}

SoundBuffer* getBuffer(const char* inPath){
	SoundEngineData& d = g_soundData;
	std::map<std::string, SoundBuffer*>::iterator it = d.bufferMap.find(inPath);
	SoundBuffer* pBuffer = NULL;
	if ( it != d.bufferMap.end() ){
		pBuffer = it->second;
		++pBuffer->refCount;
	}else{
		pBuffer = new SoundBuffer;
		lwassert(pBuffer);
		pBuffer->refCount = 1;
		//lwinfo(alutGetErrorString (alGetError()) << " " << inPath);
		pBuffer->id = alutCreateBufferFromFile (inPath);
		
		if (pBuffer->id == AL_NONE){
			delete pBuffer;
			lwerror("alutCreateBufferFromFile failed");
			return NULL;
		}
		pBuffer->filePath = inPath;
		d.bufferMap[pBuffer->filePath.c_str()] = pBuffer;
	}
	return pBuffer;
}

OSStatus  SoundEngine_LoadLoopingEffect(const char* inLoopFilePath, const char* inAttackFilePath, const char* inDecayFilePath, UInt32* outEffectID){
	lwassert(inLoopFilePath && outEffectID);
	SoundEngineData& d = g_soundData;
	SoundEffectLoop* p = new SoundEffectLoop;
	lwassert(p);
	p->pLoopBuffer = getBuffer(inLoopFilePath);
	if ( p->pLoopBuffer == NULL ){
		lwerror("getBuffer(inLoopFilePath) failed: filePath=" << inLoopFilePath);
		delete p;
		return kSoundEngineErrUnknown;
	}
	p->state = SoundEffectLoop::S_THREAD_NONE;
	if ( inAttackFilePath ){
		p->pAttackBuffer = getBuffer(inAttackFilePath);
		if ( p->pAttackBuffer == NULL ){
			lwerror("getBuffer(inAttackFilePath) failed: filePath=" << inLoopFilePath);
			delete p;
			return kSoundEngineErrUnknown;
		}
	}else{
		p->pAttackBuffer = NULL;
	}
	if ( inDecayFilePath ) {
		p->pDecayBuffer = getBuffer(inDecayFilePath);
		if ( p->pDecayBuffer == NULL ){
			lwerror("getBuffer(inDecayFilePath) failed: filePath=" << inLoopFilePath);
			delete p;
			return kSoundEngineErrUnknown;
		}
	}else{
		p->pDecayBuffer = NULL;
	}

	alGenSources (1, &(p->source));
	
	*outEffectID = p->source;

	d.soundEffectLoopMap[p->source] = p;
 
	return kSoundEngineOK;
}

OSStatus  SoundEngine_LoadEffect(const char* inPath, UInt32* outEffectID){
	lwassert(inPath && outEffectID);
	SoundEngineData& d = g_soundData;
	SoundBuffer* pBuffer = getBuffer(inPath);
	if ( pBuffer == NULL ) {
		lwerror("getBuffer failed: path=" << inPath);
		return kSoundEngineErrUnknown;
	}
	lwinfo(alutGetErrorString (alGetError()) << " " << inPath);
	ALuint source = 0;
	alGenSources (1, &source);
	lwinfo(alutGetErrorString (alGetError()) << " " << inPath);
	alSourcei (source, AL_BUFFER, pBuffer->id);
	lwinfo(alutGetErrorString (alGetError()) << " " << inPath);
	*outEffectID = source;

	d.soundEffectMap[source] = pBuffer;
	
	return kSoundEngineOK;
}

void clearQueueBuffer(UInt32 inEffectID){
	ALint numQueuedBuffers = 0;
	alGetSourcei(inEffectID, AL_BUFFERS_QUEUED, &numQueuedBuffers);
	ALuint *bufferIDs = (ALuint*)malloc(numQueuedBuffers * sizeof(ALint));
	alSourceUnqueueBuffers(inEffectID, numQueuedBuffers, bufferIDs);
	free(bufferIDs);
}

OSStatus  SoundEngine_UnloadEffect(UInt32 inEffectID){
	SoundEngine_StopEffect(inEffectID, false);
	SoundEngineData& d = g_soundData;
	std::map<ALuint, SoundEffectLoop*>::iterator it = d.soundEffectLoopMap.find(inEffectID);
	if ( it != d.soundEffectLoopMap.end() ){
		SoundEffectLoop *pEffect = it->second;
		if ( pEffect->state == SoundEffectLoop::S_THREAD_RUN ){
			pEffect->state = SoundEffectLoop::S_THREAD_KILL;
			while( pEffect->state != SoundEffectLoop::S_THREAD_DONE ){
				Sleep(10);
			}
		}
		clearQueueBuffer(inEffectID);
		releaseBuffer(pEffect->pLoopBuffer);
		if ( pEffect->pAttackBuffer ){
			releaseBuffer(pEffect->pAttackBuffer);
		}
		if ( pEffect->pDecayBuffer ){
			releaseBuffer(pEffect->pDecayBuffer);
		}
		delete pEffect;
		d.soundEffectLoopMap.erase(it); 
	}else{
		std::map<ALuint, SoundBuffer*>::iterator it = d.soundEffectMap.find(inEffectID);
		if ( it != d.soundEffectMap.end() ){
			alSourcei (inEffectID, AL_BUFFER, NULL);
			releaseBuffer(it->second);
			d.soundEffectMap.erase(it);
		}else{
			lwerror("invalid effect id");
			return kSoundEngineErrUnknown;
		}
	}
	alDeleteSources(1, &inEffectID);

	return kSoundEngineOK;
}

DWORD WINAPI workerThread(LPVOID pSoundEffectLoop){
	SoundEffectLoop* THIS = (SoundEffectLoop*)pSoundEffectLoop;
	ALint numBuffersProcessed = 0;						
	while (numBuffersProcessed < 1 && THIS->state == SoundEffectLoop::S_THREAD_RUN){
		alGetSourcei(THIS->source, AL_BUFFERS_PROCESSED, &numBuffersProcessed);
		static int i = 0;
		Sleep(10);
	}
	ALuint tmpBuffer = 0;
	alSourceUnqueueBuffers(THIS->source, 1, &tmpBuffer);
	alSourcei(THIS->source, AL_LOOPING, 1);
	THIS->state = SoundEffectLoop::S_THREAD_DONE;
	return 0;
}

OSStatus  SoundEngine_StartEffect(UInt32 inEffectID){
	SoundEngineData& d = g_soundData;
	std::map<ALuint, SoundEffectLoop*>::iterator it = d.soundEffectLoopMap.find(inEffectID);
	if ( it != d.soundEffectLoopMap.end() ){
		if ( it->second->pAttackBuffer == NULL ){
			alSourcei(inEffectID, AL_BUFFER, it->second->pLoopBuffer->id);
			alSourcei(inEffectID, AL_LOOPING, 1);
			alSourcePlay(inEffectID);
		}else{
			DWORD threadID;
			HANDLE hThread;
			it->second->state = SoundEffectLoop::S_THREAD_RUN;
			hThread = CreateThread(NULL, 0, workerThread, it->second, 0, &threadID);
			if ( hThread == NULL){ 
				lwerror("CreateThread() failed with error " << GetLastError()); 
				return kSoundEngineErrUnknown; 
			}
			CloseHandle(hThread);

			SoundEffectLoop* p = it->second;
			clearQueueBuffer(p->source);
			alSourceQueueBuffers(p->source, 1, &(p->pAttackBuffer->id));
			alSourceQueueBuffers(p->source, 1, &(p->pLoopBuffer->id));

			alSourcei(p->source, AL_LOOPING, 0);
			alSourcePlay(p->source);
		}
	}else{
		std::map<ALuint, SoundBuffer*>::iterator it = d.soundEffectMap.find(inEffectID);
		if ( it != d.soundEffectMap.end() ){
			alSourcePlay (inEffectID);
		}else{
			lwerror("invalid effect id");
			return kSoundEngineErrUnknown;
		}
	}
	
	return kSoundEngineOK;
}

OSStatus  SoundEngine_StopEffect(UInt32 inEffectID, Boolean inDoDecay){
	alSourceStop(inEffectID);
	return kSoundEngineOK;
}

OSStatus  SoundEngine_SetEffectPitch(UInt32 inEffectID, Float32 inValue){
	return kSoundEngineOK;
}

OSStatus  SoundEngine_SetEffectLevel(UInt32 inEffectID, Float32 inValue){
	alSourcef(inEffectID, AL_GAIN, inValue);
	return kSoundEngineOK;
}

OSStatus	SoundEngine_SetEffectPosition(UInt32 inEffectID, Float32 inX, Float32 inY, Float32 inZ){
	return kSoundEngineOK;
}

OSStatus  SoundEngine_SetEffectsVolume(Float32 inValue){
	return kSoundEngineOK;
}

OSStatus	SoundEngine_SetMaxDistance(Float32 inValue){
	return kSoundEngineOK;
}

OSStatus	SoundEngine_SetReferenceDistance(Float32 inValue){
	return kSoundEngineOK;
}