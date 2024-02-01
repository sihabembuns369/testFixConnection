//#include "stdafx.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <snappy-c.h>
#include "header/IPackage.h"
#include "header/minilzo.h"
#include "header/RawPackage.h"

/*-------------------------------------------------------------------
Header Specification(Total Length 16)
4 bytes	: Packet Signature
4 bytes : Packet Length
2 bytes : Packet Id
2 bytes : Common flags
bit 0 : Encrypted
bit 1 : Compressed
2 bytes : Error Code
4 bytes : Reserved
----------------------------------------------------------------------*/
CIPackage::CIPackage() : 
	pszData(NULL), lBufferSize(0), lReadPos(0), lWritePos(0)
{
}

//CIPackage::CIPackage(CRawPackage *p) : 
//	pszData(NULL), lBufferSize(0), lWritePos(0), lReadPos(0)
//{
//	SetRawData(p);
//	memset(szIdentity, 0x00, sizeof(szIdentity));
//}
//
//CIPackage::CIPackage(const char *identity, CRawPackage *p) : 
//	pszData(NULL), lBufferSize(0), lWritePos(0), lReadPos(0)
//{
//	SetRawData(p);
//	strcpy(szIdentity, identity);
//}

CIPackage::CIPackage(const char *identity, CIPackage *p) : 
	pszData(new char[p->lBufferSize]), lBufferSize(p->lBufferSize), lReadPos(p->lReadPos), lWritePos(p->lWritePos)
{
	strcpy(szIdentity, identity);
	memcpy(pszData, p->pszData, lBufferSize);
}

CIPackage::CIPackage(OL_UINT32 lsize, OL_UINT16 packageId) : pszData(NULL)
{
	AllocateBuffer(lsize);
	SetPackageId(packageId);
}

CIPackage::CIPackage(char *pdata, OL_UINT32 dataSize) : 
	pszData(pdata), lBufferSize(dataSize), lReadPos(IHEADER_LENGTH), lWritePos(dataSize)
{
	UpdatePackageLength();
}

CIPackage::~CIPackage()
{
	ReleaseBuffer();
}

CIPackage::operator char*() const
{
	return pszData;
}

CIPackage::operator const char*() const
{
	return pszData;
}

char *CIPackage::operator + (OL_UINT32 pos)
{
	return pszData + pos;
}

CIPackage &CIPackage::operator =(const CIPackage &p)
{
	ReleaseBuffer();
	lBufferSize = p.lBufferSize;
	lReadPos = p.lReadPos;
	lWritePos = p.lWritePos;
	if (lBufferSize > IHEADER_LENGTH)
	{
		try
		{
			pszData = new char[lBufferSize];
		}
		catch (...)
		{
			pszData = NULL;
			lBufferSize = lReadPos = lWritePos = 0;
			return *this;
		}
		memcpy(pszData, p.pszData, lBufferSize);
	}
	return *this;
}

OL_UINT32 CIPackage::GetBufferSize() const
{
	return lBufferSize;
}

OL_UINT32 CIPackage::GetPackageLength() const
{
	return lWritePos;
}

OL_UINT32 CIPackage::GetReadPosition() const
{
	return lReadPos;
}

inline void CIPackage::UpdatePackageLength()
{
	OL_UINT32 nwrite = htonl(lWritePos);
	memcpy(pszData + SZ_INT32, &nwrite, SZ_INT32);
}

void CIPackage::ResetReadPosition()
{
	lReadPos = IHEADER_LENGTH;
}

void CIPackage::ResetWritePosition()
{
	lWritePos = IHEADER_LENGTH;
	UpdatePackageLength();
}

void CIPackage::MoveReadPosition(OL_INT32 step)
{
	OL_UINT32 newPos = lReadPos + step;
	if (newPos >= IHEADER_LENGTH && newPos <= lBufferSize)
		lReadPos = newPos;
}

void CIPackage::MoveWritePosition(OL_INT32 step)
{
	OL_UINT32 newPos = lWritePos + step;
	if (newPos >= IHEADER_LENGTH && newPos <= lBufferSize)
	{
		lWritePos = newPos;
		UpdatePackageLength();
	}
}

