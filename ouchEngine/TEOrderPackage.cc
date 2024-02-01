#include "header/TEOrderPackage.h"
#include "header/RawPackage.h"

//--------------------------------------------------------------------
//	TE Order Packet Signature
//--------------------------------------------------------------------
//const OL_UINT32 TE_PACKET_SIGNATURE = 0xabc5adec;
const OL_UINT32 TE_PACKET_SIGNATURE = 0xb92e59b0;
//--------------------------------------------------------------------

const uint16_t FL_PACKAGE_ENCRYPTED 	= 0x0001;
const uint16_t FL_PACKAGE_COMPRESSED	= 0x0002;
const uint16_t FL_PACKAGE_PRIORITY		= 0x0004;

/*--------------------------------------------------------------------*/
CTEOrderPackage::CTEOrderPackage():fd(0), pszData(NULL), lBufferSize(0), lReadPos(0), lWritePos(0)
{
}

CTEOrderPackage::CTEOrderPackage(CRawPackage *praw):
	pszData(NULL), lBufferSize(0), lReadPos(0), lWritePos(0)
{
	SetRawData(praw);
}

CTEOrderPackage::CTEOrderPackage(OL_UINT32 bufSize, OL_UINT16 packageId):fd(0), pszData(NULL), lBufferSize(0), lReadPos(0), lWritePos(0)
{
	AllocateBuffer(bufSize);
	SetPackageId(packageId);
}

CTEOrderPackage::~CTEOrderPackage()
{
	ReleaseBuffer();
}

void CTEOrderPackage::AllocateBuffer(OL_UINT32 bufSize)
{
	ReleaseBuffer();
	lBufferSize = TE_HEADER_LENGTH + bufSize;
	pszData = new char[lBufferSize];
	if (pszData)
	{
		OL_UINT32 lsign = htonl(TE_PACKET_SIGNATURE);
		lReadPos = lWritePos = TE_HEADER_LENGTH;
		memset(pszData, 0x00, lBufferSize);
		memcpy(pszData + 8, &lsign, SZ_DWORD);
		UpdatePacketLength();
	}
}

void CTEOrderPackage::ReleaseBuffer()
{
	if (pszData)
	{
		delete [] pszData; pszData = NULL;
		lBufferSize = lReadPos = lWritePos = 0;
	}
}

void CTEOrderPackage::UpdatePacketLength()
{
	OL_UINT32 nLen = htonl(lWritePos);
	memcpy(pszData, &nLen, SZ_DWORD);
}

bool CTEOrderPackage::SetRawData(CRawPackage *praw)
{
	OL_UINT32 rawPackageLength = praw->GetDataLength();
	if (0 == rawPackageLength)
		return false;
	if (pszData)
		delete []pszData;
	lReadPos = TE_HEADER_LENGTH;
	lBufferSize = lWritePos = TE_HEADER_LENGTH + rawPackageLength;
	pszData = new char[lBufferSize];
	memset(pszData, 0x00, TE_HEADER_LENGTH);
	SetPackageId(praw->GetPackageId());
	SetErrorCode(praw->GetErrorCode());
	OL_UINT32 pkgSig = htonl(TE_PACKET_SIGNATURE);
	memcpy(pszData, &pkgSig, SZ_INT32);
	praw->ExtractData(pszData + TE_HEADER_LENGTH);
	UpdatePacketLength();
	return true;
}

void CTEOrderPackage::SetRawData(char *pszRawData, OL_UINT32 nRawDataLength)
{
	if (NULL != pszRawData && nRawDataLength >= TE_HEADER_LENGTH)
	{
		ReleaseBuffer();
		pszData = pszRawData;
		lReadPos = TE_HEADER_LENGTH;
		lBufferSize = lWritePos = nRawDataLength;
		UpdatePacketLength();
	}
}

void CTEOrderPackage::MoveReadPosition(OL_INT32 step)
{
	OL_UINT32 newPos = lReadPos + step;
	if (newPos >= TE_HEADER_LENGTH && newPos <= lBufferSize)
		lReadPos = newPos;
}

void CTEOrderPackage::MoveWritePosition(OL_INT32 step)
{
	OL_UINT32 newPos = lWritePos + step;
	if (newPos >= TE_HEADER_LENGTH && newPos <= lBufferSize)
		lWritePos = newPos;
}

unsigned char CTEOrderPackage::isEndofPackage() const
{
	return lReadPos >= lBufferSize;
}

unsigned char CTEOrderPackage::isEncrypted() const
{
	OL_UINT16 flag;
	memcpy(&flag, pszData + 14, SZ_WORD);
	return ntohs(flag) & FL_PACKAGE_ENCRYPTED;
}

unsigned char CTEOrderPackage::isCompressed() const
{
	OL_UINT16 flag;
	memcpy(&flag, pszData + 14, SZ_WORD);
	return ntohs(flag) & FL_PACKAGE_COMPRESSED;
}

