//==============================================================================
//
// Title:		ZTDR_1XX.h
// Release:		1.0.1 (07/31/2014)
// Purpose:		ZTDR driver module and DLL functionality (v1.x.x)
//
// Copyright:	HYPERLABS. All Rights Reserved.
//
//==============================================================================

#ifndef __driver_H__
#define __driver_H__

#ifdef __cplusplus
extern "C" {
#endif

	
//==============================================================================
// Include files
					 
#include "FTD2XX.h"

	
//==============================================================================
// Constants	

// Calibration
#define CALSTART_DEFAULT 540
#define CALEND_DEFAULT 3870
#define CAL_WINDOW_ZERO 0 // ns
#define CAL_WINDOW 50.0e-9  // 10 ns
#define CAL_GUARD 2.0e-9 // 0.5 ns
	
#define FILTER_WIDTH 4
#define	CAL_RECLEN 1024 // data points for calibration
	
	
// Max data points of USBFIFO
#define NPOINTS_MAX 16384
	
// Horizontal units
#define UNIT_M 0
#define UNIT_FT 1
#define UNIT_NS 2

// Vertical units
#define UNIT_MV 0
#define UNIT_NORM 1  
#define UNIT_OHM 2
#define UNIT_RHO 3

// Unit conversion
#define MtoFT 3.2808
#define FTtoM 0.3048

// FTDI functionality	
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
											 
#define STD_TIMEOUT 200 

	
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
// External variables


//==============================================================================
// Global functions

// User-facing functions
int		__stdcall 	initDevice (void);
int 	__stdcall	setEnviron (int x, int y, double start, double end, double k, int rec);
int 	__stdcall	acquireWaveform (int numAvg);
int 	__stdcall	dumpFile (char *filename);
double	__stdcall	fetchDataX (int idx);
double	__stdcall	fetchDataY (int idx);

// Other driver functions
void 	__stdcall	calAcquireWaveform (int calStepIndex);
void 	__stdcall	calDAC (void);
void 	__stdcall	calFindDiscont (void);
void 	__stdcall	calFindMean (int calStepIndex);
int 	__stdcall	calFindStepcount (void);
void 	__stdcall	calReconstructData (void);
void 	__stdcall	calSetParams (void);
void 	__stdcall	calSetupTimescale (void);
int 	__stdcall	calTimebase (void); 
double 	__stdcall	meanArray (void);
void 	__stdcall	openDevice (void);
void 	__stdcall	reconstructData (double offset);
void 	__stdcall	setupTimescale (void);
void 	__stdcall	vertCal (void);
void 	__stdcall	vertCalTimescale (void);
void 	__stdcall	vertCalZero (double windowStart);
int 	__stdcall	vertCalWriteParams (void);
int 	__stdcall	writeParams (void);

// USBFIFO functionality
char 	__stdcall	ftrdbyte (void);
void 	__stdcall	ftwrbyte (char ch);
int 	__stdcall	usbfifo_acquire (UINT8 *ret_val, UINT8 arg);
void 	__stdcall	usbfifo_close (void);
void 	__stdcall	usbfifo_getcomspd (char *buf, int len);
int 	__stdcall	usbfifo_gethostbps (void);
void 	__stdcall	usbfifo_getid (char *buf, int len);
int 	__stdcall	usbfifo_open (void);
int 	__stdcall	usbfifo_readblock (UINT8 block_no, UINT16 *buf);
int 	__stdcall	usbfifo_setparams (UINT8 freerun_en, UINT16 calstart, UINT16 calend, timeinf tmstart, timeinf tmend, UINT16 stepcount,
					   UINT16 strobecount, UINT8 noversample, UINT16 record_len, UINT16 dac0, UINT16 dac1, UINT16 dac2);


#ifdef __cplusplus
}
#endif

#endif  /* ndef __driver_H__ */
