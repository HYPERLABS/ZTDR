//==============================================================================
//
// Title:		ZTDR_1XX.h
// Purpose:		ZTDR driver module and DLL functionality (v1.x.x)
//
// Created on:	7/28/2014 at 9:10:43 AM by Brian Doxey.
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

#include "cvidef.h"
	
#include "FTD2XX.h"

	
//==============================================================================
// Constants	

#define CALSTART_DEFAULT 540
#define CALEND_DEFAULT 3870
#define CAL_WINDOW_START 10 // ns
#define CAL_WINDOW 10.0e-9  // 10 ns
#define CAL_GUARD 0.65e-9 // 0.5 ns
	
#define FILTER_WIDTH 4

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

__stdcall	typedef unsigned int UINT32;
__stdcall	typedef unsigned short UINT16;
__stdcall	typedef unsigned char UINT8;

__stdcall	struct	_delay16
{
	UINT16 frac_val;
	UINT16 int_val;
};

__stdcall	struct	_delay8
{
	UINT8 b0;
	UINT8 b1;
	UINT8 b2;
	UINT8 b3;
};


__stdcall	union _timeinf
{
	UINT32 time;
	struct _delay16 time_s;
	struct _delay8 time_b;
};

__stdcall	typedef union _timeinf timeinf;

	
//==============================================================================
// Global variables

__stdcall	int		usb_opened;

// Calibration
__stdcall	double 	calDiscLevel;
__stdcall	double 	calLevels[5];
__stdcall	double 	calThreshold;
__stdcall	int 	calIncrement;
__stdcall	double	vampl;

// Acquisition environment
__stdcall	double	diel;
__stdcall	int 	yUnits;
__stdcall	int 	xUnits;
__stdcall	double	xStart;
__stdcall	double	xEnd;	

// Number of data points acquired
__stdcall	UINT16 	recLen;

// Waveform storage
__stdcall	double 	wfmDistFt[NPOINTS_MAX]; // distance (ft)
__stdcall	double 	wfmDistM[NPOINTS_MAX]; // distance (m)
__stdcall	double 	wfmTime[NPOINTS_MAX]; // time (ns)
__stdcall	double 	wfmX[NPOINTS_MAX]; // converted to selected units 

__stdcall	UINT16 	wfm[NPOINTS_MAX]; 		// Raw data from device
__stdcall	double 	wfmFilter[NPOINTS_MAX];	// Filtered data from device
__stdcall	double	wfmData[NPOINTS_MAX]; // converted to selected units

// Start/end time for device
__stdcall	timeinf start_tm, end_tm;

// USBFIFO functionality
__stdcall	FT_HANDLE 	dev_fifo_handle;
__stdcall	FT_HANDLE 	dev_handle;
__stdcall	int 		dev_hostbps; 

__stdcall	UINT16 	calstart; 
__stdcall	UINT16 	calend;
__stdcall	UINT16 	stepcount;
__stdcall	UINT16 	stepcountArray[5];
__stdcall	int 	freerun_en;
__stdcall	UINT16 	dac0val, dac1val, dac2val;
__stdcall	UINT16 	strobecount;

__stdcall	char 	dev_comspdbuf[20];
__stdcall	char 	dev_idbuf[20];
__stdcall	int 	dev_opened;


//==============================================================================
// Global functions (sorted alphabetically)

__stdcall 	int 	acquireWaveform (void);
__stdcall	void 	calAcquireWaveform (int calStepIndex);
__stdcall	void 	calDAC (void);
__stdcall	void 	calFindDiscont (void);
__stdcall	void 	calFindMean (int calStepIndex);
__stdcall	int 	calFindStepcount (void);
__stdcall	void 	calReconstructData (void);
__stdcall	void 	calSetParams (void);
__stdcall	void 	calSetupTimescale (void);
__stdcall	int 	calTimebase (void); 
__stdcall	int 	initDevice (void);
__stdcall	double 	meanArray (void);
__stdcall	void 	openDevice (void);
__stdcall	void 	reconstructData (double offset);
__stdcall	void 	setEnviron (int x, int y, double start, double end, double k);
__stdcall	void 	setupTimescale (void);
__stdcall	void 	vertCal (void);
__stdcall	void 	vertCalTimescale (void);
__stdcall	void 	vertCalZero (double windowStart);
__stdcall	int 	vertCalWriteParams (void);
__stdcall	int 	writeParams (void);

// USBFIFO functionality
__stdcall	char 	ftrdbyte (void);
__stdcall	void 	ftwrbyte (char ch);
__stdcall	int 	usbfifo_acquire (UINT8 *ret_val, UINT8 arg);
__stdcall	void 	usbfifo_close (void);
__stdcall	void 	usbfifo_getcomspd (char *buf, int len);
__stdcall	int 	usbfifo_gethostbps (void);
__stdcall	void 	usbfifo_getid (char *buf, int len);
__stdcall	int 	usbfifo_open (void);
__stdcall	int 	usbfifo_readblock (UINT8 block_no, UINT16 *buf);
__stdcall	int 	usbfifo_setparams (UINT8 freerun_en, UINT16 calstart, UINT16 calend, timeinf tmstart, timeinf tmend, UINT16 stepcount,
					   UINT16 strobecount, UINT8 noversample, UINT16 record_len, UINT16 dac0, UINT16 dac1, UINT16 dac2);


#ifdef __cplusplus
}
#endif

#endif  /* ndef __driver_H__ */
