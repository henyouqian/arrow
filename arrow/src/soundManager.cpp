#include "stdafx.h"
#include "soundManager.h"

namespace{
	std::vector<lw::Sounds*> g_pSounds;

	
}

lw::Sounds* g_pSndShoot = NULL;
lw::Sounds* g_pSndHit = NULL;

lw::Sounds* _createSounds(const char* fileName, int instNum){
	lw::Sounds* p = lw::SoundMgr::s().createSounds(fileName, instNum);
	g_pSounds.push_back(p);
	return p;
}

void sndClear(){
	std::vector<lw::Sounds*>::iterator it = g_pSounds.begin();
	std::vector<lw::Sounds*>::iterator itEnd = g_pSounds.end();
	for ( ; it != itEnd; ++it ){
		delete (*it);
	}
	g_pSounds.clear();
}

void sndInit(){
	g_pSndHit = _createSounds("hit.wav", 5);
	g_pSndShoot = _createSounds("shoot.wav", 5);
}
