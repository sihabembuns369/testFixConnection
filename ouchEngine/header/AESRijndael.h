// AESRijndael.h: interface for the CAESRijndael class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __AESRIJNDAEL_H__
#define __AESRIJNDAEL_H__

#include "constants.h"

class CAESRijndael
{
public:
	static const OL_INT32	ERR_KEY_NOT_HEX			= -2;
	static const OL_INT32	ERR_KEY_VALUE_TOO_LONG	= -3;
	static const OL_INT32	ERR_KEY_LEN_INVALID		= -4;	//The key length must be 32, 48 or 64 hexadecimal digits
	static const OL_UINT32 BLOCK_LEN = 16;
	enum _RJ_RETURN_VAL
	{
		RJ_SUCCESS,
		RJ_FAILED
	};
public:
	CAESRijndael();
	virtual ~CAESRijndael();

	OL_UINT32 CalcBufEnc(OL_UINT32 nSrcSize);
	OL_UINT32 CalcBufDec(OL_UINT32 nSrcSize);
	OL_UINT32 CalcBufHexEnc(OL_UINT32 nSrcSize);
	OL_UINT32 CalcBufHexDec(OL_UINT32 nSrcSize);

	OL_INT32 SetCryptoKey(const char *pKey);
	_RJ_RETURN_VAL Encrypt(const char *psrc, OL_UINT32 src_len, char **pdest, OL_UINT32 &dst_len, int createBuf = 1);
	void Decrypt(const char *psrc, OL_UINT32 src_len, char **pdest, OL_UINT32 &dst_len, int createBuf = 1);
	_RJ_RETURN_VAL EncryptToHexString(const char *psrc, OL_UINT32 src_len, char **pdest, OL_UINT32 &dst_len, int createBuf = 1);	//	pdest must added 1 byte for Null terminated char
	void DecryptFromHexString(const char *psrc, OL_UINT32 src_len, char **pdest, OL_UINT32 &dst_len, int createBuf = 1);

	void ConvertToHexString(const char *psrc, OL_UINT32 src_len, char *pdest);	//	pdest must added 1 byte for Null terminated char
	OL_INT32 ConvertFromHexString(const char *psrc, char *pdest);
private:
	char key[32];
	OL_UINT32 key_len;

	__inline volatile OL_UINT64 get_rdtsc(void);
	void fillrand(char *buf, const int len);
};

#endif