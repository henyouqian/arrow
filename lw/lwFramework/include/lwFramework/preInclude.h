
#include <list>
#include <map>

#include "cml/cml.h"
namespace cml{
	typedef matrix44f_c Matrix44;
	typedef matrix33f_c Matrix33;
	typedef matrix22f_c Matrix22;
	typedef vector2f Vector2;
	typedef vector3f Vector3;
	typedef vector4f Vector4;
}

#include "PVRTools/ogles/OGLESTools.h"

#ifdef __APPLE__
#include "lwCommon/lwMinMax.h"
#endif

#include "lwColor.h"
#include "lwCommon/lwLog.h"
#include "lwCommon/lwSingleton.h"

#ifdef WIN32
#include "GLES/gl.h"
#include "GLES/egl.h"
#include "GLES/glext.h"
#else
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#endif

#define LW_ALL_IN_ONE
//#define SOUND_DISABLE

#ifdef WIN32
#	pragma comment(lib, "libgles_cm.lib")
#	pragma comment(lib, "winmm.lib")
#	ifndef SOUND_DISABLE
#		pragma comment(lib, "OpenAL32.lib")
#	endif

#	ifdef LW_ALL_IN_ONE
#		pragma comment(lib, "lwAllInOne.lib")
#	else
#		pragma comment(lib, "PVRTools.lib")
#		pragma comment(lib, "tinyxml.lib")
#		pragma comment(lib, "alut.lib")
#		pragma comment(lib, "soil.lib")
#	endif
#endif