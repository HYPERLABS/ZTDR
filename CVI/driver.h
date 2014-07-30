//==============================================================================
//
// Title:		driver.h
// Purpose:		Declaration of functions used by driver.c
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


//==============================================================================
// Types

	
//==============================================================================
// External variables

extern	int 	usb_opened;
extern	int 	calIncrement;
extern	double 	vampl;

// Acquisition environment
extern	double	diel;
extern	int		plotType;
extern	int 	yUnits;
extern	int 	xUnits;
extern	double	xStart;
extern	double	xEnd;	

// Number of data points acquired
extern	UINT16 	recLen;

// Waveform storage
extern	double 	wfmDistFt[NPOINTS_MAX]; // distance (ft)
extern	double 	wfmDistM[NPOINTS_MAX]; // distance (m)
extern	double 	wfmTime[NPOINTS_MAX]; // time (ns)
extern	double 	wfmX[NPOINTS_MAX]; // converted to selected units 

extern	UINT16 	wfm[NPOINTS_MAX]; 		// Raw data from device
extern	double 	wfmFilter[NPOINTS_MAX];	// Filtered data from device
extern	double	wfmData[NPOINTS_MAX]; // converted to selected units


//==============================================================================
// Global functions (sorted alphabetically)

void 	__stdcall 	calAcquireWaveform (int calStepIndex);
void 	__stdcall 	calDAC (void);
void 	__stdcall 	calFindDiscont (void);
void 	__stdcall 	calFindMean (int calStepIndex);
int 	__stdcall 	calFindStepcount (void);
void 	__stdcall 	calReconstructData (void);
void 	__stdcall 	calSetParams (void);
void 	__stdcall 	calSetupTimescale (void);
int 	__stdcall 	calTimebase (void); 
double 	__stdcall 	meanArray (void);
void 	__stdcall 	openDevice (void);
void 	__stdcall 	reconstructData (double offset);
void 	__stdcall 	setEnviron (int x, int y, double start, double end, double k);
void 	__stdcall 	setupTimescale (void);
void 	__stdcall 	vertCal (void);
void 	__stdcall 	vertCalTimescale (void);
void 	__stdcall 	vertCalZero (double windowStart);
int 	__stdcall 	vertCalWriteParams (void);
int 	__stdcall 	writeParams (void);


#ifdef __cplusplus
}
#endif

#endif  /* ndef __driver_H__ */
