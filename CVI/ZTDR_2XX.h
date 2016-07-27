//==============================================================================
//
// Title:		ZTDR_2XX.h
// Release:		2.0.5 (07/11/16)
// Purpose:		ZTDR driver module and DLL functionality (v2.x.x)
//
// Copyright:	(c) 2016, HYPERLABS INC. All rights reserved.
//
//==============================================================================

#ifndef __ZTDR_1XX_H__
#define __ZTDR_1XX_H__

#ifdef __cplusplus
extern "C" {
#endif


//==============================================================================
// Include files

#include "FTD2XX.h"


//==============================================================================
// Types

	typedef unsigned int UINT32;
	typedef unsigned short UINT16;
	typedef unsigned char UINT8;

	struct	_delay16
	{
		UINT16 frac_val;
		UINT16 int_val;
	};

	struct	_delay8
	{
		UINT8 b0;
		UINT8 b1;
		UINT8 b2;
		UINT8 b3;
	};

	union _timeinf
	{
		UINT32 time;
		struct _delay16 time_s;
		struct _delay8 time_b;
	};

	typedef union _timeinf timeinf;


//==============================================================================
// Global functions

	// User-facing functions
	int 	__stdcall	ZTDR_AcquireData (int numAvg);
	int 	__stdcall	ZTDR_CalAmplitude (void);
	int 	__stdcall	ZTDR_CalTimebase (void);
	int 	__stdcall	ZTDR_DumpFile (char *filename);
	double	__stdcall	ZTDR_FetchDataX (int idx);
	double	__stdcall	ZTDR_FetchDataY (int idx);
	int		__stdcall 	ZTDR_Init (void);
	int 	__stdcall	ZTDR_SetEnviron (int x, int y, double start, double end, double k, int rec);
	int 	__stdcall	ZTDR_SetRefX (double x);

	// Other driver functions
	int 	__stdcall	ZTDR_CalDAC (void);
	void 	__stdcall	ZTDR_CloseDevice (void);
	double 	__stdcall	ZTDR_FindDiscont (void);
	double 	__stdcall	ZTDR_GetMean (void);
	int 	__stdcall	ZTDR_PollDevice (int acqType);
	int 	__stdcall	ZTDR_QuantizeTimescale (void);
	int 	__stdcall	ZTDR_ReconstructData (double offset, int filter);

	// USBFIFO functionality  
	int 	__stdcall	USBFIFO_Acquire (UINT8 *ret_val, UINT8 arg);
	int 	__stdcall	USBFIFO_ReadBlock (UINT8 block_no, UINT16 *buf);
	char 	__stdcall	USBFIFO_ReadByte (void);
	FT_STATUS 	__stdcall	USBFIFO_WriteByte (char ch);

#ifdef __cplusplus
}
#endif

#endif  /* ndef __driver_H__ */
