#pragma once

// #define SZ_INT8		sizeof(INT8)
// #define SZ_INT16	sizeof(INT16)
// #define SZ_INT32	sizeof(INT32)
// #define SZ_INT64	sizeof(INT64)
// #define SZ_FLOAT	sizeof(float)
// #define SZ_DOUBLE	sizeof(double)
// #define SZ_LDOUBLE	sizeof(long double)

#define SZ_ORDERID_LEN 24
#define SZ_AMENDID_LEN 24
#define SZ_WITHDRRAWID_LEN 24
#define SZ_LOGINID_LEN 32

const uint32_t realtimeLen = 8;
const uint32_t maxPwdChars = 16;
const uint32_t IREF_LEN = 64;
const uint32_t ISTR32_LEN = 32;
const uint32_t ISTR64_LEN = 64;
const uint32_t ISTR128_LEN = 128;
const uint32_t ISTR256_LEN = 128;
const uint32_t ILOGINID_LEN = 32;
const uint32_t ILOGINPWD_LEN = 32;
const uint32_t ILOGINPIN_LEN = 32;
const uint32_t IACCID_LEN   = 32;
const uint32_t IACCNAME_LEN = 128;
const uint32_t ISTOCKCODE_LEN = 32;
const uint32_t IBOARD_LEN = 8;
//const uint32_t IIDENTITY_LEN = 64;
#ifdef INET6_ADDRSTRLEN
#undef INET6_ADDRSTRLEN
#endif
const uint32_t INET6_ADDRSTRLEN = 65;
const char realtimeConnection[] = "REALTIME";

const char ISERVER_IDENTITY[] = "ISERVER_%lu";

// const char encryptionIV[] = "1F4114E9F8B1B968";
// const char encryptionKey[] = "97DF7EF6E7D71FA3";
//-----------------------------------------------------------------------------------------------------------
const char blockCipherSeedKey[] = "54DD2CD018E0D52F";	//	16 Bytes -> 128 bits encryption
const char bcsInitialVector[] = "85A4DCC5F2584CD0";
//-----------------------------------------------------------------------------------------------------------
// const char rijndaelCryptoKey[] = "F6AFE71379BEB78AD9183A99C8F1AB38";	// 32 Bytes -> 256 bits encryption
const char DFL_EMPTY_STRING[] = "54DD2CD018E0D52F54085A4DCC5F2584";
//-----------------------------------------------------------------------------------------------------------
// static const char BOARD_RG[] = "RG";
// static const char BOARD_TN[] = "TN";
// static const char BOARD_NG[] = "NG";
// static const char BOARD_ADV[] = "ADV";

enum BOARD_CODE
{
	BYTE_RG = 0,
	BYTE_TN = 1,
	BYTE_ADV = 2,
	BYTE_NG = 3
};

//-----------------------------------------------------------------------------------------
//	Good Till Cancel Constants
//-----------------------------------------------------------------------------------------
const uint8_t GTC_CHECK_RMS = 0x01;
const uint8_t GTC_PRIORITY = 0x02;
const uint8_t GTC_AUTO_ORDER = 0x04;

// enum ORDER_COMMAND
// {
// 	CMD_BUY  = 0,
// 	CMD_SELL = 1,
// 	CMD_MARGIN_BUY = 2,
// 	CMD_SHORT_SELL = 3,
// 	CMD_PRICE_STABILIZATION = 4
// };

// enum ORDER_STATUS
// {
// 	ORDER_REJECTED = 0,
// 	ORDER_OPEN = 1,
// 	ORDER_MATCHED = 2,
// 	ORDER_AMENDING = 3,
// 	ORDER_WITHDRAWING = 4,
// 	ORDER_WITHDRAWN = 5,
// 	ORDER_AMENDED = 6
// };

