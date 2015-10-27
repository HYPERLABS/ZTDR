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

#include "asynctmr.h"
#include "constants.h"
#include "interface.h"
#include "main.h"
#include "shared.h"
#include "ZTDR_2XX.h"

//==============================================================================
// Externs

// Asynchronous timers
extern	int	asyncAcqCount;
extern	int	asyncCal;
extern	int	timerLock;


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
			if (getAutoAcq () != 1)
			{
				// Conditional calibration
				asyncCal = ASYNC_COND;
				
				// Add to acquisition queue
				asyncAcqCount++;
			}

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
int CVICALLBACK onChangeDiel (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int status;

			status = setDiel (-1);

			if (getAutoAcq () != 1)
			{
				// Conditional calibrations
				asyncCal = ASYNC_COND;
				
				// Add to acquisition queue
				asyncAcqCount++;
			}

			break;
		}

		case EVENT_RIGHT_CLICK:
		{
			break;
		}

	}

	return 0;
}

// Acquisition window changed
int CVICALLBACK onChangeWindow (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			int status;

			// Stop timers before exit
			status = SuspendAsyncTimerCallbacks ();
			
			// Don't interrupt calibration/acquisition
			while (getLED () == 1 || timerLock == 1)
			{
				// do nothing	
			}
			
			status = setWindow ();
			
			setupTimescale ();

			if (getAutoAcq () != 1)
			{
				// Conditional calibrations
				asyncCal = ASYNC_COND;
				
				// Add to acquisition queue
				asyncAcqCount++;
			}
			
			status = ResumeAsyncTimerCallbacks ();

			break;
		}

		case EVENT_RIGHT_CLICK:
		{
			break;
		}
	}

	return 0;
}

// Generic callback to acquire new waveform
int CVICALLBACK onGeneric (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			if (getAutoAcq () != 1)
			{
				// Conditional calibration
				asyncCal = ASYNC_COND;
				
				// Add to acquisition queue
				asyncAcqCount++;
			}

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
			int status;
			
			status = updateWindowSize ();
		}

		case EVENT_CLOSE:
		{
			int status;
			
			// Stop timers before exit
			status = SuspendAsyncTimerCallbacks ();
			
			// Don't interrupt calibration/acquisition
			while (getLED () == 1 || timerLock == 1)
			{
				// do nothing	
			}
			
			// Save program state on exit
			status = saveSettings (1);
			
			usbfifo_close ();
			
			status = QuitUserInterface (0);

			break;
		}
	}

	return 0;
}

// Reset to default zoom
int CVICALLBACK onResetZoom (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int status;

			// Stop timers before exit
			status = SuspendAsyncTimerCallbacks ();
			
			// Don't interrupt calibration/acquisition
			while (getLED () == 1 || timerLock == 1)
			{
				// do nothing	
			}
			
			status = resetZoom ();

			setupTimescale ();

			if (getAutoAcq () != 1)
			{
				// Conditional calibrations
				asyncCal = ASYNC_COND;
				
				// Add to acquisition queue
				asyncAcqCount++;
			}
			
			status = ResumeAsyncTimerCallbacks ();

			break;
		}

		case EVENT_RIGHT_CLICK:
		{
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
			int status;

			status = updateCursors ();

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
			int status;

			// Stop timers before exit
			status = SuspendAsyncTimerCallbacks ();
			
			// Don't interrupt calibration/acquisition
			while (getLED () == 1 || timerLock == 1)
			{
				// do nothing	
			}
			
			status = zoom ();

			setupTimescale ();

			if (getAutoAcq () != 1)
			{
				// Conditional calibrations
				asyncCal = ASYNC_COND;
				
				// Add to acquisition queue
				asyncAcqCount++;
			}
			
			status = ResumeAsyncTimerCallbacks ();

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

// Exit program
void CVICALLBACK onExit (int menuBar, int menuItem, void *callbackData,
						 int panel)
{
	int status;

	// Stop timers before exit
	status = SuspendAsyncTimerCallbacks ();

	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing
	}

	// Save program state on exit
	status = saveSettings (1);

	usbfifo_close ();

	QuitUserInterface (0);
}

// Change to dark background
void CVICALLBACK onSetBgDark (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	int status;

	status = setBg (COLOR_DARK);
}

// Change to light background
void CVICALLBACK onSetBgLight (int menuBar, int menuItem, void *callbackData,
							   int panel)
{
	int status;

	status = setBg (COLOR_LIGHT);
}

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

// Horizontal units changed to M
void CVICALLBACK onSetXM (int menuBar, int menuItem, void *callbackData,
						  int panel)
{
	int status;

	// Stop timers before exit
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = setUnitX (UNIT_M);

	setupTimescale ();

	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
		
		// Add to acquisition queue
		asyncAcqCount++;
	}
	
	status = ResumeAsyncTimerCallbacks ();
}

