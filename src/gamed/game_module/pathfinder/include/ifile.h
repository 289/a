#ifndef  __PF_IFILE_H__
#define  __PF_IFILE_H__

#include <stdio.h>
#include "shared/net/buffer.h"

namespace pathfinder
{

#define  IFILE_TEXT     0x00000001
#define  IFILE_BIN      0x00000002
#define  IFILE_READ     0x00000004
#define  IFILE_WRITE    0x00000008

class IFile
{
private:
	FILE* fp;

public:
	IFile()
	{
		fp = NULL;
	}
	~IFile()
	{
		Close();
	}

	bool   Open(const char* szPath, unsigned long flags);
	bool   Close();
	size_t Read(void* buffer, size_t size);
	size_t Write(const void* buffer, size_t size);
};


///
/// inline func
///

inline bool IFile::Open(const char* szPath, unsigned long flags)
{
	Close();

	char mode[4] = {0,0,0,0};
	if (IFILE_READ & flags && IFILE_WRITE & flags)
	{
		//read write
		mode[0] = 'w';
		mode[2] = '+';
	}
	else if (IFILE_READ & flags)
	{
		//read only
		mode[0] = 'r';
	}
    else if (IFILE_WRITE & flags)
	{
		//write only
		mode[0] = 'w';
	}
	else
	{
		return false;
	}

	// text or binary
	if (IFILE_TEXT & flags && IFILE_BIN & flags)
	{
		return false;
	}
	else if (IFILE_TEXT & flags)
	{
		mode[1] = 't';
	}
    else if (IFILE_BIN & flags)
	{
		mode[1] = 'b';
	}
	else
	{
		//If t or b is not given in mode, the default translation mode is defined by the global variable _fmode.
		return false;
	}

	fp = fopen(szPath, mode);
	return (fp!=NULL);
}

inline bool IFile::Close()
{
	if (!fp)
	{
		return true;
	}

	if(fclose(fp))
	{
		return false;
	}

	fp = NULL;
	return true;
}

inline size_t IFile::Read(void * buffer, size_t size)
{
	if (!fp || !buffer || 0==size)
	{
		return 0;
	}
	return fread(buffer, 1, size, fp);
}

inline size_t IFile::Write(const void * buffer, size_t size)
{
	if (!fp || !buffer || 0==size)
	{
		return 0;
	}
	return fwrite(buffer, 1, size, fp);
}

};

#endif // __PF_IFILE_H__
