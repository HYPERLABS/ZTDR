//==============================================================================
//
// Title:		ZTDR.h
// Purpose:		A short description of the interface.
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

void 	calAcquireWaveform (int calStepIndex);
void 	calDAC (void);
void 	calFindDiscont (void);
void 	calFindMean (int calStepIndex);
void 	calFindStepcount (void);
void 	calReconstructData (void);
void 	calSetParams (void);
void 	calSetupTimescale (void);
void 	calTimebase (void);
int 	calWriteParams (void);
void 	FindOptimalStepCount (void);
int	 	main (int argc, char *argv[]);
double 	mean_array (void);
void 	openDevice (void);
void 	reconstructData (double offset);
void 	setupTimescale (void);
void 	vertCal (void);
void 	vertCalTimescale (void);
void 	vertCalTimescale0 (double windowStart);
int 	vertCalWriteParams (void);
		
		

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __ZTDR_H__ */
