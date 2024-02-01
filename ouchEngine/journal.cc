#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include "header/journal.h"
#include "header/mutexGuard.h"

const unsigned long LOG_BUFFER_SIZE = 64 * 1024;	// 64 Kb

CJournal::CJournal() : fd(-1)
{
	pthread_mutex_init(&mutex, NULL);
}

CJournal::CJournal(const char *filename)
{
	pthread_mutex_init(&mutex, NULL);
	if ((fd = open(filename, O_CREAT | O_WRONLY, 0644)) < 0)
		printf("Can not open journal file %s\n", filename);
	else
		lseek(fd, 0, SEEK_END);
}

CJournal::~CJournal()
{
	CloseJournal();
	pthread_mutex_destroy(&mutex);
}

int CJournal::OpenJournal(const char *filename)
{
	CMutexGuard wr(&mutex);
	if (-1 != fd)
		close(fd);
	if ((fd = open(filename, O_CREAT | O_WRONLY, 0644)) < 0)
		printf("Can not open journal file : %s\n", filename);
	else
		lseek(fd, 0, SEEK_END);
	return fd;
}

void CJournal::CloseJournal()
{
	CMutexGuard wr(&mutex);
	if (-1 != fd)
	{
		close(fd); fd = -1;
	}
}

long CJournal::ProcessArg(const char *szFormat, va_list valist, char *szBuffer, unsigned long bufSize)
{
	time_t ttime = time(NULL);
	struct tm *ptm = localtime(&ttime);
	long slen = sprintf(szBuffer, "[%u-%02u-%02u %02u:%02u:%02u] - ", ptm->tm_year + 1900, 
		ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec),
		vlen = vsnprintf(szBuffer + slen, bufSize - slen, szFormat, valist);
	return ( vlen < 0 ) ? -1 : vlen + slen;
}

long CJournal::WriteBlock(int hf, const char *szBuffer, unsigned long bytesToWrite)
{
	CMutexGuard wr(&mutex);
	long nBytesWritten = 0, nTotBytesWritten = 0;
	do
	{
		if ((nBytesWritten = write(hf, szBuffer + nTotBytesWritten, bytesToWrite - nTotBytesWritten)) > 0)
			nTotBytesWritten += nBytesWritten;
		else
			break;
	} while ((unsigned long)nTotBytesWritten < bytesToWrite);
	return nTotBytesWritten;
}

//long CJournal::WriteLog(const char *psz, ...)
//{
//	va_list valist;
//	long nBytesWritten = 0;
//	char sz[LOG_BUFFER_SIZE];
//	//char *sz = (char*)malloc(LOG_BUFFER_SIZE * sizeof(char));
//
//	va_start(valist, psz);
//	long narg = ProcessArg(psz, valist, sz, LOG_BUFFER_SIZE);
//	va_end(valist);
//	if (narg > 0)
//		nBytesWritten = WriteBlock(fd, sz, narg);
//	//free(sz);
//	return nBytesWritten;
//}

long CJournal::WriteLog(const char *psz, ...)
{
	va_list valist;
	char sz[LOG_BUFFER_SIZE];
	va_start(valist, psz);
	long narg = ProcessArg(psz, valist, sz, LOG_BUFFER_SIZE);
	va_end(valist);
	return ( narg > 0 ) ? WriteBlock(fd, sz, narg) : 0;
}

void CJournal::DirectWrite(const char *psz, ...)
{
	va_list valist;
	char szBuf[LOG_BUFFER_SIZE];
	va_start(valist, psz);
	long vlen = vsnprintf(szBuf, LOG_BUFFER_SIZE, psz, valist);
	if (vlen > 0)
		write(fd, szBuf, vlen);
	va_end(valist);
}

int CJournal::rows_eq(int *a, int *b)
{
	for (int i = 0; i < 16; i++)
		if (a[i] != b[i])
			return 0;
	return 1;
}

void CJournal::dump_row(long count, int numinrow, int *chs)
{
	DirectWrite("%08lX:", count - numinrow);
	if (numinrow > 0)
	{
		int i;
		for (i = 0; i < numinrow; i++)
		{
			if (i == 8)
				DirectWrite(" :");
			DirectWrite(" %02X", chs[i]);
		}
		for (i = numinrow; i < 16; i++)
		{
			if (i == 8)
				DirectWrite(" :");
			DirectWrite("   ");
		}
		DirectWrite("  ");
		for (i = 0; i < numinrow; i++)
		{
			if (isprint(chs[i]))
				DirectWrite("%c", chs[i]);
			else
				DirectWrite(".");
		}
	}
	DirectWrite("\n");
}

