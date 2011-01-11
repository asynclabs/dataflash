//*****************************************************************************
//
//      COPYRIGHT (c) ATMEL Norway, 1996-2001
//
//      The copyright to the document(s) herein is the property of
//      ATMEL Norway, Norway.
//
//      The document(s) may be used  and/or copied only with the written
//      permission from ATMEL Norway or in accordance with the terms and
//      conditions stipulated in the agreement/contract under which the
//      document(s) have been supplied.
//
//*****************************************************************************
//
//  File........: DATAFLASH.H
//
//  Author(s)...: ATMEL Norway
//
//  Target(s)...: Independent
//
//  Description.: Defines and prototypes for AT45Dxxx
//
//  Revisions...:
//
//  YYYYMMDD - VER. - COMMENT                                       - SIGN.
//
//  20010117 - 0.10 - Generated file                                -  RM
//  20031009          port to avr-gcc/avr-libc                      - M.Thomas
//
//*****************************************************************************


/*****************************************************************************

Filename:    DataFlash.h
Description: DataFlash library file for the FlashShield 1.0

******************************************************************************

DataFlash library for the FlashShield 1.0

Copyright(c) 2011 Async Labs Inc. All rights reserved.

This program is free software; you can redistribute it and/or modify it
under the terms of version 2 of the GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.

Contact Information:
<asynclabs@asynclabs.com>

-----------------------------------------------------------------------------
Author    Date       Comment
-----------------------------------------------------------------------------
AsyncLabs 03/17/2010 Initial version
AsyncLabs 01/10/2011 Clean up

*****************************************************************************/

#include "WProgram.h"
#include <avr/pgmspace.h>

//Dataflash commands
#define FlashPageRead           0xD2    // Main memory page read
#define FlashToBuf1Transfer     0x53    // Main memory page to buffer 1 transfer
#define Buf1Read                0xD4    // Buffer 1 read
#define FlashToBuf2Transfer     0x55    // Main memory page to buffer 2 transfer
#define Buf2Read                0xD6    // Buffer 2 read
#define StatusReg               0xD7    // Status register
#define Buf1ToFlashWE           0x83    // Buffer 1 to main memory page program with built-in erase
#define Buf1Write               0x84    // Buffer 1 write
#define Buf2ToFlashWE           0x86    // Buffer 2 to main memory page program with built-in erase
#define Buf2Write               0x87    // Buffer 2 write
#define ReadMfgID               0x9F    // Read Manufacturer and Device ID

class Dataflash {
public:
    void init(uint8_t ssPin);
    void Read_DF_ID(unsigned char* data);
    void Page_To_Buffer (unsigned int PageAdr, unsigned char BufferNo);
    unsigned char Buffer_Read_Byte (unsigned char BufferNo, unsigned int IntPageAdr);
    void Buffer_Write_Byte (unsigned char BufferNo, unsigned int IntPageAdr, unsigned char Data);
    void Buffer_To_Page (unsigned char BufferNo, unsigned int PageAdr);

private:
    unsigned char slave_select;
    unsigned char DF_SPI_RW (unsigned char output);
    unsigned char Read_DF_status (void);
    void DF_CS_inactive(void);
    void DF_CS_active(void);
};

extern Dataflash dflash;
