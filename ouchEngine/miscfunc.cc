#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "header/miscfunc.h"
#include "header/config.h"

#define ENV_OLTS_ROOT "OLTS_ROOT"

void *xmalloc(size_t size)
{
	void *r = malloc(size);
	if (r == NULL) {
		printf("[ERR] Not enougth memory\n");
		exit(0);
	}
	return r;
}

void *xrealloc(void *ptr, size_t size)
{
	void *r = realloc(ptr, size);
	if (r == NULL) {
		printf("[ERR] Not enougth memory\n");
		exit(0);
	}
	return r;
}

int sneof(char *buf, size_t len, size_t lmax)
{
	size_t pos = 0;
	for (char *pBuf = buf; pos < len && pos < lmax; ++pos)
		if ('\n' == pBuf[pos])
			return pos + 1;
	return -1;
}

void setblocking(int fd)
{
	int old_flags = fcntl(fd, F_GETFL, 0);
	if (old_flags & O_NONBLOCK)
	{
		old_flags &= ~O_NONBLOCK;
		fcntl(fd, F_SETFL, old_flags);
	}
}

void setnonblocking(int fd)
{
	int old_flags = fcntl(fd, F_GETFL, 0);
	if (!(old_flags & O_NONBLOCK))
	{
		old_flags |= O_NONBLOCK;
		fcntl(fd, F_SETFL, old_flags);
	}
}

const char *GetNextToken(const char *p, char *Buf)
{
	if (NULL == p)
		return NULL;
	const char *pc = strchr(p, '|');
	if (pc)
	{
		int len = pc - p;
		strncpy(Buf, p, len);
		Buf[len] = 0x00;
		++pc;
	}
	return pc;
}

uint32_t GetCurrentDate()	// yyyymmdd
{
	time_t ttime = time(NULL);
	struct tm *ptm = localtime(&ttime);
	return ((ptm->tm_year + 1900) * 10000) + ((ptm->tm_mon + 1) * 100) + ptm->tm_mday;
}

uint32_t GetCurrentTime()	// hhmmss
{
	time_t ttime = time(NULL);
	struct tm *ptm = localtime(&ttime);
	return (ptm->tm_hour * 10000) + (ptm->tm_min * 100) + ptm->tm_sec;
}

uint32_t GetCurrentDateTime(uint32_t &ldate)
{
	struct tm ptm;
	time_t ttime = time(NULL);
	localtime_r(&ttime, &ptm);
	ldate = ((ptm.tm_year + 1900) * 10000) + ((ptm.tm_mon + 1) * 100) + ptm.tm_mday;
	return (ptm.tm_hour * 10000) + (ptm.tm_min * 100) + ptm.tm_sec;
}

//uint32_t GetCurrentTimeMs()	//	hhmmssnnn
//{
//	struct timeval tv;
//	struct timezone tz;
//	gettimeofday(&tv, &tz);
//	long hms = tv.tv_sec % SECONDS_PER_DAY;
////	hms += tz.tz_dsttime * SECONDS_PER_HOUR;
////	hms -= tz.tz_minuteswest * SECONDS_PER_MINUTE;
////	// mod `hms` to insure in positive range of [0...SEC_PER_DAY)
////	hms = (hms + SECONDS_PER_DAY) % SECONDS_PER_DAY;
//	
//	// Tear apart hms into h:m:s
//	int hour = hms / SECONDS_PER_HOUR;
//	int min = (hms % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE;
//	int sec = (hms % SECONDS_PER_HOUR) % SECONDS_PER_MINUTE; // or hms % SEC_PER_MIN
//	
//	return (hour * 10000) + (min * 100) + sec;
///*
//	int64_t hms = (tv.tv_sec % SECONDS_PER_DAY) + 
//				  (tz.tz_dsttime * SECONDS_PER_HOUR) - 
//				  (tz.tz_minuteswest * SECONDS_PER_MINUTE);
//	hms = (hms + SECONDS_PER_DAY) % SECONDS_PER_DAY;
//	
//*/
///*
//	return ((hms / SECONDS_PER_HOUR) * 10000000) + 	//	Hour
//			(((hms % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE) * 100000) +	//	Minute
//			(((hms % SECONDS_PER_HOUR) % SECONDS_PER_MINUTE) * 1000) +		//	Second
//			(tv.tv_usec / 1000);		//	Milliseconds
//*/
//	//return ((((tv.tv_sec % SECONDS_IN_DAY) / 3600) * 10000000) +	// Hour
//	//		(((tv.tv_sec % 3600) / 60) * 100000) +				// Minute
//	//		((tv.tv_sec % 60) * 1000) +							// Second
//	//		(tv.tv_usec / 1000));								// MilliSeconds
//}

