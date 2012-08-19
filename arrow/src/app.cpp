#include "stdafx.h"
#include "app.h"
#include "lwFramework/lwDB.h"
#include "taskMissile.h"
#include "taskSelect.h"
#include "soundManager.h"

int GAME_FPS = 60;

int appInit(){
#ifdef WIN32
	lw::FileSys::addDirectory("data", true);
#endif

	int r = 0;
	r = sqlite3_open(_f("data.db"), &g_pDataDb);
	lwassert(r == SQLITE_OK);
	lw::g_lwDB = g_pDataDb;
#ifdef WIN32
	r = sqlite3_open(_f("save.db"), &g_pSaveDb);
	lwassert(r == SQLITE_OK);
#endif
#ifdef __APPLE__
	std::string docDir = lw::getDocDir();
	docDir += "/save.db";
	FILE* pf = fopen(docDir.c_str(), "rb");
	if ( pf == NULL ){
		pf = fopen(_f("save.db"), "rb");
		lwassert(pf);
		fseek(pf, 0, SEEK_END);
		int len = ftell(pf);
		char buf[len];
		fseek(pf, 0, SEEK_SET);
		fread(buf, len, 1, pf);
		fclose(pf);
		FILE* pOut = fopen(docDir.c_str(), "wb");
		lwassert(pOut);
		fwrite(buf, len, 1, pOut);
		fclose(pOut);
	}

	r = sqlite3_open(docDir.c_str(), &g_pSaveDb);
	lwassert(r == SQLITE_OK);
#endif

	lw::App::Config conf;
	conf.orientation = lw::App::ORIENTATION_UP;
	conf.maxFPS = GAME_FPS;
	conf.title = L"missile";

	lw::App::create(conf);

	sndInit();
	//taskMissile.start(0);
	taskSelect.start(0);

	int retVal = 0;
#ifdef WIN32
	retVal = lw::App::run();
#endif
	return retVal;
}

void appQuit(){
	sndClear();
	lw::App::destroy();
	sqlite3_close(g_pDataDb);
	sqlite3_close(g_pSaveDb);
}