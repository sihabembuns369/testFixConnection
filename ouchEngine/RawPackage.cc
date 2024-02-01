#include "header/RawPackage.h"
#include "header/CMCrypt.h"
#include "header/miscfunc.h"
#include <cstdlib>
#include <stdlib.h>
const OL_UINT32 GP_HEADER_LENGTH = 18;
const OL_UINT32 GP_PACKAGE_SIGNATURE = 0x956cd89a;

CRawPackage::~CRawPackage()
{
	Release();
}

void CRawPackage::Release()
{
	for (_RAW_DATA_LIST_::const_iterator i = rawdata.begin(); i != rawdata.end(); ++i)
		delete *i;
	totDataLen = 0;
	rawdata.clear();
}

void CRawPackage::AddInt8(OL_UINT8 val)
{
	totDataLen += SZ_INT8;
	rawdata.push_back(new CRawData(&val, SZ_INT8));
}

void CRawPackage::AddInt16(OL_UINT16 val)
{
	val = htons(val);
	totDataLen += SZ_INT16;
	rawdata.push_back(new CRawData(&val, SZ_INT16));
}

void CRawPackage::AddInt32(OL_UINT32 val)
{
	val = htonl(val);
	totDataLen += SZ_INT32;
	rawdata.push_back(new CRawData(&val, SZ_INT32));
}

void CRawPackage::AddInt64(OL_UINT64 val)
{
	val = htobe64(val);
	totDataLen += SZ_INT64;
	rawdata.push_back(new CRawData(&val, SZ_INT64));
}

void CRawPackage::AddFloat(float val)
{
	// val = htonf(val);
	totDataLen += SZ_FLOAT;
	OL_UINT32 v = htonl(*((OL_UINT32 *)&val));
	rawdata.push_back(new CRawData(&v, SZ_INT32));
}

void CRawPackage::AddDouble(double val)
{
	// val = htond(val);
	totDataLen += SZ_DOUBLE;
	OL_UINT64 v = htobe64(*((uint64_t *)&val));
	rawdata.push_back(new CRawData(&v, SZ_INT64));
}

void CRawPackage::AddString(const char *psz)
{
	OL_UINT16 slen = strlen(psz);
	AddInt16(slen);
	if (slen)
	{
		totDataLen += slen;
		rawdata.push_back(new CRawData(psz, slen));
	}
}

void CRawPackage::AddFixedString(const char *psz, OL_UINT32 nLen)
{
	if (0 == nLen)
		return;
	totDataLen += nLen;
	rawdata.push_back(new CRawData(psz, nLen));
}

void CRawPackage::AddBinary(const char *pbin, OL_UINT32 nlen)
{
	if (0 == nlen)
		return;
	totDataLen += nlen;
	rawdata.push_back(new CRawData(pbin, nlen));
}

// void CRawPackage::AddBinary(const char *pbin, OL_UINT32 nlen)
// {
// 	AddInt32(nlen);
// 	if (nlen)
// 	{
// 		totDataLen += nlen;
// 		rawdata.push_back(new CRawData(pbin, nlen));
// 	}
// }

OL_UINT32 *CRawPackage::StartArrCountingPoint()
{
	CRawData *p = new CRawData(SZ_INT32);
	totDataLen += SZ_INT32;
	rawdata.push_back(p);
	OL_UINT32 *pi = (OL_UINT32 *)p->prawdata;
	*pi = 0;
	return pi;
}

void CRawPackage::EndArrCountingPoint(OL_UINT32 *pc)
{
	*pc = htonl(*pc);
}

OL_UINT32 CRawPackage::ExtractData(char *pszData)
{
	OL_UINT32 nBytesExtracted = 0;
	for (_RAW_DATA_LIST_::const_iterator i = rawdata.begin(); i != rawdata.end(); ++i)
	{
		memcpy(pszData + nBytesExtracted, (*i)->prawdata, (*i)->datalen);
		nBytesExtracted += (*i)->datalen;
	}
	return nBytesExtracted;
}

OL_UINT32 CRawPackage::GetDataLengthWithHeader() const
{
	return totDataLen + GP_HEADER_LENGTH;
}

OL_UINT32 CRawPackage::ExtractDataWithHeader(char *pszData)
{
	memset(pszData, 0x00, GP_HEADER_LENGTH);
	OL_UINT32 pkgLen = htonl(totDataLen + GP_HEADER_LENGTH), pkgSig = htonl(GP_PACKAGE_SIGNATURE);
	memcpy(pszData, &pkgSig, SZ_INT32);
	memcpy(pszData + 4, &pkgLen, SZ_INT32);
	memcpy(pszData + 8, &packageId, SZ_INT16);
	memcpy(pszData + 12, &errCode, SZ_INT16);
	ExtractData(pszData + GP_HEADER_LENGTH);
	return totDataLen + GP_HEADER_LENGTH;
}

OL_UINT32 CRawPackage::ExtractEncryptedData(char **pszData)
{
	CMCrypt mc(encryptionKey, strlen(encryptionKey), encryptionIV);
	OL_UINT32 lEncLen = mc.CalcEncBufferSize(totDataLen),
			  lBufferSize = lEncLen + GP_HEADER_LENGTH + SZ_INT32,
			  lExtractedSize = lBufferSize,
			  pkgSig = htonl(GP_PACKAGE_SIGNATURE);
	OL_UINT16 fl = htons(0x0001);
	*pszData = (char *)malloc(sizeof(char) * lBufferSize);
	lBufferSize = htonl(lBufferSize);
	memset(*pszData, 0x00, GP_HEADER_LENGTH + SZ_INT32);
	memcpy(*pszData, &pkgSig, SZ_INT32);
	memcpy(*pszData + 4, &lBufferSize, SZ_INT32);
	memcpy(*pszData + 8, &packageId, SZ_INT16);
	memcpy(*pszData + 10, &fl, SZ_INT16);
	memcpy(*pszData + 12, &errCode, SZ_INT16);
	if (lEncLen > totDataLen)
		memset(*pszData + GP_HEADER_LENGTH + SZ_INT32 + totDataLen, 0x00, lEncLen - totDataLen);
	// totDataLen = htonl(totDataLen);
	// memcpy(*pszData + GP_HEADER_LENGTH, &totDataLen, SZ_INT32);
	uint32_t writePos = htonl(totDataLen + GP_HEADER_LENGTH);
	memcpy(*pszData + GP_HEADER_LENGTH, &writePos, SZ_INT32);
	ExtractData(*pszData + GP_HEADER_LENGTH + SZ_INT32);
	mc.FastEncrypt(*pszData + GP_HEADER_LENGTH + SZ_INT32, lEncLen);
	return lExtractedSize;
}
