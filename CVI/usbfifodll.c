#include <stdlib.h>
#include <stdio.h>

// FTDI driver
#include "FTD2XX.h"

// Include DLL 
#include "usbfifodll.h"

#define STD_TIMEOUT 200 // serial timeout (ms)

static FT_HANDLE dev_handle;
static FT_HANDLE dev_fifo_handle;
static int dev_opened = 0;
static char dev_idbuf[20];
static char dev_comspdbuf[20];
static const int dev_hostbps = 256000;

static void ftwrbyte(char ch)
{
	int n;
	FT_Write( dev_handle, &ch, 1, &n);
}

static char ftrdbyte()
{
	char ch;
	int n;
	FT_Read( dev_handle, &ch, 1, &n);
	return ch;
}

int usbfifo_open()
{
	char ch;
	int n;
	FT_STATUS stat, statfifo;
	char buf[20];
	
	if (dev_opened)
		return 1;
		
	statfifo = FT_OpenEx("USBFIFOV1A", FT_OPEN_BY_SERIAL_NUMBER, &dev_fifo_handle);
	stat = FT_OpenEx("USBFIFOV1B", FT_OPEN_BY_SERIAL_NUMBER,  &dev_handle);
	
	if (stat != FT_OK || statfifo != FT_OK)
	{
		if (stat == FT_OK)
			FT_Close(dev_handle);
			
		if (statfifo == FT_OK)
			FT_Close(dev_fifo_handle);
			
		dev_opened = 0;
		
	}
	else
	{
		dev_opened = 1;
		
		stat = FT_SetBaudRate( dev_handle, dev_hostbps );
		if (stat != FT_OK)
			return -1;
		
		stat = FT_SetDataCharacteristics( dev_handle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE );
		if (stat != FT_OK)
			return -2;
			
		stat = FT_SetFlowControl( dev_handle, FT_FLOW_NONE, 'o', 'p' );  // o and p are bogus characters
		if (stat != FT_OK)
			return -3;
		
		stat = FT_SetTimeouts( dev_handle, STD_TIMEOUT, STD_TIMEOUT );
		if (stat != FT_OK)
			return -4;
			
		// added, then removed on 23 aug 09
		//stat = FT_SetLatencyTimer(dev_handle, 3);
		//if (stat != FT_OK)
		//	return -8;
		//FT_SetLatencyTimer(dev_fifo_handle, 2);
			
		/* read id string */
		ftwrbyte('i');
		FT_Read( dev_handle, dev_idbuf, 16, &n );
		dev_idbuf[16] = '\0';
		ftwrbyte('i');
		ch = ftrdbyte();
		
		if (ch != '.')
			return -5;
		
		
		/* read comspeed comspeed */
		ftwrbyte('s');
		FT_Read( dev_handle, dev_comspdbuf, 16, &n );
		dev_comspdbuf[16] = '\0';
		ftwrbyte('s');
		ch = ftrdbyte();
		
		if (ch != '.')
			return -6;
			
		
		if (strncmp(dev_idbuf, "USBFIFO", 7) != 0)
			return -7;
		
		FT_ClrDtr(dev_handle);
		// toggle reset
		FT_SetRts(dev_handle);
		FT_ClrRts(dev_handle);
	}
	return dev_opened;
}

int usbfifo_gethostbps(void)
{
	return dev_hostbps;
}

void usbfifo_devicereset(void)
{
	if (!dev_opened)
		return;
	
	FT_ClrDtr(dev_handle);
	// toggle reset
	FT_SetRts(dev_handle);
	FT_ClrRts(dev_handle);
}

void usbfifo_close(void)
{
	FT_STATUS stat;
	
	if (!dev_opened)
		return;
	
	stat = FT_Close (dev_handle);
	stat = FT_Close (dev_fifo_handle);
	
	dev_opened = 0;
}


int usbfifo_isok()
{
	return dev_opened;
}

void usbfifo_getid(char *buf, int len)
{
	int i;
	for (i=0;i<strlen(dev_idbuf)+1 && i < len && i<19;i++)
		*buf++ = dev_idbuf[i];
}

void usbfifo_getcomspd(char *buf, int len)
{
	int i;
	for (i=0;i<strlen(dev_comspdbuf)+1 && i < len && i<19;i++)
		*buf++ = dev_comspdbuf[i];
}

void usbfifo_listdevs()
{
	int i;
	FT_STATUS stat;
	int numDevs;
	FT_DEVICE_LIST_INFO_NODE *devInfo;
		
	stat = FT_CreateDeviceInfoList(&numDevs);
	if (stat == FT_OK)
	{
		printf("Number of FTDI devices connected is %d\n", numDevs);
		
		devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);
		
		stat = FT_GetDeviceInfoList(devInfo, &numDevs);
		for (i=0;i<numDevs;i++)
		{
		
			printf("Dev# %d\n", i);
			printf("   Type=0x%x\n",devInfo[i].Type);
			printf("   ID=0x%x\n",devInfo[i].ID);
			printf("   LocId=0x%x\n",devInfo[i].LocId);
			printf("   SerialNumber=%s\n",devInfo[i].SerialNumber);
			printf("   Description=%s\n",devInfo[i].Description);
			printf("   ftHandle=0x%x\n\n",devInfo[i].ftHandle);
		}
		
		
		free (devInfo);
	}
}


