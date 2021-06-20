//
// Created by nathan on 05-05-21.
//

#include "crc8.h"

//authored by Nathan
uint8_t crc8_fast(uint8_t *data, uint16_t n) {
    uint8_t crc = 0xFF;
    if (data)
    {
        while (n--)
        {
            crc = crc8_lut[crc ^ *(data++)];
        }
    }
    return crc;
}

//authored by Nathan
bool crc8_fast_compare(uint8_t crc, uint8_t *data, uint16_t n)
{
    return crc == crc8_fast(data, n);
}