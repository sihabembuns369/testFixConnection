#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "header/CMCrypt.h"

//char cm_mode[] = "cbc";
char cm_mode[] = MCRYPT_ECB;
char cm_algo[] = "rijndael-128";

CMCrypt::CMCrypt(const char *key, OL_UINT32 keylen, const char *IV)
{
	strcpy(szIV, IV);
	strcpy(szKey, key);
	key_len = keylen;
	td = mcrypt_module_open(cm_algo, NULL, cm_mode, NULL);
	block_size = mcrypt_enc_get_block_size(td);
	mcrypt_module_close(td);
}

CMCrypt::~CMCrypt()
{
}

OL_UINT32 CMCrypt::CalcEncBufferSize(OL_UINT32 nDataSize)
{
	//printf("[CalcEncBufferSize] - DataSize : %u; blockSize : %u;\n", nDataSize, block_size);
	OL_UINT32 nDiv = nDataSize % block_size;
	if (0 == nDiv)
		return nDataSize;
	return nDataSize + (block_size - nDiv);
}

void CMCrypt::ConvertToHexString(const char *psrc, OL_UINT32 src_len, char *pdest)
{
	OL_UINT32 i = 0;
	for (; i < src_len; ++i)
	{
		unsigned char n = (*(psrc + i) & 0xf0) >> 4;
		if (n <= 9)
			*(pdest + (i << 1)) = n + '0';
		else
			*(pdest + (i << 1)) = n + 'A' - 10;
		n = *(psrc + i) & 0x0f;
		if (n <= 9)
			*(pdest + (i << 1) + 1) = n + '0';
		else
			*(pdest + (i << 1) + 1) = n + 'A' - 10;
	}
	*(pdest + (i << 1)) = 0x00;
}

OL_INT32 CMCrypt::ConvertFromHexString(const char *psrc, char *pdest)
{
	OL_UINT32 i = 0, n = 0;
	char ch, *pch = (char*)psrc;
	while (*pch)
	{
		ch = toupper(*pch++);
		if (ch >= '0' && ch <= '9')
			n = (n << 4) + (ch - '0');
		else if (ch >= 'A' && ch <= 'F')
			n = (n << 4) + ch - 'A' + 10;
		else
			return ERR_KEY_NOT_HEX;
		if (i++ & 1)
			pdest[(i >> 1) - 1] = (char)(n & 0xff);
	}
	return 0;
}

OL_INT32 CMCrypt::FastEncrypt(char *psrc, OL_UINT32 src_len)
{
	if(src_len % block_size != 0)
		return 1;
	if (MCRYPT_FAILED == (td = mcrypt_module_open(cm_algo, NULL, cm_mode, NULL)))
		return -1;
	if (mcrypt_generic_init(td, szKey, key_len, szIV) < 0)
		return -2;
	OL_INT32 lRet = mcrypt_generic(td, psrc, src_len);
	mcrypt_generic_deinit(td);
	mcrypt_module_close(td);
	return lRet;
}

OL_INT32 CMCrypt::FastDecrypt(char *psrc, OL_UINT32 src_len)
{
	if(src_len % block_size != 0)
		return 1;
	if (MCRYPT_FAILED == (td = mcrypt_module_open(cm_algo, NULL, cm_mode, NULL)))
		return -1;
	if (mcrypt_generic_init(td, szKey, key_len, szIV) < 0)
		return -2;
	OL_INT32 lRet = mdecrypt_generic(td, psrc, src_len);
	mcrypt_generic_deinit(td);
	mcrypt_module_close(td);
	return lRet;
}

OL_INT32 CMCrypt::EncryptToHexString(const char *psrc, OL_UINT32 src_len, char *pdest)
{
	OL_UINT32 enc_len = CalcEncBufferSize(src_len);
	char *penc = new char[enc_len];
	memcpy(penc, psrc, src_len);
	if (enc_len > src_len)
		memset(penc + src_len, 0x00, enc_len - src_len);
	if (MCRYPT_FAILED == (td = mcrypt_module_open(cm_algo, NULL, cm_mode, NULL)))
		return -1;
	if (mcrypt_generic_init(td, szKey, key_len, szIV) < 0)
		return -2;
	OL_INT32 lRet = mcrypt_generic(td, penc, enc_len);
	mcrypt_generic_deinit(td);
	mcrypt_module_close(td);
	ConvertToHexString(penc, enc_len, pdest);
	delete []penc; penc = NULL;
	return lRet;
}

OL_INT32 CMCrypt::DecryptFromHexString(const char *psrc, OL_UINT32 src_len, char *pdest)
{
	OL_UINT32 dec_len = src_len >> 1;
	ConvertFromHexString(psrc, pdest);
	if (MCRYPT_FAILED == (td = mcrypt_module_open(cm_algo, NULL, cm_mode, NULL)))
		return -1;
	if (mcrypt_generic_init(td, szKey, key_len, szIV) < 0)
		return -2;
	OL_INT32 lRet = mdecrypt_generic(td, pdest, dec_len);
	mcrypt_generic_deinit(td);
	mcrypt_module_close(td);
	return lRet;
}