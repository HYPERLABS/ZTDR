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

	void 	acquire (void);
	void 	changeAutoCal (void);
	void 	changeAutoScale (void);
	void 	changeBg (int color);
	void 	changeDiel (void);
	void 	changePlot (int plot);
	void 	changeUnitX (int unit);
	void 	changeUnitY (int unit);
	void 	checkDirs (void);
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
	void	showVersion (void);
	void 	startTimer (char label[16], int log);
	void 	stopTimer (char label[16], int log);
	void	updateCursors (void);
	void	updateSize (void);
	void 	updateTimestamp (void);
	void 	writeMsgCal (int msg);
	void	writeMsgVertCal (int msg);
	void 	zoom (void);


#ifdef __cplusplus
}
#endif

#endif  /* ndef __ZTDR_H__ */
