/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Amazon Web Services
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
void Axp192_I2CInit();

void Axp192_WriteBytes(uint8_t reg_addr, uint8_t *data, uint16_t length);

void Axp192_ReadBytes(uint8_t reg_addr, uint8_t *data, uint16_t length);

void Axp192_Write8Bit(uint8_t reg_addr, uint8_t value);

void Axp192_WriteBits(uint8_t reg_addr, uint8_t data, uint8_t bit_pos, uint8_t bit_length);

uint8_t Axp192_Read8Bit(uint8_t reg_addr);

uint16_t Axp192_Read12Bit(uint8_t reg_addr);

uint16_t Axp192_Read13Bit(uint8_t reg_addr);

uint16_t Axp192_Read16Bit(uint8_t reg_addr);

uint32_t Axp192_Read24Bit(uint8_t reg_addr);

uint32_t Axp192_Read32Bit(uint8_t reg_addr);


#ifdef __cplusplus
}
#endif
