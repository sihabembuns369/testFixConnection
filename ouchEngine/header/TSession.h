#pragma once
#include <cstdint>
#include <unistd.h>

const uint16_t TSES_BREAK			= 0x0000;
const uint16_t TSES_PREOP			= 0x0001;
const uint16_t TSES_SESSION1		= 0x0002;
const uint16_t TSES_SESSION2		= 0x0004;
const uint16_t TSES_POST_TRADING	= 0x0008;
const uint16_t TSES_BREAK_SESSION2	= 0x0010;	//	For NG Orders
const uint16_t TSES_BEFORE_MARKET_OPEN  = 0x0040;
const uint16_t TSES_AFTER_MARKET_CLOSED = 0x0080;
const uint16_t TSES_SESSIONS		= TSES_SESSION1 | TSES_SESSION2 | TSES_POST_TRADING;
const uint16_t TSES_PREOPENING		= TSES_PREOP | TSES_SESSION1 | TSES_SESSION2 | TSES_POST_TRADING;
const uint16_t TSES_NG_SESSIONS		= TSES_SESSION1 | TSES_SESSION2 | TSES_POST_TRADING | TSES_BREAK_SESSION2;

const uint16_t TSES_CALL_SESSION1		= 0x0100;
const uint16_t TSES_CALL_SESSION2		= 0x0200;
const uint16_t TSES_CALL_SESSION3		= 0x0400;
const uint16_t TSES_CALL_SESSION4		= 0x0800;
const uint16_t TSES_CALL_SESSION5		= 0x1000;
const uint16_t TSES_CALL_POST_TRADING	= 0x2000;
const uint16_t TSES_CALL_AUCTION_SESSIONS = TSES_CALL_SESSION1 | TSES_CALL_SESSION2 | 
	TSES_CALL_SESSION3 | TSES_CALL_SESSION4 | TSES_CALL_SESSION5 | TSES_CALL_POST_TRADING;

class CTSession
{
public:
	//CTSession() : bSuspend(false), csession(0), current(0) {};
	CTSession(uint16_t s) : bSuspend(false), csession(s), current(0) {};
	~CTSession(){};

	inline void Resume() { bSuspend = false; };
	inline void Suspend(uint32_t dwMilliseconds = 0xffffffff)
	{
		while (bSuspend && dwMilliseconds > 0)
		{
			dwMilliseconds--;
			usleep(1000);
		};
	};
	inline uint16_t SynchronizeState()
	{
		uint16_t result = csession & current;
		bSuspend = (0 == result);
		return result;
	};
	inline uint16_t SetCurrentSession(uint16_t c)
	{
		current = c;
		return SynchronizeState();
	};
	inline uint16_t CheckSession() { return csession & current; };

	const uint16_t GetCSession() const { return csession; };

private:
	bool bSuspend;
	uint16_t csession, current;	//Bit 0 -> Preopening; 1 -> Session 1; 2 -> Session 2; 3 -> Post Trading; 4 -> Break After Session 2
};



