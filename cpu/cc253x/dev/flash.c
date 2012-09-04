/*
Copyright (c) 2012, Philippe Retornaz
Copyright (c) 2012, EPFL STI IMT LSRO1 -- Mobots group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote
   products derived from this software without specific prior
   written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "flash.h"
#include "../cc253x.h"
#include "dma.h"
#include "watchdog.h"

#if DMA_ON
#ifdef DMA_FLASH_CHANNEL

void
cc253x_flash_read(uint8_t page, uint16_t offset, uint8_t *data, uint16_t len)
{
  uint8_t bank;
  uint8_t flag = EA;
  __xdata uint8_t * f;
  
  EA = 0;
  bank = MEMCTR;
  
  MEMCTR = 0x8 | (page >> 4);
  
  f = (__xdata uint8_t *) (0x8000 | (page & ((1 << 4) - 1)) << 11 | offset);
  while(len--)
    *data++ = *f++;
  
  MEMCTR = bank;
  EA = flag;
}

uint8_t
cc253x_flash_erase(uint8_t page)
{
  uint8_t ret = 0;
  uint8_t flag = EA;
  EA = 0;
  // Erasing a page take 20ms
  watchdog_periodic();
  
  while(FCTL & 0x80);
  FADDRH = page << 1;
  FCTL |= 0x1;
  while(FCTL & 0x80);

  if(FCTL & (1 << 5)) {
    ret = CC253X_FLASH_ABORT;
  }
  
  if(CHIPID = 0x95) {
    watchdog_periodic();
    FADDRH = (page << 1) | 0x1;
    FCTL |= 0x1;
    while(FCTL & 0x80);

    if(FCTL & (1 << 5)) {
      ret = CC253X_FLASH_ABORT;
    }
  }
  
  watchdog_periodic();
  
  EA = flag;

  return ret;
}

// offset is the word address.
// len must be a multiple of 4
static uint8_t
_do_write(uint16_t offset, __xdata uint8_t *data, uint16_t len)
{
  watchdog_periodic();
  dma_reset(DMA_FLASH_CHANNEL); // Not sure it's usefull.

  dma_conf[DMA_FLASH_CHANNEL].src_h = (uint16_t) data >> 8;
  dma_conf[DMA_FLASH_CHANNEL].src_l = (uint16_t) data;
  dma_conf[DMA_FLASH_CHANNEL].dst_h = (uint16_t) &FWDATA >> 8;
  dma_conf[DMA_FLASH_CHANNEL].dst_l = (uint16_t) &FWDATA;
  dma_conf[DMA_FLASH_CHANNEL].len_h = len >> 8;
  dma_conf[DMA_FLASH_CHANNEL].len_l = len;
  dma_conf[DMA_FLASH_CHANNEL].wtt = DMA_T_FLASH | DMA_SINGLE;
  dma_conf[DMA_FLASH_CHANNEL].inc_prio = DMA_SRC_INC_1 | DMA_DST_INC_NO |
                                         DMA_PRIO_HIGH;

  FADDRH = offset >> 8;
  FADDRL = offset;
  
  DMA_ARM(DMA_FLASH_CHANNEL);
  FCTL |= 0x2;
  
  while(FCTL & 0x80);
  
  DMA_ABORT(DMA_FLASH_CHANNEL);
  
  watchdog_periodic();
  
  if(FCTL & (1 << 5))
    return CC253X_FLASH_ABORT;
  return 0;
}

uint8_t
cc253x_flash_write(uint8_t page, uint16_t offset, uint8_t *data, uint16_t len)
{
  static __xdata uint8_t word[4]; // So we can write byte and not only words
  uint8_t i;
  uint8_t ret = 0;
  uint8_t m = offset & 0x3;

  if(m) {
    // Non aligned start of write.
    for(i = 0; i < m; i++) {
      word[i] = 0xFF;
    }
    
    for(; i < 4 && len; i++, offset++, len--) {
      word[i] = *data++;
    } 
   
    for(; i < 4; i++, offset++) {
      word[i] = 0xFF;
    }
    
    ret = _do_write((page << 9) + (offset >> 2) - 1, word, 4);
    if(ret) {
      return ret;
    }
  }
  
  if(!len) {
    return ret;
  }
  
  // Truncate len to the last full word.
  m = len & 0x3;
  len &= ~0x3;
  
  if(len) {
    // Check that we are in X memory
    if(((uint8_t *) &data)[2] != 0x0) {
      return CC253X_FLASH_ERROR_BUS;
    }
    ret = _do_write((page << 9) + (offset >> 2), (__xdata uint8_t *) data, len);
    offset += len;
    data += len;
    if(ret) {
      return ret;
    }
  }
  
  if(m) {
    for(i = 0; i < m; i++) {
      word[i] = *data++;
    }
    for(i; i < 4; i++) {
      word[i] = 0xFF;
    }
    ret = _do_write((page << 9) + (offset >> 2), word, 4);
  }
  return ret;
}

#endif
#endif

