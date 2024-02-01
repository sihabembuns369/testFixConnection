#pragma once
#include "constants.h"

const OL_UINT32 IPACKAGE_SIGNATURE = 0x332fad12;

const OL_UINT16 IPKG_HEART_BEAT = 0xffff;
const OL_UINT16 IPKG_LOGOUT_REQ = 0x00ff;
const OL_UINT16 IPKG_LOGIN_REQ = 0x0001;
const OL_UINT16 IPKG_VALIDATE_PIN = 0x0002;
const OL_UINT16 IPKG_CHG_PIN_PWD = 0x0003;
const OL_UINT16 IPKG_REQ_TL = 0x0004;
const OL_UINT16 IPKG_REQ_STK_BALANCE = 0x0005;
const OL_UINT16 IPKG_REQ_ACC_INFO = 0x0006;
const OL_UINT16 IPKG_REQ_REALIZED_GAIN_LOSS = 0x0007;
const OL_UINT16 IPKG_REQ_TRADE_CONFIRMATION = 0x0008;
const OL_UINT16 IPKG_REQ_TAX_REPORT = 0x0009;
const OL_UINT16 IPKG_REQ_PORTFOLIO = 0x000A;
const OL_UINT16 IPKG_REQ_PORTFOLIO_RETURN = 0x000B;
const OL_UINT16 IPKG_REQ_FUND_WITHDRAW_INFO = 0x000C;
const OL_UINT16 IPKG_REQ_FUND_WITHDRAW_LIST = 0x000D;
const OL_UINT16 IPKG_REQ_FUND_WITHDRAW = 0x000E;
const OL_UINT16 IPKG_REQ_CANCEL_FUND_WITHDRAW = 0x000F;
const OL_UINT16 IPKG_REQ_FINANCIAL_HISTORY = 0x0010;
const OL_UINT16 IPKG_REQ_EXERCISE_INFO = 0x0011;
const OL_UINT16 IPKG_REQ_EXERCISE_LIST = 0x0012;
const OL_UINT16 IPKG_REQ_EXERCISE = 0x0013;
const OL_UINT16 IPKG_REQ_CANCEL_EXERCISE = 0x0014;
const OL_UINT16 IPKG_REQ_STOCK_INFO = 0x0015;
const OL_UINT16 IPKG_REQ_TRADING_HOLIDAY = 0x0016;
const OL_UINT16 IPKG_REQ_CA_INFO = 0x0017;
const OL_UINT16 IPKG_REQ_TRANSACTIONS_REPORT = 0x0018;
const OL_UINT16 IPKG_REQ_MONTHLY_POSITION = 0x0019;
const OL_UINT16 IPKG_REQ_CASH_LEDGER = 0x001A;
const OL_UINT16 IPKG_REQ_SINGLE_STOCK_BALANCE = 0x001B;
const OL_UINT16 IPKG_VALIDATE_LOGIN_PASSWORD = 0x001C;
const OL_UINT16 IPKG_REQ_CUST_PROTOCOL_LIST = 0x001d;
const OL_UINT16 IPKG_REQ_SPEED_ORDER_BOOK = 0x0020;
//--------------------------------------------------------------------
//	ORDER PACKAGE REQUEST
//--------------------------------------------------------------------
const OL_UINT16 IPKG_REQ_ORDER = 0x0080;
const OL_UINT16 IPKG_REQ_AMEND = 0x0081;
const OL_UINT16 IPKG_REQ_WITHDRAW = 0x0082;
const OL_UINT16 IPKG_REQ_ORDER_LIST = 0x0083;
const OL_UINT16 IPKG_REQ_TRADE_LIST = 0x0084;
const OL_UINT16 IPKG_REQ_REJECTED_ORDER_MESSAGE = 0x0085;
const OL_UINT16 IPKG_REQ_HISTORICAL_ORDER_LIST = 0x0086;
const OL_UINT16 IPKG_REQ_HISTORICAL_TRADE_LIST = 0x0087;
const OL_UINT16 IPKG_REQ_NEW_GTC_ORDER = 0x0088;
const OL_UINT16 IPKG_REQ_CANCEL_GTC_ORDER = 0x0089;
const OL_UINT16 IPKG_REQ_GTC_ORDER_LIST = 0x008A;

const OL_UINT16 IPKG_REQ_NEW_BREAK_ORDER = 0x008C;
const OL_UINT16 IPKG_REQ_NEW_TRAILING_ORDER = 0x008D;
const OL_UINT16 IPKG_REQ_CANCEL_BREAK_ORDER = 0x008E;
const OL_UINT16 IPKG_REQ_CANCEL_TRAILING_ORDER = 0x008F;

const OL_UINT16 IPKG_REQ_BREAK_ORDER_LIST = 0x0090;
const OL_UINT16 IPKG_REQ_TRAILING_ORDER_LIST = 0x0091;

