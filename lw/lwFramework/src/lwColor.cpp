#include "stdafx.h"
#include "lwColor.h"

namespace lw{

	const Color COLOR_BLACK = Color((GLubyte)0, (GLubyte)0, (GLubyte)0, (GLubyte)255);
	const Color COLOR_WHITE = Color((GLubyte)255, (GLubyte)255, (GLubyte)255, (GLubyte)255);

	bool operator == (const Color& a, const Color& b){
		return ( a.a == b.a && a.r == b.r && a.g == b.g && a.b == b.b );
	}

	bool operator != (const Color& a, const Color& b){
		return !( a.a == b.a && a.r == b.r && a.g == b.g && a.b == b.b );
	}

} //namespace lw