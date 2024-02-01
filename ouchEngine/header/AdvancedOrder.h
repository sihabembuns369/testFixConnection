#ifndef __ADVANCED_ORDER__
#define __ADVANCED_ORDER__

#include "constants.h"
#define RG_BOARD "RG"
#define TN_BOARD "TN"
#define ADV_BOARD "ADV"
#define ORDER_REF_LEN 64

typedef struct __QUOTE__
{
	char szStockCode[32];	//	STOCKCODE.BOARD
	OL_UINT32 lTime;	//	hhmmss
	OL_UINT32 lPrevPrice;
	OL_INT32 lPrevChg;
	OL_INT32 lPrevChgRate;
	OL_UINT32 lLastPrice;
	OL_INT32 lChg;
	OL_INT32 lChgRate;
	OL_UINT32 lOpenPrice;
	OL_UINT32 lHighPrice;
	OL_UINT32 lLowPrice;
	OL_UINT32 lAvgPrice;
	OL_UINT32 lFreq;
	OL_UINT64 lVolume;
	OL_UINT64 lValue;
	OL_UINT64 lMarketCap;
	OL_UINT32 lBestBidPrice;
	OL_UINT64 lBestBidVolume;
	OL_UINT32 lBestOfferPrice;
	OL_UINT64 lBestOfferVolume;
} QUOTE;

typedef struct __AUTO_ORDER__	//	Break Order
{
	char szId[24];	//	Auto Order Id;
	char szCustId[16];
	OL_UINT8 cmd;	// 0:Buy; 1:Sell; 2:Margin Buy; 3:Short Sell;
	char szBoard[6];
	char szStockCode[32];
	OL_UINT32 price;
	OL_UINT64 volume;
	OL_UINT32 autoPriceStep;	// Automatic Buy/Sell if Matched
	OL_UINT8 status;	// 0:RMS Failed; 1:Open; 2:Sent; 3:Withdrawn;
	OL_UINT32 OTime;
	OL_UINT32 STime;

	OL_UINT8 TPriceType;	// 0:Best Bid Price; 1:Best Offer Price; 2:Last Price; 3:Avg Price
	OL_UINT8 TPriceCriteria;	// 0:=; 1:<; 2:>;
	OL_UINT32 TPrice;
	OL_UINT8 TVolumeType;	// 0:None; 1:Traded Volume; 2:Best Bid Volume; 3:Best Offer Volume; (Always Logical AND)
	OL_UINT8 TVolumeCriteria;	// 0:<=; 1:>=;
	OL_UINT64 TVolume;
	OL_UINT32 startDate;
	OL_UINT32 dueDate;
} AUTO_ORDER;

typedef struct __TRAILING_ORDER__
{
	char szId[24];	//	Trailing Order Id;
	char szCustId[16];
	OL_UINT8 cmd;	// 0:Buy; 1:Sell; 2:Margin Buy; 3:Short Sell;
	char szBoard[6];
	char szStockCode[32];
	OL_UINT64 volume;	//	Volume to Order
	OL_UINT32 autoPriceStep;	// Automatic Buy/Sell if Matched
	OL_UINT8 status;	// 0:RMS Failed; 1:Open; 2:Sent; 3:Withdrawn;
	OL_UINT32 OTime;
	OL_UINT32 STime;
	OL_UINT32 startDate;
	OL_UINT32 dueDate;
	//	------------- Criteria for Trigger -----------------------------------------
	OL_UINT32 TPriceDrop;	// Price Lower Than (For Trailing Buy Only)
	OL_UINT8 TPriceType;	// Trigger Price 0:Best Bid Price; 1:Best Offer Price; 2:Last Price; 3:Avg Price; 4:High Price; 5:Low Price (Only for Trailing Buy);
	OL_INT32 TStepPercentage;	// Trailing Step in Percentage;
	OL_UINT32 TPrice;	//	Trigger Price
	OL_UINT32 StopPrice;	//	Stop Price to Execute Order
	OL_UINT8 ExecPriceType;	// 0 : Use Stop Price; [1-5] : Use Best Price + [1-5]
	OL_UINT32 ExecPrice;	// Executed Price;
} TRAILING_ORDER;