uint32_t GetCurrentTimeMs()	//	hhmmssnnn
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm *ptm = localtime(&tv.tv_sec);
	return (ptm->tm_hour * 10000000) + (ptm->tm_min * 100000) + (ptm->tm_sec * 1000) + (tv.tv_usec / 1000);
}

uint32_t GetCurrentTimeMs(uint32_t &ldate)	//	return hhmmssnnn and yyyymmdd for ldate
{
	struct tm ptm;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &ptm);
	ldate = ((ptm.tm_year + 1900) * 10000) + ((ptm.tm_mon + 1) * 100) + ptm.tm_mday;
	return (ptm.tm_hour * 10000000) + (ptm.tm_min * 100000) + (ptm.tm_sec * 1000) + (tv.tv_usec / 1000);
}

time_t DateAdd(time_t ldate, long ndays)
{
	long n = ndays * 864000;
	return ldate + n;
}

OL_INT32 CalculateJulianPeriod(OL_INT32 year, OL_INT32 month, OL_INT32 day)
{
	OL_UINT32 a = (14 - month) / 12;
	OL_UINT32 y = year + 4800 - a;
	OL_UINT32 m = month + 12 * a - 3;
	return day + (153 * m + 2) / 5 + y * 365 + y / 4 - y / 100 + y / 400 - 32045;
}

uint32_t CalculateWeekYear(uint32_t ldate)	// Output Format yyyyww
{
	OL_INT32 day = ldate % 100, month = (ldate / 100) % 100, year = ldate / 10000;
	OL_INT32 julianPeriod = CalculateJulianPeriod(year, month, day);

	OL_INT32 d4 = (julianPeriod + 31741 - (julianPeriod % 7)) % 146097 % 36524 % 1461;
	OL_INT32 L  = d4 / 1460;
	OL_INT32 d1 = ((d4 - L) % 365) + L;
	OL_INT32 WeekNumber = d1 / 7 + 1;

	//OL_INT32 WeekDay = julianPeriod % 7 + 1;

	// Week number 1 and 52/52 of a year may actually be in the
	//	previous/next year. Adjust the year number for those cases
	OL_INT32 WeekYear = year + ((WeekNumber == 1) & (month == 12)) - ((WeekNumber > 51) & (month == 1));
	return (WeekYear * 100) + WeekNumber;
}

char* TrimRight(char *sz)
{
	for (register int i = strlen(sz) - 1; i >= 0; --i)
		if (' ' == sz[i] || '\t' == sz[i] || '\r' == sz[i] || '\n' == sz[i])
			sz[i] = 0x00;
		else break;
	return sz;
}

char* TrimLeft(char *sz)
{
	char *pc = sz;
	while (*pc)
	{
		if (' ' == *pc || '\t' == *pc || '\r' == *pc || '\n' == *pc)
			++pc;
		else
		{
			if (pc != sz)
			{
				char *psz = sz;
				while (*pc)
				{
					*psz = *pc;
					++psz; ++pc;
				}
				*psz = 0x00;
			}
			break;
		}
	}
	return sz;
}

char* Trim(char *sz)
{
	return TrimLeft(TrimRight(sz));
}

void strtoupper(char *szT)
{
	for (char *pc = szT; *pc; ++pc)
		if ((*pc >= 'a') && (*pc <= 'z'))
			*pc += ('A' - 'a');
}

void strtolower(char *szT)
{
	for (char *pc = szT; *pc; ++pc)
		if ((*pc > 0x40) && (*pc < 0x5b))
			*pc |= 0x60;
}

size_t explode(const char split, char *input, char **tP, unsigned int limit) // Explode a string in an array
{
	size_t i = 0;
	tP[0] = input;
	for (i = 0; *input; input++)
	{
		if (*input == split)
		{
			i++;
			*input = '\0';
			if(*(input + 1) != '\0' && *(input + 1) != split)
				tP[i] = input + 1;
			else
				i--;
		}
		if ((i+1) == limit)
			return i;
	}
	return i;
}

