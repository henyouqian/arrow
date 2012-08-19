// lwFirst.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "app.h"
//#include <vld.h>

//sqlite3* g_pSaveDb = NULL;

int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc(2030);

	int retVal = appInit();
	appQuit();

	return retVal;
}