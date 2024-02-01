#pragma once
#include <map>
#include <string>
#include <stdint.h>

class CTEOrderPackage;
class CStockInfo
{
public:
	enum STK_FLAGS
	{
		SI_ACTIVE	  = 0x00000001,
		SI_MARGINABLE = 0x00000002,
		SI_SHORT_SELL = 0x00000004,
		SI_RIGHT	  = 0x00000008,
		SI_WARRANT	  = 0x00000010,
		SI_ETF		  = 0x00000020,
		SI_WATCH_CALL = 0x00000400,
		SI_PREOPENING = 0x80000000
	};
	struct STOCK_INFO
	{
		uint32_t
				sectorId,
				lotSize,
				flags;
		char szStockCode[32];
	};
	typedef std::map<std::string, STOCK_INFO*> _SSTOCKINFO;

public:
	CStockInfo();
	~CStockInfo();

	void Release();
	void ProcessStockInfo(CTEOrderPackage *pkg);
	bool IsEmpty();
	bool IsProcess();
	bool GetStockFlags(const char *szStockCode, uint32_t &flags);
	bool GetStockInfo(const char *szStockCode, STOCK_INFO &si);
	bool IsValidStockCode(const char *szStockCode);
	void ResetProcess() {isProcess = false;};
	uint32_t GetStockFlags(const char *szStockCode);

	void SetStockInfo(uint32_t orderbookId, const char *stockCode, const char *board, const char *instrument, const char *remark2, uint32_t sectorId, uint32_t sharesLot);
	
private:
	bool isProcess;
	 _SSTOCKINFO sstk;
	 pthread_mutex_t mutex;
};
