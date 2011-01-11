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
//  File........: DATAFLASH.C
//
//  Author(s)...: ATMEL Norway
//
//  Target(s)...: All AVRs with built-in HW SPI
//
//  Description.: Functions to access the Atmel AT45Dxxx Dataflash series
//                Supports 512Kbit - 64Mbit
//
//  Revisions...:
//
//  YYYYMMDD - VER. - COMMENT                                       - SIGN.
//
//  20011017 - 1.00 - Beta release                                  -  RM
//  20011017 - 0.10 - Generated file                                -  RM
//  20031009          port to avr-gcc/avr-libc                      - M.Thomas
//  20040121          added compare and erase function              - M.Thomas
//
//*****************************************************************************
/* 
   remark mthomas: If you plan to use the Dataflash functions in own code
   for (battery powered) devices: disable the "chip select" after accessing
   the Dataflash. The current draw with cs enabled is "very" high. You can
   simply use the macro DF_CS_inactive already defined by Atmel after every
   DF access

   The coin-cell battery on the Butterfly is not a reliable power-source if data
   in the flash-array should be changed (write/erase).
   See the Dataflash datasheet for the current needed during write-accesses.
 */
//*****************************************************************************
/*
  Dirk Spaanderman: changed the dataflash code to a c++ library for arduino
 */


/*****************************************************************************

Filename:    DataFlash.cpp
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

#include "dataflash.h"

// configuration for the Atmel AT45DB161D device
#define PAGE_BITS    10
#define PAGE_SIZE    528

void Dataflash::init(unsigned char ssPin)
{
    char clr;

    // setup the slave select pin
    slave_select = ssPin;
    pinMode (slave_select, OUTPUT);

    // setup the SPI pins
    pinMode(11, OUTPUT); // MOSI
    pinMode(12, INPUT);  // MISO
    pinMode(13, OUTPUT); // SCK

    // disable the SPI device
    digitalWrite(slave_select, HIGH);

    // configure the SPI registers
    SPCR = (1<<SPE)|(1<<MSTR);
    SPSR = (1<<SPI2X);

    // clear the SPI registers
    clr = SPSR;
    clr = SPDR;
}

unsigned char Dataflash::DF_SPI_RW(unsigned char output)
{
    SPDR = output;                 // Start the transmission
    while (!(SPSR & (1<<SPIF)))    // Wait the end of the transmission
    {
    };
    return SPDR;
}

unsigned char Dataflash::Read_DF_status (void)
{
    unsigned char result;

    DF_CS_active();
    result = DF_SPI_RW(StatusReg);
    result = DF_SPI_RW(0x00);
    DF_CS_inactive();

    return result;
}

void Dataflash::Read_DF_ID (unsigned char* data)
{
    DF_CS_active();
    DF_SPI_RW(ReadMfgID);
    data[0] = DF_SPI_RW(0x00);
    data[1] = DF_SPI_RW(0x00);
    data[2] = DF_SPI_RW(0x00);
    data[3] = DF_SPI_RW(0x00);
    DF_CS_inactive();
}

/*****************************************************************************
 *
 *  Function name : Page_To_Buffer
 *
 *  Returns :       None
 *  
 *	Parameters :	BufferNo	->	Decides usage of either buffer 1 or 2
 * 
 *			        PageAdr		->	Address of page to be transferred to buffer
 * 
 *	Purpose :	Transfers a page from flash to Dataflash SRAM buffer
 * 					
 * 
 ******************************************************************************/
void Dataflash::Page_To_Buffer (unsigned int PageAdr, unsigned char BufferNo)
{
    DF_CS_active();
    if (1 == BufferNo)                                              //transfer flash page to buffer 1
    {
        DF_SPI_RW(FlashToBuf1Transfer);
        DF_SPI_RW((unsigned char)(PageAdr >> (16 - PAGE_BITS)));    //upper part of page address
        DF_SPI_RW((unsigned char)(PageAdr << (PAGE_BITS - 8)));     //lower part of page address
        DF_SPI_RW(0x00);                                            //don't cares
    }
#ifdef USE_BUFFER2
    else if (2 == BufferNo)                                         //transfer flash page to buffer 2
    {
        DF_SPI_RW(FlashToBuf2Transfer);
        DF_SPI_RW((unsigned char)(PageAdr >> (16 - PAGE_BITS)));	//upper part of page address
        DF_SPI_RW((unsigned char)(PageAdr << (PAGE_BITS - 8)));     //lower part of page address
        DF_SPI_RW(0x00);						                    //don't cares
    }
#endif
    DF_CS_inactive();

    while(!(Read_DF_status() & 0x80));      //monitor the status register, wait until busy-flag is high
}

/*****************************************************************************
 *  
 *	Function name : Buffer_Read_Byte
 *  
 *	Returns :		One read byte (any value)
 *
 *	Parameters :	BufferNo	->	Decides usage of either buffer 1 or 2
 * 
 *					IntPageAdr	->	Internal page address
 *  
 *	Purpose :		Reads one byte from one of the Dataflash
 * 
 *					internal SRAM buffers
 * 
 ******************************************************************************/