bool CTEOrderPackage::isFirstPriority() const
{
	uint16_t flag = 0;
	memcpy(&flag, pszData + 14, SZ_WORD);
	return ntohs(flag) & FL_PACKAGE_PRIORITY;
}

unsigned char CTEOrderPackage::CheckSignature() const
{
	OL_UINT32 lsign;
	memcpy(&lsign, pszData + 8, SZ_DWORD);
	return ntohl(lsign) == TE_PACKET_SIGNATURE;
}

void CTEOrderPackage::SetPackageId(OL_UINT16 packageId)
{
	packageId = htons(packageId);
	memcpy(pszData + 12, &packageId, SZ_WORD);
}

OL_UINT16 CTEOrderPackage::GetPackageId() const
{
	OL_UINT16 packetId;
	memcpy(&packetId, pszData + 12, SZ_WORD);
	return ntohs(packetId);
}

void CTEOrderPackage::SetErrorCode(OL_UINT16 errCode)
{
	errCode = htons(errCode);
	memcpy(pszData + 16, &errCode, SZ_WORD);
}

OL_UINT16 CTEOrderPackage::GetErrorCode() const
{
	OL_UINT16 errCode;
	memcpy(&errCode, pszData + 16, SZ_WORD);
	return ntohs(errCode);
}

void CTEOrderPackage::SetServerFd(OL_UINT32 fd)
{
	fd = htonl(fd);
	memcpy(pszData + 20, &fd, SZ_INT32);
}

OL_UINT32 CTEOrderPackage::GetServerFd()
{
	OL_UINT32 fd;
	memcpy(&fd, pszData + 20, SZ_INT32);
	return ntohl(fd);
}

void CTEOrderPackage::AddByte(unsigned char val)
{
	if ((lWritePos + SZ_BYTE) <= lBufferSize)
	{
		memcpy(pszData + lWritePos, &val, SZ_BYTE);
		lWritePos += SZ_BYTE;
		UpdatePacketLength();
	}
}

void CTEOrderPackage::AddWord(OL_UINT16 val)
{
	if ((lWritePos + SZ_WORD) <= lBufferSize)
	{
		val = htons(val);
		memcpy(pszData + lWritePos, &val, SZ_WORD);
		lWritePos += SZ_WORD;
		UpdatePacketLength();
	}
}

void CTEOrderPackage::AddDWord(OL_UINT32 val)
{
	if ((lWritePos + SZ_DWORD) <= lBufferSize)
	{
		val = htonl(val);
		memcpy(pszData + lWritePos, &val, SZ_DWORD);
		lWritePos += SZ_DWORD;
		UpdatePacketLength();
	}
}

void CTEOrderPackage::AddInt64(OL_UINT64 val)
{
	if ((lWritePos + SZ_INT64) <= lBufferSize)
	{
		val = htobe64(val);
		memcpy(pszData + lWritePos, &val, SZ_INT64);
		lWritePos += SZ_INT64;
		UpdatePacketLength();
	}
}

void CTEOrderPackage::AddDouble(double val)
{
	if ((lWritePos + SZ_DOUBLE) <= lBufferSize)
	{
		OL_UINT64 v = htobe64(*((OL_UINT64*)&val));
		memcpy(pszData + lWritePos, &v, SZ_INT64);
		lWritePos += SZ_INT64;
		UpdatePacketLength();
	}
}

void CTEOrderPackage::AddString(const char *pstr)
{
	OL_UINT16 slen = strlen(pstr);
	if ((lWritePos + slen + SZ_INT16) <= lBufferSize)
	{
		AddWord(slen);
		if (slen > 0)
		{
			strncpy(pszData + lWritePos, pstr, slen);
			lWritePos += slen;
			UpdatePacketLength();
		}
	}
}

void CTEOrderPackage::AddString(const char *pstr, OL_UINT32 stringlen)
{
	if ((lWritePos + stringlen) <= lBufferSize)
	{
		strncpy(pszData + lWritePos, pstr, stringlen);
		lWritePos += stringlen;
		UpdatePacketLength();
	}
}

void CTEOrderPackage::AddBinary(const char *pbinary, OL_UINT32 datalen)
{
	if ((lWritePos + datalen) <= lBufferSize)
	{
		memcpy(pszData + lWritePos, pbinary, datalen);
		lWritePos += datalen;
		UpdatePacketLength();
	}
}

void CTEOrderPackage::FillBuffer(char ch, OL_UINT32 nBytes)
{
	if ((lWritePos + nBytes) <= lBufferSize)
	{
		memset(pszData + lWritePos, ch, nBytes);
		lWritePos += nBytes;
		UpdatePacketLength();
	}
}

