#include "header/find_itch.h"
#include <string.h>
#include "header/idxdtfeed.h"

int find_idx_board(const char *szBoard)
{
	if (0 == strcmp(szBoard, IDX_RG_BOARD))
		return LIDX_RG_BOARD;
	else if (0 == strcmp(szBoard, IDX_TN_BOARD))
		return LIDX_TN_BOARD;
	else if (0 == strcmp(szBoard, IDX_NG_BOARD))
		return LIDX_NG_BOARD;
	return -1;
}

int convert_idx_board(const unsigned int iboard, char *szBoard)
{
	if (LIDX_RG_BOARD == iboard)
		strcpy(szBoard, IDX_RG_BOARD);
	else if (LIDX_TN_BOARD == iboard)
		strcpy(szBoard, IDX_TN_BOARD);
	else if (LIDX_NG_BOARD == iboard)
		strcpy(szBoard, IDX_NG_BOARD);
	else if (LIDX_TS_BOARD == iboard)
		strcpy(szBoard, IDX_TS_BOARD);
	else return -1;
	return 0;
}

int compare_idx_broker(const void *t1, const void *t2)
{
	IDX_BROKER *s1 = (IDX_BROKER*)t1, *s2 = (IDX_BROKER*)t2;
	return strcmp(s1->szBrokerId, s2->szBrokerId);
}

int compare_brk_trade_summary(const void *s1, const void *s2)
{
	BROKER_TRX_SUMM *t1 = (BROKER_TRX_SUMM*)s1, *t2 = (BROKER_TRX_SUMM*)s2;
	return strcmp(t1->szStockCode, t2->szStockCode);
}

int compare_idx_stock(const void *t1, const void *t2)
{
	IDX_STOCK *s1 = (IDX_STOCK*)t1, *s2 = (IDX_STOCK*)t2;
	return strcmp(s1->szStockCode, s2->szStockCode);
}

int compare_idx_index(const void *t1, const void *t2)
{
	IDX_INDEX *s1 = (IDX_INDEX*)t1, *s2 = (IDX_INDEX*)t2;
	return strcmp(s1->szIndexCode, s2->szIndexCode);
}

int compare_idx_trade_book(const void *t1, const void *t2)
{
	IDX_TRADE_BOOK *s1 = (IDX_TRADE_BOOK*)t1, *s2 = (IDX_TRADE_BOOK*)t2;
	return s2->lPrice - s1->lPrice;
}

int compare_idx_broker_summary(const void *t1, const void *t2)
{
	IDX_BROKER_SUMMARY *s1 = (IDX_BROKER_SUMMARY*)t1, *s2 = (IDX_BROKER_SUMMARY*)t2;
	return strcmp(s1->szBrokerId, s2->szBrokerId);
}

int compare_idx_sector(const void *t1, const void *t2) {
	return ((ISHM_SECTOR_DIRECTORY*)t1)->sectorId - ((ISHM_SECTOR_DIRECTORY*)t2)->sectorId;
}

int compare_idx_issuer_directory(const void *t1, const void *t2) {
	return ((ISHM_ISSUER_DIRECTORY*)t1)->issuerId - ((ISHM_ISSUER_DIRECTORY*)t2)->issuerId;
}

int compare_idx_orderbook_directory(const void *t1, const void *t2) {
	return ((ISHM_ORDERBOOK_DIRECTORY*)t1)->orderbookId - ((ISHM_ORDERBOOK_DIRECTORY*)t2)->orderbookId;
}

int compare_idx_futures_directory(const void *t1, const void *t2) {
	return ((ISHM_FUTURES_DIRECTORY*)t1)->orderbookId - ((ISHM_FUTURES_DIRECTORY*)t2)->orderbookId;
}

int compare_idx_options_directory(const void *t1, const void *t2) {
	return ((ISHM_OPTIONS_DIRECTORY*)t1)->orderbookId - ((ISHM_OPTIONS_DIRECTORY*)t2)->orderbookId;
}

int compare_idx_participant_directory(const void *t1, const void *t2) {
	return ((ISHM_PARTICIPANT_DIRECTORY*)t1)->participantId - ((ISHM_PARTICIPANT_DIRECTORY*)t2)->participantId;
}

int compare_idx_index_directory(const void *t1, const void *t2) {
	return ((ISHM_INDEX_DIRECTORY*)t1)->orderbookId - ((ISHM_INDEX_DIRECTORY*)t2)->orderbookId;
}

int compare_idx_index_member_directory(const void *t1, const void *t2) {
	return ((ISHM_INDEX_MEMBER_DIRECTORY*)t1)->memberOrderbookId - ((ISHM_INDEX_MEMBER_DIRECTORY*)t2)->memberOrderbookId;
}

