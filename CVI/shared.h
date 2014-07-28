//==============================================================================
//
// Title:		shared.h
// Purpose:		Declaration of routines used by shared.c
//
// Created on:	7/22/2014 at 8:40:39 PM by Brian Doxey.
// Copyright:	HYPERLABS. All Rights Reserved.
//
//==============================================================================

#ifndef __ZTDR_H__
#define __ZTDR_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"
		
#include "usbfifo.h"


//==============================================================================
// Constants


//==============================================================================
// Types


//==============================================================================
// External variables


//==============================================================================
// Global functions (TO DO: sort)

void 	acquire (void);
void 	calAcquireWaveform (int calStepIndex);
void 	calDAC (void);
void 	calFindDiscont (void);
void 	calFindMean (int calStepIndex);
void 	calFindStepcount (void);
void 	calReconstructData (void);
void 	calSetParams (void);
void 	calSetupTimescale (void);
void 	calTimebase (void);
void 	changePlot (int unit);
void 	changeUnitX (int unit);
void 	changeUnitY (int unit);
void 	checkDirs (void);
void	clearWaveform (void);
void	main (int argc, char *argv[]);
double 	mean_array (void);
void	minMax (double a[], int len, double *min, double *max);
void 	openDevice (void);
void	printWaveform (void);
void 	recallWaveform (void);
void 	reconstructData (double offset);
void	resetZoom (void);
void 	savePNG (void);
void 	storeWaveform (int format);
void 	setAuto (void);
void 	setupTimescale (void);
void	showVersion (void);
void	updateCursors (void);
void	updateSize (void);
void 	vertCal (void);
void 	vertCalTimescale (void);
int 	vertCalWriteParams (void);
void 	vertCalZero (double windowStart);
int 	writeParams (void);
void 	zoom (void);
		
		

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __ZTDR_H__ */
