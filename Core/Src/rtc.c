#include "rtc.h"

#define LSE		0b01
#define LSI		0b10
#define HSI		0b11

/*
 * Converts 2 digit Decimal to BCD format
 * param: 	Byte to be converted.
 * retval: BCD Converted byte
 */
static uint8_t byte2bcd(uint8_t byte);

/*
 * Convert from 2 digit BCD to Binary
 * param  BCD value to be converted.
 * retval Converted word
 */
static uint8_t bcd2byte(uint8_t bcd);

void rtc_init()
{
	/* Enable access to RTC registers */
	SET_BIT(PWR->CR, PWR_CR_DBP);

	SET_BIT(RCC->BDCR, RCC_BDCR_BDRST);
	__NOP();
	CLEAR_BIT(RCC->BDCR, RCC_BDCR_BDRST);
	/* Turn off RTC clock */
	CLEAR_BIT(RCC->BDCR, RCC_BDCR_RTCEN);

	SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);
	while (!READ_BIT(RCC->BDCR, RCC_BDCR_LSERDY));
	/* RCC clock source */
	MODIFY_REG(RCC->BDCR, RCC_BDCR_RTCSEL, LSE<<RCC_BDCR_RTCSEL_Pos);

	/* Turn on RTC clock */
	SET_BIT(RCC->BDCR, RCC_BDCR_RTCEN);
}

void rtc_set_datetime(DateTime_st *pdatetime)
{
	uint32_t bcdtime, bcddate;

	bcdtime = 	((byte2bcd(pdatetime->hours))   <<RTC_TR_HU_Pos)  |
				((byte2bcd(pdatetime->minutes)) <<RTC_TR_MNU_Pos) |
				((byte2bcd(pdatetime->seconds)) <<RTC_TR_SU_Pos);

	bcddate = 	((byte2bcd(pdatetime->year))  <<RTC_DR_YU_Pos)  |
				((byte2bcd(pdatetime->month)) <<RTC_DR_MU_Pos)  |
				((byte2bcd(pdatetime->date))  <<RTC_DR_DU_Pos);

	/* Enable access to RTC registers */
	SET_BIT(PWR->CR, PWR_CR_DBP);

	WRITE_REG(RTC->WPR, 0xCA);
	WRITE_REG(RTC->WPR, 0x53);
	/* Enter in init mode in order to change time and date */
	SET_BIT(RTC->ISR, RTC_ISR_INIT);
	while (!READ_BIT(RTC->ISR, RTC_ISR_INITF));
	/* Set up prescaler */
//	RTC->PRER = 0x007F00FF;

	WRITE_REG(RTC->DR, bcddate);
	WRITE_REG(RTC->TR, bcdtime);

	CLEAR_BIT(RTC->ISR, RTC_ISR_INIT);
	/* Disable access to RTC registers */
	WRITE_REG(RTC->WPR, 0xFF);
	PWR->CR &= ~PWR_CR_DBP;
}

bool rtc_get_datetime(DateTime_st *pdatetime)
{
	bool ret = false;

	if (READ_BIT(RTC->ISR, RTC_ISR_RSF))
	{
		pdatetime->hours = bcd2byte((RTC->TR & (RTC_TR_HT_Msk | RTC_TR_HU_Msk ))>>RTC_TR_HU_Pos);
		pdatetime->minutes = bcd2byte((RTC->TR & (RTC_TR_MNT_Msk | RTC_TR_MNU_Msk))>>RTC_TR_MNU_Pos);
		pdatetime->seconds = bcd2byte((RTC->TR & (RTC_TR_ST_Msk | RTC_TR_SU_Msk ))>>RTC_TR_SU_Pos);

		pdatetime->year = ((RTC->DR >> RTC_DR_YT_Pos) * 10) + ((RTC->DR >> RTC_DR_YU_Pos) & 0x0f);
		pdatetime->month = ((RTC->DR >> RTC_DR_MT_Pos) & 1) * 10 + ((RTC->DR >> RTC_DR_MU_Pos) & 0x0f);
		pdatetime->date = ((RTC->DR >> RTC_DR_DT_Pos) & 3) * 10 + (RTC->DR & 0x0f);

		pdatetime->timestamp = rtc_to_timestamp(pdatetime);

		ret = true;
	}

	return ret;
}



