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

#define FILTER_WIDTH 4	

	
//==============================================================================
// Types

	
//==============================================================================
// External variables

int 	usb_opened;


//==============================================================================
// Global functions (sorted alphabetically)

void 	calAcquireWaveform (int calStepIndex);
void 	calDAC (void);
void 	calFindDiscont (void);
void 	calFindMean (int calStepIndex);
void 	calFindStepcount (void);
void 	calReconstructData (void);
void 	calSetParams (void);
void 	calSetupTimescale (void);
void 	calTimebase (void); 
double 	mean_array (void);
void 	openDevice (void);
void 	reconstructData (double offset);
void 	setEnviron (int x, int y, double start, double end, double k);
void 	setupTimescale (void);
void 	vertCal (void);
void 	vertCalTimescale (void);
void 	vertCalZero (double windowStart);
int 	vertCalWriteParams (void);
int 	writeParams (void);


#ifdef __cplusplus
}
#endif

#endif  /* ndef __driver_H__ */