unsigned char Dataflash::Buffer_Read_Byte (unsigned char BufferNo, unsigned int IntPageAdr)
{
    unsigned char data;

    DF_CS_active();
    if (1 == BufferNo)      //read byte from buffer 1
    {
        DF_SPI_RW(Buf1Read);			//buffer 1 read op-code
        DF_SPI_RW(0x00);				//don't cares
        DF_SPI_RW((unsigned char)(IntPageAdr>>8));  //upper part of internal buffer address
        DF_SPI_RW((unsigned char)(IntPageAdr));     //lower part of internal buffer address
        DF_SPI_RW(0x00);				//don't cares
        data = DF_SPI_RW(0x00);			//read byte
    }
#ifdef USE_BUFFER2
    else if (2 == BufferNo)  //read byte from buffer 2
    {
        DF_SPI_RW(Buf2Read);            //buffer 2 read op-code
        DF_SPI_RW(0x00);                //don't cares
        DF_SPI_RW((unsigned char)(IntPageAdr>>8));  //upper part of internal buffer address
        DF_SPI_RW((unsigned char)(IntPageAdr));     //lower part of internal buffer address
        DF_SPI_RW(0x00);                //don't cares
        data = DF_SPI_RW(0x00);         //read byte
    }
#endif
    DF_CS_inactive();

    return data;    //return the read data byte
}

/*****************************************************************************
 *  
 *	Function name : Buffer_Write_Byte
 * 
 *	Returns :		None
 *  
 *	Parameters :	IntPageAdr	->	Internal page address to write byte to
 * 
 *			BufferAdr	->	Decides usage of either buffer 1 or 2
 * 
 *			Data		->	Data byte to be written
 *  
 *	Purpose :		Writes one byte to one of the Dataflash
 * 
 *					internal SRAM buffers
 *
 ******************************************************************************/
void Dataflash::Buffer_Write_Byte (unsigned char BufferNo, unsigned int IntPageAdr, unsigned char Data)
{
    DF_CS_active();
    if (1 == BufferNo)              //write byte to buffer 1
    {
        DF_SPI_RW(Buf1Write);       //buffer 1 write op-code
        DF_SPI_RW(0x00);            //don't cares
        DF_SPI_RW((unsigned char)(IntPageAdr>>8));  //upper part of internal buffer address
        DF_SPI_RW((unsigned char)(IntPageAdr));     //lower part of internal buffer address
        DF_SPI_RW(Data);            //write data byte
    }
#ifdef USE_BUFFER2
    else if (2 == BufferNo)         //write byte to buffer 2
    {
        DF_SPI_RW(Buf2Write);       //buffer 2 write op-code
        DF_SPI_RW(0x00);            //don't cares
        DF_SPI_RW((unsigned char)(IntPageAdr>>8));  //upper part of internal buffer address
        DF_SPI_RW((unsigned char)(IntPageAdr));     //lower part of internal buffer address
        DF_SPI_RW(Data);            //write data byte
    }
#endif
    DF_CS_inactive();
}

/*****************************************************************************
 * 
 * 
 *	Function name : Buffer_To_Page
 * 
 *	Returns :		None
 *  
 *	Parameters :	BufferAdr	->	Decides usage of either buffer 1 or 2
 * 
 *			PageAdr		->	Address of flash page to be programmed
 *  
 *	Purpose :	Transfers a page from Dataflash SRAM buffer to flash
 * 
 *			 
 ******************************************************************************/
void Dataflash::Buffer_To_Page (unsigned char BufferNo, unsigned int PageAdr)
{
    DF_CS_active();
    if (1 == BufferNo)
    {
        DF_SPI_RW(Buf1ToFlashWE);           //buffer 1 to flash with erase op-code
        DF_SPI_RW((unsigned char)(PageAdr >> (16 - PAGE_BITS))); //upper part of page address
        DF_SPI_RW((unsigned char)(PageAdr << (PAGE_BITS - 8)));  //lower part of page address
        DF_SPI_RW(0x00);                    //don't cares
    }
#ifdef USE_BUFFER2
    else if (2 == BufferNo)
    {
        DF_SPI_RW(Buf2ToFlashWE);           //buffer 2 to flash with erase op-code
        DF_SPI_RW((unsigned char)(PageAdr >> (16 - PAGE_BITS)));    //upper part of page address
        DF_SPI_RW((unsigned char)(PageAdr << (PAGE_BITS - 8)));     //lower part of page address
        DF_SPI_RW(0x00);                    //don't cares
    }
#endif
    DF_CS_inactive();

    while(!(Read_DF_status() & 0x80));      //monitor the status register, wait until busy-flag is high
}

void Dataflash::DF_CS_inactive()
{
    digitalWrite(slave_select,HIGH);
}
void Dataflash::DF_CS_active()
{
    digitalWrite(slave_select,LOW);
}

Dataflash dflash;
