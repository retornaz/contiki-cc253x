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

#ifndef __CC253x_FLASH_H__
#define __CC253x_FLASH_H__

#include "contiki.h"

// On cc2533, we emulate the page size of 2048
#define CC253X_FLASH_PAGE_SIZE  2048
#define CC253X_FLASH_WORD_SIZE	4

#define CC253X_FLASH_SUCCESS            0x0

// Page is locked
#define CC253X_FLASH_ABORT		0x1
// The data pointer is not in X memory (write operation)
#define CC253X_FLASH_ERROR_BUS		0x2

uint8_t
cc253x_flash_erase(uint8_t page);

uint8_t
cc253x_flash_write(uint8_t page, uint16_t offset, uint8_t * data, uint16_t len);

void 
cc253x_flash_read(uint8_t page, uint16_t offset, uint8_t *data, uint16_t len);

#endif