struct ORDER
{
	uint8_t iBoard;
	uint8_t sourceId;
	uint8_t cmd;
	uint32_t price;
	uint64_t volume;
	uint8_t bRandomize;
	uint32_t nSplit;
	uint8_t bPriceStep;
	uint32_t priceStep;
	uint8_t bAutomaticOrder;
	uint32_t autoPriceStep;
	char szLoginId[64];
	char szClientIP[INET6_ADDRSTRLEN];
	char szAccId[16];
	char szRef[IREF_LEN];
	char szStockCode[32];
	char szIdentity[64];
};
struct AMEND
{
	uint8_t iBoard;
	uint8_t sourceId;
	uint8_t cmd;
	uint32_t newPrice;
	uint64_t newVolume;
	char szLoginId[64];
	char szClientIP[INET6_ADDRSTRLEN];
	char szAccId[16];
	char szRef[IREF_LEN];
	char szStockCode[32];
	char szOrderId[24];
	char szIDXId[32];
	char szIdentity[64];
};
struct WITHDRAW
{
	uint8_t iBoard;
	uint8_t sourceId;
	uint8_t cmd;
	uint32_t price;
	char szLoginId[64];
	char szClientIP[INET6_ADDRSTRLEN];
	char szAccId[16];
	char szRef[IREF_LEN];
	char szStockCode[32];
	char szOrderId[24];
	char szIDXId[32];
	char szIdentity[64];
};
struct GTC
{
	uint8_t iBoard;
	uint8_t sourceId;
	uint8_t cmd;
	uint8_t sessionId;	// GTC Session Id
	uint32_t price;
	uint64_t volume;
	uint8_t bAutomaticOrder;
	uint32_t autoPriceStep;
	uint32_t fdate;	//	Effective Date
	uint32_t tdate;	//	Due Date
	char szLoginId[64];
	char szClientIP[INET6_ADDRSTRLEN];
	char szAccId[16];
	char szGTCId[22];
	char szRef[IREF_LEN];
	char szStockCode[32];
	char szIdentity[64];
};

struct BREAK_ORDER
{
	uint8_t iBoard;	//	0 : RG; 1 : TN; 2 : ADV;
	uint8_t sourceId;
	uint8_t cmd;	// 0 : BUY; 1 : SELL;
	uint32_t price;
	uint64_t volume;
	uint8_t condPriceType;	// 0 : Best Bid Price; 1 : Best Offer Price; 2 : Last Price; 3 : Avg Price;
	uint8_t condPriceComp;	// 0 : Equal to; 1 : < (Less than); 2 : > (More than);
	uint32_t condPrice;
	uint8_t condVolType;	// 0 : None; 1 : Traded Volume; 2 : Best Bid Volume; 3 : Best Offer Volume;
	uint8_t condVolComp;	// 0 : <= (Less than equal to); 1 : >= (Greater than equal to);
	uint64_t condVol;
	uint32_t autoPriceStep;	//	> 0 Then Activated Automatic Buy / Sell on Matched
	uint32_t fdate;	//	Effective Date
	uint32_t tdate;	//	Due Date
	char szLoginId[64];
	char szClientIP[INET6_ADDRSTRLEN];
	char szAccId[16];
	char szOrderId[22];
	char szRef[IREF_LEN];
	char szStockCode[32];
	char szIdentity[64];
};

struct TRAILING_ORDER
{
	uint8_t iBoard;
	uint8_t sourceId;
	uint8_t cmd;
	uint8_t execPriceType;	//	0 - 5 (Best Bid / Offer + N
	uint64_t volume;
	uint32_t dropPrice;	//	for Trailing Buy Only
	uint32_t trailingStep;	//	In Percentage (2 decimals)
	uint8_t trailingPriceType;	// 0 : Best Bid Price; 1 : Best Offer Price; 2 : Last Price; 3 : Avg Price; 4 : High Price; 5 : Lowest Price(For Trailing Buy);
	uint32_t autoPriceStep;	//	> 0 Then Activated Automatic Buy / Sell on Matched
	uint32_t fdate;	//	Effective Date
	uint32_t tdate;	//	Due Date
	char szLoginId[64];
	char szClientIP[INET6_ADDRSTRLEN];
	char szAccId[16];
	char szOrderId[22];
	char szRef[IREF_LEN];
	char szStockCode[32];
	char szIdentity[64];
};

// const uint32_t WMU_SERVER_STATUS = WM_USER + 1;
// const uint32_t WMU_DEALER_STATUS = WM_USER + 2;
// const uint32_t WMU_WATCH_NOTIFY = WM_USER + 3;
// const uint32_t WMU_CONNECT_TO_RT = WM_USER + 5;
// const uint32_t WMU_DISCONNECT_FROM_RT = WM_USER + 7;

