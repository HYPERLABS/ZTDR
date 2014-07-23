//==============================================================================
//
// Title:		usbfifo.h
// Purpose:		A short description of the interface.
//
// Created on:	7/22/2014 at 8:05:26 PM by Brian Doxey.
// Copyright:	HYPERLABS. All Rights Reserved.
//
//==============================================================================

#ifndef __usbfifo_H__
#define __usbfifo_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"
		
#include "FTD2XX.h"

	
//==============================================================================
// Constants

		
//==============================================================================
// Types
		
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

// TO DO: are the BIG_ENDIAN references still valid?
struct _delay16
{
#ifdef BIG_ENDIAN
	UINT16 int_val;
	UINT16 frac_val;
#else
	UINT16 frac_val;
	UINT16 int_val;
#endif
};

struct _delay8
{
#ifdef BIG_ENDIAN
	UINT8 b3;
	UINT8 b2;
	UINT8 b1;
	UINT8 b0;
#else
	UINT8 b0;
	UINT8 b1;
	UINT8 b2;
	UINT8 b3;
#endif
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

char 	ftrdbyte (void);
void 	ftwrbyte (char ch);
void 	usbfifo_getcomspd (char *buf, int len); 
int 	usbfifo_gethostbps (void);
void 	usbfifo_getid (char *buf, int len);
int 	usbfifo_open (void);


#ifdef __cplusplus
    }
#endif

#endif  /* ndef __usbfifo_H__ */
