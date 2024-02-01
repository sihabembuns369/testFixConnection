// AESRijndael.cpp: implementation of the CAESRijndael class.
//
//////////////////////////////////////////////////////////////////////
#include "header/AESRijndael.h"
#include "header/aes.h"
#include <time.h>
#include <ctype.h>
#include <string.h>

#define RAND(a,b) (((a = 36969 * (a & 65535) + (a >> 16)) << 16) + \
                    (b = 18000 * (b & 65535) + (b >> 16))  )
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAESRijndael::CAESRijndael():key_len(0)
{
	memset(key, 0x00, sizeof(key));
}

CAESRijndael::~CAESRijndael()
{

}

OL_UINT32 CAESRijndael::CalcBufEnc(OL_UINT32 nSrcSize)
{
	return nSrcSize + BLOCK_LEN;
}

OL_UINT32 CAESRijndael::CalcBufDec(OL_UINT32 nSrcSize)
{
	if (nSrcSize <= BLOCK_LEN)
		return 0;
	return nSrcSize - BLOCK_LEN;
}

OL_UINT32 CAESRijndael::CalcBufHexEnc(OL_UINT32 nSrcSize)
{
	return (nSrcSize + BLOCK_LEN) << 1;
}

OL_UINT32 CAESRijndael::CalcBufHexDec(OL_UINT32 nSrcSize)
{
	return nSrcSize >> 1;
}

void CAESRijndael::ConvertToHexString(const char *psrc, OL_UINT32 src_len, char *pdest)
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

OL_INT32 CAESRijndael::ConvertFromHexString(const char *psrc, char *pdest)
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

__inline volatile OL_UINT64 CAESRijndael::get_rdtsc(void)
{
	OL_UINT32 lo = time(NULL),
		hi = lo & -36969l;
	return (((OL_UINT64)hi) << 32) | lo;
}

void CAESRijndael::fillrand(char *buf, const OL_INT32 len)
{
	char r[4];
	OL_UINT32 count = 4;
	static OL_UINT32 a[2], mt = 1;
    if(mt)
	{
		mt = 0;
		*(OL_UINT64*)a = get_rdtsc();
	}
    for(int i = 0; i < len; ++i)
    {
        if(count == 4)
        {
            *(OL_UINT32*)r = RAND(a[0], a[1]);
            count = 0;
        }
        buf[i] = r[count++];
    }
}

OL_INT32 CAESRijndael::SetCryptoKey(const char *pKey)
{
	OL_INT32 i = 0, by = 0;
	char ch, *pch = (char*)pKey;
	while (i < 64 && *pch)
	{
		ch = toupper(*pch++);
		if (ch >= '0' && ch <= '9')
			by = (by << 4) + ch - '0';
		else if (ch >= 'A' && ch <= 'F')
			by = (by << 4) + ch - 'A' + 10;
		else
			return ERR_KEY_NOT_HEX;
		if (i++ & 1)
			key[(i >> 1) - 1] = by & 0xff;
	}
	if (*pch)
		return ERR_KEY_VALUE_TOO_LONG;
	else if (i < 32 || (i & 15))
		return ERR_KEY_LEN_INVALID;
	key_len = i >> 1;
	return 0;
}

CAESRijndael::_RJ_RETURN_VAL CAESRijndael::EncryptToHexString(const char *psrc, OL_UINT32 src_len, char **pdest, OL_UINT32 &dst_len, int createBuf)
{
	char *pEncrypted;
	OL_UINT32 dwEncryptedLen;
	if (RJ_FAILED == Encrypt(psrc, src_len, &pEncrypted, dwEncryptedLen))
		return RJ_FAILED;
	dst_len	= dwEncryptedLen << 1;
	if (createBuf)
		*pdest = new char[dst_len + 1];
	ConvertToHexString(pEncrypted, dwEncryptedLen, *pdest);
	delete []pEncrypted; pEncrypted = NULL;
	return RJ_SUCCESS;
}

void CAESRijndael::DecryptFromHexString(const char *psrc, OL_UINT32 src_len, char **pdest, OL_UINT32 &dst_len, int createBuf)
{
	OL_UINT32 dwEncryptedLen = src_len >> 1;
	char *pEncrypted = new char[dwEncryptedLen];
	ConvertFromHexString(psrc, pEncrypted);
	if (createBuf)
		*pdest = new char[CalcBufDec(dwEncryptedLen) + 1];
	Decrypt(pEncrypted, dwEncryptedLen, pdest, dst_len, 0);
	(*pdest)[dst_len] = 0x00; delete []pEncrypted; pEncrypted = NULL;
}

