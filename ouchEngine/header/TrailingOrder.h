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

class CTrailingOrder
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
		uint32_t lCurDate;
		CSharedMem<ITCH_SHM> shm;
		ITCH_SHM *pidxdata;
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
