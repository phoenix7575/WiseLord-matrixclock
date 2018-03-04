#include "dht22.h"

#include <util/delay.h>
#include "pins.h"

static dht22Dev dev;

static uint8_t dht22Sensor = 0;

static int16_t humidity, temp;

void dht22Init(void)
{
    DDR(DHT_DATA) &= ~DHT_DATA_LINE;
    PORT(DHT_DATA) &= ~DHT_DATA_LINE;
}

static uint8_t dht22GetBit(void)
{
    uint8_t i;

    for (i = 255; i && !(PIN(DHT_DATA) & DHT_DATA_LINE); i--)
        _delay_us(1);                           // Wait zero

    for (i = 255; i && (PIN(DHT_DATA) & DHT_DATA_LINE); i--)
        _delay_us(1);                           // Measure bit length

    return (i < 230);
}

static uint8_t dht22GetByte(void)
{
    int8_t i;

    uint8_t ret = 0;

    for (i = 0; i < 8; i++) {
        ret <<= 1;
        ret |= dht22GetBit();
    }

    return ret;
}

void dht22Read(void)
{
    int8_t i;
    uint8_t cksum;
    uint8_t data;

    if (!(PIN(DHT_DATA) & DHT_DATA_LINE))       // Line is busy
        return;

    DDR(DHT_DATA) |= DHT_DATA_LINE;
    _delay_us(800);
    DDR(DHT_DATA) &= ~DHT_DATA_LINE;
    _delay_us(70);

    if (PIN(DHT_DATA) & DHT_DATA_LINE)          // No DHT22 response
        return;

    dht22Sensor = 1;

    dht22GetBit();                              // Wait for response finish

    cksum = 0;
    data = 0xFF;
    for (i = 4; i >= 0; i--) {
        data = dht22GetByte ();
        dev.data[i] = data;
        if (i > 0)
            cksum += data;                      // Checksum of first 4 bytes
    }
    if (cksum == data) {
        humidity = dev.humidity;
        temp = dev.temp;
        if (temp & 0x8000)                      // Fix negative temp value
            temp = -(temp & 0x7FFF);
    }
}

int16_t dht22GetHumidity()
{
    return humidity;
}

int16_t dht22GetTemp()
{
    return temp;
}

uint8_t dht22HaveSensor(void)
{
    return dht22Sensor;
}
