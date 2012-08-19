#ifndef __LW_FILE_SYS_H__
#define __LW_FILE_SYS_H__


namespace lw{
#ifdef WIN32
	class FileSys
	{
	public:
		static bool addDirectory(const char* dir, bool recursive);
	};
#endif //#ifdef WIN32
#ifdef __APPLE__
	const char* getDocDir();
#endif
	
} //namespace lw


class _f
{
public:
	_f(const char* filename);
	operator const char*(){
		return _pathStr.c_str();
	}
	bool isValid(){
		return !_pathStr.empty();
	}

private:
	std::string _pathStr;
};


#endif //__LW_FILE_SYS_H__