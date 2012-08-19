#include "stdafx.h"
#include "lwGLExtension.h"

namespace lw{

	namespace{
		CPVRTglesExt* g_pExt = NULL;

		class GLExtensionCleanner{
		public:
			~GLExtensionCleanner(){
				if ( g_pExt ) {
					delete g_pExt;
				}
			}
		};

		GLExtensionCleanner cleanner;
	}

	CPVRTglesExt& GLExtension::get(){
		if ( g_pExt == NULL ) {
			g_pExt = new CPVRTglesExt;
			lwassert(g_pExt);
			g_pExt->LoadExtensions();
		}
		return *g_pExt;
	}

	

} //namespace lw