const OL_UINT16 IPKG_REQ_SPEED_ORDER_AMEND = 0x0092;
const OL_UINT16 IPKG_REQ_SPEED_ORDER_WITHDRAW = 0x0093;

//--------------------------------------------------------------------
//	REALTIME Order Reply - WILL SEND TO REALTIME Connection Name
//--------------------------------------------------------------------
const OL_UINT16 IPKG_REP_NEW_ORDER = 0x0022;
const OL_UINT16 IPKG_REP_AMEND_ORDER = 0x0023;
const OL_UINT16 IPKG_REP_TRADE = 0x0024;
const OL_UINT16 IPKG_BROADCAST_LOGIN = 0x0040;
const OL_UINT16 IPKG_SPEED_ORDER_BOOK = 0x0041;
//--------------------------------------------------------------------

const OL_UINT32 IIDENTITY_LEN	= 64;
const OL_UINT32 IHEADER_LENGTH	= 23;
const OL_UINT32 IMAX_COMPRESS	= 4 * 1024;
class CRawPackage;
class CIPackage
{
public:
	char szIdentity[IIDENTITY_LEN];
	enum ITRIM_STRING
	{
		ITRIM_NONE = 0,
		ITRIM_LEFT = 1,
		ITRIM_RIGHT = 2,
		ITRIM_BOTH = 3
	};
	CIPackage();
	CIPackage(CRawPackage *p);
	CIPackage(const char *identity, CRawPackage *p);
	CIPackage(const char *identity, CIPackage *p);
	CIPackage(OL_UINT32 lsize, OL_UINT16 packageId);
	CIPackage(char *pdata, OL_UINT32 dataSize);
	virtual ~CIPackage();

	char *operator +(OL_UINT32 pos);
	operator char*() const;
	operator const char*() const;
	CIPackage &operator =(const CIPackage &p);

	OL_UINT32 GetBufferSize() const;
	OL_UINT32 GetPackageLength() const;
	OL_UINT32 GetReadPosition() const;

	OL_UINT16 GetPackageId() const;
	OL_UINT16 GetErrorCode() const;
	OL_UINT8 GetSourceId() const;
	OL_UINT32 GetPackageDate() const;
	OL_UINT32 GetPackageTime() const;

	bool isEndofPacket();
	bool isEncrypted();
	bool isCompressed();
	bool isCRCMatch();
	bool CheckSignature();

	void EncryptPackage();
	void CompressPackage();
	void DecryptPackage();
	void DecompressPackage();
	void UpdateCRCPackage();
	void ReleaseBuffer();

	bool AllocateBuffer(OL_UINT32 bufSize);
	bool SetRawData(CRawPackage *pRawPackage);
	void SetRawData(char *pszRawData, OL_UINT32 nRawDataLength);
	void SetIdentity(const char *identity) { strcpy(szIdentity, identity); };

	void ResetReadPosition();
	void ResetWritePosition();
	void MoveReadPosition(OL_INT32 step);
	void MoveWritePosition(OL_INT32 step);

	void SetPackageId(OL_UINT16 packetId);
	void SetErrorCode(OL_UINT16 errCode);
	void SetSourceId(OL_UINT8 srcId);
	void UpdatePackageTimeStamp();

	void AddInt8(OL_UINT8 val);
	void AddInt16(OL_UINT16 val);
	void AddInt32(OL_UINT32 val);
	void AddInt64(OL_UINT64 val);
	//void AddFloat(float	val);
	//void AddDouble(double val);
	//void AddLDouble(long double val);
	void AddString(const char *pstr);
	void AddString(const char *pstr, OL_UINT16 stringlen);
	void AddBinary(const char *pbinary, OL_UINT32 datalen);
	void GetString(OL_UINT16 stringlen, char *pstr, ITRIM_STRING op = ITRIM_NONE);
	void GetBinary(OL_UINT32 datalen, char *pbinary);
	void FillBuffer(char ch, OL_UINT32 nBytes);

	OL_UINT8 GetInt8();
	OL_UINT16 GetInt16();
	OL_UINT32 GetInt32();
	OL_UINT64 GetInt64();
	//float GetFloat();
	//double GetDouble();
	//long double	GetLDouble();

private:
	char *pszData;
	OL_UINT32 lBufferSize, lReadPos, lWritePos;

	void TrimLeft(char *psz);
	void TrimRight(char *psz);

	void CompressLzo();
	void DecompressLzo();
	void CompressSnappy();
	void DecompressSnappy();
	void EncryptRijndael();
	void DecryptRijndael();
	void EncryptBlockCipherSeed();
	void DecryptBlockCipherSeed();

	OL_UINT32 CalculateCRC();
	OL_UINT32 CalcComprLzoBufferUsage(OL_UINT32 nBytestoCompress);
	inline void	UpdatePackageLength();
};


