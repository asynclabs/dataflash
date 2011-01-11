
/*****************************************************************************

Filename:    DataFlash_Intro.pde
Description: DataFlash sample application for the FlashShield 1.0

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

#include <DataFlash.h>

void setup()
{
	Serial.begin(115200);
	Serial.print('h',BYTE);
	Serial.print('i',BYTE);
	Serial.print('\n',BYTE);
	
	/* using pin D10 as DataFlash slave select */
	dflash.init(10);
}

void loop()
{
	unsigned char data[4];
	char i = 0;
	char buf;
	
	dflash.Read_DF_ID(data);
	
	Serial.print("Mfg ID: ");
	Serial.println(data[0], HEX);
	Serial.print("Dev ID: ");
	Serial.print(data[1], HEX);
	Serial.println(data[2], HEX);
	
	/* write a sequence of integers into
	 * the DataFlash SRAM buffer 1 */
	for (i = 0; i < 10; i++) {
		dflash.Buffer_Write_Byte(1, i, i);
	}
	
	/* program the data in the SRAM buffer 1
	 * to Page 0 in the DataFlash */
	dflash.Buffer_To_Page(1, 0);
	
	/* write zeros into the SRAM buffer 1 */
	for (i = 0; i < 10; i++) {
		dflash.Buffer_Write_Byte(1, i, 0);
	}
	
	/* transfer the data from Page 0 in the
	 * DataFlash into the SRAM buffer 1 */
	dflash.Page_To_Buffer(0, 1);
	
	/* print the integers from the SRAM buffer 1
	 * to the serial port */
	for (i = 0; i < 10; i++) {
		Serial.print(dflash.Buffer_Read_Byte(1, i), HEX);
	}
	
	/* wait here */
	while(1) {
		delay(1000);
	}
}