// Convert Date/Time structures to epoch time
uint32_t rtc_to_timestamp(DateTime_st *pdatetime)
{
	uint8_t  a;
	uint16_t y;
	uint8_t  m;
	uint32_t JDN;

	// These hardcore math's are taken from http://en.wikipedia.org/wiki/Julian_day

	// Calculate some coefficients
	a = (14 - pdatetime->month) / 12;
	y = (pdatetime->year + 2000) + 4800 - a; // years since 1 March, 4801 BC
	m = pdatetime->month + (12 * a) - 3; // since 1 March, 4801 BC

	// Gregorian calendar date compute
    JDN  = pdatetime->date;
    JDN += (153 * m + 2) / 5;
    JDN += 365 * y;
    JDN += y / 4;
    JDN += -y / 100;
    JDN += y / 400;
    JDN  = JDN - 32045;
    JDN  = JDN - JULIAN_DATE_BASE;    // Calculate from base date
    JDN *= 86400;                     // Days to seconds
    JDN += pdatetime->hours * 3600;    // ... and today seconds
    JDN += pdatetime->minutes * 60;
    JDN += pdatetime->seconds;

	return JDN;
}

// Convert epoch time to Date/Time structures
void rtc_from_timestamp(uint32_t timestamp, DateTime_st *pdatetime, int8_t offset)
{
	timestamp += offset * 3600;

	uint32_t tm;
	uint32_t t1;
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	uint32_t e;
	uint32_t m;
	int16_t  year  = 0;
	int16_t  month = 0;
	int16_t  dow   = 0;
	int16_t  mday  = 0;
	int16_t  hour  = 0;
	int16_t  min   = 0;
	int16_t  sec   = 0;
	uint64_t JD    = 0;
	uint64_t JDN   = 0;

	// These hardcore math's are taken from http://en.wikipedia.org/wiki/Julian_day

	JD  = ((timestamp + 43200) / (86400 >>1 )) + (2440587 << 1) + 1;
	JDN = JD >> 1;

    tm = timestamp; t1 = tm / 60; sec  = tm - (t1 * 60);
    tm = t1;    	t1 = tm / 60; min  = tm - (t1 * 60);
    tm = t1;    	t1 = tm / 24; hour = tm - (t1 * 24);

    dow   = JDN % 7;
    a     = JDN + 32044;
    b     = ((4 * a) + 3) / 146097;
    c     = a - ((146097 * b) / 4);
    d     = ((4 * c) + 3) / 1461;
    e     = c - ((1461 * d) / 4);
    m     = ((5 * e) + 2) / 153;
    mday  = e - (((153 * m) + 2) / 5) + 1;
    month = m + 3 - (12 * (m / 10));
    year  = (100 * b) + d - 4800 + (m / 10);

    pdatetime->year    = year - 2000;
    pdatetime->month   = month;
    pdatetime->date    = mday;
    pdatetime->hours   = hour;
    pdatetime->minutes = min;
    pdatetime->seconds = sec;

	rtc_set_datetime(pdatetime);
}


static uint8_t byte2bcd(uint8_t byte)
{
	uint8_t bcdhigh = 0;

	while (byte >= 10)
	{
		bcdhigh++;
		byte -= 10;
	}

	return  ((uint8_t)(bcdhigh << 4) | byte);
}

static uint8_t bcd2byte(uint8_t bcd)
{
	uint8_t tmp = 0;

	tmp = ((uint8_t)(bcd & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;

	return (tmp + (bcd & (uint8_t)0x0F));
}