uint32_t LoadHolidaysMap(uint32_t &lcurDate, __HOLIDAYS_MAP__ &hmap)
{
	time_t ttime = time(NULL);
	uint32_t nDatas = 0, ldate = 0;
	char szBuf[1024] = {}, szDate[16] = {}, szDesc[128] = {};
	struct tm *ptm = localtime(&ttime);
	lcurDate = (ptm->tm_year + 1900) * 10000 + (ptm->tm_mon + 1) * 100 + ptm->tm_mday;
	GetConfigDir(szBuf, HOLIDAYS_LIST_NAME);
	hmap.clear();
	FILE *fp = fopen(szBuf, "r");
	if (NULL == fp)
		return 0;
	while (fgets(szBuf, sizeof(szBuf), fp))
	{
		sscanf(szBuf, "%[^|]|%[^|]|", szDate, szDesc);
		ldate = atoi(szDate);
		if (ldate >= lcurDate)
		{
			nDatas++;
			hmap.insert(std::make_pair(ldate, szDesc));
		}
	}
	fclose(fp);
	return nDatas;
}

void LoadHoliday(__TSET__ &hset)
{
	char szBuf[1024] = {}, szDate[16] = {}, szDesc[128] = {};
	GetConfigDir(szBuf, HOLIDAYS_LIST_NAME);
	hset.clear();
	FILE *fp = fopen(szBuf, "r");
	if (NULL == fp)
		return;
	while (fgets(szBuf, sizeof(szBuf), fp))
	{
		sscanf(szBuf, "%[^|]|%[^|]|", szDate, szDesc);
		hset.insert(atoi(szDate));
	}
	fclose(fp);
}

int isHoliday(uint32_t &lcurDate)
{
	time_t ttime;
	time(&ttime);
	struct tm *ptm = localtime(&ttime);
	lcurDate = (ptm->tm_year + 1900) * 10000 + (ptm->tm_mon + 1) * 100 + ptm->tm_mday;
	if (0 == ptm->tm_wday || 6 == ptm->tm_wday)
		return 1;
	__TSET__ hset;
	LoadHoliday(hset);
	if (hset.end() != hset.find(lcurDate))
		return 1;
	return 0;
}

bool isHoliday(uint32_t &lcurDate, uint32_t &weekDay)
{
	time_t ttime = time(NULL);
	struct tm *ptm = localtime(&ttime);
	lcurDate = (ptm->tm_year + 1900) * 10000 + (ptm->tm_mon + 1) * 100 + ptm->tm_mday;
	if (0 == (weekDay = ptm->tm_wday))	//	Sunday
		return true;
	__TSET__ hset;
	LoadHoliday(hset);
	if (hset.end() != hset.find(lcurDate))
		return true;
	return false;
}

void GetBaseDir(char *szDir)
{
	char *p = getenv(ENV_OLTS_ROOT);
	if (NULL == p)
		p = getenv("HOME");
	if (p)
		strcpy(szDir, p);
	else
		strcpy(szDir, "/OLTS_ITCH");
}

void GetWin64AppFolder(char *szDir)
{
	GetBaseDir(szDir);
	strcat(szDir, "/clientApps/win64");
}

void GetConfigDir(char *szDir, const char *fileName)
{
	char szf[256];
	GetBaseDir(szf);
	sprintf(szDir, "%s/%s/%s", szf, "cfg", fileName);
}

void GetLogDir(char *szDir, const char *fileName)
{
	char szf[256];
	GetBaseDir(szf);
	sprintf(szDir, "%s/%s/%s", szf, "log", fileName);
}

void GetOUCHLogDir(char *szDir, const char *fileName)
{
	char szf[512] = {};
	GetBaseDir(szf);
	sprintf(szDir, "%s/%s/%s", szf, "log/OUCH", fileName);
}

void GetDataDir(char *szDir, const char *fileName)
{
	char szf[256];
	GetBaseDir(szf);
	sprintf(szDir, "%s/%s", szf, "data");
	if (access(szDir, 0600))
		mkdir(szDir, 0600);
	sprintf(szDir, "%s/%s/%s", szf, "data", fileName);
}