bool CIPackage::AllocateBuffer(OL_UINT32 bufSize)
{
	if (pszData)
	{
		delete []pszData; pszData = NULL;
	}
	lBufferSize = bufSize + IHEADER_LENGTH;
	try
	{
		pszData = new char[lBufferSize];
	}
	catch (...)
	{
		pszData = NULL;
		lBufferSize = lReadPos = lWritePos = 0;
		return false;
	}
	memset(pszData, 0x00, lBufferSize);
	lReadPos = htonl(IPACKAGE_SIGNATURE);
	memcpy(pszData, &lReadPos, SZ_INT32);	//	Packet Signature
	lReadPos = lWritePos = IHEADER_LENGTH;
	UpdatePackageLength();
	return true;
}

void CIPackage::ReleaseBuffer()
{
	if (pszData)
	{
		delete []pszData; pszData = NULL;
	}
	lBufferSize = lReadPos = lWritePos = 0;
}

//bool CIPackage::SetRawData(CRawPackage *pRawPackage)
//{
//	OL_UINT32 rawPacketLength = pRawPackage->GetDataLength();
//	if (rawPacketLength)
//	{
//		if (pszData)
//			delete[]pszData;
//		lBufferSize = lWritePos = rawPacketLength + IHEADER_LENGTH;
//		try
//		{
//			pszData = new char[lBufferSize];
//		}
//		catch (...)
//		{
//			pszData = NULL;
//			lBufferSize = lReadPos = lWritePos = 0;
//			return false;
//		}
//		memset(pszData, 0x00, lBufferSize);
//		SetPackageId(pRawPackage->GetPackageId());
//		SetErrorCode(pRawPackage->GetErrCode());
//		SetSourceId(pRawPackage->GetSourceId());
//		lReadPos = htonl(IPACKAGE_SIGNATURE);
//		memcpy(pszData, &lReadPos, 4);	//	Packet Signature
//		lReadPos = IHEADER_LENGTH;
//		pRawPackage->ExtractData(pszData + IHEADER_LENGTH);
//		UpdatePackageLength();
//	}
//	return true;
//}

void CIPackage::SetRawData(char *pszRawData, OL_UINT32 nRawDataLength)
{
	if (NULL != pszRawData && nRawDataLength >= IHEADER_LENGTH)
	{
		if (pszData)
			delete[]pszData;
		pszData = pszRawData;
		lReadPos = IHEADER_LENGTH;
		lBufferSize = lWritePos = nRawDataLength;
		UpdatePackageLength();
	}
}

bool CIPackage::CheckSignature()
{
	OL_UINT32 lsign = 0;
	memcpy(&lsign, pszData, SZ_INT32);
// 	lsign = ntohl(lsign);
// 	return lsign == IPACKAGE_SIGNATURE;
	return IPACKAGE_SIGNATURE == ntohl(lsign);
}

bool CIPackage::isEndofPacket()
{
	return lReadPos >= lBufferSize;
}

bool CIPackage::isEncrypted()
{
	OL_UINT16 flag;
	memcpy(&flag, pszData + 10, SZ_INT16);
	return ntohs(flag) & 0x0001;
}

bool CIPackage::isCompressed()
{
	OL_UINT16 flag;
	memcpy(&flag, pszData + 10, SZ_INT16);
	return 0 != (ntohs(flag) & 0x0002);
}

void CIPackage::SetPackageId(OL_UINT16 packageId)
{
	packageId = htons(packageId);
	memcpy(pszData + 8, &packageId, SZ_INT16);
}

void CIPackage::SetErrorCode(OL_UINT16 errCode)
{
	errCode = htons(errCode);
	memcpy(pszData + 12, &errCode, SZ_INT16);
}

void CIPackage::SetSourceId(OL_UINT8 srcId)
{
	pszData[14] = srcId;
}

void CIPackage::UpdatePackageTimeStamp()
{
	time_t ttime = time(NULL);
	struct tm *ptm = localtime(&ttime);
	OL_UINT32 ldate = htonl((ptm->tm_year + 1900) * 10000 + (ptm->tm_mon + 1) * 100 + ptm->tm_mday), 
			  ltime = htonl(ptm->tm_hour * 10000 + ptm->tm_min * 100 + ptm->tm_sec);
	memcpy(pszData + 15, &ldate, SZ_INT32);
	memcpy(pszData + 19, &ltime, SZ_INT32);
}

