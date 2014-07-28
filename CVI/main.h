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


//==============================================================================
// Constants

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
	
extern	int		diel;
extern	int		plotType;
extern	int 	yUnits;
extern	int 	xUnits;
extern	int		xStart;
extern	int		xEnd;	


//==============================================================================
// Global functions (TO DO: sort)

void 	acquire (void);
void 	changeAuto (void);
void 	changePlot (int unit);
void 	changeUnitX (int unit);
void 	changeUnitY (int unit);
void 	checkDirs (void);
void	clearWaveform (void);
void	main (int argc, char *argv[]);
void	printWaveform (void);
void 	recallWaveform (void);
void	resetZoom (void);
void 	savePNG (void);
void 	storeWaveform (int format);
void	showVersion (void);
void	updateCursors (void);
void	updateSize (void);
void 	zoom (void);


#ifdef __cplusplus
}
#endif

#endif  /* ndef __ZTDR_H__ */