void CJournal::dump_row(std::string &s, long count, int numinrow, int *chs)
{
	char sz[256];
	char *psz = sz + sprintf(sz, "%08lX:", count - numinrow);
	if (numinrow > 0)
	{
		int i = 0;
		for (; i < numinrow; i++)
		{
			if (i == 8)
			{
				*psz = ' ';
				psz[1] = ':';
				psz += 2;
			}
			psz += sprintf(psz, " %02X", chs[i]);
		}
		for (i = numinrow; i < 16; i++)
		{
			if (i == 8)
			{
				*psz = ' ';
				psz[1] = ':';
				psz += 2;
			}
			*psz = psz[1] = psz[2] = ' ';
			psz += 3;
		}
		*psz = psz[1] = ' ';
		psz += 2;
		for (i = 0; i < numinrow; i++)
			*psz++ = isprint(chs[i])?chs[i]:'.';
	}
	*psz = '\r';
	psz[1] = '\n';
	psz[2] = 0x00;
	s.append(sz);
}
/*
void CJournal::Binary_Dump(void const *buffer, size_t len)
{
	long count = 0;
	int chs[16], numinrow = 0;
	unsigned char *buf = (unsigned char *) buffer;

	time_t ttime = time(NULL);
	struct tm *ptm = localtime(&ttime);
	CMutexGuard wr(&mutex);
	DirectWrite("[%u-%02u-%02u %02u:%02u:%02u] - Binary Dump ........\n", ptm->tm_year + 1900, 
		ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	for (size_t i = 0; i < len; i++)
	{
		int ch = buf[i];
		if (numinrow == 16)
		{
			dump_row(count, numinrow, chs);
			numinrow = 0;
		}
		count++;
		chs[numinrow++] = ch;
	}
	dump_row(count, numinrow, chs);
	if (numinrow != 0)
		DirectWrite("%08lX:\n", count);
}
*/
void CJournal::Binary_Dump(void const *buffer, size_t len)
{
	char sz[256];
	long count = 0;
	int chs[16], numinrow = 0;
	unsigned char *buf = (unsigned char *) buffer;

	time_t ttime = time(NULL);
	struct tm *ptm = localtime(&ttime);
	sprintf(sz, "[%u-%02u-%02u %02u:%02u:%02u] - Binary Dump ........\r\n", ptm->tm_year + 1900, 
		ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	std::string s(sz);
	for (size_t i = 0; i < len; i++)
	{
		int ch = buf[i];
		if (numinrow == 16)
		{
			dump_row(s, count, numinrow, chs);
			numinrow = 0;
		}
		count++;
		chs[numinrow++] = ch;
	}
	dump_row(s, count, numinrow, chs);
	if (0 != numinrow)
	{
		sprintf(sz, "%08lX:\r\n", count);
		s.append(sz);
	}
	WriteBlock(fd, s.c_str(), (unsigned long)s.size());
}
/*
void CJournal::Binary_Dump(void const *buffer, size_t len)
{
	unsigned char *buf = (unsigned char *) buffer;
	long count = 0;
	int numinrow = 0;
	int chs[16];
	int oldchs[16] = {0};
	int showed_dots = 0;
	size_t i;
	
	time_t ttime;
	time(&ttime);
	tm *ptm = localtime(&ttime);
	CMutexGuard wr(&mutex);
	DirectWrite("[%u-%02u-%02u %02u:%02u:%02u] - Binary Dump ........\n", ptm->tm_year + 1900, 
		ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	for (i = 0; i < len; i++)
	{
		int ch = buf[i];
		if (numinrow == 16)
		{
			int i;
			if (rows_eq(oldchs, chs))
			{
				if (!showed_dots)
				{
					showed_dots = 1;
					DirectWrite("          .. .. .. .. .. .. .. .. : .. .. .. .. .. .. .. ..\n");
				}
			}
			else
			{
				showed_dots = 0;
				dump_row(count, numinrow, chs);
			}
			for (i=0; i<16; i++)
			{
				oldchs[i] = chs[i];
			}
			numinrow = 0;
		}
		count++;
		chs[numinrow++] = ch;
	}
	dump_row(count, numinrow, chs);
	if (numinrow != 0)
	{
		DirectWrite("%08lX:\n", count);
	}
}
*/
void CJournal::WriteOuchInLog(char const *buffer, size_t bufLen, const char *szArg, ...)
{
	long count = 0;
	int numinrow = 0, chs[16] = {};
	char *szBuf = new char[LOG_BUFFER_SIZE];
	unsigned char *buf = (unsigned char *)buffer;

	va_list	arg;
	va_start(arg, szArg);
	long slen = ProcessArg(szArg, arg, szBuf, LOG_BUFFER_SIZE);
	va_end(arg);

	if (slen > 0)
	{
		szBuf[slen++] = '\r';
		szBuf[slen++] = '\n';
		szBuf[slen++] = 0x00;
	}
	std::string s(szBuf);
	for (size_t i = 0; i < bufLen; i++)
	{
		if (numinrow == 16)
		{
			dump_row(s, count, numinrow, chs);
			numinrow = 0;
		}
		count++;
		chs[numinrow++] = buf[i];
	}
	dump_row(s, count, numinrow, chs);
	if (0 != numinrow)
	{
		sprintf(szBuf, "%08lX:\r\n", count);
		s.append(szBuf);
	}
	WriteBlock(fd, s.c_str(), (unsigned long)s.size());
	delete []szBuf; szBuf = NULL;
}

void CJournal::WriteOuchOutLog(uint8_t msgType, const char *uid, const char *szDesc, char const *buffer, size_t bufLen)
{
	long count = 0;
	int numinrow = 0, chs[16] = {};
	char *szBuf = new char[LOG_BUFFER_SIZE];
	unsigned char *buf = (unsigned char *)buffer;

	time_t ttime = time(NULL);
	struct tm *ptm = localtime(&ttime);
	sprintf(szBuf, "[%u-%02u-%02u %02u:%02u:%02u][%s][MsgType:%c] - %s\r\n"
		"[%u-%02u-%02u %02u:%02u:%02u][%s][MsgType:%c] - OUCH Msg Dump ...\r\n", 
		ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, uid, msgType, szDesc,
		ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, uid, msgType);
	std::string s(szBuf);
	for (size_t i = 0; i < bufLen; i++)
	{
		if (numinrow == 16)
		{
			dump_row(s, count, numinrow, chs);
			numinrow = 0;
		}
		count++;
		chs[numinrow++] = buf[i];
	}
	dump_row(s, count, numinrow, chs);
	if (0 != numinrow)
	{
		sprintf(szBuf, "%08lX:\r\n", count);
		s.append(szBuf);
	}
	WriteBlock(fd, s.c_str(), (unsigned long)s.size());
	delete []szBuf; szBuf = NULL;
}
