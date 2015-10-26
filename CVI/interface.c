//==============================================================================
//
// Title:		interface.c
// Purpose:		Definitions of all UIR callback functions utilized in ZTDR
//				Contains largely structural code; most execution in main.c
//
// Copyright:	(c) 2015, HYPERLABS INC. All rights reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include "constants.h"
#include "interface.h"
#include "main.h"
#include "shared.h"
#include "ZTDR_2XX.h"

//==============================================================================
// Externs


//==============================================================================
// Global variables


//==============================================================================
// Callback functions from controls (sorted alphabetically)









// Acquire waveform manually
int CVICALLBACK onAcquire (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			acquire (1);

			break;
		}

		case EVENT_RIGHT_CLICK:
		{
			break;
		}
	}

	return 0;
}



// Waveform averaging changed
int CVICALLBACK onChangeAverage (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			acquire (1);

			break;
		}

		case EVENT_RIGHT_CLICK:
		{
			break;
		}
	}

	return 0;
}

// Dielectric (K) changed
int CVICALLBACK onChangeK (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			changeDiel ();

			setupTimescale ();

			acquire (1);

			break;
		}

		case EVENT_RIGHT_CLICK:
		{
			break;
		}

	}

	return 0;
}

// Window start changed
int CVICALLBACK onChangeStart (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			resizeWindow ();

			setupTimescale ();

			acquire (1);

			break;
		}

		case EVENT_RIGHT_CLICK:
		{
			break;
		}
	}

	return 0;
}

// Window width changed
int CVICALLBACK onChangeEnd (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			resizeWindow ();

			setupTimescale ();

			acquire (1);

			break;
		}

		case EVENT_RIGHT_CLICK:
		{
			break;
		}
	}

	return 0;
}

// Reset UI to clear recalled waveform settings
int CVICALLBACK onClear (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			clearWaveform ();

			setupTimescale ();

			acquire (1);

			break;
		}

		case EVENT_RIGHT_CLICK:
		{
			break;
		}
	}

	return 0;
}

// Reset to default zoom
int CVICALLBACK onReset (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			resetZoom ();

			resizeWindow ();

			setupTimescale ();

			acquire (1);

			break;
	}
	return 0;
}

// Generic callback that just acquires a new waveform
int CVICALLBACK onGeneric (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			acquire (1);

			break;
		}

		case EVENT_RIGHT_CLICK:
		{
			break;
		}
	}

	return 0;
}

// Basic panel behavior
int CVICALLBACK onPanel (int panel, int event, void *callbackData,
						 int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_PANEL_SIZE:
		{
			updateSize ();
		}

		case EVENT_GOT_FOCUS:
		{
			break;
		}

		case EVENT_LOST_FOCUS:
		{
			break;
		}

		case EVENT_CLOSE:
		{
			// Save program state on exit
			saveSettings (1);
			
			usbfifo_close ();

			QuitUserInterface (0);

			break;
		}
	}

	return 0;
}

// Update cursors on waveform acquisition
int CVICALLBACK onWaveform (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			updateCursors ();

			break;
		}

		case EVENT_VAL_CHANGED:
		{
			break;
		}

		case EVENT_RIGHT_CLICK:
		{
			break;
		}
	}

	return 0;
}

// Zoom on selection
int CVICALLBACK onZoom (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			zoom ();

			resizeWindow ();

			setupTimescale ();

			acquire (1);

			break;

		}

		case EVENT_RIGHT_CLICK:
		{
			break;
		}
	}

	return 0;
}


//==============================================================================
// Callback functions from menu commands (sorted alphabetically)

// Display changed to DOTS
void CVICALLBACK onSetPlotDots (int menuBar, int menuItem, void *callbackData,
								int panel)
{
	int status;

	status = setPlot (0);

	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
				
		// Add to acquisition queue
		asyncAcqCount++;
	}
}

// Display changed to THIN_LINE
void CVICALLBACK onSetPlotThin (int menuBar, int menuItem, void *callbackData,
								int panel)
{
	int status;

	status = setPlot (1);

	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
				
		// Add to acquisition queue
		asyncAcqCount++;
	}
}

// Display changed to FAT_LINE
void CVICALLBACK onSetPlotThick (int menuBar, int menuItem, void *callbackData,
								 int panel)
{
	int status;

	status = setPlot (2);

	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
				
		// Add to acquisition queue
		asyncAcqCount++;
	}
}









// Change to dark background
void CVICALLBACK onChangeBg1 (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	changeBg (0);

	acquire (1);
}

// Change to light background
void CVICALLBACK onChangeBg2 (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	changeBg (1);

	acquire (1);
}

// Horizontal units changed to M
void CVICALLBACK onChangeX1 (int menuBar, int menuItem, void *callbackData,
							 int panel)
{
	changeUnitX (0);

	resizeWindow ();

	setupTimescale ();

	acquire (1);
}