OL_UINT16 CIPackage::GetPackageId() const
{
	OL_UINT16 packageid;
	memcpy(&packageid, pszData + 8, SZ_INT16);
	return ntohs(packageid);
}

OL_UINT16 CIPackage::GetErrorCode() const
{
	OL_UINT16 errcode;
	memcpy(&errcode, pszData + 12, SZ_INT16);
	return ntohs(errcode);
}

OL_UINT8 CIPackage::GetSourceId() const
{
	return pszData[14];
}

OL_UINT32 CIPackage::GetPackageDate() const
{
	OL_UINT32 ldate;
	memcpy(&ldate, pszData + 15, SZ_INT32);
	return ntohl(ldate);
}

OL_UINT32 CIPackage::GetPackageTime() const
{
	OL_UINT32 ltime;
	memcpy(&ltime, pszData + 19, SZ_INT32);
	return ntohl(ltime);
}

void CIPackage::AddInt8(OL_UINT8 val)
{
//	ASSERT((lWritePos + SZ_INT8) <= lBufferSize);
	pszData[lWritePos] = val;
	lWritePos += SZ_INT8;
	UpdatePackageLength();
}

void CIPackage::AddInt16(OL_UINT16 val)
{
//	ASSERT((lWritePos + SZ_INT16) <= lBufferSize);
	val = htons(val);
	memcpy(pszData + lWritePos, &val, SZ_INT16);
	lWritePos += SZ_INT16;
	UpdatePackageLength();
}

void CIPackage::AddInt32(OL_UINT32 val)
{
//	ASSERT((lWritePos + SZ_INT32) <= lBufferSize);
	val = htonl(val);
	memcpy(pszData + lWritePos, &val, SZ_INT32);
	lWritePos += SZ_INT32;
	UpdatePackageLength();
}

void CIPackage::AddInt64(OL_UINT64 val)
{
//	ASSERT((lWritePos + SZ_INT64) <= lBufferSize);
	//val = htonll(val);
	val = htobe64(val);
	memcpy(pszData + lWritePos, &val, SZ_INT64);
	lWritePos += SZ_INT64;
	UpdatePackageLength();
}

//void CIPackage::AddFloat(float val)
//{
////	ASSERT((lWritePos + SZ_FLOAT) <= lBufferSize);
//	val = htonf(val);
//	memcpy(pszData + lWritePos, &val, SZ_FLOAT);
//	lWritePos += SZ_FLOAT;
//	UpdatePackageLength();
//}
//
//void CIPackage::AddDouble(double val)
//{
////	ASSERT((lWritePos + SZ_DOUBLE) <= lBufferSize);
//	val = htond(val);
//	memcpy(pszData + lWritePos, &val, SZ_DOUBLE);
//	lWritePos += SZ_DOUBLE;
//	UpdatePackageLength();
//}
//
//void CIPackage::AddLDouble(long double val)
//{
////	ASSERT((lWritePos + SZ_LDOUBLE) <= lBufferSize);
//	memcpy(pszData + lWritePos, &val, SZ_LDOUBLE);
//	lWritePos += SZ_LDOUBLE;
//	UpdatePackageLength();
//}

void CIPackage::AddString(const char *pstr)
{
	OL_UINT16 slen = strlen(pstr);
	AddInt16(slen);
	if (slen)
	{
		strncpy(pszData + lWritePos, pstr, slen);
		lWritePos += slen;
		UpdatePackageLength();
	}
}

void CIPackage::AddString(const char *pstr, OL_UINT16 stringlen)
{
	if (0 == stringlen)
		return;
//	ASSERT((lWritePos + stringlen) <= lBufferSize);
	strncpy(pszData + lWritePos, pstr, stringlen);
	lWritePos += stringlen;
	UpdatePackageLength();
}

void CIPackage::AddBinary(const char *pbinary, OL_UINT32 datalen)
{
	if (0 == datalen)
		return;
//	ASSERT((lWritePos + datalen) <= lBufferSize);
	memcpy(pszData + lWritePos, pbinary, datalen);
	lWritePos += datalen;
	UpdatePackageLength();
}

void CIPackage::FillBuffer(char ch, OL_UINT32 nBytes)
{
//	ASSERT((lWritePos + nBytes) <= lBufferSize);
	memset(pszData + lWritePos, ch, nBytes);
	lWritePos += nBytes;
	UpdatePackageLength();
}

