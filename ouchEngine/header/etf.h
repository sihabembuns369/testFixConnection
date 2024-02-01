#include <map>
#include <vector>
#include <string>
#include <zmq.h>
#include "itch_shm.h"
#include "SharedMem.h"
#include "AdvancedOrder.h"
#include "TEOrderPackage.h"

class CJournal;
class CPriceFraction;

typedef struct _ETFItem
{
	_ETFItem(const char *stock, OL_UINT32 vol):volume(vol), percentage(0) {
		strcpy(szStockCode, stock);
	};
	char szStockCode[32];
	OL_UINT32 volume;			//	Lembar Saham dalam 1 unit kreasi
	double percentage;
} ETFItem;

class CETFForecast
{
public:
	CETFForecast();
	CETFForecast(OL_UINT32 unitSize, OL_UINT64 vol);
	~CETFForecast() {};
	
	CETFForecast& operator= (CETFForecast &f);
	
	OL_UINT32 unit;
	OL_UINT64 volume;
	OL_INT64 sellValue;
	OL_INT64 buyValue;
	OL_INT64 curValue;
	double curNav;
	double buyNav;
	double sellNav;
	void ClearValues();
	void ClearBuyValues();
	void ClearSellValues();
};

class CETF
{
public:
	CETF(ITCH_SHM *p, const char *dealerid, const char *etf, const char *desc, const char *bufAccount, const char *tradeAccount, OL_UINT32 uSize, OL_INT64 lCash, 
		OL_UINT32 mCU, double spreadp, double up, double disc, uint8_t fl);
	~CETF();
	
	void Release();
	
public:
	CJournal *journal;
	char szETF[32];
	char szDealerId[16];	//	Owner of this ETF, Only Owner can modify this ETF
	char szBufAccount[16];	//	Buffer Account
	char szTradeAccount[16];	//	Trade Account for underlying transactions.
	char szDescription[128];
	OL_UINT32 maxCU;		//	Max Creation Unit per Batch
	OL_UINT32 unitSize;		//	Creation Unit Size
	OL_UINT32 unitShares;	//	Number of Shares in 1 Creation Unit
	OL_INT64 cash;
	double spread;		//	% Spread for Transaction to Buffer Account
	double upPrice;		//	% for Up Price
	double discPrice;	//	% for Disc Price
	uint8_t flags;		//	Bit 0 : Active;

	typedef std::vector<ETFItem*> _UNDERLYING;
	_UNDERLYING underlying;
	pthread_mutex_t utex;
	
	typedef std::vector<CETFForecast*> _FORECAST;
	_FORECAST forecast;
	
	typedef std::vector<std::string> _UNDERLYING_MEMO;
	_UNDERLYING_MEMO umemo;
	pthread_mutex_t ftex;

	void InitForecast();
	void ReleaseForecast();
	void RecalculateForecast();
	void RescanUnderlyingStatus();
	bool CalculateForecast(CETFForecast *p);

	CTEOrderPackage *ComposeForecastPackage();
	void ContinueProcessETFInit(CTEOrderPackage *pkg);
	
	void GetETFForecast(OL_UINT32 idx, CETFForecast *p);

private:
	uint16_t OBDEPTH;
	ITCH_SHM *pidxdata;
	void RecalculatePercentage();
};

class CETFProcess
{
public:
	CETFProcess(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, CPriceFraction *pfrac, _QUEUE_ *pqueue);
	~CETFProcess();

	void Release();
	void ReleaseETFBatchOrders();
	void ReqETFInit();
	void ProcessETFInit(CTEOrderPackage *pkg);

	//OL_UINT32 ProcessData(QUOTE *q);	// return number of order matched criteria
	void AddOrder(ETF_BATCH_ORDER *o);
	void WithdrawOrder(const char *orderId);
	void ContinueExecETFBatchOrder(const char *orderId);
	//void ProcessOrderReply(CTEOrderPackage *pkg);
	void ProcessSubscETFOrder(CTEOrderPackage *pkg);
	void ProcessUnsubscETFOrder(CTEOrderPackage *pkg);
	void SendHeartBeat();
	void RecalculateForecast();
	void RequestResendAllETFBatchOrders();
//	void CleanupAllTrailingOrder();
	//void RequestResendAllTrailingOrder();
	bool ComposeETFOrderPackage(ETF_BATCH_ORDER *o);
	//void SendUpdateTrailingState(TRAILING_ORDER *o);
	void GetOrderDescriptions(char *szT, ETF_BATCH_ORDER *o);

private:
	uint32_t lCurDate;
	bool bRunning, bHoliday;
	pthread_t cid, fid;	//	cid -> client_worker; fid -> forecast_worker;
	pthread_mutex_t mtex;
	CSharedMem<ITCH_SHM> shm;
	ITCH_SHM *pidxdata;
	CJournal *journal, *received, *executed;
	CPriceFraction *pf;

