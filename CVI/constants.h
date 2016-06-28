//==============================================================================
//
// Title:		constants.h
// Purpose:		All definitions for ZTDR.
//
// Copyright:	(c) 2015, HYPERLABS INC. All Rights Reserved.
//
//==============================================================================

#ifndef __constants_H__
#define __constants_H__

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"

//==============================================================================
// Constants

// Calibration
#define CALSTART_DEFAULT 540
#define CALEND_DEFAULT 3870
#define CAL_WINDOW_ZERO 0 // ns
#define CAL_WINDOW 50.0e-9  // 10 ns
#define CAL_GUARD 2.0e-9 // 0.5 ns

#define FILTER_WIDTH 4
#define	CAL_RECLEN 1024 // data points for calibration

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
#define M_TO_FT 3.2808
#define FT_TO_M 0.3048
	
// Message output locations
#define MSG_NONE	-1
#define MSG_MAIN	0
	
// Asynchronus timer
#define	ASYNC_NO	-1
#define	ASYNC_YES	0
#define	ASYNC_MSG	1
#define	ASYNC_COND	2
#define ASYNC_TIME	3
	
// Interface background
#define COLOR_DARK	0
#define	COLOR_LIGHT	1

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

#define STD_TIMEOUT 1000
											

#ifdef __cplusplus
}
#endif

#endif  /* ndef __constants_H__ */
