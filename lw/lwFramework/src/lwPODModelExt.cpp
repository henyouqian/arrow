#include "stdafx.h"
#include "lwPODModelExt.h"

namespace lw{

	AnimChannel* AnimChannel::create(FILE* pf, int frameCount){
		int type = 0;
		fread(&type, sizeof(type), 1, pf);
		if ( type == 0 ){
			return new LinerChannel(pf, frameCount);
		}else if ( type == 1 ){
			return new StepChannel(pf);
		}else{
			lwerror("invalid type: " << type);
		}
		return NULL;
	}

	LinerChannel::LinerChannel(FILE* pf, int frameCount){
		double d = 0;
		for ( int i = 0; i < frameCount; ++i ){
			fread(&d, sizeof(d), 1, pf);
			_frames.push_back((float)d);
		}
	}

	float LinerChannel::value(float t){
		int numFrames = (int)_frames.size();
		if ( numFrames == 0 ){
			return 0;
		}else if ( numFrames == 1 ){
			return _frames[0];
		}else{
			int iT = (int)t;
			iT = max(0, min(numFrames-2, iT));
			return cml::lerp(_frames[iT], _frames[iT+1], t-(float)iT);
		}
	}

	StepChannel::StepChannel(FILE* pf){
		int key = 0;
		double d = 0;
		while (1){
			fread(&key, sizeof(key), 1, pf);
			if ( key == -1 ){
				break;
			}
			fread(&d, sizeof(d), 1, pf);
			_frames[key] = (float)d;
		}
	}

	float StepChannel::value(float t){
		int numFrames = (int)_frames.size();
		if ( numFrames == 0 ){
			return 0;
		}else if ( numFrames == 1 ){
			return _frames.begin()->second;
		}else{
			std::map<int, float>::iterator it = _frames.upper_bound((int)t);
			if ( it != _frames.begin() ){
				--it;	
			}
			return it->second;
		}
		
	}

} //namespace lw