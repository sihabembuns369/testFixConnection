#include <vector>
#include <map>
#include <string>
#include <zmq.h>
#include "GQueue.h"
#include "itch_shm.h"
#include "SharedMem.h"
#include "AdvancedOrder.h"

class CJournal;
class CTEOrderPackage;

class CAutoOrder
{
public:
	typedef std::map<std::string, AUTO_ORDER*> __BOMAP__;	//	Map by Break Order Id
	typedef std::multimap<std::string, AUTO_ORDER*> __AOMAP__;	//	Map by Stock Code
	class CAutoProcess
	{
	public:
		CAutoProcess(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, const char *identity, const char *addr);
		~CAutoProcess();

		void Release();
		void AddOrder(AUTO_ORDER *o);
		void WithdrawOrder(const char *orderId);
		OL_UINT32 ProcessData(QUOTE *q);	// return number of order matched criteria
		void ProcessOrderReply(CTEOrderPackage *pkg);
		void ProcessSubscBreakOrder(CTEOrderPackage *pkg);
		void ProcessUnsubscBreakOrder(CTEOrderPackage *pkg);
		void SendHeartBeat();
		void CleanupAllBreakOrder();
		void RequestResendAllBreakOrder();
		void GetOrderDescriptions(char *szT, AUTO_ORDER *o);
		void ComposeOrderPackage(AUTO_ORDER *o);
	private:
		bool bRunning;
		bool bHoliday;
		pthread_t tid;
		__AOMAP__ amap;
		__BOMAP__ bmap;
		uint32_t lCurDate;
		CSharedMem<ITCH_SHM> shm;
		ITCH_SHM *pidxdata;
		CJournal *journal, *received, *executed;
		char szIdentity[32];	//	AutoOrder_A	// B C D dst
		void *context, *client;
		pthread_mutex_t mtex, ztex;
		bool CheckOrder(AUTO_ORDER *o);
		void WriteExecutedOrderLog(AUTO_ORDER *o);
		static void client_worker(void *args);
	};
	typedef std::vector<CAutoProcess*> __PVECTOR__;
public:
	CAutoOrder(CJournal *pjournal, CJournal *preceived, CJournal *pexecuted, const char *addr);
	~CAutoOrder();

	void AddQuote(QUOTE *q);
	void StartThread(OL_UINT16 n);
	void StopThread();

private:
	OL_UINT16 nThread;
	bool bThreadRunning;
	pthread_t *ptid;
	typedef CGQueue<QUOTE> _QUEUE_;
	_QUEUE_ queue;
	CJournal *journal, *received, *executed;
	__PVECTOR__ arrProcess;

	static void quote_worker(void *args);
};
