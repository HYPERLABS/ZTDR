//==============================================================================
//
// Title:		ZTDR_2XX.h
// Release:		2.0.2 (12/30/15)
// Purpose:		ZTDR driver module and DLL functionality (v2.x.x)
//
// Copyright:	(c) 2015, HYPERLABS INC. All rights reserved.
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
	int 	__stdcall	ZTDR_CalAmplitude (void);
	int 	__stdcall	ZTDR_CalTimebase (void);
	int		__stdcall 	ZTDR_Init (void);
	
	
	
	
	
	
	
	
	int 	__stdcall	setEnviron (int x, int y, double start, double end, double k, int rec);
	int 	__stdcall	setRefX (double x);
	int 	__stdcall	acquireWaveform (int numAvg);
	int 	__stdcall	dumpFile (char *filename);
	double	__stdcall	fetchDataX (int idx);
	double	__stdcall	fetchDataY (int idx);

	// Other driver functions
	void 	__stdcall	ZTDR_CloseDevice (void);
	int 	__stdcall	ZTDR_PollDevice (int acqType);
	
	
	int 	__stdcall	calDAC (void);
	double 	__stdcall	calFindDiscont (void);
	double 	__stdcall	meanArray (void);
	int 	__stdcall	reconstructData (double offset, int filter);
	int 	__stdcall	setupTimescale (void);
	int 	__stdcall	writeParams (void);

	// USBFIFO functionality  
	
	FT_STATUS 	__stdcall	ftwrbyte (char ch);
	
	
	
	char 	__stdcall	ftrdbyte (void);
	int 	__stdcall	usbfifo_acquire (UINT8 *ret_val, UINT8 arg);
	int 	__stdcall	usbfifo_readblock (UINT8 block_no, UINT16 *buf);
	int 	__stdcall	usbfifo_setparams (UINT8 freerun_en, UINT16 calstart, UINT16 calend, timeinf tmstart, timeinf tmend, UINT16 stepcount,
										   UINT16 strobecount, UINT8 noversample, UINT16 record_len, UINT16 dac0, UINT16 dac1, UINT16 dac2);


#ifdef __cplusplus
}
#endif

#endif  /* ndef __driver_H__ */
