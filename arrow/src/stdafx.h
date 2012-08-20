
#ifdef WIN32
#pragma once
#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						
#include <tchar.h>
#endif //#ifdef WIN32

#include <algorithm>
#include <sstream>
#include <set>
#include <map>

#ifdef __OBJC__
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#endif

#include "lwFramework/preInclude.h"
#include "lwFramework/lwUtil.h"
#include "lwFramework/lwFileSys.h"
#include "lwFramework/lwPODModel.h"
#include "lwFramework/lwCamera.h"
#include "lwFramework/lwTask.h"
#include "lwFramework/lwApp.h"
#include "lwFramework/lwPODModelRes.h"
#include "lwFramework/lwPODModelCollector.h"
#include "lwFramework/lwPODTextureRes.h"
#include "lwFramework/lwSprite.h"
#include "lwFramework/lwClip.h"
#include "lwFramework/lwSound.h"
#include "lwFramework/lwFont.h"
#include "lwFramework/lwUIMenu.h"
#include "lwFramework/lwUIButton.h"
#include "lwFramework/lwUICheckbox.h"
#include "lwFramework/lwUIText.h"
#include "lwFramework/lwTouchEvent.h"
#include "lwFramework/lwResourceHolder.h"
#include "lwFramework/lwTextField.h"
#include "lwFramework/lwHTTPClient.h"

#include "lwCommon/lwLog.h"
#include "lwCommon/lwSingleton.h"

#include "Box2D.h"

#include <sqlite3.h>

extern sqlite3* g_pDataDb;
extern sqlite3* g_pSaveDb;


extern int GAME_FPS;
