#include "header/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CConfig::CConfig()
{
}

CConfig::~CConfig()
{
}

int CConfig::LoadConfig(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (NULL == fp)
	{
		printf("Unable to open %s\n", filename);
		return 0;
	}
	char lines[5120], *tkn[3], szSection[64] = {};
	int open_brace = 0, curline = 0;
	while (fgets(lines, sizeof(lines), fp))
	{
		++curline;
		if (*lines == '#' || *lines == '\r' || *lines == '\n' || *lines == '\0')
			continue;
		int i, len = strlen(lines);
		switch (open_brace)
		{
		case 0:
			for (i = 0; i < len; i++)
				if (lines[i] == '{')
				{
					open_brace = 1;
					lines[i] = '\0';
					memset(szSection, 0x00, sizeof(szSection));
					strncpy(szSection, Trim(lines), 50);
					break;
				}
			if (!open_brace)
			{
				printf("[Error] Parse error in configuration file (out of brace) at line %i\n", curline);
				return 0;
			}
			break;
		case 1:
			for (i = 0; i < len; i++)
				if (lines[i] == '}')
				{
					open_brace = 0;
					break;
				}
			if (open_brace)
			{
				int nTok = explode('=', lines, tkn, 3);
				if (nTok > 1)
				{
					printf("[Error] Parse error in configuration file (illegal equality \"=\") at line %i\n", curline);
					return 0;
				}
				if (nTok == 1)
				{
					cfg[szSection][Trim(tkn[0])] = Trim(tkn[1]);
				}
				else
				{
					// def->key[0] = '\0';
					// def->val = xstrdup(trim(lines));
				}
			}
			break;
		}
	}
	fclose(fp);
	return 1;
};

void CConfig::GetValue(const char *section, const char *key, const char *def, char *buffer, unsigned int bufsize)
{
	memset(buffer, 0x00, bufsize);
	if (cfg[section][key].length() > 0)
		strncpy(buffer, cfg[section][key].c_str(), bufsize - 1);
	else
		strncpy(buffer, def, bufsize - 1);
}

long CConfig::GetValue(const char *section, const char *key, long ldef)
{
	if (cfg[section][key].length() > 0)
		ldef = atol(cfg[section][key].c_str());
	return ldef;
}
