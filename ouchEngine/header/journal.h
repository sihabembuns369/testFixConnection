#ifndef __JOURNAL_H__
#define __JOURNAL_H__

#include <string>
#include <cstdarg>

class CJournal
{
public:
	CJournal();
	CJournal(const char *filename);
	~CJournal();

	int OpenJournal(const char *filename);
	void CloseJournal();
	long WriteLog(const char *sz, ...);
	void Binary_Dump(void const *buffer, size_t len);
	
	void WriteOuchInLog(char const *buffer, size_t bufLen, const char *szArg, ...);
	void WriteOuchOutLog(uint8_t msgType, const char *uid, const char *szDesc, char const *buffer, size_t bufLen);

private:
	int fd;
	pthread_mutex_t mutex;

	int rows_eq(int *a, int *b);
	void DirectWrite(const char *sz, ...);
	void dump_row(long count, int numinrow, int *chs);
	void dump_row(std::string &s, long count, int numinrow, int *chs);
	long WriteBlock(int hf, const char *szBuffer, unsigned long bytesToWrite);
	long ProcessArg(const char *szFormat, va_list valist, char *szBuffer, unsigned long bufSize);
};

#endif