void GetDataDatsDir(char *szDir, const char *fileName)
{
	char szf[2048] = {};
	GetBaseDir(szf);
	sprintf(szDir, "%s/%s", szf, "data");
	if (access(szDir, 0600))
		mkdir(szDir, 0600);
	strcat(szDir, "/dats");
	if (access(szDir, 0600))
		mkdir(szDir, 0600);
	strcat(szDir, "/");
	strcat(szDir, fileName);
}

void GetTmpDir(char *szDir, const char *fileName)
{
	char szf[256];
	GetBaseDir(szf);
	sprintf(szDir, "%s/%s/%s", szf, "tmp", fileName);
}

void GetFifoDir(char *szDir, const char *filename)
{
	char szf[256];
	GetBaseDir(szf);
	sprintf(szDir, "%s/%s/%s", szf, "fifo", filename);
}
	
int spc_email_isvalid(const char *address)
{
	int count = 0;
	const char *c, *domain; 
	static char rfc822_specials[] = "()<>@,;:\\\"[]";
	/* first we validate the name portion (name@domain) */ 
	for (c = address; *c; c++)
	{
		if (*c == '\"' && (c == address || *(c - 1) == '.' || *(c - 1) ==  '\"')) 
		{
			while (*++c) 
			{
				if (*c == '\"')
					break; 
				if (*c == '\\' && (*++c == ' '))
					continue; 
				if (*c <= ' ' || *c >= 127) 
					return 0;
			}
			if (!*c++)
				return 0;
			if (*c == '@')
				break;
			if (*c != '.')
				return 0; 
			continue;
		} 
		if (*c == '@')
			break;
		if (*c <= ' ' || *c >= 127)
			return 0;
		if (strchr(rfc822_specials, *c))
			return 0;
	}
	if (c == address || *(c - 1) == '.')
		return 0;
	/* next we validate the domain portion (name@domain) */
	if (!*(domain = ++c))
		return 0;
	do
	{
		if (*c == '.')
		{
			if (c == domain || *(c - 1) == '.')
				return 0;
			count++;
		}
		if (*c <= ' ' || *c >= 127)
			return 0;
		if (strchr(rfc822_specials, *c))
			return 0;
	} while (*++c); 
	return (count >= 1);
}

OL_UINT32 ConvertTimeStamp(OL_UINT32 nSecs)
{
	OL_UINT32 h = nSecs / 3600,
			  m = (nSecs % 3600) / 60,
			  s = nSecs % 60;
	return (h * 10000) + (m * 100) + s;
}

OL_UINT32 ConvertTimeStamp(OL_UINT32 nSecs, OL_UINT32 nanoSecs)
{
	nSecs += (nanoSecs / 1000000000);
	OL_UINT32 h = nSecs / 3600,
			  m = (nSecs % 3600) / 60,
			  s = nSecs % 60;
	return (h * 10000) + (m * 100) + s;
}

void ConvertTimeStamp(OL_UINT32 nSecs, OL_UINT32 &h, OL_UINT32 &m, OL_UINT32 &s)
{
	h = nSecs / 3600;
	m = (nSecs % 3600) / 60;
	s = nSecs % 60;
}

OL_UINT32 ConvertNSTimeStamp(OL_UINT64 nanoSecs)
{
	OL_UINT32 n = (nanoSecs % 1000000000) / 1000000;
	return (ConvertTimeStamp(nanoSecs / 1000000000) * 1000) + n;
}

#define CFG_SCHEDULE_SECTION "SCHEDULE_%02u"
#define CFG_SCHEDULE_BEFORE_MARKET_OPEN "before_market_open"
#define CFG_SCHEDULE_AFTER_MARKET_CLOSED "after_market_closed"
#define CFG_SCHEDULE_BEGIN_PREOPENING "begin_preopening"
#define CFG_SCHEDULE_END_PREOPENING "end_preopening"
#define CFG_SCHEDULE_BEGIN_SESSION_1 "begin_session_1"
#define CFG_SCHEDULE_BEGIN_SESSION_2 "begin_session_2"
#define CFG_SCHEDULE_BEGIN_POST_TRADING "begin_post_trading"
#define CFG_SCHEDULE_END_SESSION_1 "end_session_1"
#define CFG_SCHEDULE_END_SESSION_2 "end_session_2"
#define CFG_SCHEDULE_END_POST_TRADING "end_post_trading"
#define CFG_SCHEDULE_PREOPENING_ACTIVE "preopening_active"
#define CFG_SCHEDULE_SESSION_1_ACTIVE "session_1_active"
#define CFG_SCHEDULE_SESSION_2_ACTIVE "session_2_active"
#define CFG_SCHEDULE_POST_TRADING_ACTIVE "post_trading_active"
#define CFG_SCHEDULE_BEFORE_MARKET_OPEN_ACTIVE "before_market_open_active"
#define CFG_SCHEDULE_AFTER_MARKET_CLOSED_ACTIVE "after_market_closed_active"

