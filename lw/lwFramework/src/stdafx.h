// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef __LW_FRAMEWORK_STDAFX_H__
#define __LW_FRAMEWORK_STDAFX_H__

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef WIN32
#include <Windows.h>
#include <mmsystem.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alut.h"
#endif

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <CFNetwork/CFNetwork.h>
#include "AL/alut.h"
#endif


#ifdef __OBJC__
#include <Foundation/Foundation.h> //before include pvrtgloble.h
#endif

#ifdef __cplusplus
#include <list>
#include <vector>
#include <string>
#include <set>

#include "tinyxml/tinyxml.h"
#include "preInclude.h"

#endif


#endif //__LW_FRAMEWORK_STDAFX_H__
// TODO: reference additional headers your program requires here
