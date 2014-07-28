//==============================================================================
//
// Title:		shared.h
// Purpose:		Declaration of routines used by main.c
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
void 	changePlot (int unit);
void 	changeUnitX (int unit);
void 	changeUnitY (int unit);
void 	checkDirs (void);
void	clearWaveform (void);
void	main (int argc, char *argv[]);
void	minMax (double a[], int len, double *min, double *max);
void	printWaveform (void);
void 	recallWaveform (void);
void	resetZoom (void);
void 	savePNG (void);
void 	storeWaveform (int format);
void 	setAuto (void);
void	showVersion (void);
void	updateCursors (void);
void	updateSize (void);
void 	zoom (void);


#ifdef __cplusplus
}
#endif

#endif  /* ndef __ZTDR_H__ */
