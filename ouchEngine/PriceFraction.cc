#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "header/miscfunc.h"
#include "header/PriceFraction.h"

CPriceFraction::CPriceFraction(void):lsize(0)
{
	memset(frac, 0x00, sizeof(frac));
}

CPriceFraction::~CPriceFraction(void)
{
}

void CPriceFraction::ResetPriceFrac()
{
	lsize = 0;
	memset(frac, 0x00, sizeof(frac));
}

bool CPriceFraction::LoadData()
{
	char szBuf[1024];
	GetConfigDir(szBuf, FRAC_DATA);
	ResetPriceFrac();
	FILE *fp = fopen(szBuf, "r");
	if (NULL == fp)
		return false;
	uint32_t lprice, lfrac;
	while (fgets(szBuf, sizeof(szBuf), fp))
	{
		sscanf(szBuf, "%u|%u|", &lprice, &lfrac);
		frac[lsize].price = lprice;
		frac[lsize].frac = (uint16_t)lfrac;
		if (++lsize >= 20)
			break;
	}
	fclose(fp);
	return true;
}

uint16_t CPriceFraction::GetFraction(const uint32_t price)
{
	if (0 == price || 0 == lsize)
		return 0;
	uint16_t s = lsize;
	while (s--)
		if (price >= frac[s].price)
			return frac[s].frac;
	return 0;
}

CPriceFraction::FRAC *CPriceFraction::GetFractionStruc(const uint32_t price)
{
	if (0 == price || 0 == lsize)
		return NULL;
	uint16_t s = lsize;
	while (s--)
		if (price >= frac[s].price)
			return &frac[s];
	return NULL;
}

uint32_t CPriceFraction::GetAtomicFracPrice(uint32_t price, bool bdirec)
{
	FRAC *pf = GetFractionStruc(price);
	if (NULL == pf)
		return price;
	uint32_t r = price % pf->frac;
	if (0 == r)
		return price;
	return bdirec ? price + (pf->frac - r) : price - r;
}

uint32_t CPriceFraction::GetFracPrice(uint32_t price)
{
	FRAC *pf = GetFractionStruc(price);
	if (NULL == pf)
		return price;
	return price - (price % pf->frac);
}

uint32_t CPriceFraction::GetFracPrice(uint32_t price, bool bdirec)
{
	FRAC *pf = GetFractionStruc(price);
	if (NULL == pf)
		return price;
	if (bdirec)	//	Up
	{
		price += pf->frac;
		price -= (price % pf->frac);
	}
	else	//	Down
	{
		if (price == pf->price)
		{
			pf = GetFractionStruc(price - 1);
			if (NULL == pf)
				return price;
		}
		price -= pf->frac;
		price -= (price % pf->frac);
	}
	return price;
}

uint32_t CPriceFraction::GetFracPrice(uint32_t price, bool bdirec, uint16_t step)
{
	if (0 == price)
		return price;
	if (0 == step)
		return GetFracPrice(price, bdirec);
	FRAC *pf = NULL;
	if (bdirec)	//	Up
	{
		while (step--)
		{
			pf = GetFractionStruc(price);
			if (NULL == pf)
				return price;
			price += pf->frac;
			price -= (price % pf->frac);
		}
	}
	else	//	Down
	{
		while (step--)
		{
			pf = GetFractionStruc(price - 1);
			if (NULL == pf)
				return price;
			price -= pf->frac;
			price -= (price % pf->frac);
		}
	}
	return price;
}
