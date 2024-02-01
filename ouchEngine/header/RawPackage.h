#ifndef __RAW_PACKAGE_H__
#define __RAW_PACKAGE_H__

#include <list>
#include <string.h>
#include <arpa/inet.h>
#include "constants.h"

class CRawPackage
{
public:
	CRawPackage():bIsPriority(false) {totDataLen = packageId = errCode = 0;};
	CRawPackage(OL_UINT16 pkgId):bIsPriority(false) {packageId = htons(pkgId); totDataLen = errCode = 0;};
	virtual ~CRawPackage();
	
	void Release();
	void SetPackageId(OL_UINT16 id) {packageId = htons(id);};
	void SetErrorCode(OL_UINT16 err) {errCode = htons(err);};
	void SetFirstPriority(bool b) {bIsPriority = b;};
	void AddInt8(OL_UINT8 val);
	void AddInt16(OL_UINT16 val);
	void AddInt32(OL_UINT32 val);
	void AddInt64(OL_UINT64 val);
	void AddFloat(float val);
	void AddDouble(double val);
	void AddString(const char *psz);
	void AddFixedString(const char *psz, OL_UINT32 nLen);
	void AddBinary(const char *pbin, OL_UINT32 nLen);
	OL_UINT32 GetDataLength() const {return totDataLen;};
	OL_UINT32 GetDataLengthWithHeader() const;
	OL_UINT16 GetPackageId() const {return ntohs(packageId);};
	OL_UINT16 GetErrorCode() const {return ntohs(errCode);};
	bool isFirstPriority() const {return bIsPriority;};
	OL_UINT32 *StartArrCountingPoint();
	void EndArrCountingPoint(OL_UINT32 *pc);
	OL_UINT32 ExtractData(char *pszData);
	OL_UINT32 ExtractDataWithHeader(char *pszData);
	OL_UINT32 ExtractEncryptedData(char **pszData);

private:
	bool bIsPriority;
	OL_UINT32 totDataLen;
	OL_UINT16 packageId, errCode;
	class CRawData
	{
	public:
		CRawData(OL_UINT32 lsize):datalen(lsize) {prawdata = new char[datalen];};
		CRawData(const void *pdata, OL_UINT32 lsize):datalen(lsize)
		{
			prawdata = new char[datalen];
			memcpy(prawdata, pdata, lsize);
		};
		~CRawData(){delete []prawdata;};
		friend class CRawPackage;
	private:
		char *prawdata;
		OL_UINT32 datalen;
	};
	typedef std::list<CRawData*> _RAW_DATA_LIST_;
	_RAW_DATA_LIST_	rawdata;
};

#endif