int find_idx_stock(const char *stockCode, IDX_STOCK *data, size_t total, size_t datasize)
{
	IDX_STOCK *pr = (IDX_STOCK*)bsearch(stockCode, data, total, datasize, (int (*)(const void*, const void*))compare_idx_stock);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

int find_idx_broker(const char *brokerId, IDX_BROKER *data, size_t total, size_t datasize)
{
	IDX_BROKER *pr = (IDX_BROKER*)bsearch(brokerId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_broker);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

int find_idx_index(const char *index, IDX_INDEX *data, size_t total, size_t datasize)
{
	IDX_INDEX *pr = (IDX_INDEX*)bsearch(index, data, total, datasize, (int (*)(const void*, const void*))compare_idx_index);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

int find_idx_index_directory(OL_UINT32 orderbookId, ISHM_INDEX_DIRECTORY *data, size_t total, size_t datasize)
{
	ISHM_INDEX_DIRECTORY *pr = (ISHM_INDEX_DIRECTORY*)bsearch(&orderbookId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_index_directory);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

ISHM_INDEX_DIRECTORY* find_idx_index_directory_obj(OL_UINT32 orderbookId, ISHM_INDEX_DIRECTORY *data, size_t total, size_t datasize) {
	return (ISHM_INDEX_DIRECTORY*)bsearch(&orderbookId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_index_directory);
}

int find_idx_index_member_directory(OL_UINT32 memberOrderbookId, ISHM_INDEX_MEMBER_DIRECTORY *data, size_t total, size_t datasize)
{
	ISHM_INDEX_MEMBER_DIRECTORY *pr = (ISHM_INDEX_MEMBER_DIRECTORY*)bsearch(&memberOrderbookId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_index_member_directory);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

ISHM_INDEX_MEMBER_DIRECTORY* find_idx_index_member_directory_obj(OL_UINT32 memberOrderbookId, ISHM_INDEX_MEMBER_DIRECTORY *data, size_t total, size_t datasize) {
	return (ISHM_INDEX_MEMBER_DIRECTORY*)bsearch(&memberOrderbookId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_index_member_directory);
}

int find_idx_trade_book(OL_UINT32 price, IDX_TRADE_BOOK *data, size_t total, size_t datasize)
{
	IDX_TRADE_BOOK *pr = (IDX_TRADE_BOOK*)bsearch(&price, data, total, datasize, (int (*)(const void*, const void*))compare_idx_trade_book);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

int find_idx_broker_summary(const char *szBrokerId, IDX_BROKER_SUMMARY *data, size_t total, size_t datasize)
{
	IDX_BROKER_SUMMARY *pr = (IDX_BROKER_SUMMARY*)bsearch(szBrokerId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_broker_summary);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

int find_idx_sector(OL_UINT32 sectorId, ISHM_SECTOR_DIRECTORY *data, size_t total, size_t datasize)
{
	ISHM_SECTOR_DIRECTORY *pr = (ISHM_SECTOR_DIRECTORY*)bsearch(&sectorId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_sector);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

ISHM_SECTOR_DIRECTORY* find_idx_sector_obj(OL_UINT32 sectorId, ISHM_SECTOR_DIRECTORY *data, size_t total, size_t datasize) {
	return (ISHM_SECTOR_DIRECTORY*)bsearch(&sectorId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_sector);
}

int find_idx_issuer_directory(OL_UINT32 issuerId, ISHM_ISSUER_DIRECTORY *data, size_t total, size_t datasize)
{
	ISHM_ISSUER_DIRECTORY *pr = (ISHM_ISSUER_DIRECTORY*)bsearch(&issuerId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_issuer_directory);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

ISHM_ISSUER_DIRECTORY* find_idx_issuer_directory_obj(OL_UINT32 issuerId, ISHM_ISSUER_DIRECTORY *data, size_t total, size_t datasize) {
	return (ISHM_ISSUER_DIRECTORY*)bsearch(&issuerId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_issuer_directory);
} 

int find_idx_orderbook_directory(OL_UINT32 orderbookId, ISHM_ORDERBOOK_DIRECTORY *data, size_t total, size_t datasize)
{
	ISHM_ORDERBOOK_DIRECTORY *pr = (ISHM_ORDERBOOK_DIRECTORY*)bsearch(&orderbookId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_orderbook_directory);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

ISHM_ORDERBOOK_DIRECTORY* find_idx_orderbook_directory_obj(OL_UINT32 orderbookId, ISHM_ORDERBOOK_DIRECTORY *data, size_t total, size_t datasize) {
	return (ISHM_ORDERBOOK_DIRECTORY*)bsearch(&orderbookId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_orderbook_directory);
}

int find_idx_futures_directory(OL_UINT32 orderbookId, ISHM_FUTURES_DIRECTORY *data, size_t total, size_t datasize)
{
	ISHM_FUTURES_DIRECTORY *pr = (ISHM_FUTURES_DIRECTORY*)bsearch(&orderbookId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_futures_directory);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

ISHM_FUTURES_DIRECTORY* find_idx_futures_directory_obj(OL_UINT32 orderbookId, ISHM_FUTURES_DIRECTORY *data, size_t total, size_t datasize) {
	return (ISHM_FUTURES_DIRECTORY*)bsearch(&orderbookId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_futures_directory);
}

int find_idx_options_directory(OL_UINT32 orderbookId, ISHM_OPTIONS_DIRECTORY *data, size_t total, size_t datasize)
{
	ISHM_OPTIONS_DIRECTORY *pr = (ISHM_OPTIONS_DIRECTORY*)bsearch(&orderbookId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_options_directory);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

ISHM_OPTIONS_DIRECTORY* find_idx_options_directory_obj(OL_UINT32 orderbookId, ISHM_OPTIONS_DIRECTORY *data, size_t total, size_t datasize) {
	return (ISHM_OPTIONS_DIRECTORY*)bsearch(&orderbookId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_options_directory);
}

int find_idx_participant_directory(OL_UINT32 participantId, ISHM_PARTICIPANT_DIRECTORY *data, size_t total, size_t datasize)
{
	ISHM_PARTICIPANT_DIRECTORY *pr = (ISHM_PARTICIPANT_DIRECTORY*)bsearch(&participantId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_participant_directory);
	if (pr)
		return ((OL_UINT64)pr - (OL_UINT64)data) / datasize;
	return -1;
}

ISHM_PARTICIPANT_DIRECTORY* find_idx_participant_directory_obj(OL_UINT32 participantId, ISHM_PARTICIPANT_DIRECTORY *data, size_t total, size_t datasize) {
	return (ISHM_PARTICIPANT_DIRECTORY*)bsearch(&participantId, data, total, datasize, (int (*)(const void*, const void*))compare_idx_participant_directory);
}
