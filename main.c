#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "max7219.h"
#include "fonts.h"
#include "mtimer.h"
#include "i2c.h"
#include "ds1307.h"

#define BIPER_DDR	DDRD
#define BIPER_PORT	PORTD
#define BIPER_PIN	(1<<PD5)

int8_t *dateTime;

uint8_t strbuf[20];

uint8_t *mkNumString(int16_t number, uint8_t width, uint8_t lead)
{
	uint8_t numdiv;
	uint8_t sign = lead;
	int8_t i;

	if (number < 0) {
		sign = '-';
		number = -number;
	}

	for (i = 0; i < width; i++)
		strbuf[i] = lead;
	strbuf[width] = '\0';
	i = width - 1;

	while (number > 0 || i == width - 1) {
		numdiv = number % 10;
		strbuf[i] = numdiv + 0x30;
		if (numdiv >= 10)
			strbuf[i] += 7;
		i--;
		number /= 10;
	}

	if (i >= 0)
		strbuf[i] = sign;

	return strbuf;
}

void showTime(uint32_t mask)
{
	static int8_t oldDateTime[7];

	max7219SetX(0);
	max7219LoadString(mkNumString(dateTime[HOUR], 2, '0'));
	max7219SetX(12);
	max7219LoadString(mkNumString(dateTime[MIN], 2, '0'));

	if (oldDateTime[HOUR] / 10 != dateTime[HOUR] / 10)
		mask  |= 0xF00000;
	if (oldDateTime[HOUR] % 10 != dateTime[HOUR] % 10)
		mask  |= 0x078000;
	if (oldDateTime[MIN] / 10 != dateTime[MIN] / 10)
		mask  |= 0x000F00;
	if (oldDateTime[MIN] % 10 != dateTime[MIN] % 10)
		mask  |= 0x000078;

	max7219PosData(10, dateTime[SEC] & 0x01 ? 0x00 : 0x24);
	max7219PosData(23, dateTime[SEC]);

	max7219SwitchBuf(mask, MAX7219_EFFECT_SCROLL_DOWN);

	oldDateTime[HOUR] = dateTime[HOUR];
	oldDateTime[MIN] = dateTime[MIN];

	return;
}

int main(void)
{
	max7219Init();
	max7219Fill(0x00);
	max7219LoadFont(font_ks0066_ru_08);

	I2CInit();

	mTimerInit();
	sei();

	uint8_t i;
	BIPER_DDR |= BIPER_PIN;
	for (i = 0; i < 10; i++) {
		BIPER_PORT &= ~BIPER_PIN;
		_delay_ms(20);
		BIPER_PORT |= BIPER_PIN;
		_delay_ms(20);
	}

	showTime(0xFFFFFF);

	while(1) {

		dateTime = readTime();

		showTime(0x000000);

		if (dateTime[SEC] == 10) {
			max7219SetX(0);
			max7219LoadString((uint8_t*)" ПОНЕДЕЛЬНИК, 4 АВГУСТА 2014г. ");
			max7219Scroll();
			dateTime = readTime();
			showTime(0xFFFFFF);
		}

		if (dateTime[SEC] == 40) {
			max7219SetX(0);
			max7219LoadString((uint8_t*)" Температура 24.1·C ");
			max7219Scroll();
			dateTime = readTime();
			showTime(0xFFFFFF);
		}
	}

	return 0;
}