void CIPackage::GetString(OL_UINT16 stringlen, char *pstr, ITRIM_STRING op)
{
//	ASSERT((lReadPos + stringlen) <= lBufferSize);
	if ((0 == stringlen) || ((lReadPos + stringlen) > lBufferSize))
	{
		*pstr = 0x00;
		return;
	}
	strncpy(pstr, pszData + lReadPos, stringlen);
	pstr[stringlen] = 0x00;
	lReadPos += stringlen;
	switch (op)
	{
	case ITRIM_NONE:
		break;
	case ITRIM_LEFT:
		TrimLeft(pstr);
		break;
	case ITRIM_RIGHT:
		TrimRight(pstr);
		break;
	case ITRIM_BOTH:
		TrimRight(pstr);
		TrimLeft(pstr);
		break;
	}
}

void CIPackage::GetBinary(OL_UINT32 datalen, char *pbinary)
{
//	ASSERT((lReadPos + datalen) <= lBufferSize);
	if ((0 == datalen) || ((lReadPos + datalen) > lBufferSize))
	{
		*pbinary = 0x00;
		return;
	}
	memcpy(pbinary, pszData + lReadPos, datalen);
	lReadPos += datalen;
}

OL_UINT8 CIPackage::GetInt8()
{
//	ASSERT((lReadPos + SZ_INT8) <= lBufferSize);
	if ((lReadPos + SZ_INT8) > lBufferSize)
		return 0;
	OL_UINT8 val = pszData[lReadPos];
	lReadPos += SZ_INT8;
	return val;
}

OL_UINT16 CIPackage::GetInt16()
{
//	ASSERT((lReadPos + SZ_INT16) <= lBufferSize);
	if ((lReadPos + SZ_INT16) > lBufferSize)
		return 0;
	OL_UINT16 val = 0;
	memcpy(&val, pszData + lReadPos, SZ_INT16);
	lReadPos += SZ_INT16;
	return ntohs(val);
}

OL_UINT32 CIPackage::GetInt32()
{
//	ASSERT((lReadPos + SZ_INT32) <= lBufferSize);
	if ((lReadPos + SZ_INT32) > lBufferSize)
		return 0;
	OL_UINT32 val = 0;
	memcpy(&val, pszData + lReadPos, SZ_INT32);
	lReadPos += SZ_INT32;
	return ntohl(val);
}

OL_UINT64 CIPackage::GetInt64()
{
//	ASSERT((lReadPos + SZ_INT64) <= lBufferSize);
	if ((lReadPos + SZ_INT64) > lBufferSize)
		return 0;
	OL_UINT64 val = 0;
	memcpy(&val, pszData + lReadPos, SZ_INT64);
	lReadPos += SZ_INT64;
	return be64toh(val);
	//return ntohll(val);
}

//float CIPackage::GetFloat()
//{
////	ASSERT((lReadPos + SZ_FLOAT) <= lBufferSize);
//	if ((lReadPos + SZ_FLOAT) > lBufferSize)
//		return 0;
//	float val = 0;
//	memcpy(&val, pszData + lReadPos, SZ_FLOAT);
//	lReadPos += SZ_FLOAT;
//	return ntohf(val);
//}
//
//double CIPackage::GetDouble()
//{
////	ASSERT((lReadPos + SZ_DOUBLE) <= lBufferSize);
//	if ((lReadPos + SZ_DOUBLE) > lBufferSize)
//		return 0;
//	double val = 0;
//	memcpy(&val, pszData + lReadPos, SZ_DOUBLE);
//	lReadPos += SZ_DOUBLE;
//	return ntohd(val);
//}
//
//long double CIPackage::GetLDouble()
//{
////	ASSERT((lReadPos + SZ_LDOUBLE) <= lBufferSize);
//	if ((lReadPos + SZ_LDOUBLE) > lBufferSize)
//		return 0;
//	long double val = 0;
//	memcpy(&val, pszData + lReadPos, SZ_LDOUBLE);
//	lReadPos += SZ_LDOUBLE;
//	return val;
//}

void CIPackage::TrimLeft(char *psz)
{
	register int pos = 0;
	while (' ' == psz[pos])
		++pos;
	if (pos)
		strcpy(psz, psz + pos);
}