// const uint16_t IPKG_HEART_BEAT = 0x0000;
// const uint16_t IPKG_LOGOUT_REQ = 0x00ff;
// const uint16_t IPKG_LOGIN_REQ = 0x0001;
// const uint16_t IPKG_VALIDATE_PIN = 0x0002;
// const uint16_t IPKG_CHG_PIN_PWD = 0x0003;
// const uint16_t IPKG_REQ_TL = 0x0004;
// const uint16_t IPKG_REQ_STK_BALANCE = 0x0005;
// const uint16_t IPKG_REQ_ACC_INFO = 0x0006;
// const uint16_t IPKG_REQ_REALIZED_GAIN_LOSS = 0x0007;
// const uint16_t IPKG_REQ_TRADE_CONFIRMATION = 0x0008;
// const uint16_t IPKG_REQ_TAX_REPORT = 0x0009;
// const uint16_t IPKG_REQ_PORTFOLIO = 0x000A;
// const uint16_t IPKG_REQ_PORTFOLIO_RETURN = 0x000B;
// const uint16_t IPKG_REQ_FUND_WITHDRAW_INFO = 0x000C;
// const uint16_t IPKG_REQ_FUND_WITHDRAW_LIST = 0x000D;
// const uint16_t IPKG_REQ_FUND_WITHDRAW = 0x000E;
// const uint16_t IPKG_REQ_CANCEL_FUND_WITHDRAW = 0x000F;
// const uint16_t IPKG_REQ_FINANCIAL_HISTORY = 0x0010;
// const uint16_t IPKG_REQ_EXERCISE_INFO = 0x0011;
// const uint16_t IPKG_REQ_EXERCISE_LIST = 0x0012;
// const uint16_t IPKG_REQ_EXERCISE = 0x0013;
// const uint16_t IPKG_REQ_CANCEL_EXERCISE = 0x0014;
// const uint16_t IPKG_REQ_STOCK_INFO = 0x0015;
// const uint16_t IPKG_REQ_TRADING_HOLIDAY = 0x0016;
// const uint16_t IPKG_REQ_CA_INFO = 0x0017;
// const uint16_t IPKG_REQ_TRANSACTIONS_REPORT = 0x0018;
// const uint16_t IPKG_REQ_MONTHLY_POSITION = 0x0019;
// const uint16_t IPKG_REQ_CASH_LEDGER = 0x001A;
// const uint16_t IPKG_REQ_SINGLE_STOCK_BALANCE = 0x001B;
// const uint16_t IPKG_VALIDATE_LOGIN_PASSWORD = 0x001C;
// const uint16_t IPKG_REQ_CUST_PROTOCOL_LIST = 0x001D;
// const uint16_t IPKG_REQ_SPEED_ORDER_BOOK = 0x0020;
//--------------------------------------------------------------------
//	ORDER PACKAGE REQUEST
//--------------------------------------------------------------------
// const uint16_t IPKG_REQ_ORDER = 0x0080;
// const uint16_t IPKG_REQ_AMEND = 0x0081;
// const uint16_t IPKG_REQ_WITHDRAW = 0x0082;
// const uint16_t IPKG_REQ_ORDER_LIST = 0x0083;
// const uint16_t IPKG_REQ_TRADE_LIST = 0x0084;
// const uint16_t IPKG_REQ_REJECTED_ORDER_MESSAGE = 0x0085;
// const uint16_t IPKG_REQ_HISTORICAL_ORDER_LIST = 0x0086;
// const uint16_t IPKG_REQ_HISTORICAL_TRADE_LIST = 0x0087;
// const uint16_t IPKG_REQ_NEW_GTC_ORDER = 0x0088;
// const uint16_t IPKG_REQ_CANCEL_GTC_ORDER = 0x0089;
// const uint16_t IPKG_REQ_GTC_ORDER_LIST = 0x008A;

// const uint16_t IPKG_REQ_NEW_BREAK_ORDER = 0x008C;
// const uint16_t IPKG_REQ_NEW_TRAILING_ORDER = 0x008D;
// const uint16_t IPKG_REQ_CANCEL_BREAK_ORDER = 0x008E;
// const uint16_t IPKG_REQ_CANCEL_TRAILING_ORDER = 0x008F;

