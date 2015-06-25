#ifndef MAX7219_H
#define MAX7219_H

#include <inttypes.h>
#include "pins.h"
#include "matrix.h"

#if defined(MAX7219_X3)
#define MAX7219
#define MAX7219_NUM_USED 3
#elif defined(MAX7219MOD_X3)
#define MAX7219MOD
#define MAX7219_NUM_USED 3
#endif

#ifndef MAX7219_NUM_USED
#define MAX7219_NUM_USED 4
#endif

// Registers
#define MAX7219_NO_OP			0x00
#define MAX7219_DIGIT_0			0x01
#define MAX7219_DIGIT_1			0x02
#define MAX7219_DIGIT_2			0x03
#define MAX7219_DIGIT_3			0x04
#define MAX7219_DIGIT_4			0x05
#define MAX7219_DIGIT_5			0x06
#define MAX7219_DIGIT_6			0x07
#define MAX7219_DIGIT_7			0x08
#define MAX7219_DEC_MODE		0x09
#define MAX7219_INTENSITY		0x0A
#define MAX7219_SCAN_LIMIT		0x0B
#define MAX7219_SHUTDOWN		0x0C
#define MAX7219_DISP_TEST		0x0F

void max7219SendCmd(uint8_t reg, uint8_t data);
void max7219SendDataBuf(uint8_t *buf, uint8_t rotate);
void max7219Init(void);

#endif /* MAX7219_H */
