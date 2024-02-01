#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <map>
#include "miscfunc.h"

using namespace std;

class CConfig
{
public:
	CConfig();
	~CConfig();

	int LoadConfig(const char *filename);
	void GetValue(const char *section, const char *key, const char *def, char *buffer, unsigned int bufsize);
	long GetValue(const char *section, const char *key, long ldef);
private:
	typedef map<string, string> CFG_DEF;
	typedef map<string, CFG_DEF> CONFIG;
	CONFIG cfg;
};

#endif
