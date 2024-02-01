#ifndef __PRICEFRACTION_H__
#define __PRICEFRACTION_H__

#define FRAC_DATA "PriceFrac.dat"

class CPriceFraction
{
public:
	typedef struct _FRAC_
	{
		uint32_t price;
		uint16_t frac;
	} FRAC;
	CPriceFraction(void);
	~CPriceFraction(void);

	bool LoadData();
	void ResetPriceFrac();
	//void RequestServerPriceFrac();
	//void InitPriceFrac(CGPacket *pkg);
	FRAC *GetFractionStruc() {return &frac[0];};
	FRAC *GetFractionStruc(const uint32_t price);
	uint16_t GetFractionSize() const {return lsize;};
	uint16_t GetFraction(const uint32_t price);
	uint32_t GetFracPrice(uint32_t price);	//	Normalize to the nearest price fraction
	uint32_t GetFracPrice(uint32_t price, bool bdirec);	// TRUE -> Up; FALSE -> Down
	uint32_t GetFracPrice(uint32_t price, bool bdirec, uint16_t step); // TRUE -> Up; FALSE -> Down
	uint32_t GetAtomicFracPrice(uint32_t price, bool bdirec);	//	TRUE -> Round Up; FALSE -> Round Down

private:
	FRAC frac[20];
	uint16_t lsize;
};

#endif