// const uint16_t IPKG_REQ_BREAK_ORDER_LIST = 0x0090;
// const uint16_t IPKG_REQ_TRAILING_ORDER_LIST = 0x0091;

// const uint16_t IPKG_REQ_SPEED_ORDER_AMEND = 0x0092;
// const uint16_t IPKG_REQ_SPEED_ORDER_WITHDRAW = 0x0093;

//--------------------------------------------------------------------
//	REALTIME Order Reply - WILL SEND TO REALTIME Connection Name
//--------------------------------------------------------------------
// const uint16_t IPKG_REP_NEW_ORDER = 0x0022;
// const uint16_t IPKG_REP_AMEND_ORDER = 0x0023;
// const uint16_t IPKG_REP_TRADE = 0x0024;
// const uint16_t IPKG_BROADCAST_LOGIN = 0x0040;
// const uint16_t IPKG_SPEED_ORDER_BOOK = 0x0041;

//-----------------------------------------------------------------------------------------
//	Packet Error Code
//-----------------------------------------------------------------------------------------
// const uint16_t CP_ERR_UNKNOWN = 0xffff;
// const uint16_t CP_ERR_DB = 0x0001;
// const uint16_t CP_ERR_NOT_FOUND = 0x0002;
// const uint16_t CP_ERR_LOGIN = 0x0003;
// const uint16_t CP_ERR_MAX_CONNECTED = 0x0004;
// const uint16_t CP_ERR_PASSWD_NOT_MATCH = 0x0005;
// const uint16_t CP_ERR_PASSWD_EXPIRED = 0x0006;
// const uint16_t CP_ERR_EXEC_QUERY = 0x0007;
// const uint16_t CP_ERR_LACK_VOLUME = 0x0008;
// const uint16_t CP_ERR_LACK_TRADE_LIMIT = 0x0009;
// const uint16_t CP_ERR_NO_SHARE = 0x000a;
// const uint16_t CP_ERR_REJECTED_ORDER = 0x000b;
// const uint16_t CP_ERR_AMENDING_ORDER = 0x000c;
// const uint16_t CP_ERR_WITHDRAWING_ORDER = 0x000d;
// const uint16_t CP_ERR_WITHDRAWN_ORDER = 0x000e;
// const uint16_t CP_ERR_DONE_ORDER = 0x000f;

// const uint16_t CP_ERR_PERMISSION = 0x0010;
// const uint16_t CP_ERR_CREATE_TABLES = 0x0011;
// const uint16_t CP_ERR_PRINTING_IN_PROGRESS = 0x0012;
// const uint16_t CP_ERR_RESENDING = 0x0013;
// const uint16_t CP_ERR_NEED_AUTH = 0x0014;
// const uint16_t CP_ERR_WARNING_ONLY = 0x0015;

// const uint16_t CP_ERR_CUST_SUSPENDED = 0x0016;
// const uint16_t CP_ERR_MARGINABLE = 0x0017;	//	Customer Margin can only Trade Marginable Stock
// const uint16_t CP_ERR_INVALID_PRICE = 0x0018;
// const uint16_t CP_ERR_INVALID_VOLUME = 0x0019;
// const uint16_t CP_ERR_GROUP_MEMBER_NOT_QUALIFIED = 0x001a;
// const uint16_t CP_ERR_POTENTIAL_CROSS_ORDER = 0x001b;
// const uint16_t CP_ERR_INVALID_STOCK_CODE = 0x001c;
// const uint16_t CP_ERR_SYARIAH = 0x001d;
// const uint16_t CP_ERR_PWD_PIN_USED_BEFORE = 0x001e;
// const uint16_t CP_ERR_SAME_ORDER_FOUNDED = 0x001f;
// const uint16_t CP_ERR_CREATE_FILE = 0x0020;
// const uint16_t CP_ERR_MIN_PWD_CHARS = 0x0021;
// const uint16_t CP_ERR_PWD_COMPLEXITY = 0x0022;
// const uint16_t CP_ERR_PROCESSING = 0x0023;
// const uint16_t CP_ERR_MAX_PWD_CHARS = 0x0024;
// const uint16_t CP_ERR_MOCK_TRADING = 0x0025;
// const uint16_t CP_ERR_NO_CONNECTION = 0x0026;	//	Reject order if not connected between TE and IServer
// const uint16_t CP_ERR_PACKAGE_TIME_OUT = 0x0027;	//	Package Time out
// const uint16_t CP_ERR_INACTIVE = 0x0028;	//	Login Id Inactive
// const uint16_t CP_ERR_MAX_RETRY = 0x0029;	//	Max Retry PIN / Password