void CIPackage::TrimRight(char *psz)
{
	register size_t pos = strlen(psz);
	if (pos)
		while (' ' == psz[--pos])
		{
			psz[pos] = 0x00;
			if (0 == pos) break;
		}
}

void CIPackage::CompressPackage()
{
	//CompressLzo();
	CompressSnappy();
}

void CIPackage::DecompressPackage()
{
	//DecompressLzo();
	DecompressSnappy();
}

void CIPackage::EncryptPackage()
{
	/*
	switch (alg)
	{
	case UP_CRYPTO_RIJNDAEL:
		EncryptRijndael();
		break;
	case UP_CRYPTO_BLOCK_CIPHER_SEED:
		EncryptBlockCipherSeed();
		break;
	}
	*/
}

void CIPackage::DecryptPackage()
{
	/*
	unsigned char alg = pszData[18];
	switch (alg)
	{
	case UP_CRYPTO_RIJNDAEL:
		DecryptRijndael();
		break;
	case UP_CRYPTO_BLOCK_CIPHER_SEED:
		DecryptBlockCipherSeed();
		break;
	}
	*/
}

void CIPackage::CompressSnappy()
{
	//if (lWritePos > IHEADER_LENGTH)
	//{
	//	OL_UINT16 flags = 0;
	//	memcpy(&flags, pszData + 10, SZ_INT16);
	//	if (!((flags = ntohs(flags)) & 0x0002))
	//	{
	//		size_t out_len = snappy_max_compressed_length(lWritePos - IHEADER_LENGTH);
	//		char *pcompressed = new char[out_len + IHEADER_LENGTH + SZ_INT32];
	//		if (SNAPPY_OK == snappy_compress(pszData + IHEADER_LENGTH, lWritePos - IHEADER_LENGTH, pcompressed + IHEADER_LENGTH + SZ_INT32, &out_len))
	//		{
	//			memcpy(pcompressed, pszData, IHEADER_LENGTH);
	//			delete[]pszData; pszData = pcompressed; pcompressed = NULL;
	//			lWritePos = htonl(lWritePos);
	//			memcpy(pszData + IHEADER_LENGTH, &lWritePos, SZ_INT32);
	//			lBufferSize = lReadPos = lWritePos = IHEADER_LENGTH + (UINT32)out_len + SZ_INT32;
	//			flags = htons(flags | 0x0002);
	//			memcpy(pszData + 10, &flags, SZ_INT16);
	//			UpdatePackageLength();
	//		}
	//	}
	//}
}

void CIPackage::DecompressSnappy()
{
	OL_UINT16 flags = 0;
	memcpy(&flags, pszData + 10, SZ_INT16);
	if ((flags = ntohs(flags)) & 0x0002)
	{
		OL_UINT32 nOriBufSize;
		memcpy(&nOriBufSize, pszData + IHEADER_LENGTH, SZ_INT32);
		nOriBufSize = ntohl(nOriBufSize);
		char *pDecompr = new char[nOriBufSize];
		memcpy(pDecompr, pszData, IHEADER_LENGTH);
		size_t out_len = nOriBufSize - IHEADER_LENGTH;
		if (SNAPPY_OK == snappy_uncompress(pszData + IHEADER_LENGTH + SZ_INT32, lWritePos - IHEADER_LENGTH - SZ_INT32, pDecompr + IHEADER_LENGTH, &out_len))
		{
			lReadPos = IHEADER_LENGTH;
			lBufferSize = lWritePos = nOriBufSize;
			delete[]pszData; pszData = pDecompr; pDecompr = NULL;
			flags = htons(flags & 0xfffd);
			memcpy(pszData + 10, &flags, SZ_INT16);
			UpdatePackageLength();
		}
	}
}

OL_UINT32 CIPackage::CalcComprLzoBufferUsage(OL_UINT32 nBytestoCompress)
{
	return nBytestoCompress + nBytestoCompress / 64 + 16 + 3;
}

