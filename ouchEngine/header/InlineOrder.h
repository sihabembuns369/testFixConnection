#include <vector>
#include <map>
#include <string>
#include <zmq.h>
#include "GQueue.h"
#include "itch_shm.h"
#include "SharedMem.h"
#include "AdvancedOrder.h"

class CJournal;
class CPriceFraction;
class CTEOrderPackage;

class CInlineOrder
{
public:
	typedef std::map<std::string, INLINE_ORDER*> __IOMAP__;	//	Map by Inline Order Id
	typedef std::multimap<std::string, INLINE_ORDER*> __AOMAP__;	//	Map by Stock Code
	class CInlineProcess
	{
	public:
		CInlineProcess(uint32_t lotSize, CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, CPriceFraction *pfrac, const char *identity, const char *addr);
		~CInlineProcess();

		void Release();
		void AddOrder(INLINE_ORDER *o);
		void WithdrawOrder(const char *orderId);
		OL_UINT32 ProcessData(QUOTE *q);	// return number of order matched criteria
		void ProcessOrderReply(CTEOrderPackage *pkg);
		void ProcessSubscInlineOrder(CTEOrderPackage *pkg);
		void ProcessUnsubscInlineOrder(CTEOrderPackage *pkg);
		void SendHeartBeat();
		void CleanupAllInlineOrder();
		void RequestResendAllInlineOrder();
		void SendUpdateInlineState(INLINE_ORDER *o);
		void GetOrderDescriptions(char *szT, INLINE_ORDER *o);
		void ComposeOrderPackage(INLINE_ORDER *o);
		OL_UINT64 GetAccumOBVolume(IDX_STOCK *ps, OL_UINT8 cmd, OL_UINT16 depth, OL_UINT32 &lPrice);
		
		void SetParams(uint16_t lVolumeRatio, uint16_t obDepth) { 
			limitVolRatio = lVolumeRatio;
			orderBookDepth = obDepth;
		};
		
	private:
		bool bRunning;
		bool bHoliday;
		pthread_t tid;
		__AOMAP__ amap;
		__IOMAP__ imap;
		uint32_t lCurDate, SHARES_LOT;
		uint16_t limitVolRatio, orderBookDepth;
		CSharedMem<ITCH_SHM> shm;
		ITCH_SHM *pidxdata;
		CJournal *journal, *received, *executed;
		CPriceFraction *pf;
		char szIdentity[64];	//	InlineOrder_A	// B C D dst
		void *context, *client;
		pthread_mutex_t mtex, ztex;
		bool CheckOrder(INLINE_ORDER *o);
		void WriteExecutedOrderLog(INLINE_ORDER *o);
		static void client_worker(void *args);
	};
	typedef std::vector<CInlineProcess*> __PVECTOR__;
public:
	CInlineOrder(OL_UINT32 lotSize, CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, CPriceFraction *pfrac, const char *addr);
	~CInlineOrder();

	void AddQuote(QUOTE *q);
	void StartThread(OL_UINT16 n);
	void StopThread();
	void PrintQuote(QUOTE *q);

private:
	OL_UINT32 SHARES_LOT;
	OL_UINT16 nThread, limitVolRatio, orderBookDepth;
	CPriceFraction *pf;
	bool bThreadRunning;
	pthread_t *ptid;
	typedef CGQueue<QUOTE> _QUEUE_;
	_QUEUE_ queue;
	CJournal *journal, *received, *executed;
	__PVECTOR__ arrProcess;

	static void quote_worker(void *args);
};
