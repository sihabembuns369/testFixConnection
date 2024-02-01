#ifndef __SOUPBIN_H__
#define __SOUPBIN_H__

#include "constants.h"
#include <cstddef>
#include <cstring>

const OL_UINT16 SB_HEADER_LENGTH = 3;

const OL_UINT64 REQ_FROM_CURRENT	= 0;
const OL_UINT64 REQ_FROM_BEGINNING	= 1;

const OL_UINT8 SB_PKG_DEBUG				= '+';
const OL_UINT8 SB_PKG_LOGIN_ACCEPTED	= 'A';
const OL_UINT8 SB_PKG_LOGIN_REJECTED	= 'J';
const OL_UINT8 SB_PKG_SERVER_HEARTBEAT	= 'H';
const OL_UINT8 SB_PKG_SEQUENCED 		= 'S';
const OL_UINT8 SB_PKG_END_OF_SESSION	= 'Z';
const OL_UINT8 SB_PKG_LOGIN_REQUEST		= 'L';
const OL_UINT8 SB_PKG_UNSEQUENCED		= 'U';
const OL_UINT8 SB_PKG_CLIENT_HEART_BEAT	= 'R';
const OL_UINT8 SB_PKG_LOGOUT_REQ		= 'O';
const OL_UINT8 SB_PKG_USER_DEFINED		= 'Z';
//------------------------------------------------
//	Login Rejected Codes
//------------------------------------------------
const OL_UINT8 LOGIN_NOT_AUTHORIZED			= 'A';
const OL_UINT8 LOGIN_SESSION_UNAVAILABLE	= 'S';

typedef struct _SOUPBIN_CFG_DATA_
{
	char szUserId[32];
	char szPassword[32];
	uint16_t servers,	//	Number of Servers ( Max 5 )
			serverPort[5];
	char szServerIPAddress[5][65];
} SOUPBIN_CFG_DATA;

class CRawPackage;

class CSoupBin
{
public:	
	enum SB_TRIM_STRING
	{
		SB_TRIM_NONE  = 0,
		SB_TRIM_LEFT  = 1,
		SB_TRIM_RIGHT = 2,
		SB_TRIM_BOTH  = 3
	};
public:
	CSoupBin();
	CSoupBin(CRawPackage *praw);
	CSoupBin(OL_UINT16 pkgSize, OL_UINT8 pkgId);
	CSoupBin(char *pdat, OL_UINT16 pkgSize, const char *identity = NULL, bool copyData = false);
	virtual ~CSoupBin();

	CSoupBin &operator =(const CSoupBin &p);
	char *operator +(unsigned long pos) {return pszData + pos;};
	operator char*() const {return pszData;};
	operator const char*() const {return pszData;};

	OL_UINT16 GetBufferSize() const {return lBufferSize;};
	OL_UINT16 GetPackageLength() const {return lWritePos;};
	OL_UINT16 GetReadPosition() const {return lReadPos;};

	inline OL_UINT8 GetPackageId() const { return pszData[2]; };
	inline OL_UINT8 GetMessageType() const { return pszData[3]; };

	void ComposeLogoutRequest();
	void ComposeClientHeartBeat();
	void ComposeLoginRequest(const char *uid, const char *pwd, const char *session, OL_UINT64 seq = REQ_FROM_BEGINNING);

	void ReleaseBuffer();
	void AllocateBuffer(OL_UINT16 bufSize);
	bool SetRawData(CRawPackage *praw);
	void SetRawData(char *pszRawData, OL_UINT16 nRawDataLength);

	void SetIdentity(const char *szId);

	void ResetReadPosition() {lReadPos = SB_HEADER_LENGTH;};
	void ResetWritePosition();
	void MoveReadPosition(OL_INT16 step);
	void MoveWritePosition(OL_INT16 step);

	void SetPackageId(OL_UINT8 pkgId);

	void AddInt8(OL_UINT8 val);
	void AddInt16(OL_UINT16 val);
	void AddInt32(OL_UINT32 val);
	void AddInt64(OL_UINT64 val);
	void AddString(const char *pstr, OL_UINT16 sLen);
	void AddBinary(const char *pbinary, OL_UINT16 datalen);
	void GetString(char *pstr);
	void GetString(OL_UINT16 stringlen, char *pstr, SB_TRIM_STRING op = SB_TRIM_NONE);
	void GetBinary(OL_UINT16 datalen, char *pbinary);

	OL_UINT8 GetInt8();
	OL_UINT16 GetInt16();
	OL_UINT32 GetInt32();
	OL_UINT64 GetInt64();

	inline const char *GetIdentity() const { return szIdentity; };
	inline size_t GetIdentityLength() const { return strlen(szIdentity); };

protected:
	char *pszData, szIdentity[64];
	OL_UINT16 lBufferSize, lReadPos, lWritePos;

	void TrimLeft(char *psz);
	void TrimRight(char *psz);
	void UpdatePackageLength();
};

#endif