void CIPackage::CompressLzo()
{
	if (lWritePos > IHEADER_LENGTH)
	{
		OL_UINT16 flags;
		memcpy(&flags, pszData + 10, SZ_INT16);
		flags = ntohs(flags);
		if (!(flags & 0x0002))
		{
			if (LZO_E_OK == lzo_init())
			{
				try
				{
					lzo_uint out_len;
					lzo_align_t __LZO_MMODEL wrk[LZO1X_1_MEM_COMPRESS + (sizeof(lzo_align_t) - 1) / sizeof(lzo_align_t)];
					char *pcompressed = new char[CalcComprLzoBufferUsage(lWritePos - IHEADER_LENGTH)];
					if (LZO_E_OK == lzo1x_1_compress((const unsigned char*)pszData + IHEADER_LENGTH,
						lWritePos - IHEADER_LENGTH, (unsigned char*)pcompressed, &out_len, wrk))
					{
						char *pT = new char[out_len + IHEADER_LENGTH + SZ_INT32];
						memcpy(pT, pszData, IHEADER_LENGTH);
						delete[]pszData; pszData = pT; pT = NULL;
						lWritePos = htonl(lWritePos);
						memcpy(pszData + IHEADER_LENGTH, &lWritePos, SZ_INT32);
						lBufferSize = lReadPos = lWritePos = IHEADER_LENGTH + out_len + SZ_INT32;
						memcpy(pszData + IHEADER_LENGTH + SZ_INT32, pcompressed, out_len);
						flags |= 0x0002;
						flags = htons(flags);
						memcpy(pszData + 10, &flags, SZ_INT16);
						UpdatePackageLength();
					}
					delete []pcompressed; pcompressed = NULL;
				}
				catch (...)
				{
					//TRACE("[CIPackage::CompressLzo] - Memory Exception.\n");
				}
			}
		}
	}
}

void CIPackage::DecompressLzo()
{
	OL_UINT16 flags;
	memcpy(&flags, pszData + 10, SZ_INT16);
	flags = ntohs(flags);
	if (flags & 0x0002)
	{
		if (LZO_E_OK == lzo_init())
		{
			lzo_uint nDecomprLen;
			OL_UINT32 nOriBufSize;
			memcpy(&nOriBufSize, pszData + IHEADER_LENGTH, SZ_INT32);
			nOriBufSize = ntohl(nOriBufSize);
			char *pDecompr = new char[nOriBufSize];
			memcpy(pDecompr, pszData, IHEADER_LENGTH);
			if (LZO_E_OK == lzo1x_decompress((const unsigned char*)pszData + IHEADER_LENGTH + SZ_INT32,
				lBufferSize - IHEADER_LENGTH - SZ_INT32, (unsigned char*)pDecompr + IHEADER_LENGTH, &nDecomprLen, NULL))
			{
				lReadPos = IHEADER_LENGTH;
				lBufferSize = lWritePos = nOriBufSize;
				delete[]pszData; pszData = pDecompr; pDecompr = NULL;
				flags &= 0xfffd;
				flags = htons(flags);
				memcpy(pszData + 10, &flags, SZ_INT16);
				UpdatePackageLength();
			}
			else
			{
				delete []pDecompr; pDecompr = NULL;
			}
		}
	}
}

void CIPackage::EncryptRijndael()
{
	/*
	if (lBufferSize > UP_HEADER_LENGTH)
	{
	UINT16 flags;
	memcpy(&flags, pszData + 14, SZ_INT16);
	flags = ntohs(flags);
	if (!(flags & 0x0001))
	{
	UINT32	lEncLen = rijndael.CalcBufEnc(lBufferSize - UP_HEADER_LENGTH) + UP_HEADER_LENGTH + SZ_INT32;
	char *pT, *pszenc = new char[lEncLen];
	memcpy(pszenc, pszData, UP_HEADER_LENGTH);
	pT = pszenc + UP_HEADER_LENGTH + SZ_INT32;
	rijndael.Encrypt(pszData + UP_HEADER_LENGTH, lBufferSize - UP_HEADER_LENGTH, &pT, lEncLen, 0);
	memcpy(pszenc + UP_HEADER_LENGTH, &lBufferSize, SZ_INT32);	//	Original Buffer Size
	lBufferSize = lReadPos = lWritePos = lEncLen + UP_HEADER_LENGTH + SZ_INT32;
	delete[]pszData; pszData = pszenc; pszenc = NULL;
	flags |= 0x0001; pszData[18] = UP_CRYPTO_RIJNDAEL;
	flags = htons(flags);
	memcpy(pszData + 14, &flags, SZ_INT16);
	UpdatePackageLength();
	}
	}
	*/
}

