#include "header/SoupBin.h"
#include "header/RawPackage.h"
#include <stdio.h>
#include "header/CMCrypt.h"
#include "header/minilzo.h"

CSoupBin::CSoupBin() : 
	pszData(NULL), lBufferSize(0), lReadPos(0), lWritePos(0)
{
	szIdentity[0] = 0x00;
}

CSoupBin::CSoupBin(CRawPackage *praw) : 
	pszData(NULL), lBufferSize(0), lReadPos(0), lWritePos(0)
{
	SetRawData(praw);
	szIdentity[0] = 0x00;
}

CSoupBin::CSoupBin(OL_UINT16 pkgSize, OL_UINT8 pkgId) : 
	pszData(NULL)
{
	AllocateBuffer(pkgSize);
	SetPackageId(pkgId);
	szIdentity[0] = 0x00;
}

CSoupBin::CSoupBin(char *pdat, OL_UINT16 pkgSize, const char *identity, bool copyData)
{
	if (pkgSize < SB_HEADER_LENGTH)
	{
		pszData = NULL;
		lBufferSize = lReadPos = lWritePos = 0;
		return;
	}
	lReadPos = SB_HEADER_LENGTH;
	lBufferSize = lWritePos = pkgSize;
	if (copyData)
	{
		pszData = new char[lBufferSize];
		memcpy(pszData, pdat, pkgSize);
	}
	else
		pszData = pdat;
	UpdatePackageLength();
	if (identity)
		strcpy(szIdentity, identity);
	else
		szIdentity[0] = 0x00;
}

CSoupBin::~CSoupBin()
{
//	printf("CSoupBin::~CSoupBin()\n");
	ReleaseBuffer();
}

CSoupBin &CSoupBin::operator = (const CSoupBin &p)
{
	ReleaseBuffer();
	if (p.lBufferSize >= SB_HEADER_LENGTH)
	{
		lBufferSize = p.lBufferSize;
		lReadPos = p.lReadPos;
		lWritePos = p.lWritePos;
		pszData = new char[lBufferSize];
		memcpy(pszData, p.pszData, lBufferSize);
	}
	return *this;
}

void CSoupBin::ResetWritePosition()
{
	lWritePos = SB_HEADER_LENGTH;
	UpdatePackageLength();
}

void CSoupBin::MoveReadPosition(OL_INT16 step)
{
	OL_INT32 newPos = lReadPos + step;
	if (newPos >= SB_HEADER_LENGTH && newPos <= lBufferSize)
		lReadPos = newPos;
}

void CSoupBin::MoveWritePosition(OL_INT16 step)
{
	OL_INT32 newPos = lWritePos + step;
	if (newPos >= SB_HEADER_LENGTH && newPos <= lBufferSize)
	{
		lWritePos = newPos;
		UpdatePackageLength();
	}
}

void CSoupBin::UpdatePackageLength()
{
	OL_UINT16 pkgLen = htons(lWritePos - SZ_INT16);
	memcpy(pszData, &pkgLen, SZ_INT16);
}

void CSoupBin::AllocateBuffer(OL_UINT16 bufSize)
{
	if (pszData)
		delete []pszData;
	pszData = new char[lBufferSize = (SB_HEADER_LENGTH + bufSize)];
	memset(pszData, 0x00, lBufferSize);
	lReadPos = lWritePos = SB_HEADER_LENGTH;
	UpdatePackageLength();
}

void CSoupBin::ReleaseBuffer()
{
	if (pszData)
	{
//		printf("Delete pszData.\n");
		delete []pszData; pszData = NULL;
	}
	lBufferSize = lWritePos = lReadPos = 0;
}

bool CSoupBin::SetRawData(CRawPackage *praw)
{
	OL_UINT16 rawPackageLength = praw->GetDataLength();
	if (0 == rawPackageLength)
		return false;
	if (pszData)
		delete []pszData;
	pszData = new char[lBufferSize = lWritePos = (lReadPos = SB_HEADER_LENGTH) + rawPackageLength];
	SetPackageId(praw->GetPackageId());
	praw->ExtractData(pszData + SB_HEADER_LENGTH);
	UpdatePackageLength();
	return true;
}

void CSoupBin::SetRawData(char *pszRaw, OL_UINT16 nRawDataLength)
{
	if ((NULL == pszRaw) || (nRawDataLength < SB_HEADER_LENGTH))
		return;
	if (pszData)
		delete []pszData;
	pszData = pszRaw;
	lReadPos = SB_HEADER_LENGTH;
	lBufferSize = lWritePos = nRawDataLength;
	UpdatePackageLength();
}

void CSoupBin::SetPackageId(OL_UINT8 pkgId) 
{
	pszData[2] = pkgId;
}

void CSoupBin::SetIdentity(const char *szId)
{
	strcpy(szIdentity, szId);
}

// const char *CSoupBin::GetIdentity() const
// {
// 	return szIdentity;
// }

// size_t CSoupBin::GetIdentityLength() const
// {
// 	return strlen(szIdentity);
// }

void CSoupBin::ComposeLogoutRequest()
{
	if (pszData)
		delete [] pszData;
	pszData = new char[lBufferSize = lReadPos = lWritePos = SB_HEADER_LENGTH];
	pszData[2] = SB_PKG_LOGOUT_REQ;
	UpdatePackageLength();
}