	typedef std::map<std::string, CETF*> _ETFMAP;	//	ETFId, CETF
	_ETFMAP emap;
	_QUEUE_ *outq;

	//	ETF Batch Order Map
	typedef std::multimap<std::string, ETF_BATCH_ORDER*> __SMAP__;	//	Map by ETF Code
	__SMAP__ smap;
	//__SMAP__ idmap;
	pthread_mutex_t stex;

	bool IsDuplicateOrderId(ETF_BATCH_ORDER *o);

	bool CheckOrder(ETF_BATCH_ORDER *o);
	bool IsValidOrder(ETF_BATCH_ORDER *o);
	void WriteExecutedOrderLog(ETF_BATCH_ORDER *o, OL_UINT64 eUnit);

	void ProcessCheckOrders();
	//static void client_worker(void *args);
	static void forecast_worker(void *args);

	CETF* GetETF(const char *etfCode);
	CETF* SetETF(const char *dealerId, const char *etf, const char *desc, const char *bufAccount, const char *tradeAccount, OL_UINT32 uSize, OL_INT64 lCash, OL_UINT32 mCU, double spread, double up, double disc, uint8_t fl);
};

/*
class CETF
{
public:
	typedef std::map<std::string, TRAILING_ORDER*> __BOMAP__;	//	Map by Break Order Id
	typedef std::multimap<std::string, TRAILING_ORDER*> __AOMAP__;	//	Map by Stock Code
	class CTrailingProcess
	{
	public:
		CTrailingProcess(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, CPriceFraction *pfrac, const char *identity, const char *addr);
		~CTrailingProcess();

		void Release();
		void AddOrder(TRAILING_ORDER *o);
		void WithdrawOrder(const char *orderId);
		OL_UINT32 ProcessData(QUOTE *q);	// return number of order matched criteria
		void ProcessOrderReply(CTEOrderPackage *pkg);
		void ProcessSubscTrailingOrder(CTEOrderPackage *pkg);
		void ProcessUnsubscTrailingOrder(CTEOrderPackage *pkg);
		void SendHeartBeat();
		void CleanupAllTrailingOrder();
		void RequestResendAllTrailingOrder();
		void SendUpdateTrailingState(TRAILING_ORDER *o);
		void GetOrderDescriptions(char *szT, TRAILING_ORDER *o);
		void ComposeOrderPackage(TRAILING_ORDER *o);
	private:
		bool bRunning;
		bool bHoliday;
		pthread_t tid;
		__AOMAP__ amap;
		__BOMAP__ bmap;
		int lCurDate;
		CSharedMem<IDX_SHM> shm;
		IDX_SHM *pidxdata;
		CJournal *journal, *received, *executed;
		CPriceFraction *pf;
		char szIdentity[32];	//	TrailingOrder_A	// B C D dst
		void *context, *client;
		pthread_mutex_t mtex, ztex;
		bool CheckOrder(TRAILING_ORDER *o);
		void WriteExecutedOrderLog(TRAILING_ORDER *o);
		static void client_worker(void *args);
	};
	typedef std::vector<CTrailingProcess*> __PVECTOR__;
public:
	CTrailingOrder(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, CPriceFraction *pfrac, const char *addr);
	~CTrailingOrder();

	void AddQuote(QUOTE *q);
	void StartThread(OL_UINT16 n);
	void StopThread();

private:
	OL_UINT16 nThread;
	CPriceFraction *pf;
	bool bThreadRunning;
	pthread_t *ptid;
	typedef CGQueue<QUOTE> _QUEUE_;
	_QUEUE_ queue;
	CJournal *journal, *received, *executed;
	__PVECTOR__ arrProcess;

	static void quote_worker(void *args);
};
*/