#define CFG_CALL_AUCTION_SESSION_1_ACTIVE "call_auction_session_1_active"
#define CFG_CALL_AUCTION_SESSION_2_ACTIVE "call_auction_session_2_active"
#define CFG_CALL_AUCTION_SESSION_3_ACTIVE "call_auction_session_3_active"
#define CFG_CALL_AUCTION_SESSION_4_ACTIVE "call_auction_session_4_active"
#define CFG_CALL_AUCTION_SESSION_5_ACTIVE "call_auction_session_5_active"
#define CFG_CALL_AUCTION_POST_TRADING_ACTIVE "call_auction_post_trading_active"
#define CFG_SCHEDULE_BEGIN_CALL_AUCTION_SESSION_1 "begin_call_auction_session_1"
#define CFG_SCHEDULE_BEGIN_CALL_AUCTION_SESSION_2 "begin_call_auction_session_2"
#define CFG_SCHEDULE_BEGIN_CALL_AUCTION_SESSION_3 "begin_call_auction_session_3"
#define CFG_SCHEDULE_BEGIN_CALL_AUCTION_SESSION_4 "begin_call_auction_session_4"
#define CFG_SCHEDULE_BEGIN_CALL_AUCTION_SESSION_5 "begin_call_auction_session_5"
#define CFG_SCHEDULE_BEGIN_CALL_AUCTION_POST_TRADING "begin_call_auction_post_trading"
#define CFG_SCHEDULE_END_CALL_AUCTION_SESSION_1 "end_call_auction_session_1"
#define CFG_SCHEDULE_END_CALL_AUCTION_SESSION_2 "end_call_auction_session_2"
#define CFG_SCHEDULE_END_CALL_AUCTION_SESSION_3 "end_call_auction_session_3"
#define CFG_SCHEDULE_END_CALL_AUCTION_SESSION_4 "end_call_auction_session_4"
#define CFG_SCHEDULE_END_CALL_AUCTION_SESSION_5 "end_call_auction_session_5"
#define CFG_SCHEDULE_END_CALL_AUCTION_POST_TRADING "end_call_auction_post_trading"