void CIPackage::EncryptBlockCipherSeed()
{
	/*
	if (lBufferSize > UP_HEADER_LENGTH)
	{
	UINT16 flags;
	memcpy(&flags, pszData + 14, SZ_INT16);
	flags = ntohs(flags);
	if (!(flags & 0x0001))
	{
	char *pszenc, *pT;
	UINT32 lEncLen;
	CBlockCipherSeed seed;
	seed.InitSeed(blockCipherSeedKey, bcsInitialVector);
	if (CTR_SUCCESS == seed.Encrypt(pszData + UP_HEADER_LENGTH, lBufferSize - UP_HEADER_LENGTH, pszenc, lEncLen, 1))
	{
	pT = new char[lEncLen + UP_HEADER_LENGTH + SZ_INT32];
	memcpy(pT, pszData, UP_HEADER_LENGTH);
	memcpy(pT + UP_HEADER_LENGTH + SZ_INT32, pszenc, lEncLen);
	memcpy(pT + UP_HEADER_LENGTH, &lBufferSize, SZ_INT32);
	delete[]pszenc; pszenc = NULL;
	delete[]pszData; pszData = pT; pT = NULL;
	lBufferSize = lReadPos = lWritePos = lEncLen + UP_HEADER_LENGTH + SZ_INT32;
	flags |= 0x0001; pszData[18] = UP_CRYPTO_BLOCK_CIPHER_SEED;
	flags = htons(flags);
	memcpy(pszData + 14, &flags, SZ_INT16);
	UpdatePackageLength();
	}
	}
	}
	*/
}

void CIPackage::DecryptRijndael()
{
	/*
	UINT16 flags;
	memcpy(&flags, pszData + 14, SZ_INT16);
	flags = ntohs(flags);
	if (flags & 0x0001)
	{
	char *pdec, *pT;
	UINT32 lDecLen;
	memcpy(&lDecLen, pszData + UP_HEADER_LENGTH, SZ_INT32);
	pdec = new char[lDecLen];
	memcpy(pdec, pszData, UP_HEADER_LENGTH);
	pT = pdec + UP_HEADER_LENGTH;
	rijndael.Decrypt(pszData + UP_HEADER_LENGTH + SZ_INT32, lBufferSize - UP_HEADER_LENGTH - SZ_INT32, &pT, lDecLen, 0);
	delete[]pszData; pszData = pdec; pdec = NULL;
	lReadPos = UP_HEADER_LENGTH;
	lBufferSize = lWritePos = lDecLen + UP_HEADER_LENGTH;
	flags &= 0xfffe; pszData[18] = 0x00;
	flags = htons(flags);
	memcpy(pszData + 14, &flags, SZ_INT16);
	UpdatePackageLength();
	}
	*/
}

void CIPackage::DecryptBlockCipherSeed()
{
	/*
	UINT16 flags;
	memcpy(&flags, pszData + 14, SZ_INT16);
	flags = ntohs(flags);
	if (flags & 0x0001)
	{
	CBlockCipherSeed seed;
	if (CTR_SUCCESS == seed.InitSeed(blockCipherSeedKey, bcsInitialVector))
	{
	char *pdec, *pT;
	UINT32 lDecLen;
	memcpy(&lDecLen, pszData + UP_HEADER_LENGTH, SZ_INT32);
	pdec = new char[lDecLen];
	pT = pdec + UP_HEADER_LENGTH;
	if (CTR_SUCCESS == seed.Decrypt(pszData + UP_HEADER_LENGTH + SZ_INT32, lBufferSize - UP_HEADER_LENGTH - SZ_INT32, pT, lDecLen, 0))
	{
	memcpy(pdec, pszData, UP_HEADER_LENGTH);
	delete[]pszData; pszData = pdec; pdec = NULL;
	lReadPos = UP_HEADER_LENGTH;
	lBufferSize = lWritePos = lDecLen + UP_HEADER_LENGTH;
	flags &= 0xfffe; pszData[18] = 0x00;
	flags = htons(flags);
	memcpy(pszData + 14, &flags, SZ_INT16);
	UpdatePackageLength();
	}
	else
	delete[]pdec;
	}
	}
	*/
}

