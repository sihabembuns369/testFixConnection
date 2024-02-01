#include <vector>
#include <map>
#include <string>
#include <zmq.h>
#include "GQueue.h"
#include "itch_shm.h"
#include "SharedMem.h"
#include "AdvancedOrder.h"
#include "PriceFraction.h"

class CJournal;
class CTEOrderPackage;

#define BASKET_ORDER_IDENTITY "BASKETORDER"

class CBasketOrder
{
public:
	class CBasketOrderParams
	{
	public:
		CBasketOrderParams();
		~CBasketOrderParams() {};
		
		uint32_t TWAIT_AR;		//	Waktu tunggu jika no bid / offer (Auto Rejection)
		uint32_t TWAIT_FRAC;	//	Waktu tunggu jika 1 fraksi harga habis, dan untuk lanjut ke fraksi harga berikutnya
		uint16_t NFRAC;			//	Jumlah up / down fraksi harga saat Eksekusi
		uint16_t SPREAD_FRAC;	//	Jumlah max lebar fraksi harga bid - offer
		int64_t MAX_VOLUME;		//	Max lembar saham per transaksi.
	};
	typedef std::map<std::string, BASKET_ORDER*> __OMAP__;	//	Map by Break Order Id
	typedef std::multimap<std::string, BASKET_ORDER*> __SMAP__;	//	Map by Stock Code
	class CEProcess
	{
	public:
		CEProcess(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, CPriceFraction *frac, CBasketOrderParams *param, const char *identity, const char *addr);
		~CEProcess();

		void Release();
		void AddOrder(BASKET_ORDER *o);
		void WithdrawOrder(const char *orderId);
		OL_UINT32 ProcessData(QUOTE *q);	// return number of order matched criteria
		void ProcessOrderReply(CTEOrderPackage *pkg);
		void ProcessSubscBasketUnderlyingOrder(CTEOrderPackage *pkg);
		void ProcessUnsubscBasketOrder(CTEOrderPackage *pkg);
		void SendHeartBeat();
		void CleanupAllBasketOrder();
		void RequestResendAllBasketOrder();
		void GetOrderDescriptions(char *szT, BASKET_ORDER *o, OL_UINT64 execVolume);
		//void ComposeETFBatchOrder(ETF_BATCH_ORDER *o);
		void ComposeOrderPackage(BASKET_ORDER *o, OL_UINT32 price, OL_UINT64 volume);

	private:
		bool bRunning;
		bool bHoliday;
		pthread_t tid, oid;
		__OMAP__ omap;
		__SMAP__ smap;
		OL_UINT32 lCurDate;
		CSharedMem<ITCH_SHM> shm;
		ITCH_SHM *pidxdata;
		CJournal *journal, *received, *executed;
		char szIdentity[32];
		CPriceFraction *pf;
		CBasketOrderParams *op;
		void *context, *client;
		pthread_mutex_t mtex, ztex;
		//bool CheckOrder(AUTO_ORDER *o);
		long GetAutoRejectionPercentage(uint32_t price);
		void WriteExecutedOrderLog(BASKET_ORDER *o, OL_UINT64 execVolume);
		static void client_worker(void *args);
		static void order_worker(void *args);
	};

public:
	CBasketOrder(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, const char *addr);
	~CBasketOrder();

	void AddQuote(QUOTE *q);
	void StartThread(OL_UINT16 n);
	void StopThread();
	void SetParams(uint32_t idx, uint32_t war, uint32_t wfrac, uint16_t nfrac, uint16_t spreadfrac, uint64_t maxvolume) {
		op[idx].TWAIT_AR = war;
		op[idx].TWAIT_FRAC = wfrac;
		op[idx].NFRAC = nfrac;
		op[idx].SPREAD_FRAC = spreadfrac;
		op[idx].MAX_VOLUME = maxvolume;
		journal->WriteLog("[SetParams] - Idx:%u; Wait_AR:%u; Wait_Frac:%u; NFrac:%u; Spread_Frac:%u; MaxVol:%llu;\n",
			idx, op[idx].TWAIT_AR, op[idx].TWAIT_FRAC, op[idx].NFRAC, op[idx].SPREAD_FRAC, op[idx].MAX_VOLUME);
	};

private:
	pthread_t *ptid;
	OL_UINT16 nThread;
	CEProcess *eprocess;
	bool bThreadRunning;
	CBasketOrderParams op[3];
	CPriceFraction pf;
	typedef CGQueue<QUOTE> _QUEUE_;
	_QUEUE_ queue;
	CJournal *journal, *received, *executed;

	static void quote_worker(void *args);
};
