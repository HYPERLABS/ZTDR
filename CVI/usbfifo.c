//==============================================================================
//
// Title:		usbfifo.c
// Purpose:		Updated USBFIFO functionality for HL1101
//
// Created on:	7/22/2014 at 8:05:26 PM by Brian Doxey.
// Copyright:	HYPERLABS. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include "usbfifo.h"

// TO DO: REMOVE
#include "callback.h"


//==============================================================================
// Constants					 

const int 	dev_hostbps = 256000;

//==============================================================================
// Types

//==============================================================================
// Static global variables

//==============================================================================
// Static functions

//==============================================================================
// Global variables (roughly grouped by function)

// Debug mode variables | TO DO: cleanup
char 	dev_comspdbuf[20];
char 	dev_idbuf[20];
int 	dev_opened = 0;

FT_HANDLE 	dev_fifo_handle;
FT_HANDLE 	dev_handle;

// Serial timeout (ms)
#define STD_TIMEOUT 200


//==============================================================================
// Global functions (alphabetical, not grouped by function)

// Read FTDI byte
static char ftrdbyte(void)
{
	char ch;
	int n;
	
	FT_Read (dev_handle, &ch, 1, &n);
	
	return ch;
}

// Write FTDI byte
void ftwrbyte(char ch)
{
	int n;
	
	FT_Write (dev_handle, &ch, 1, &n);
}


// Get device comm speed
void usbfifo_getcomspd (char *buf, int len)
{
	int i;
	
	for (i = 0; i < strlen (dev_comspdbuf) + 1 && i < len && i < 19; i++)
	{
		*buf++ = dev_comspdbuf[i];
	}
}

// Get host BPS
int usbfifo_gethostbps (void)
{
	return dev_hostbps;
}

// Get device ID
void usbfifo_getid (char *buf, int len)
{
	int i;
	
	for (i = 0; i < strlen (dev_idbuf) + 1 && i < len && i < 19; i++)
	{
		*buf++ = dev_idbuf[i];
	}
}

// Open FTDI for use by software
int usbfifo_open()
{
	char ch;
	int n;
	FT_STATUS stat, statfifo;
	char buf[20];
	
	if (dev_opened)
	{
		return 1;
	}
		
	statfifo = FT_OpenEx ("USBFIFOV1A", FT_OPEN_BY_SERIAL_NUMBER, &dev_fifo_handle);
	stat = FT_OpenEx ("USBFIFOV1B", FT_OPEN_BY_SERIAL_NUMBER, &dev_handle);
	
	if (stat != FT_OK || statfifo != FT_OK)
	{
		if (stat == FT_OK)
		{
			FT_Close(dev_handle);
		}
			
		if (statfifo == FT_OK)
		{
			FT_Close(dev_fifo_handle);
		}
			
		dev_opened = 0;
	}
	// TO DO: Debug mode functionality, not currently in use
	else
	{
		dev_opened = 1;
		
		stat = FT_SetBaudRate (dev_handle, dev_hostbps);
		if (stat != FT_OK)
		{
			return -1;
		}
		
		stat = FT_SetDataCharacteristics (dev_handle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
		
		if (stat != FT_OK)
		{
			return -2;
		}
			
		stat = FT_SetFlowControl (dev_handle, FT_FLOW_NONE, 'o', 'p');  // o and p are bogus characters
		
		if (stat != FT_OK)
		{
			return -3;
		}
		
		stat = FT_SetTimeouts (dev_handle, STD_TIMEOUT, STD_TIMEOUT);
		
		if (stat != FT_OK)
		{
			return -4;
		}
			
		// Read id string
		ftwrbyte('i');
		FT_Read (dev_handle, dev_idbuf, 16, &n);
		dev_idbuf[16] = '\0';
		ftwrbyte ('i');
		ch = ftrdbyte ();
		
		if (ch != '.')
		{
			return -5;
		}
		
		
		// Read compseed
		ftwrbyte('s');
		FT_Read( dev_handle, dev_comspdbuf, 16, &n );
		dev_comspdbuf[16] = '\0';
		ftwrbyte('s');
		ch = ftrdbyte();
		
		if (ch != '.')
		{
			return -6;
		}
			
		
		if (strncmp(dev_idbuf, "USBFIFO", 7) != 0)
		{
			return -7;
		}
		
		FT_ClrDtr(dev_handle);
		
		FT_SetRts(dev_handle);
		FT_ClrRts(dev_handle);
	}
	
	return dev_opened;
}
