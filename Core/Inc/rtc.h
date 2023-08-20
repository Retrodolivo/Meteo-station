#ifndef INC_RTC_H_
#define INC_RTC_H_

#include "main.h"


typedef struct
{
	uint8_t date;
	uint8_t month;
	uint8_t year;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	uint32_t timestamp;
} DateTime_st;

#define JULIAN_DATE_BASE     2440588   // Unix epoch time in Julian calendar (UnixTime = 00:00:00 01.01.1970 => JDN = 2440588)

void rtc_init(void);
void rtc_set_datetime(DateTime_st *pdatetime);
bool rtc_get_datetime(DateTime_st *pdatetime);
uint32_t rtc_to_timestamp(DateTime_st *pdatetime);
void rtc_from_timestamp(uint32_t timestamp, DateTime_st *pdatetime, int8_t offset);

#endif /* INC_RTC_H_ */