// Horizontal units changed to FT
void CVICALLBACK onChangeX2 (int menuBar, int menuItem, void *callbackData,
							 int panel)
{
	changeUnitX (1);

	resizeWindow ();

	setupTimescale ();

	acquire (1);
}

// Horizontal units changed to NS
void CVICALLBACK onChangeX3 (int menuBar, int menuItem, void *callbackData,
							 int panel)
{
	changeUnitX (2);

	resizeWindow ();

	setupTimescale ();

	acquire (1);
}

// Vertical units changed to mV
void CVICALLBACK onChangeY1 (int menuBar, int menuItem, void *callbackData,
							 int panel)
{
	changeUnitY (0);

	acquire (1);
}

// Vertical units changed to Norm
void CVICALLBACK onChangeY2 (int menuBar, int menuItem, void *callbackData,
							 int panel)
{
	changeUnitY (1);

	acquire (1);
}

// Vertical units changed to Ohm
void CVICALLBACK onChangeY3 (int menuBar, int menuItem, void *callbackData,
							 int panel)
{
	changeUnitY (2);

	acquire (1);
}

// Vertical units changed to Rho
void CVICALLBACK onChangeY4 (int menuBar, int menuItem, void *callbackData,
							 int panel)
{
	changeUnitY (3);

	acquire (1);
}

// Clear stored waveform (from menu)
void CVICALLBACK onClearMenu (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	clearWaveform ();

	setupTimescale ();

	acquire (1);
}

// Save CSV file
void CVICALLBACK onCSV (int menuBar, int menuItem, void *callbackData,
						int panel)
{
	storeWaveform (0);
}

// Save settings
void CVICALLBACK onLoadSettings (int menuBar, int menuItem, void *callbackData,
								 int panel)
{
	loadSettings (0);
	
	acquire (1);
}

// Save CSV and PNG with comments
void CVICALLBACK onMultiSave (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	
}

// Exit program
void CVICALLBACK onExit (int menuBar, int menuItem, void *callbackData,
						 int panel)
{
	// Save program state on exit
	saveSettings (1);
	
	usbfifo_close ();

	QuitUserInterface (0);
}

// Save waveform as PNG
void CVICALLBACK onPNG (int menuBar, int menuItem, void *callbackData,
						int panel)
{
	savePNG ();
}

// Print waveform
void CVICALLBACK onPrint (int menuBar, int menuItem, void *callbackData,
						  int panel)
{
	printWaveform ();
}

// Recall waveform
void CVICALLBACK onRecall (int menuBar, int menuItem, void *callbackData,
						   int panel)
{
	recallWaveform ();
}

// Restore default settings
void CVICALLBACK onResetSettings (int menuBar, int menuItem, void *callbackData,
								  int panel)
{
	resetSettings ();
	
	acquire (1);
}

// Save settings
void CVICALLBACK onSaveSettings (int menuBar, int menuItem, void *callbackData,
								 int panel)
{
	saveSettings (0);
}

// Reset to absolute zero
void CVICALLBACK onResetZero (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	setZero (0.0);
			
	acquire (1);
}

// Set x-axis zero to open
void CVICALLBACK onSetZero (int menuBar, int menuItem, void *callbackData,
							int panel)
{
	setZero (-1.0);
			
	acquire (1);
}

// Store waveform
void CVICALLBACK onStore (int menuBar, int menuItem, void *callbackData,
						  int panel)
{
	storeWaveform (1);
}

// Timebase calibration
void CVICALLBACK onTimeCal (int menuBar, int menuItem, void *callbackData,
							int panel)
{	
	int status;

	// Suspend callbacks
	status = SuspendTimerCallbacks ();

	// Write calibration message
	// writeMsgCal (0);

	int calMsg = calTimebase ();
	// writeMsgCal (calMsg);

	acquire (1);
	
	// Reenable callbacks
	status = ResumeTimerCallbacks ();
}

// Vertical calibration
void CVICALLBACK onVertCal (int menuBar, int menuItem, void *callbackData,
							int panel)
{
	int calStatus = vertCal ();
	
	// writeMsgVertCal (calStatus);

	acquire (1);
}


//==============================================================================
// Debug callback

// Debug button
int CVICALLBACK onDebug (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			/*
			int status;

			
			int numAvg = 4;
			int length = 1024;

			status = setEnviron (0, 0, 0.00, 10.0, 2.25, length);
			status = acquireWaveform (numAvg);

			double wfmDataX[length];
			double wfmDataY[length];

			for (int i = 0; i < length; i++)
			{
				wfmDataX[i] = fetchDataX (i);
				wfmDataY[i] = fetchDataY (i);
			}

			dumpFile ("eggwool.csv");
			*/
		}

		break;
	}
	return 0;
}