void CTEOrderPackage::GetString(OL_UINT32 stringlen, char *pstr, GP_TRIM_STRING op)
{
	if ((lReadPos + stringlen) > lBufferSize)
	{
		*pstr = 0x00;
		return;
	}
	pstr[stringlen] = 0x00;
	if (stringlen > 0)
	{
		strncpy(pstr, pszData + lReadPos, stringlen);
		lReadPos += stringlen;
		switch (op)
		{
		case GP_TRIM_NONE:
			break;
		case GP_TRIM_LEFT:
			TrimLeft(pstr);
			break;
		case GP_TRIM_RIGHT:
			TrimRight(pstr);
			break;
		case GP_TRIM_BOTH:
			TrimRight(pstr);
			TrimLeft(pstr);
			break;
		}
	}
}

void CTEOrderPackage::GetBinary(OL_UINT32 datalen, char *pbinary)
{
	if ((lReadPos + datalen) <= lBufferSize)
	{
		memcpy(pbinary, pszData + lReadPos, datalen);
		lReadPos += datalen;
	}
}

unsigned char CTEOrderPackage::GetByte()
{
	if ((lReadPos + SZ_BYTE) > lBufferSize)
		return 0;
	unsigned char val;
	memcpy(&val, pszData + lReadPos, SZ_BYTE);
	lReadPos += SZ_BYTE;
	return val;
}

OL_UINT16 CTEOrderPackage::GetWord()
{
	if ((lReadPos + SZ_WORD) > lBufferSize)
		return 0;
	OL_UINT16 val;
	memcpy(&val, pszData + lReadPos, SZ_WORD);
	lReadPos += SZ_WORD;
	return ntohs(val);
}

OL_UINT32 CTEOrderPackage::GetDWord()
{
	if ((lReadPos + SZ_DWORD) > lBufferSize)
		return 0;
	OL_UINT32 val;
	memcpy(&val, pszData + lReadPos, SZ_DWORD);
	lReadPos += SZ_DWORD;
	return ntohl(val);
}

OL_UINT64 CTEOrderPackage::GetInt64()
{
	if ((lReadPos + SZ_INT64) > lBufferSize)
		return 0;
	OL_UINT64 val;
	memcpy(&val, pszData + lReadPos, SZ_INT64);
	lReadPos += SZ_INT64;
	return be64toh(val);
}

double CTEOrderPackage::GetDouble()
{
	if ((lReadPos + SZ_INT64) > lBufferSize)
		return 0;
	OL_UINT64 val;
	memcpy(&val, pszData + lReadPos, SZ_INT64);
	lReadPos += SZ_INT64;
	val = be64toh(val);
	return *((double*)&val);
}

void CTEOrderPackage::TrimLeft(char *psz)
{
	register int pos = 0;
	while (' ' == psz[pos])
		++pos;
	if (pos)
		strcpy(psz, psz + pos);
}

void CTEOrderPackage::TrimRight(char *psz)
{
	register int pos = strlen(psz);
	if (pos)
		while (' ' == psz[--pos])
		{
			psz[pos] = 0x00;
			if (pos == 0) break;
		}
}

int CTEOrderPackage::isBigEndian()
{
	union {long l; char c[sizeof(long)];} u;
	u.l = 1;
	return (1 == u.c[sizeof(long) - 1]);
}
/*
OL_UINT64 CTEOrderPackage::htonll(OL_UINT64 hostval)
{
	if (!isBigEndian())
	{
		OL_UINT64 retVal;
		char srcbuf[SZ_INT64], descbuf[SZ_INT64];
		memcpy(srcbuf, &hostval, SZ_INT64);
		descbuf[0] = srcbuf[7];
		descbuf[1] = srcbuf[6];
		descbuf[2] = srcbuf[5];
		descbuf[3] = srcbuf[4];
		descbuf[4] = srcbuf[3];
		descbuf[5] = srcbuf[2];
		descbuf[6] = srcbuf[1];
		descbuf[7] = srcbuf[0];
		memcpy(&retVal, descbuf, SZ_INT64);
		return retVal;
	}
	return hostval;
}

OL_UINT64 CTEOrderPackage::ntohll(OL_UINT64 netval)
{
	if (!isBigEndian())
	{
		OL_UINT64 retVal;
		char srcbuf[SZ_INT64], descbuf[SZ_INT64];
		memcpy(srcbuf, &netval, SZ_INT64);
		descbuf[0] = srcbuf[7];
		descbuf[1] = srcbuf[6];
		descbuf[2] = srcbuf[5];
		descbuf[3] = srcbuf[4];
		descbuf[4] = srcbuf[3];
		descbuf[5] = srcbuf[2];
		descbuf[6] = srcbuf[1];
		descbuf[7] = srcbuf[0];
		memcpy(&retVal, descbuf, SZ_INT64);
		return retVal;
	}
	return netval;
}
*/
void CTEOrderPackage::EncryptPackage()
{
}

void CTEOrderPackage::DecryptPackage()
{
}
