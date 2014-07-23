#include <stdlib.h>
#include <stdio.h>

// FTDI driver
#include "FTD2XX.h"

// Include DLL 
//#include "usbfifodll.h"	    
#include "usbfifo.h"










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
