#ifndef __FIND_IDX_H__
#define __FIND_IDX_H__

#include <stdlib.h>
#include "itch_shm.h"

extern int find_idx_board(const char *szBoard);

extern int convert_idx_board(const unsigned int iboard, char *szBoard);

extern int compare_idx_broker(const void *t1, const void *t2);

extern int compare_brk_trade_summary(const void *s1, const void *s2);

extern int compare_idx_stock(const void *t1, const void *t2);

extern int compare_idx_index(const void *t1, const void *t2);

extern int compare_idx_trade_book(const void *t1, const void *t2);

extern int compare_idx_broker_summary(const void *t1, const void *t2);

extern int compare_idx_sector(const void *t1, const void *t2);

extern int compare_idx_issuer_directory(const void *t1, const void *t2);

extern int compare_idx_orderbook_directory(const void *t1, const void *t2);

extern int compare_idx_futures_directory(const void *t1, const void *t2);

extern int compare_idx_options_directory(const void *t1, const void *t2);

extern int compare_idx_participant_directory(const void *t1, const void *t2);

extern int compare_idx_index_directory(const void *t1, const void *t2);

extern int compare_idx_index_member_directory(const void *t1, const void *t2);

extern int find_idx_stock(const char *stockCode, IDX_STOCK *data, size_t total, size_t datasize);

extern int find_idx_broker(const char *brokerId, IDX_BROKER *data, size_t total, size_t datasize);

extern int find_idx_index(const char *index, IDX_INDEX *data, size_t total, size_t datasize);

extern int find_idx_index_directory(OL_UINT32 orderbookId, ISHM_INDEX_DIRECTORY *data, size_t total, size_t datasize);

extern ISHM_INDEX_DIRECTORY* find_idx_index_directory_obj(OL_UINT32 orderbookId, ISHM_INDEX_DIRECTORY *data, size_t total, size_t datasize);

extern int find_idx_index_member_directory(OL_UINT32 memberOrderbookId, ISHM_INDEX_MEMBER_DIRECTORY *data, size_t total, size_t datasize);

extern ISHM_INDEX_MEMBER_DIRECTORY* find_idx_index_member_directory_obj(OL_UINT32 memberOrderbookId, ISHM_INDEX_MEMBER_DIRECTORY *data, size_t total, size_t datasize);

extern int find_idx_trade_book(OL_UINT32 price, IDX_TRADE_BOOK *data, size_t total, size_t datasize);

extern int find_idx_broker_summary(const char *szBrokerId, IDX_BROKER_SUMMARY *data, size_t total, size_t datasize);

extern int find_idx_sector(OL_UINT32 sectorId, ISHM_SECTOR_DIRECTORY *data, size_t total, size_t datasize);

extern ISHM_SECTOR_DIRECTORY* find_idx_sector_obj(OL_UINT32 sectorId, ISHM_SECTOR_DIRECTORY *data, size_t total, size_t datasize);

extern int find_idx_issuer_directory(OL_UINT32 issuerId, ISHM_ISSUER_DIRECTORY *data, size_t total, size_t datasize);

extern ISHM_ISSUER_DIRECTORY* find_idx_issuer_directory_obj(OL_UINT32 issuerId, ISHM_ISSUER_DIRECTORY *data, size_t total, size_t datasize);

extern int find_idx_orderbook_directory(OL_UINT32 orderbookId, ISHM_ORDERBOOK_DIRECTORY *data, size_t total, size_t datasize);

extern ISHM_ORDERBOOK_DIRECTORY* find_idx_orderbook_directory_obj(OL_UINT32 orderbookId, ISHM_ORDERBOOK_DIRECTORY *data, size_t total, size_t datasize);

extern int find_idx_futures_directory(OL_UINT32 orderbookId, ISHM_FUTURES_DIRECTORY *data, size_t total, size_t datasize);

extern ISHM_FUTURES_DIRECTORY* find_idx_futures_directory_obj(OL_UINT32 orderbookId, ISHM_FUTURES_DIRECTORY *data, size_t total, size_t datasize);

extern int find_idx_options_directory(OL_UINT32 orderbookId, ISHM_OPTIONS_DIRECTORY *data, size_t total, size_t datasize);

extern ISHM_OPTIONS_DIRECTORY* find_idx_options_directory_obj(OL_UINT32 orderbookId, ISHM_OPTIONS_DIRECTORY *data, size_t total, size_t datasize);

extern int find_idx_participant_directory(OL_UINT32 participantId, ISHM_PARTICIPANT_DIRECTORY *data, size_t total, size_t datasize);

extern ISHM_PARTICIPANT_DIRECTORY* find_idx_participant_directory_obj(OL_UINT32 participantId, ISHM_PARTICIPANT_DIRECTORY *data, size_t total, size_t datasize);

#endif