// const uint16_t CP_ERR_MIN_AMOUNT = 0x0030;
// const uint16_t CP_ERR_INVALID_DATE = 0x0031;
// const uint16_t CP_ERR_NOT_ENOUGH_CASH = 0x0032;
//-----------------------------------------------------------------------------------------

//const uint16_t PK_HTS_HEART_BEAT = 0xffff;
const uint16_t PK_HTS_NEW_REGULAR_ORDER = 0x0001;
const uint16_t PK_HTS_NEW_CASH_ORDER = 0x0002;
const uint16_t PK_HTS_NEW_ADVERTISEMENT_ORDER = 0x0003;

const uint16_t PK_HTS_AMEND_REGULAR_ORDER = 0x0006;
const uint16_t PK_HTS_AMEND_CASH_ORDER = 0x0007;
const uint16_t PK_HTS_AMEND_ADVERTISEMENT_ORDER = 0x0008;

const uint16_t PK_HTS_WITHDRAW_REGULAR_ORDER = 0x000a;
const uint16_t PK_HTS_WITHDRAW_CASH_ORDER = 0x000b;
const uint16_t PK_HTS_WITHDRAW_ADVERTISEMENT_ORDER = 0x000c;

const uint16_t PK_HTS_PORTFOLIO_REQ = 0x0010;
const uint16_t PK_HTS_ORDER_LIST_REQ = 0x0011;	//	Request by Customer Id
const uint16_t PK_HTS_TRADE_LIST_REQ = 0x0012;	//	Request by Customer Id
const uint16_t PK_HTS_HISTORY_ORDER_LIST_REQ = 0x0013;	//	Request by Customer Id
const uint16_t PK_HTS_HISTORY_TRADE_LIST_REQ = 0x0014;	//	Request by Customer Id

const uint16_t PK_HTS_REQ_ALL_ORDER_LIST = 0x0015;
const uint16_t PK_HTS_REQ_ALL_TRADE_LIST = 0x0016;
const uint16_t PK_HTS_REQ_ALL_AMEND_LIST = 0x0017;
const uint16_t PK_HTS_REQ_ALL_WITHDRAW_LIST = 0x0018;
const uint16_t PK_HTS_REQ_ALL_NEGDEAL_LIST = 0x0019;
const uint16_t PK_HTS_NEW_GTC = 0x001a;
const uint16_t PK_HTS_CANCEL_GTC = 0x001b;
const uint16_t PK_HTS_REQ_GTC_LIST = 0x001c;
const uint16_t PK_HTS_SUBSC_ORDER_REPLY = 0x0020;
const uint16_t PK_HTS_SUBSC_TRADE_REPLY = 0x0021;
const uint16_t PK_HTS_NEW_ORDER_REPLY = 0x0022;
const uint16_t PK_HTS_UPDATE_ORDER_REPLY = 0x0023;
const uint16_t PK_HTS_TRADE_REPLY = 0x0024;
const uint16_t PK_HTS_REQ_STOCK_BALANCE = 0x0025;	//	Request by Customer Id
const uint16_t PK_HTS_QUICK_SELL_PORTFOLIO = 0x0026;

const uint16_t PK_HTS_NEW_BREAK_ORDER = 0x0030;
const uint16_t PK_HTS_CANCEL_BREAK_ORDER = 0x0031;
const uint16_t PK_HTS_REQ_BREAK_ORDER_LIST = 0x0032;

const uint16_t PK_HTS_NEW_TRAILING_ORDER = 0x0034;
const uint16_t PK_HTS_CANCEL_TRAILING_ORDER = 0x0035;
const uint16_t PK_HTS_REQ_TRAILING_ORDER_LIST = 0x0036;
