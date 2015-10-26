//==============================================================================
//
// Title:		main.h
// Purpose:		Main ZTDR functionality for LabWindows/CVI
//
// Copyright:	(c) 2015, HYPERLABS INC. All rights reserved.
//
//==============================================================================

#ifndef __main_H__
#define __main_H__

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"


//==============================================================================
// Global functions

	int 	acquire (int doDraw);
	int 	calibrate (int showMsg);
	double 	checkCalTime (void);
	int 	checkDirs (void);
	int 	setCalTime (void);
	int		showVersion (void);
	int		updateCursors (void);
	int 	updateTimestamp (void);
	
	int 	CVICALLBACK onAsyncTimer (int reserved, int timerId, int event, void *callbackData, int eventData1, int eventData2);




	void 	changeBg (int color);
	void 	changeUnitX (int unit);
	void 	changeUnitY (int unit);
	void	clearWaveform (void);
	int 	loadSettings (int isAuto);
	void	main (int argc, char *argv[]);
	void	printWaveform (void);
	void 	recallWaveform (void);
	void 	resetSettings (void);
	void	resetZoom (void);
	void 	resizeWindow (void);
	void 	savePNG (void);
	void 	saveSettings (int isAuto);
	void	setZero (double x);
	void 	storeWaveform (int format);
	void	updateSize (void);
	void 	zoom (void);


#ifdef __cplusplus
}
#endif

#endif  /* ndef __ZTDR_H__ */
