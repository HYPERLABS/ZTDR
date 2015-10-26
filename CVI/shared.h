//==============================================================================
//
// Title:		shared.h
// Purpose:		Shared functionality used across multiple files
//
// Copyright:	(c) 2015, HYPERLABS INC. All rights reserved.
//
//==============================================================================

#ifndef __shared_H__
#define __shared_H__

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"


//==============================================================================
// Global functions

	int		getAutoAcq (void);
	int 	getAutoscale (void);
	int 	getLED (void);
	int		getNumAvg (void);
	int		setAutoAcq (int checked);
	int 	setAutoscale (int checked);
	int 	setLED (int light);
	int		setNumAvg (int num);
	int 	setPlot (int plot);				// no corresponding get
	int 	startTimer (void);
	int	 	stopTimer (char label[16], int log);
	int 	writeMessage (int code, char message[256], int field);
	

#ifdef __cplusplus
}
#endif

#endif  /* ndef __shared_H__ */
