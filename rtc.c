#include "rtc.h"

#include <avr/pgmspace.h>
#include "i2csw.h"

RTC_type rtc;

const static RTC_type rtcMin PROGMEM = {0, 0, 0, 1, 1, 1, 1, RTC_NOEDIT};
const static RTC_type rtcMax PROGMEM = {59, 59, 23, 7, 31, 12, 99, RTC_NOEDIT};

static void rtcWeekDay(void)
{
    uint8_t a, y, m;

    a = (rtc.month > 2 ? 0 : 1);
    y = 12 + rtc.year - a;
    m = rtc.month + 12 * a - 2;

    rtc.wday = (rtc.date + y + (y / 4) + ((31 * m) / 12)) % 7;
    if (rtc.wday == 0)
        rtc.wday = 7;
}

static uint8_t rtcDaysInMonth(void)
{
    uint8_t ret = rtc.month;

    if (ret == 2) {
        ret = rtc.year & 0x03;
        ret = (ret ? 28 : 29);
    } else {
        if (ret > 7)
            ret++;
        ret |= 30;
    }

    return ret;
}

void rtcReadTime(void)
{
    uint8_t temp;
    uint8_t i;

    I2CswStart(RTC_I2C_ADDR);
    I2CswWriteByte(RTC_SEC);
    I2CswStart(RTC_I2C_ADDR | I2C_READ);
    for (i = RTC_SEC; i <= RTC_YEAR; i++) {
        temp = I2CswReadByte((i == RTC_YEAR) ? I2C_NOACK : I2C_ACK);
        *((int8_t *)&rtc + i) = rtcBinDecToDec(temp);
    }
    I2CswStop();
}

void rtcCorrSec(void)
{
    I2CswStart(RTC_I2C_ADDR);
    I2CswWriteByte(RTC_SEC);
    I2CswWriteByte(rtcDecToBinDec(rtc.sec));
    I2CswStop();
}

static void rtcSaveTime(void)
{
    uint8_t i;
    uint8_t etm = rtc.etm;

    if (etm > RTC_WDAY) {
        rtcWeekDay();
        etm = RTC_WDAY;
    }

    I2CswStart(RTC_I2C_ADDR);
    I2CswWriteByte(etm);
    if (etm == RTC_SEC) {
        I2CswWriteByte(0);
    } else {
        for (i = etm; i <= RTC_YEAR; i++)
            I2CswWriteByte(rtcDecToBinDec(*((int8_t *)&rtc + i)));
    }

    I2CswStop();
}

void rtcNextEditParam(void)
{
    switch (rtc.etm) {
    case RTC_HOUR:
    case RTC_MIN:
        rtc.etm--;
        break;
    case RTC_SEC:
        rtc.etm = RTC_DATE;
        break;
    case RTC_DATE:
    case RTC_MONTH:
        rtc.etm++;
        break;
    default:
        rtc.etm = RTC_HOUR;
        break;
    }
}

void rtcChangeTime(int8_t diff)
{
    int8_t *time = (int8_t *)&rtc + rtc.etm;
    int8_t timeMax = pgm_read_byte((int8_t *)&rtcMax + rtc.etm);
    int8_t timeMin = pgm_read_byte((int8_t *)&rtcMin + rtc.etm);

    if (rtc.etm == RTC_DATE)
        timeMax = rtcDaysInMonth();

    *time += diff;

    if (*time > timeMax)
        *time = timeMin;
    if (*time < timeMin)
        *time = timeMax;

    rtcSaveTime();
}


uint8_t rtcBinDecToDec(uint8_t num)
{
    return (num >> 4) * 10 + (num & 0x0F);
}


uint8_t rtcDecToBinDec(uint8_t num)
{
    return ((num / 10) << 4) + (num % 10);
}
