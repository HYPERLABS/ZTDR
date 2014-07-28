//==============================================================================
//
// Title:		usbfifo.h
// Purpose:		Declaration of functions used by usbfifo.c
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
int 	usbfifo_acquire (UINT8 *ret_val, UINT8 arg);
void 	usbfifo_close (void);
void 	usbfifo_getcomspd (char *buf, int len);
int 	usbfifo_gethostbps (void);
void 	usbfifo_getid (char *buf, int len);
int 	usbfifo_open (void);
int 	usbfifo_readblock (UINT8 block_no, UINT16 *buf);
int 	usbfifo_setparams (UINT8 freerun_en, UINT16 calstart, UINT16 calend, timeinf tmstart, timeinf tmend, UINT16 stepcount,
						   UINT16 strobecount, UINT8 noversample, UINT16 record_len, UINT16 dac0, UINT16 dac1, UINT16 dac2);


#ifdef __cplusplus
}
#endif

#endif  /* ndef __usbfifo_H__ */