CAESRijndael::_RJ_RETURN_VAL CAESRijndael::Encrypt(const char *psrc, OL_UINT32 src_len, char **pdest, OL_UINT32 &dst_len, int createBuf)
{
	if (0 == key_len)
		return RJ_FAILED;
	aes_encrypt_ctx	ctx;
	memset(&ctx, 0x00, sizeof(ctx));
	if (EXIT_FAILURE == aes_encrypt_key((const unsigned char*)key, key_len, &ctx))
		return RJ_FAILED;
	OL_UINT32 i, len, rlen = src_len;
	char buf[BLOCK_LEN], dbuf[BLOCK_LEN << 1];
	dst_len	= src_len + BLOCK_LEN;
	if (createBuf)
		*pdest = new char[dst_len];
	fillrand(dbuf, BLOCK_LEN);
	if (rlen <= BLOCK_LEN)
	{
		len	  = src_len;
		rlen -= len;
		memcpy(dbuf + BLOCK_LEN, psrc, src_len);
		for (i = len; i < BLOCK_LEN; ++i)
			dbuf[i + BLOCK_LEN] = 0x00;
		for (i = 0; i < BLOCK_LEN; ++i)
			dbuf[i + BLOCK_LEN] ^= dbuf[i];
		if (EXIT_FAILURE == aes_encrypt((const unsigned char*)dbuf + BLOCK_LEN, (unsigned char*)dbuf + len, &ctx))
		{
			dst_len = 0;
			if (createBuf)
			{
				delete []*pdest; *pdest = NULL;
			}
			return RJ_FAILED;
		}
		len += BLOCK_LEN;
		memcpy(*pdest, dbuf, len);
	}
	else
	{
		OL_UINT32 nReadPos = 0, nWritePos = 0;
		len = BLOCK_LEN;
		memcpy(*pdest, dbuf, BLOCK_LEN); nWritePos += BLOCK_LEN;
		while (rlen > 0)
		{
			rlen -= len;
			memcpy(buf, psrc + nReadPos, BLOCK_LEN); nReadPos += BLOCK_LEN;
			for (i = 0; i < BLOCK_LEN; ++i)
				buf[i] ^= dbuf[i];
			if (EXIT_FAILURE == aes_encrypt((const unsigned char*)buf, (unsigned char*)dbuf, &ctx))
			{
				dst_len = 0;
				if (createBuf)
				{
					delete []*pdest; *pdest = NULL;
				}
				return RJ_FAILED;
			}
			if (rlen > 0 && rlen < BLOCK_LEN)
			{
				for (i = 0; i < BLOCK_LEN; ++i)
					dbuf[i + BLOCK_LEN] = dbuf[i];
				memcpy(dbuf, psrc + nReadPos, rlen);
				for (i = 0; i < BLOCK_LEN - rlen; ++i)
					dbuf[rlen + i] = 0;
				for (i = 0; i < BLOCK_LEN; ++i)
					dbuf[i] ^= dbuf[i + BLOCK_LEN];
				if (EXIT_FAILURE == aes_encrypt((const unsigned char*)dbuf, (unsigned char*)dbuf, &ctx))
				{
					dst_len = 0;
					if (createBuf)
					{
						delete []*pdest; *pdest = NULL;
					}
					return RJ_FAILED;
				}
				len = rlen + BLOCK_LEN; rlen = 0;
			}
			memcpy(*pdest + nWritePos, dbuf, len); nWritePos += len;
		}
	}
	return RJ_SUCCESS;
}

void CAESRijndael::Decrypt(const char *psrc, OL_UINT32 src_len, char **pdest, OL_UINT32 &dst_len, int createBuf)
{
	aes_decrypt_ctx	ctx;
	memset(&ctx, 0x00, sizeof(ctx));
	char dbuf[BLOCK_LEN << 1];
	unsigned long i, rlen = src_len;
	dst_len = src_len - BLOCK_LEN;
	if (createBuf)
		*pdest = new char[dst_len];
	aes_decrypt_key((const unsigned char*)key, key_len, &ctx);
	if (rlen <= (BLOCK_LEN << 1))
	{
		memcpy(dbuf, psrc, src_len);
		aes_decrypt((const unsigned char*)dbuf + dst_len, (unsigned char*)dbuf + BLOCK_LEN, &ctx);
		for (i = 0; i < dst_len; ++i)
			dbuf[i] ^= dbuf[i + BLOCK_LEN];
		memcpy(*pdest, dbuf, dst_len);
	}
	else
	{
		OL_INT32 nReadPos = BLOCK_LEN, nWritePos = 0, len = BLOCK_LEN;
		char buf1[BLOCK_LEN], buf2[BLOCK_LEN], *b1 = buf1, *b2 = buf2, *bt;
		rlen -= BLOCK_LEN;
		memcpy(b1, psrc, BLOCK_LEN);
		while (rlen > 0)
		{
			memcpy(b2, psrc + nReadPos, BLOCK_LEN); nReadPos += BLOCK_LEN;
			rlen -= BLOCK_LEN;
			aes_decrypt((const unsigned char*)b2, (unsigned char*)dbuf, &ctx);
			if (rlen > 0 && rlen < BLOCK_LEN)
			{
				memcpy(b2, psrc + nReadPos, rlen);
				for (i = rlen; i < BLOCK_LEN; ++i)
					b2[i] = dbuf[i];
				for (i = 0; i < rlen; ++i)
					dbuf[i + BLOCK_LEN] = dbuf[i] ^ b2[i];
				aes_decrypt((const unsigned char*)b2, (unsigned char*)dbuf, &ctx);
				len = rlen + BLOCK_LEN; rlen = 0;
			}
			for (i = 0; i < BLOCK_LEN; ++i)
				dbuf[i] ^= b1[i];
			memcpy(*pdest + nWritePos, dbuf, len); nWritePos += len;
			bt = b1; b1 = b2; b2 = bt;
		}
	}
}