typedef struct __INLINE_ORDER__
{
	char szId[24];	//	Inline Order Id;
	char szCustId[16];
	char szBoard[8];
	char szStockCode[32];
	char szInputUser[64];
	OL_UINT8 cmd;	// 0:Buy; 1:Sell; 2:Margin Buy; 3:Short Sell;
	OL_UINT8 sourceId;
	OL_UINT8 expiry;
	OL_UINT8 orderType;
	OL_UINT8 orderOption;
	OL_UINT32 LimitPrice;	//	 Can be 0
	OL_UINT64 LimitVolume;	//	This is Minimum Volume to Execute
	OL_UINT64 OVolume;	//	First Time Volume.
	OL_UINT64 TVolume;	//	Traded Volume.
	OL_UINT64 ExecVolume;	//	Executed Volume
	OL_UINT64 TradedValue;	//	Traded Value
	OL_UINT64 CapturedMarketVolume;	//	Captured Market Volume
	OL_UINT64 TradedMarketVolume;	//	TradedMarketVolume
	OL_UINT32 LastExecPrice;	//	Last Executed Price
	OL_UINT64 LastExecVolume;	//	Last Executed Volume
	OL_UINT8 BelowExecRatio;	//	1% - 100%
	OL_UINT8 UpperExecRatio;	//	0% - 100%
	OL_UINT32 Flags;	//	Bit 0 : Traded Market Volume Starting NOW;
	OL_UINT8 status;//	0 : Rejected, RMS Failed; 1 : Open; 2 : Fully Executed; 5 : Withdrawn;
	OL_UINT32 ODate;
	//OL_UINT32 LastExecutedTime;
	//char szReference[64];
} INLINE_ORDER;

typedef struct __ETF_BATCH_ORDER__
{
	char szId[16];
	char szCustId[16];
	char szETFCode[32];
	OL_UINT8 cmd;
	OL_UINT8 status;		//	0:Rejected; 1:Open; 2:Fully Executed; 5:Withrawn;
	OL_UINT8 orderOption;	//	Delay Option -- 0 : Conservative, 50s; 1 : Moderate, 20s; 3 : Aggresive, 10s;
	double limitNav;		//	Limit Order; 0 : Disable
	double lastExecutedNav;	//	Last Executed Nav
	OL_INT64 OUnit;			//	Starting Units	(Creation Unit)
	OL_INT64 TUnit;			//	Executed Units	(Creation Unit)
	OL_INT64 maxCU;			//	Max Creation Unit per Batch
	OL_UINT32 ldate;		//	Order Date yyyymmdd
} ETF_BATCH_ORDER;

typedef struct __ETF_ORDER__
{
	char szId[16];
	char szRef[16];
	char szCustId[16];
	char szETFCode[32];
	char szBoard[6];
	char szStockCode[32];
	OL_UINT8 cmd;
	OL_UINT8 status;		//	0:Rejected; 1:Open; 2:Fully Executed; 5:Withrawn;
	OL_UINT8 orderOption;	//	Delay Option -- 0 : Conservative, 50s; 1 : Moderate, 20s; 3 : Aggresive, 10s;
	OL_INT64 volume;		//	Initial Volume
	OL_INT64 executed;		//	Executed Volume
	OL_INT64 remain;		//	Remain Volume
	OL_UINT32 execPrice;	//	Last Executed Price
	OL_UINT32 nRetry;		//	Pending Retry
	OL_UINT32 ldate;		//	Order Date	yyyymmdd
} ETF_ORDER;

typedef struct __BASKET_ORDER__
{
	char szId[16];
	char szRef[16];
	char szCustId[16];
	char szBasketCode[32];
	char szBoard[6];
	char szStockCode[32];
	OL_UINT8 cmd;
	OL_UINT8 status;		//	0:Rejected; 1:Open; 2:Fully Executed; 5:Withrawn;
	OL_UINT8 orderOption;	//	Delay Option -- 0 : Conservative, 50s; 1 : Moderate, 20s; 3 : Aggresive, 10s;
	OL_UINT16 autoPriceStep;
	OL_INT64 volume;		//	Initial Volume
	OL_INT64 executed;		//	Executed Volume
	OL_INT64 remain;		//	Remain Volume
	OL_UINT32 execPrice;	//	Last Executed Price
	OL_UINT32 nRetry;		//	Pending Retry
	OL_UINT32 ldate;		//	Order Date	yyyymmdd
} BASKET_ORDER;

#endif
