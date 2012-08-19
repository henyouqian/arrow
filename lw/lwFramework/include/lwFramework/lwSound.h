#ifndef __LW_SOUND_H__
#define __LW_SOUND_H__

namespace lw{
	
	//class Sound{
	//public:
	//	static void init();
	//	static void quit();
	//	static void clear();
	//	static void main(float dt);
	//	static Sound* create(const char* fileName, bool loop);
	//	static Sound* create(const char* attackFileName, const char* loopFileName);
	//	
	//	virtual void play() = 0;
	//	virtual void stop() = 0;
	//	virtual void setVolume(float v) = 0;

	//	virtual ~Sound() {}
	//};

	class SoundMgr;

	struct SoundBuffer{
		unsigned int id;
		int refCount;
		const char* fileName;
	};
	struct SoundSource{
		unsigned int source;
		bool used;
	};

	//single instance
	class Sound{
	public:
		~Sound();
		bool play();
		void stop();
		void setLoop(bool b){
			_isLoop = b;
		}
		bool isPlaying(){
			return _source != 0;
		}
		void setVolume(float volume){
			_volume = volume;
		}

	private:
		Sound(SoundBuffer* pBuffer);
		Sound(SoundBuffer* pAttackBuffer, SoundBuffer* pLoopBuffer);
		void onPlayEnd();

		bool _isLoop;
		SoundBuffer* _pBuffer;
		SoundBuffer* _pLoopBuffer;
		unsigned int _source;
		float _volume;

		friend class SoundMgr;
	};

	//multi instance
	class Sounds{
	public:
		~Sounds();
		Sound* play();
		void setVolume(float volume);

	private:
		Sounds(const char* fileName, int instanceNum, bool& ok);
		std::vector<Sound*> _sounds;
		int _currIdx;
		friend class SoundMgr;
	};

	class SoundMgr : public Singleton<SoundMgr>{
	public:
		SoundMgr(int maxSources);
		~SoundMgr();
		Sound* createSound(const char* fileName);
		Sound* createSound(const char* attackFileName, const char* loopFileName);
		Sounds* createSounds(const char* fileName, int instanceNum);
		void main(float dt);

	private:
		void releaseSoundBuffer(SoundBuffer* pSoundBuffer);
		unsigned int getFreeSource();
		SoundBuffer* getBuffer(const char* fileName);

		std::map<std::string, SoundBuffer*> _bufferMap;		
		std::vector<SoundSource> _sources;
		std::list<Sound*> _noLoopPlayingSounds;

		friend class Sound;
	};

	

} //namespace lw

#endif //__LW_SOUND_H__