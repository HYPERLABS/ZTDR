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
// Global functions

int	 	main (int argc, char *argv[]);
void 	openDevice (void);
void 	setupTimescale (void);
		
		

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __ZTDR_H__ */