void CSoupBin::ComposeClientHeartBeat()
{
	if (pszData)
		delete [] pszData;
	pszData = new char[lBufferSize = lReadPos = lWritePos = SB_HEADER_LENGTH];
	pszData[2] = SB_PKG_CLIENT_HEART_BEAT;
	UpdatePackageLength();
}

void CSoupBin::ComposeLoginRequest(const char *uid, const char *pwd, const char *session, OL_UINT64 seq)
{
	if (pszData)
		delete [] pszData;
	pszData = new char[lBufferSize = (lWritePos = lReadPos = SB_HEADER_LENGTH) + 46];
	pszData[2] = SB_PKG_LOGIN_REQUEST;
	AddString(uid, 6);
	AddString(pwd, 10);
	AddString(session, 10);
	char szT[32] = {};
	sprintf(szT, "%20lu", seq);
	AddString(szT, 20);
}

void CSoupBin::AddInt8(OL_UINT8 val)
{
	memcpy(pszData + lWritePos, &val, SZ_INT8);
	lWritePos += SZ_INT8;
	UpdatePackageLength();
}

void CSoupBin::AddInt16(OL_UINT16 val)
{
	val = htons(val);
	memcpy(pszData + lWritePos, &val, SZ_INT16);
	lWritePos += SZ_INT16;
	UpdatePackageLength();
}

void CSoupBin::AddInt32(OL_UINT32 val)
{
	val = htonl(val);
	memcpy(pszData + lWritePos, &val, SZ_INT32);
	lWritePos += SZ_INT32;
	UpdatePackageLength();
}

void CSoupBin::AddInt64(OL_UINT64 val)
{
	val = htobe64(val);
	memcpy(pszData + lWritePos, &val, SZ_INT64);
	lWritePos += SZ_INT64;
	UpdatePackageLength();
}

void CSoupBin::AddString(const char *pstr, OL_UINT16 strLen)
{
	strncpy(pszData + lWritePos, pstr, strLen);
	lWritePos += strLen;
	UpdatePackageLength();
}

void CSoupBin::AddBinary(const char *pbin, OL_UINT16 dataLen)
{
	memcpy(pszData + lWritePos, pbin, dataLen);
	lWritePos += dataLen;
	UpdatePackageLength();
}

void CSoupBin::GetString(char *pstr)
{
	size_t slen = strlen(pszData + lReadPos);
	
	if ((0 == slen) || ((lReadPos + slen) > lBufferSize))
	{
		*pstr = 0x00;
		lReadPos += slen + 1;
		return;
	}
	strcpy(pstr, pszData + lReadPos);
	lReadPos += slen + 1;
}

void CSoupBin::GetString(OL_UINT16 slen, char *pstr, SB_TRIM_STRING op)
{
	if ((0 == slen) || ((lReadPos + slen) > lBufferSize))
	{
		*pstr = 0x00;
		return;
	}
	strncpy(pstr, pszData + lReadPos, slen);
	pstr[slen] = 0x00;
	lReadPos += slen;
	switch (op)
	{
	case SB_TRIM_NONE:
		break;
	case SB_TRIM_LEFT:
		TrimLeft(pstr);
		break;
	case SB_TRIM_RIGHT:
		TrimRight(pstr);
		break;
	case SB_TRIM_BOTH:
		TrimRight(pstr);
		TrimLeft(pstr);
		break;
	}
}

void CSoupBin::GetBinary(OL_UINT16 dataLen, char *pbin)
{
	memcpy(pbin, pszData + lReadPos, dataLen);
	lReadPos += dataLen;
}

OL_UINT8 CSoupBin::GetInt8()
{
	if ((lReadPos + SZ_INT8) > lBufferSize)
		return 0;
	return pszData[lReadPos++];
}

OL_UINT16 CSoupBin::GetInt16()
{
	if ((lReadPos + SZ_INT16) > lBufferSize)
		return 0;
	OL_UINT16 val = 0;
	memcpy(&val, pszData + lReadPos, SZ_INT16);
	lReadPos += SZ_INT16;
	//return ntohs(val);
	return be16toh(val);
}

OL_UINT32 CSoupBin::GetInt32()
{
	if ((lReadPos + SZ_INT32) > lBufferSize)
		return 0;
	OL_UINT32 val = 0;
	memcpy(&val, pszData + lReadPos, SZ_INT32);
	lReadPos += SZ_INT32;
	//return ntohl(val);
	return be32toh(val);
}

OL_UINT64 CSoupBin::GetInt64()
{
	if ((lReadPos + SZ_INT64) > lBufferSize)
		return 0;
	OL_UINT64 val = 0;
	memcpy(&val, pszData + lReadPos, SZ_INT64);
	lReadPos += SZ_INT64;
	return be64toh(val);
}

void CSoupBin::TrimLeft(char *psz)
{
	register OL_UINT16 pos = 0;
	while (' ' == psz[pos])
		++pos;
	if (pos)
		strcpy(psz, psz + pos);
}

void CSoupBin::TrimRight(char *psz)
{
	register OL_UINT16 pos = strlen(psz);
	if (pos)
		while (' ' == psz[--pos])
		{
			psz[pos] = 0x00;
			if (0 == pos)
				break;
		}
}
