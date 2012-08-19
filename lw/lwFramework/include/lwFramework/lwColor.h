#ifndef __LW_COLOR_H__
#define __LW_COLOR_H__

namespace lw{
	
	struct Color{
		Color():r(255), g(255), b(255), a(255){}
		Color(GLubyte rr, GLubyte gg, GLubyte bb, GLubyte aa):r(rr), g(gg), b(bb), a(aa){	
		}
		Color(int rr, int gg, int bb, int aa):r(rr), g(gg), b(bb), a(aa){	
		}
		Color(float rr, float gg, float bb, float aa){
			set(rr, gg, bb, aa);
		}
		void set(float rr, float gg, float bb, float aa){
			rr *= 255;
			gg *= 255;
			bb *= 255;
			aa *= 255;
			r = (GLubyte)max(min(rr, 255.f), 0.f);
			g = (GLubyte)max(min(gg, 255.f), 0.f);
			b = (GLubyte)max(min(bb, 255.f), 0.f);
			a = (GLubyte)max(min(aa, 255.f), 0.f);
		}
		void set(int rr, int gg, int bb, int aa){
			r = (GLubyte)rr;
			g = (GLubyte)gg;
			b = (GLubyte)bb;
			a = (GLubyte)aa;
		}
		GLubyte r, g, b, a;
	};

	bool operator == (const Color& a, const Color& b);
	bool operator != (const Color& a, const Color& b);

	extern const Color COLOR_BLACK;
	extern const Color COLOR_WHITE;

} //namespace lw

#endif //__LW_COLOR_H__