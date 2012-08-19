#ifndef __LW_FONT_H__
#define __LW_FONT_H__

namespace lw{
	
	class FontRes;
	class Font;

	enum FontAlign{
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT,
	};

	struct FontComposingData{
	public:
		void set(Font* pFont, const wchar_t* text, FontAlign align, float w, float scale);
	
		std::wstring text;
		std::vector<int> linesOffset;
		FontAlign align;
		float scale;
	};

	class Font{
	public:
		static Font* create(const char* fileName);
		~Font();

		void drawText(const wchar_t* text, float x, float y, const Color& color = COLOR_WHITE, float scale = 1.0f, float rotate = 0.f);
		void drawText(FontComposingData* pComposingData, float x, float y, const Color& color = COLOR_WHITE, float rotate = 0.f);
		void getSize(int& outX, int& outY, const wchar_t* text);

		FontRes* getFontRes(){
			return _pRes;
		}
		int getH();

	private:
		Font(FontRes* pRes);

	private:
		FontRes* _pRes;

	public:
		static void cleanup();
	};
	

} //namespace lw

#endif //__LW_FONT_H__