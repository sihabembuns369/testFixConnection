#ifndef __GPACKAGE_H__
#define __GPACKAGE_H__

#include "constants.h"
#include <cstddef>
#include <cstring>

class CRawPackage;
class CAESRijndael;

class CGPackage
{
public:
	static const OL_UINT32 GP_HEADER_LENGTH = 18;
	enum GP_TRIM_STRING
	{
		GP_TRIM_NONE  = 0,
		GP_TRIM_LEFT  = 1,
		GP_TRIM_RIGHT = 2,
		GP_TRIM_BOTH  = 3
	};
	enum GP_COMPRESSION_ALGO
	{
		GP_COMPRESS_LZ0 = 0,
		GP_COMPRESS_LZ4 = 1,
		GP_COMPRESS_SNAPPY = 2
	};
public:
	CGPackage(CAESRijndael *p = NULL);
	CGPackage(CRawPackage *praw, CAESRijndael *p = NULL);
	CGPackage(OL_UINT32 pkgSize, OL_UINT16 pkgId, CAESRijndael *p = NULL);
	CGPackage(char *pdat, OL_UINT32 pkgSize, bool copyData = true, CAESRijndael *p = NULL);
	virtual ~CGPackage();

	CGPackage &operator =(const CGPackage &p);
	char *operator +(unsigned long pos) {return pszData + pos;};
	operator char*() const {return pszData;};
	operator const char*() const {return pszData;};
	
	inline OL_UINT32 GetBufferSize() const {return lBufferSize;};
	inline OL_UINT32 GetPackageLength() const {return lWritePos;};
	inline OL_UINT32 GetReadPosition() const {return lReadPos;};

	OL_UINT16 GetPackageId() const;
	OL_UINT16 GetErrorCode() const;

	void SetFirstPriority(bool b);
	void SetContiguousPackage(bool b);
	void SetResendPackage(bool b);
	bool isResendPackage();
	bool isFirstPriority();
	bool isEndofPacket() {return lReadPos >= lBufferSize;};
	bool isEncrypted();
	bool isCompressed();
	bool isCRCMatch();
	bool CheckSignature();
	bool isContiguousPackage();

	void EncryptAES();
	void DecryptAES();
	void EncryptPackage();
	void DecryptPackage();
	void CompressPackage();
	void DecompressPackage();

	void CompressLZ4();
	bool DecompressLZ4();
	void CompressLZ0();
	bool DecompressLZ0();
	void CompressSnappy();
	bool DecompressSnappy();

	void CompressPackage(GP_COMPRESSION_ALGO algo = GP_COMPRESS_SNAPPY);
	bool DecompressPackage(GP_COMPRESSION_ALGO algo = GP_COMPRESS_SNAPPY);

	//void UpdateCRCPacket();
	void ReleaseBuffer();

	void AllocateBuffer(OL_UINT32 bufSize);
	bool SetRawData(CRawPackage *praw);
	void SetRawData(char *pszRawData, OL_UINT32 nRawDataLength);

	void ResetReadPosition() {lReadPos = GP_HEADER_LENGTH;};
	void ResetWritePosition();
	void MoveReadPosition(OL_INT32 step);
	void MoveWritePosition(OL_INT32 step);

	void SetPackageId(OL_UINT16 pkgId);
	void SetErrorCode(OL_UINT16 errCode);

	OL_UINT32 *StartArrCountingPoint();
	void EndArrCountingPoint(OL_UINT32 *pc);

	void AddInt8(OL_UINT8 val);
	void AddInt16(OL_UINT16 val);
	void AddInt32(OL_UINT32 val);
	void AddInt64(OL_UINT64 val);
	void AddFloat(float	val);
	void AddDouble(double val);
	void AddLDouble(long double val);
	void AddString(const char *pstr);
	void AddString(const char *pstr, OL_UINT16 sLen);
	void AddBinary(const char *pbinary, OL_UINT32 datalen);
	void GetString(OL_UINT16 stringlen, char *pstr, GP_TRIM_STRING op = GP_TRIM_NONE);
	void GetBinary(OL_UINT32 datalen, char *pbinary);

	OL_UINT8 GetInt8();
	OL_UINT16 GetInt16();
	OL_UINT32 GetInt32();
	OL_UINT64 GetInt64();
	float GetFloat();
	double GetDouble();
	long double GetLDouble();

	inline const char *GetReference() const { return szReference; };
	inline void SetReference(const char *sz) { strcpy(szReference, sz); };

private:
	char *pszData;
	char szReference[64];
	CAESRijndael *prijndael;
	OL_UINT32 lBufferSize, lReadPos, lWritePos;

	void TrimLeft(char *psz);
	void TrimRight(char *psz);

	void UpdatePackageLength();
	unsigned long CalculateCRC();
	OL_UINT32 CalcComprLzoBufferUsage(OL_UINT32 nBytestoCompress);
};

#endif