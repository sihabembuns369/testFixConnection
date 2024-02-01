#ifndef __CMCRYPT_H__
#define __CMCRYPT_H__

#include <mcrypt.h>
#include "constants.h"

class CMCrypt
{
public:
	static const OL_INT32 ERR_KEY_NOT_HEX = -2;
	CMCrypt(const char *key, OL_UINT32 keylen, const char *IV);
	virtual ~CMCrypt();

	OL_UINT32 CalcBufHexEnc(OL_UINT32 nSrcSize) {return nSrcSize << 1;};
	OL_UINT32 CalcBufHexDec(OL_UINT32 nSrcSize) {return nSrcSize >> 1;};
	OL_UINT32 CalcEncBufferSize(OL_UINT32 nDataSize);

	void ConvertToHexString(const char *psrc, OL_UINT32 src_len, char *pdest);	//	pdest must added 1 byte for Null terminated char
	OL_INT32 ConvertFromHexString(const char *psrc, char *pdest);

	OL_INT32 FastEncrypt(char *psrc, OL_UINT32 src_len);
	OL_INT32 FastDecrypt(char *psrc, OL_UINT32 src_len);
	OL_INT32 EncryptToHexString(const char *psrc, OL_UINT32 src_len, char *pdest);	//	pdest must added 1 byte for Null terminated char
	OL_INT32 DecryptFromHexString(const char *psrc, OL_UINT32 src_len, char *pdest);

private:
	MCRYPT td;
	char szKey[256], szIV[256];
	OL_UINT32 block_size, key_len;
};

#endif