bool LoadTradingSchedule(SCHEDULE *schedule)
{
	CConfig cfg;
	char szFileName[2048] = {}, szSection[128] = {};
	GetConfigDir(szFileName, CFG_TRADING_SCHEDULE_FILE_NAME);
	if (0 == cfg.LoadConfig(szFileName))
		return false;
	//	Load Schedule Config
	for (int i = 0; i < 7; i++)
	{
		sprintf(szSection, CFG_SCHEDULE_SECTION, i);
		schedule[i].preopening_active = cfg.GetValue(szSection, CFG_SCHEDULE_PREOPENING_ACTIVE, 0);
		schedule[i].session_1_active = cfg.GetValue(szSection, CFG_SCHEDULE_SESSION_1_ACTIVE, 0);
		schedule[i].session_2_active = cfg.GetValue(szSection, CFG_SCHEDULE_SESSION_2_ACTIVE, 0);
		schedule[i].post_trading_active = cfg.GetValue(szSection, CFG_SCHEDULE_POST_TRADING_ACTIVE, 0);
		schedule[i].before_market_open_active = cfg.GetValue(szSection, CFG_SCHEDULE_BEFORE_MARKET_OPEN_ACTIVE, 0);
		schedule[i].after_market_closed_active = cfg.GetValue(szSection, CFG_SCHEDULE_AFTER_MARKET_CLOSED_ACTIVE, 0);

		schedule[i].call_auction_session_1_active = cfg.GetValue(szSection, CFG_CALL_AUCTION_SESSION_1_ACTIVE, 0);
		schedule[i].call_auction_session_2_active = cfg.GetValue(szSection, CFG_CALL_AUCTION_SESSION_2_ACTIVE, 0);
		schedule[i].call_auction_session_3_active = cfg.GetValue(szSection, CFG_CALL_AUCTION_SESSION_3_ACTIVE, 0);
		schedule[i].call_auction_session_4_active = cfg.GetValue(szSection, CFG_CALL_AUCTION_SESSION_4_ACTIVE, 0);
		schedule[i].call_auction_session_5_active = cfg.GetValue(szSection, CFG_CALL_AUCTION_SESSION_5_ACTIVE, 0);
		schedule[i].call_auction_post_trading_active = cfg.GetValue(szSection, CFG_CALL_AUCTION_POST_TRADING_ACTIVE, 0);
		
		schedule[i].before_market_open = cfg.GetValue(szSection, CFG_SCHEDULE_BEFORE_MARKET_OPEN, 73000000);
		schedule[i].after_market_closed = cfg.GetValue(szSection, CFG_SCHEDULE_AFTER_MARKET_CLOSED, 170000000);
		
		schedule[i].begin_preopening = cfg.GetValue(szSection, CFG_SCHEDULE_BEGIN_PREOPENING, 84500000);
		schedule[i].begin_session_1 = cfg.GetValue(szSection, CFG_SCHEDULE_BEGIN_SESSION_1, 90000000);
		schedule[i].begin_session_2 = cfg.GetValue(szSection, CFG_SCHEDULE_BEGIN_SESSION_2, 133000000);
		schedule[i].begin_post_trading = cfg.GetValue(szSection, CFG_SCHEDULE_BEGIN_POST_TRADING, 160500000);
		
		schedule[i].begin_call_auction_session_1 = cfg.GetValue(szSection, CFG_SCHEDULE_BEGIN_CALL_AUCTION_SESSION_1, 90000000);
		schedule[i].begin_call_auction_session_2 = cfg.GetValue(szSection, CFG_SCHEDULE_BEGIN_CALL_AUCTION_SESSION_2, 133000000);
		schedule[i].begin_call_auction_session_3 = cfg.GetValue(szSection, CFG_SCHEDULE_BEGIN_CALL_AUCTION_SESSION_3, 133000000);
		schedule[i].begin_call_auction_session_4 = cfg.GetValue(szSection, CFG_SCHEDULE_BEGIN_CALL_AUCTION_SESSION_4, 133000000);
		schedule[i].begin_call_auction_session_5 = cfg.GetValue(szSection, CFG_SCHEDULE_BEGIN_CALL_AUCTION_SESSION_5, 133000000);
		schedule[i].begin_call_auction_post_trading = cfg.GetValue(szSection, CFG_SCHEDULE_BEGIN_CALL_AUCTION_POST_TRADING, 160500000);
		
		schedule[i].end_preopening = cfg.GetValue(szSection, CFG_SCHEDULE_END_PREOPENING, 85900000);
		schedule[i].end_session_1 = cfg.GetValue(szSection, CFG_SCHEDULE_END_SESSION_1, 113000000);
		schedule[i].end_session_2 = cfg.GetValue(szSection, CFG_SCHEDULE_END_SESSION_2, 160000000);
		schedule[i].end_post_trading = cfg.GetValue(szSection, CFG_SCHEDULE_END_POST_TRADING, 163000000);
		
		schedule[i].end_call_auction_session_1 = cfg.GetValue(szSection, CFG_SCHEDULE_END_CALL_AUCTION_SESSION_1, 113000000);
		schedule[i].end_call_auction_session_2 = cfg.GetValue(szSection, CFG_SCHEDULE_END_CALL_AUCTION_SESSION_2, 160000000);
		schedule[i].end_call_auction_session_3 = cfg.GetValue(szSection, CFG_SCHEDULE_END_CALL_AUCTION_SESSION_3, 133000000);
		schedule[i].end_call_auction_session_4 = cfg.GetValue(szSection, CFG_SCHEDULE_END_CALL_AUCTION_SESSION_4, 133000000);
		schedule[i].end_call_auction_session_5 = cfg.GetValue(szSection, CFG_SCHEDULE_END_CALL_AUCTION_SESSION_5, 133000000);
		schedule[i].end_call_auction_post_trading = cfg.GetValue(szSection, CFG_SCHEDULE_END_CALL_AUCTION_POST_TRADING, 163000000);
	}
	return true;
}
