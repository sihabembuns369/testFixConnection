#include <string.h>
#include "header/StockInfo.h"
#include "header/TEOrderPackage.h"

CStockInfo::CStockInfo() : isProcess(false)
{
	sstk.clear();
	pthread_mutex_init(&mutex, NULL);
}

CStockInfo::~CStockInfo()
{
	Release();
	pthread_mutex_destroy(&mutex);
}

bool CStockInfo::GetStockInfo(const char *szStockCode, STOCK_INFO &si)
{
	bool b = true;
	pthread_mutex_lock(&mutex);
	_SSTOCKINFO::const_iterator i = sstk.find(szStockCode);
	if (i != sstk.end())
		memcpy(&si, i->second, sizeof(STOCK_INFO));
	else
		b = false;
	pthread_mutex_unlock(&mutex);
	return b;
}

bool CStockInfo::GetStockFlags(const char *szStockCode, uint32_t &flags)
{
	bool b = true;
	pthread_mutex_lock(&mutex);
	_SSTOCKINFO::const_iterator i = sstk.find(szStockCode);
	if (i != sstk.end())
		flags = i->second->flags;
	else
		b = false;
	pthread_mutex_unlock(&mutex);
	return b;
}

bool CStockInfo::IsValidStockCode(const char *szStockCode)
{
	pthread_mutex_lock(&mutex);
	_SSTOCKINFO::const_iterator i = sstk.find(szStockCode);
	bool b = (i != sstk.end());
	pthread_mutex_unlock(&mutex);
	return b;
}

uint32_t CStockInfo::GetStockFlags(const char *szStockCode)
{
	pthread_mutex_lock(&mutex);
	_SSTOCKINFO::const_iterator i = sstk.find(szStockCode);
	uint32_t flags = (i == sstk.end())?0:i->second->flags;
	pthread_mutex_unlock(&mutex);
	return flags;
}

bool CStockInfo::IsProcess()
{
	return isProcess;
}

bool CStockInfo::IsEmpty()
{
	pthread_mutex_lock(&mutex);
	bool b = sstk.empty();
	pthread_mutex_unlock(&mutex);
	return b;
}

void CStockInfo::Release()
{
	pthread_mutex_lock(&mutex);
	for (_SSTOCKINFO::const_iterator pos = sstk.begin(); pos != sstk.end(); ++pos)
		delete pos->second;
	sstk.clear();
	isProcess = false;
	pthread_mutex_unlock(&mutex);
}

void CStockInfo::ProcessStockInfo(CTEOrderPackage *pkg)
{
	Release();
	STOCK_INFO *psi = NULL;
	uint32_t nRec = pkg->GetDWord();
	pthread_mutex_lock(&mutex);
	for (uint32_t i = 0; i < nRec; ++i)
	{
		try
		{
			if (NULL != (psi = new STOCK_INFO))
			{
				uint8_t isPreopen = pkg->GetByte();	//	IsPreopening
				psi->flags = pkg->GetDWord();
				if (isPreopen)
					psi->flags |= SI_PREOPENING;
				psi->lotSize = pkg->GetDWord();
				pkg->GetDWord();	//	StockId
				//uint32_t stockId = pkg->GetDWord();	//	StockId
				uint16_t slen = pkg->GetWord(); pkg->GetString(slen, psi->szStockCode, CTEOrderPackage::GP_TRIM_RIGHT);
				sstk.insert(_SSTOCKINFO::value_type(psi->szStockCode, psi));
			}
		}
		catch (...)
		{
			//jonecLogger.WriteJonecErrLog("[CStockInfo::ProcessStockInfo] - Unknown Exception.\n");
		}
	}
	isProcess = true;
	pthread_mutex_unlock(&mutex);
}

void CStockInfo::SetStockInfo(uint32_t orderbookId, const char *stockCode, const char *board, const char *instrument, const char *remark2, uint32_t sectorId, uint32_t sharesLot)
{
	STOCK_INFO *psi = NULL;
	pthread_mutex_lock(&mutex);
	_SSTOCKINFO::const_iterator i = sstk.find(stockCode);
	if (i == sstk.end())
	{
		psi = new STOCK_INFO;
		strcpy(psi->szStockCode, stockCode);
		sstk.insert(_SSTOCKINFO::value_type(psi->szStockCode, psi));
	}
	else
		psi = i->second;
	psi->flags = 0;
	psi->lotSize = sharesLot;
	psi->sectorId = sectorId;
	strcpy(psi->szStockCode, stockCode);
	if (0 == strcmp(instrument, INSTRUMENT_ORDI_PREOPEN))
		psi->flags |= SI_PREOPENING;
	else if (0 == strcmp(instrument, INSTRUMENT_RGHI))
		psi->flags |= SI_RIGHT;
	else if (0 == strcmp(instrument, INSTRUMENT_WARI))
		psi->flags |= SI_WARRANT;
	else if (0 == strcmp(instrument, INSTRUMENT_WATCH_CALL))
		psi->flags |= SI_WATCH_CALL;
	else if (0 == strcmp(instrument, INSTRUMENT_RGHI_CALL))
		psi->flags |= (SI_WATCH_CALL | SI_RIGHT);
	else if (0 == strcmp(instrument, INSTRUMENT_WARI_CALL))
		psi->flags |= (SI_WATCH_CALL | SI_WARRANT);
	else if (0 == strcmp(instrument, INSTRUMENT_MUTI))
		psi->flags |= SI_ETF;
	if ('M' == remark2[2])	//	Marginable Stocks
		psi->flags |= SI_MARGINABLE;
	else if ('S' == remark2[2])
		psi->flags |= (SI_MARGINABLE | SI_SHORT_SELL);
	pthread_mutex_unlock(&mutex);
}