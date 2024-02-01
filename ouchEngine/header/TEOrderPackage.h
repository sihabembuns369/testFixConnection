#ifndef __CTEORDERPACKET_H__
#define __CTEORDERPACKET_H__

#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <string.h>
#include "constants.h"
#include "GQueue.h"

#define SZ_BYTE	sizeof(OL_UINT8)
#define SZ_WORD	sizeof(OL_UINT16)
#define SZ_DWORD sizeof(OL_UINT32)
//#define SZ_INT64 sizeof(OL_UINT64)

//enum TE_ORDER_SOURCE
//{
//	SRC_FLOOR = 0,
//	SRC_DEALER	= 1,
//	SRC_GTC	= 2,
//	SRC_AUTO = 3,
//	SRC_WEB = 4, 
//	SRC_APP = 5, 
//	SRC_APH = 6,	//	Android Phone
//	SRC_TAB = 7,	//	Android Tablet
//	SRC_IPH = 8,	//	Iphone
//	SRC_IPAD = 9,	//	IPad
//	SRC_TEL	= 10,	//	Telegram Order
//	SRC_ETF	= 88	//	ETF Order
//};

//-----------------------------------------------------------------------------------------

const OL_UINT32	TE_HEADER_LENGTH = 24;

class CRawPackage;

class CTEOrderPackage
{
public:
	enum GP_TRIM_STRING
	{
		GP_TRIM_NONE,
		GP_TRIM_LEFT,
		GP_TRIM_RIGHT,
		GP_TRIM_BOTH
	};
public:
	CTEOrderPackage();
	CTEOrderPackage(CRawPackage *praw);
	CTEOrderPackage(OL_UINT32 bufSize, OL_UINT16 packageId);
	virtual ~CTEOrderPackage();

	operator char*() const {return pszData;};
	operator const char*() const {return pszData;};
	char *operator +(OL_UINT32 pos) {return pszData + pos;};

	OL_UINT32 GetBufferSize() const {return lBufferSize;};
	OL_UINT32 GetPackageLength() const {return lWritePos;};
	OL_UINT32 GetReadPosition() const {return lReadPos;};

	void ReleaseBuffer();
	void AllocateBuffer(OL_UINT32 bufSize);
	bool SetRawData(CRawPackage *praw);
	void SetRawData(char *pszRawData, OL_UINT32 nRawDataLength);

	void EncryptPackage();
	void DecryptPackage();

	void ResetReadPosition() {lReadPos = TE_HEADER_LENGTH;};
	void ResetWritePosition() {lWritePos = TE_HEADER_LENGTH;};
	void MoveReadPosition(OL_INT32 step);
	void MoveWritePosition(OL_INT32 step);

	unsigned char isEndofPackage() const;
	unsigned char isEncrypted() const;
	unsigned char isCompressed() const;
	unsigned char CheckSignature() const;

	bool isFirstPriority() const;

	void SetPackageId(OL_UINT16 packageId);
	void SetErrorCode(OL_UINT16 errCode);
	OL_UINT16 GetPackageId() const;
	OL_UINT16 GetErrorCode() const;

	OL_UINT32 GetServerFd();
	void SetServerFd(OL_UINT32 fd);

	void AddByte(unsigned char val);
	void AddWord(OL_UINT16 val);
	void AddDWord(OL_UINT32 val);
	void AddInt64(OL_UINT64 val);
	void AddDouble(double val);
	void AddString(const char *pstr);
	void AddString(const char *pstr, OL_UINT32 stringlen);
	void AddBinary(const char *pbinary, OL_UINT32 datalen);
	void GetString(OL_UINT32 stringlen, char *pstr, GP_TRIM_STRING op = GP_TRIM_NONE);
	void GetBinary(OL_UINT32 datalen, char *pbinary);
	void FillBuffer(char ch, OL_UINT32 nBytes);

	OL_UINT8  GetByte();
	OL_UINT16 GetWord();
	OL_UINT32 GetDWord();
	OL_UINT64 GetInt64();
	double GetDouble();

	void SetFd(OL_INT32 lfd) {fd = lfd;};
	OL_INT32 GetFd() const {return fd;};

private:
	OL_INT32 fd;
	char *pszData;
	OL_UINT32 lBufferSize, lReadPos, lWritePos;

	void TrimLeft(char *psz);
	void TrimRight(char *psz);
	void UpdatePacketLength();

	int isBigEndian();
//	OL_UINT64 htonll(OL_UINT64 hostval);
//	OL_UINT64 ntohll(OL_UINT64 netval);
};

typedef CGQueue<CTEOrderPackage> _QUEUE_;

#endif
