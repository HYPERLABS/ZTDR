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

#include "FTD2XX.h"

#include "usbfifo.h"


//==============================================================================
// Constants

#define NPARAMS 26
 
#define IDX_FREERUN 0
#define IDX_STEPCNT_UPPER 1
#define IDX_STEPCNT_LOWER 2
#define IDX_RECLEN_UPPER 3
#define IDX_RECLEN_LOWER 4
#define IDX_DAC0_UPPER 5
#define IDX_DAC0_LOWER 6 
#define IDX_DAC1_UPPER 7
#define IDX_DAC1_LOWER 8
#define IDX_DAC2_UPPER 9
#define IDX_DAC2_LOWER 10
#define IDX_CALSTART_UPPER 11
#define IDX_CALSTART_LOWER 12
#define IDX_CALEND_UPPER 13
#define IDX_CALEND_LOWER 14
#define IDX_TMSTART_B3 15  
#define IDX_TMSTART_B2 16
#define IDX_TMSTART_B1 17
#define IDX_TMSTART_B0 18
#define IDX_TMEND_B3 19	 
#define IDX_TMEND_B2 20
#define IDX_TMEND_B1 21
#define IDX_TMEND_B0 22
#define IDX_OVERSAMPLE 23
#define IDX_STROBECNT_UPPER 24
#define IDX_STROBECNT_LOWER 25

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
// Global functions (alphabetical, not by functionality)

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

// Acquire from FDTI device
int usbfifo_acquire (UINT8 *ret_val, UINT8 arg)
{
	char ch;
	FT_STATUS stat;

	if (!dev_opened)
	{
		return 0;
	}

	ftwrbyte ('a');
	ftwrbyte (arg);
	stat = FT_SetTimeouts (dev_handle, 1000, 1000);
	*ret_val = ftrdbyte ();

	ch = ftrdbyte();

	stat = FT_SetTimeouts (dev_handle, STD_TIMEOUT, STD_TIMEOUT);

	if (ch != '.')
	{
		return -1;
	}
	else
	{
		return 1;
	}

}

// Close FTDI device
void usbfifo_close (void)
{
	FT_STATUS stat;
	
	if (!dev_opened)
	{
		return;
	}
	
	stat = FT_Close (dev_handle);
	stat = FT_Close (dev_fifo_handle);
	
	dev_opened = 0;
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

// Read data blocks of acquisition
int usbfifo_readblock(UINT8 block_no, UINT16 *buf)
{
#define BLOCK_LEN 256

	char ch;
	int n, ret,i;
	UINT8 rawbuf8[2*BLOCK_LEN];
	
	if (!dev_opened)
	{
		return 0;
	}
	
	ftwrbyte ('b');
	ftwrbyte (block_no);
	
	ret = FT_Read (dev_fifo_handle, rawbuf8, BLOCK_LEN * 2, &n);
	
	ch = ftrdbyte();
	
	if (ch == 'f')
	{
		// handle failure, clean out FIFO and return error
		while (FT_Read (dev_fifo_handle, rawbuf8, 1, &n) == FT_OK && n == 1);
		return -1;
	}
	
	if (n != 2*BLOCK_LEN)
	{
		return -2;
	}
	
	for (i=0;i<BLOCK_LEN;i++)
	{
		buf[i] = (((UINT16) rawbuf8[2 * i + 1]) << 8) | ((UINT16) rawbuf8[2 * i]);
	}
	
	return 1;
}

// Set parameters for acquisition
int usbfifo_setparams (UINT8 freerun_en, UINT16 calstart, UINT16 calend, timeinf tmstart, timeinf tmend, UINT16 stepcount,
					   UINT16 strobecount, UINT8 noversample, UINT16 record_len, UINT16 dac0, UINT16 dac1, UINT16 dac2)
{
	static UINT8 params[NPARAMS];
	int ch;
	int n;
	int ret;

	if (!dev_opened)
	{
		return 0;
	}

	params[IDX_FREERUN] = freerun_en;
	params[IDX_STEPCNT_UPPER] = stepcount >> 8;
	params[IDX_STEPCNT_LOWER] = stepcount;
	params[IDX_RECLEN_UPPER] = record_len >> 8;
	params[IDX_RECLEN_LOWER] = record_len;
	params[IDX_DAC0_UPPER] = dac0 >> 8;
	params[IDX_DAC0_LOWER] = dac0;
	params[IDX_DAC1_UPPER] = dac1 >> 8;
	params[IDX_DAC1_LOWER] = dac1;
	params[IDX_DAC2_UPPER] = dac2 >> 8;
	params[IDX_DAC2_LOWER] = dac2;
	params[IDX_CALSTART_UPPER] = calstart >> 8;
	params[IDX_CALSTART_LOWER] = calstart;
	params[IDX_CALEND_UPPER] = calend >> 8;
	params[IDX_CALEND_LOWER] = calend;
	params[IDX_TMSTART_B3] = tmstart.time_b.b3;
	params[IDX_TMSTART_B2] = tmstart.time_b.b2;
	params[IDX_TMSTART_B1] = tmstart.time_b.b1;
	params[IDX_TMSTART_B0] = tmstart.time_b.b0;
	params[IDX_TMEND_B3] = tmend.time_b.b3;
	params[IDX_TMEND_B2] = tmend.time_b.b2;
	params[IDX_TMEND_B1] = tmend.time_b.b1;
	params[IDX_TMEND_B0] = tmend.time_b.b0;
	params[IDX_OVERSAMPLE] = noversample;
	params[IDX_STROBECNT_UPPER] = strobecount >> 8;
	params[IDX_STROBECNT_LOWER] = strobecount;

	ftwrbyte('p');
	ret = FT_Write(dev_handle, params, NPARAMS, &n);
	ch = ftrdbyte();
	
	int egg = 1;

	if (ch != '.')
	{
		// No record received
		return -1;
	}

	if (n != NPARAMS)
	{
		// Incorrect number of params passed
		return -2;
	}

	return 1;
}