int usbfifo_checkmem(int *nlocs, int *failval)
{
#define CHKMEMLEN 6
	unsigned char buf[CHKMEMLEN];
	char str[128];
	int i; unsigned short x,y;
	
	if (!dev_opened)
		return -1;
	
	ftwrbyte('m');
	buf[0] = ftrdbyte();
	buf[1] = ftrdbyte();
	buf[2] = ftrdbyte();
	buf[3] = ftrdbyte();
	buf[4] = ftrdbyte();
	buf[5] = ftrdbyte();
						
	if (buf[CHKMEMLEN-1] != '.')
		return -1;
	
	x = (((unsigned short)buf[1])<<8) 
				| (((unsigned short)buf[2])&0x00FF);
	y = (((unsigned short)buf[3])<<8) 
				| (((unsigned short)buf[4])&0x00FF);
	
	*nlocs = x;
	*failval = y;
	
	
	/*if (buf[0] == 's')
		sprintf(str, "Mem test success, %d locations verified.", x);
	else
		sprintf(str, "Mem test failed at offset %d, readback=%d", x, y);*/
	
	return buf[0] == 's';
}

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


int usbfifo_setparams(
		UINT8 freerun_en,
		UINT16 calstart,
		UINT16 calend,
		timeinf tmstart,
		timeinf tmend,
		UINT16 stepcount,
		UINT16 strobecount,
		UINT8 noversample,
		UINT16 record_len, 
		UINT16 dac0,   
		UINT16 dac1, 
		UINT16 dac2 )
{
	static UINT8 params[NPARAMS];
	int ch;
	int n;
	int ret;
	
	if (!dev_opened)
		return 0;
	
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
	
	if (ch != '.')
		return -1;
	
	if (n != NPARAMS)
		return -2;
	
	return 1;
}

int usbfifo_readblock(UINT8 block_no, UINT16 *buf)
{
#define BLOCK_LEN 256

	char ch;
	int n, ret,i;
	UINT8 rawbuf8[2*BLOCK_LEN];
	
	if (!dev_opened)
		return 0;
	
	ftwrbyte( 'b' );
	ftwrbyte( block_no );
	
	ret = FT_Read( dev_fifo_handle, rawbuf8, BLOCK_LEN*2, &n );
	
	ch = ftrdbyte();
	if (ch == 'f')
	{
		// handle failure, clean out FIFO and return error
		while (FT_Read(dev_fifo_handle, rawbuf8, 1, &n) == FT_OK && n == 1);
		return -1;
	}
	
	if (n != 2*BLOCK_LEN)
		return -2;
	
	for (i=0;i<BLOCK_LEN;i++)
		buf[i] = (((UINT16)rawbuf8[2*i+1]) << 8) | ((UINT16)rawbuf8[2*i]);
	
	return 1;
}
		
int usbfifo_acquire(UINT8 *ret_val, UINT8 arg)
{
	char ch;
//	char testCh;
	FT_STATUS stat;
	
//	testCh = '.';
//	printf("\nTestChar %d\n", testCh);
	
	if (!dev_opened)
		return 0;
	
//	stat = FT_SetTimeouts( dev_handle, 1000, 1000 );
		
	ftwrbyte('a');
	ftwrbyte(arg);
	stat = FT_SetTimeouts( dev_handle, 1000, 1000 );
	*ret_val = ftrdbyte();
//	printf("\nWrite a and Return Val String = %c\n",*ret_val);
//	printf("\nWrite a and Return Val Decimal = %d\n",*ret_val);
	
	
	ch = ftrdbyte();
//	printf("Second Read String = %c\n",ch);
//	printf("Second Read Decimal = %d\n",ch);
	
	stat = FT_SetTimeouts( dev_handle, STD_TIMEOUT, STD_TIMEOUT );
	
	if (ch != '.')
		return -1;
	//	return 1;	//	  Aki  HACK
	else
		return 1;
	
}

int usbfifo_command(UINT16 *ret_val, UINT8 cmd, UINT16 arg0, UINT16 arg1)
{  
	char ch;
	UINT8 ret1, ret0;
	
	if (!dev_opened)
		return 0;
	
	ftwrbyte('f');
	ftwrbyte( cmd );
	ftwrbyte( (arg0 >> 8) & 0x00FF );
	ftwrbyte( arg0 & 0x00FF );
	ftwrbyte( (arg1 >> 8) & 0x00FF );
	ftwrbyte( arg1 & 0x00FF );
	
	ret0 = ftrdbyte();
	ret1 = ftrdbyte();
	
	*ret_val = (((UINT16)ret0) << 8) | ((UINT16)ret1);
	
	ch = ftrdbyte();
	if (ch != '.')
		return -1;
	else
		return 1;
}
