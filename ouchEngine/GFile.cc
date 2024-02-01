#include "header/GFile.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

CGFile::CGFile():hf(-1)
{
	memset(szFileName, 0x00, sizeof(szFileName));
}

CGFile::~CGFile()
{
	Close();
}

void CGFile::Close()
{
	if (-1 != hf)
	{
		close(hf);
		memset(szFileName, 0x00, sizeof(szFileName));
		hf = -1;
	}
}

int CGFile::Open(const char *szFile)
{
	Close();
	if (-1 != (hf = open(szFile, O_RDONLY, 0766)))
		strcpy(szFileName, szFile);
	return hf;
}
/*
long CGFile::IDXReadLine(char *data, long maxDataLen)
{
	char c, *bufx = data;
	--maxDataLen;
	if (4 == addidx)
	{
		addidx = 0;
		strcpy(data, "IDX|");
		data += 4;
		maxDataLen -= 4;
	}
	while (--maxDataLen > 0)
	{
		if (--cnt <= 0)
		{
			if ((cnt = read(hf, b, sizeof(b))) > 0)
				bp = b;
			else if (cnt < 0)
			{
				if (EINTR == errno || ETIMEDOUT == errno || EAGAIN == errno || EWOULDBLOCK == errno)
				{
					maxDataLen++;	// the while will decrement
					continue;
				}
				return -1;
			}
			else //if (0 == cnt)
				return 0;
		}
		*data++ = c = *bp++;
		if ((data - bufx) < 30)
			continue;
		if (0x0a == c && 0x0d == *(data-2))
		{
			*(data) = 0x00;
			return data - bufx;
		}
		else
		{
			if ('I' == c)
				addidx = 1;
			else if (1 == addidx && 'D' == c)
				++addidx;
			else if (2 == addidx && 'X' == c)
				++addidx;
			else if (3 == addidx && '|' == c)
			{
				if ((data - bufx) > 30)
				{
					++addidx;
					*(data-4) = 0x0d;
					*(data-3) = 0x0a;
					*(data-2) = 0x00;
					return data - bufx - 2;
				}
				else
					addidx = 0;
			}
			else
				addidx = 0;
		}
	}
	return -1;
}

long CGFile::ReadLine(char *data, long maxDataLen)
{
	char *bufx = data;
	--maxDataLen;
	while (--maxDataLen > 0)
	{
		if (--cnt <= 0)
		{
			if ((cnt = read(hf, b, sizeof(b))) > 0)
				bp = b;
			else if (cnt < 0)
			{
				if (EINTR == errno || ETIMEDOUT == errno || EAGAIN == errno || EWOULDBLOCK == errno)
				{
					maxDataLen++;	// the while will decrement
					continue;
				}
				return -1;
			}
			else //if (0 == cnt)
				return 0;
		}
		if ('\n' == (*data++ = *bp++))
		{
			*(data) = 0x00;
			return data - bufx;
		}
	}
	return -1;
}
*/
long CGFile::IDXReadLine(char *data, long maxDataLen)
{
	static int cnt = 0;
	static char b[65536], *bp;
	char c, *bufx = data;
	while (--maxDataLen > 0)
	{
		if (--cnt <= 0)
		{
			if ((cnt = read(hf, b, sizeof(b))) > 0)
				bp = b;
			else
				return cnt;
		}
		*data++ = c = *bp++;
		if (0x0d == c && 0x0a == *bp)
		{
			*data++ = c = *bp++;
			*(data) = 0x00;
			--cnt;
			return data - bufx;
		}
	}
	return -1;
}

long CGFile::ReadLine(char *data, long maxDataLen)
{
	static int cnt = 0;
	static char b[65536], *bp;
	char *bufx = data;
	while (--maxDataLen > 0)
	{
		if (--cnt <= 0)
		{
			if ((cnt = read(hf, b, sizeof(b))) > 0)
				bp = b;
			else
				return cnt;
		}
		if ('\n' == (*data++ = *bp++))
		{
			*(data) = 0x00;
			return data - bufx;
		}
	}
	return -1;
}

long CGFile::ReadBlock(char *pszBuf, unsigned long nBytesToRead)
{
	long nTotBytesRead = 0, nBytesRead = 0;
	do
	{
		nBytesRead = read(hf, pszBuf + nTotBytesRead, nBytesToRead - nTotBytesRead);
		if (nBytesRead > 0)
			nTotBytesRead += nBytesRead;
		else
			break;
	} while ((unsigned long)nTotBytesRead < nBytesToRead);
	return (nTotBytesRead > 0)?nTotBytesRead:nBytesRead;
}
