#ifndef __MISCFUNC_H__
#define __MISCFUNC_H__

#include <set>
#include <map>
#include <string>
#include <time.h>
#include <stdint.h>
#include "constants.h"

typedef struct _SCHEDULE_
{
	bool session_1_active;
	bool session_2_active;
	bool preopening_active;
	bool post_trading_active;
	bool before_market_open_active;
	bool after_market_closed_active;
	bool call_auction_session_1_active;
	bool call_auction_session_2_active;
	bool call_auction_session_3_active;
	bool call_auction_session_4_active;
	bool call_auction_session_5_active;
	bool call_auction_post_trading_active;
	//	hhmmssxxx
	uint32_t before_market_open;
	uint32_t after_market_closed;
	uint32_t begin_session_1;
	uint32_t begin_session_2;
	uint32_t begin_preopening;
	uint32_t begin_post_trading;
	uint32_t begin_call_auction_session_1;
	uint32_t begin_call_auction_session_2;
	uint32_t begin_call_auction_session_3;
	uint32_t begin_call_auction_session_4;
	uint32_t begin_call_auction_session_5;
	uint32_t begin_call_auction_post_trading;
	//	hhmmssxxx
	uint32_t end_session_1;	
	uint32_t end_session_2;
	uint32_t end_preopening;
	uint32_t end_post_trading;
	uint32_t end_call_auction_session_1;
	uint32_t end_call_auction_session_2;
	uint32_t end_call_auction_session_3;
	uint32_t end_call_auction_session_4;
	uint32_t end_call_auction_session_5;
	uint32_t end_call_auction_post_trading;
} SCHEDULE;

const uint32_t SECONDS_PER_DAY = 86400;	//	24 * 60 * 60
const uint32_t SECONDS_PER_HOUR = 3600;	//	60 * 60
const uint32_t SECONDS_PER_MINUTE = 60;	//	60

#define SECONDS_PER_DAY 86400	//	24 * 60 * 60
#define SECONDS_PER_HOUR 3600	//	60 * 60
#define SECONDS_PER_MINUTE 60	//	60

typedef std::set<int> __TSET__;
typedef std::map<uint32_t, std::string> __HOLIDAYS_MAP__;

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
int sneof(char *buf, size_t len, size_t lmax);
void setblocking(int fd);
void setnonblocking(int fd);
const char *GetNextToken(const char *p, char *Buf);
uint32_t GetCurrentDate();	// yyyymmdd
uint32_t GetCurrentTime();	// hhmmss
uint32_t GetCurrentDateTime(uint32_t &ldate);	//	return hhmmss and yyyymmdd for ldate
uint32_t GetCurrentTimeMs();	//	hhmmssnnn
uint32_t GetCurrentTimeMs(uint32_t &ldate);	//	return hhmmssnnn and yyyymmdd for ldate
time_t DateAdd(time_t ldate, long ndays);
uint32_t CalculateWeekYear(uint32_t ldate);	// Format yyyyww
char* TrimRight(char *sz);
char* TrimLeft(char *sz);
char* Trim(char *sz);
void strtoupper(char *szT);
void strtolower(char *szT);
size_t explode(const char split, char *input, char **tP, unsigned int limit); // Explode a string in an array
int spc_email_isvalid(const char *address);

void LoadHoliday(__TSET__ &hset);
uint32_t LoadHolidaysMap(uint32_t &lcurDate, __HOLIDAYS_MAP__ &hmap);
int isHoliday(uint32_t &lcurDate);	// 1 -> Holiday
bool isHoliday(uint32_t &lcurDate, uint32_t &weekDay);

void GetBaseDir(char *szDir);
void GetWin64AppFolder(char *szDir);
void GetConfigDir(char *szDir, const char *fileName);
void GetDataDir(char *szDir, const char *fileName);
void GetDataDatsDir(char *szDir, const char *fileName);
void GetLogDir(char *szDir, const char *fileName);
void GetTmpDir(char *szDir, const char *fileName);
void GetFifoDir(char *szDir, const char *fileName);
void GetOUCHLogDir(char *szDir, const char *filename);

void GetWin64AppFolder(char *szDir);

bool LoadTradingSchedule(SCHEDULE *schedule);

//	Convert number of seconds since midnight to time
OL_UINT32 ConvertTimeStamp(OL_UINT32 nSecs);	//	Output hhmmss
OL_UINT32 ConvertTimeStamp(OL_UINT32 nSecs, OL_UINT32 nanoSecs);	//	Output hhmmss
OL_UINT32 ConvertNSTimeStamp(OL_UINT64 nanoSecs);	//	Convert Nano Seconds Since Midnight to hhmmssnnn
void ConvertTimeStamp(OL_UINT32 nSecs, OL_UINT32 &h, OL_UINT32 &m, OL_UINT32 &s);

#endif
