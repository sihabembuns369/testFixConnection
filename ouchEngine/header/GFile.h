#ifndef __GFILE_H__
#define __GFILE_H__

class CGFile
{
public:
	CGFile();
	~CGFile();

	int Open(const char *szFile);
	void Close();
	long ReadLine(char *data, long maxDataLen);
	long IDXReadLine(char *data, long maxDataLen);
	long ReadBlock(char *pszBuf, unsigned long nBytesToRead);

private:
	int hf;
	char szFileName[1024];
};

#endif