// Horizontal units changed to FT
void CVICALLBACK onSetXFt (int menuBar, int menuItem, void *callbackData,
						   int panel)
{
	int status;

	// Stop timers before exit
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = setUnitX (UNIT_FT);

	setupTimescale ();

	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
		
		// Add to acquisition queue
		asyncAcqCount++;
	}
	
	status = ResumeAsyncTimerCallbacks ();
}

// Horizontal units changed to NS
void CVICALLBACK onSetXNs (int menuBar, int menuItem, void *callbackData,
						   int panel)
{
	int status;

	// Stop timers before exit
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = setUnitX (UNIT_NS);

	setupTimescale ();

	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
		
		// Add to acquisition queue
		asyncAcqCount++;
	}
	
	status = ResumeAsyncTimerCallbacks ();
}

// Vertical units changed to mV
void CVICALLBACK onSetYMV (int menuBar, int menuItem, void *callbackData,
						   int panel)
{
	int status;

	// Stop timers before exit
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = setUnitY (UNIT_MV);

	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
		
		// Add to acquisition queue
		asyncAcqCount++;
	}
	
	status = ResumeAsyncTimerCallbacks ();
}

// Vertical units changed to Norm
void CVICALLBACK onSetYNorm (int menuBar, int menuItem, void *callbackData,
							 int panel)
{
	int status;

	// Stop timers before exit
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = setUnitY (UNIT_NORM);

	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
		
		// Add to acquisition queue
		asyncAcqCount++;
	}
	
	status = ResumeAsyncTimerCallbacks ();
}

// Vertical units changed to Ohm
void CVICALLBACK onSetYOhm (int menuBar, int menuItem, void *callbackData,
							int panel)
{
	int status;

	// Stop timers before exit
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = setUnitY (UNIT_OHM);

	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
		
		// Add to acquisition queue
		asyncAcqCount++;
	}
	
	status = ResumeAsyncTimerCallbacks ();
}

// Vertical units changed to Rho
void CVICALLBACK onSetYRho (int menuBar, int menuItem, void *callbackData,
							int panel)
{
	int status;

	// Stop timers before exit
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = setUnitY (UNIT_RHO);

	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
		
		// Add to acquisition queue
		asyncAcqCount++;
	}
	
	status = ResumeAsyncTimerCallbacks ();
}

// Reset to absolute zero
void CVICALLBACK onResetZero (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	int status;
	
	// Stop timers before exit
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = setZero (0.0);
	
	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
		
		// Add to acquisition queue
		asyncAcqCount++;
	}
	
	status = ResumeAsyncTimerCallbacks ();
}

// Set x-axis zero to open
void CVICALLBACK onSetZero (int menuBar, int menuItem, void *callbackData,
							int panel)
{
	int status;
	
	// Stop timers before exit
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = setZero (-1.0);

	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
		
		// Add to acquisition queue
		asyncAcqCount++;
	}
	
	status = ResumeAsyncTimerCallbacks ();
}

// Timebase calibration
void CVICALLBACK onCalTimebase (int menuBar, int menuItem, void *callbackData,
								int panel)
{	
	// Conditional calibrations
	asyncCal = ASYNC_TIME;
		
	// Add to acquisition queue
	asyncAcqCount++;
}

// Vertical calibration
void CVICALLBACK onCalibrate (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	// Force recalibration
	asyncCal = ASYNC_MSG;
				
	// Add to acquisition queue
	asyncAcqCount++;
}







// Recall waveform
void CVICALLBACK onRecall (int menuBar, int menuItem, void *callbackData,
						   int panel)
{
	recallWaveform ();
}

// Clear stored waveform (from menu)
void CVICALLBACK onClearMenu (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	clearWaveform ();

	setupTimescale ();

	acquire (1);
}

// Store waveform
void CVICALLBACK onStore (int menuBar, int menuItem, void *callbackData,
						  int panel)
{
	storeWaveform (1);
}




// Print waveform
void CVICALLBACK onPrint (int menuBar, int menuItem, void *callbackData,
						  int panel)
{
	printWaveform ();
}

// Save CSV file
void CVICALLBACK onCSV (int menuBar, int menuItem, void *callbackData,
						int panel)
{
	storeWaveform (0);
}

// Save CSV and PNG with comments
void CVICALLBACK onMultiSave (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	
}

// Save waveform as PNG
void CVICALLBACK onPNG (int menuBar, int menuItem, void *callbackData,
						int panel)
{
	savePNG ();
}


 

// Save settings
void CVICALLBACK onLoadSettings (int menuBar, int menuItem, void *callbackData,
								 int panel)
{
	loadSettings (0);
	
	acquire (1);
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
