// #include "header/find_idx.h"
#include <string.h>
#include "header/idxdtfeed.h"

int find_idx_board(const char *szBoard)
{
	if (0 == strcmp(szBoard, IDX_RG_BOARD))
		return 0;
	else if (0 == strcmp(szBoard, IDX_TN_BOARD))
		return 1;
	else if (0 == strcmp(szBoard, IDX_NG_BOARD))
		return 2;
	//	else if (0 == strcmp(szBoard, IDX_TS_BOARD))
	//		return 3;
	return -1;
}

int convert_idx_board(const unsigned int iboard, char *szBoard)
{
	if (0 == iboard)
		strcpy(szBoard, IDX_RG_BOARD);
	else if (1 == iboard)
		strcpy(szBoard, IDX_TN_BOARD);
	else if (2 == iboard)
		strcpy(szBoard, IDX_NG_BOARD);
	else if (3 == iboard)
		strcpy(szBoard, IDX_TS_BOARD);
	else
		return -1;
	return 0;
}

int compare_idx_broker(const void *t1, const void *t2)
{
	IDX_BROKER *s1 = (IDX_BROKER *)t1, *s2 = (IDX_BROKER *)t2;
	return strcmp(s1->szBrokerId, s2->szBrokerId);
}

int compare_brk_trade_summary(const void *s1, const void *s2)
{
	BROKER_TRX_SUMM *t1 = (BROKER_TRX_SUMM *)s1, *t2 = (BROKER_TRX_SUMM *)s2;
	return strcmp(t1->szStockCode, t2->szStockCode);
}

int compare_idx_stock(const void *t1, const void *t2)
{
	IDX_STOCK *s1 = (IDX_STOCK *)t1, *s2 = (IDX_STOCK *)t2;
	return strcmp(s1->szStockCode, s2->szStockCode);
}

int compare_idx_index(const void *t1, const void *t2)
{
	IDX_INDEX *s1 = (IDX_INDEX *)t1, *s2 = (IDX_INDEX *)t2;
	return strcmp(s1->szIndexCode, s2->szIndexCode);
	// return strcmp(s2->szIndexCode, s1->szIndexCode);
}

int compare_idx_trade_book(const void *t1, const void *t2)
{
	IDX_TRADE_BOOK *s1 = (IDX_TRADE_BOOK *)t1, *s2 = (IDX_TRADE_BOOK *)t2;
	return s2->lPrice - s1->lPrice;
}

int compare_idx_broker_summary(const void *t1, const void *t2)
{
	IDX_BROKER_SUMMARY *s1 = (IDX_BROKER_SUMMARY *)t1, *s2 = (IDX_BROKER_SUMMARY *)t2;
	return strcmp(s1->szBrokerId, s2->szBrokerId);
}

int compare_idx_sector(const void *t1, const void *t2)
{
	IDX_SECTOR *s1 = (IDX_SECTOR *)t1, *s2 = (IDX_SECTOR *)t2;
	return strcmp(s1->szId, s2->szId);
}

int find_idx_stock(const char *stockCode, IDX_STOCK *data, size_t total, size_t datasize)
{
	IDX_STOCK *pr = (IDX_STOCK *)bsearch(stockCode, data, total, datasize, (int (*)(const void *, const void *))compare_idx_stock);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

int find_idx_broker(const char *brokerId, IDX_BROKER *data, size_t total, size_t datasize)
{
	IDX_BROKER *pr = (IDX_BROKER *)bsearch(brokerId, data, total, datasize, (int (*)(const void *, const void *))compare_idx_broker);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

int find_idx_index(const char *index, IDX_INDEX *data, size_t total, size_t datasize)
{
	IDX_INDEX *pr = (IDX_INDEX *)bsearch(index, data, total, datasize, (int (*)(const void *, const void *))compare_idx_index);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

int find_idx_trade_book(OL_UINT32 price, IDX_TRADE_BOOK *data, size_t total, size_t datasize)
{
	IDX_TRADE_BOOK *pr = (IDX_TRADE_BOOK *)bsearch(&price, data, total, datasize, (int (*)(const void *, const void *))compare_idx_trade_book);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

int find_idx_broker_summary(const char *szBrokerId, IDX_BROKER_SUMMARY *data, size_t total, size_t datasize)
{
	IDX_BROKER_SUMMARY *pr = (IDX_BROKER_SUMMARY *)bsearch(szBrokerId, data, total, datasize, (int (*)(const void *, const void *))compare_idx_broker_summary);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

int find_idx_sector(const char *szSectorId, IDX_SECTOR *data, size_t total, size_t datasize)
{
	IDX_SECTOR *pr = (IDX_SECTOR *)bsearch(szSectorId, data, total, datasize, (int (*)(const void *, const void *))compare_idx_sector);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}
