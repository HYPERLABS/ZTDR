//==============================================================================
//
// Title:		interface.c
// Purpose:		Definitions of all UIR callback functions utilized in ZTDR
//				Contains largely structural code; most execution in main.c
//
// Copyright:	(c) 2018, HYPERLABS INC. All rights reserved.
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

			// Stop timers before exit
			status = SuspendAsyncTimerCallbacks ();
			
			// Don't interrupt calibration/acquisition
			while (getLED () == 1 || timerLock == 1)
			{
				// do nothing	
			}
			
			status = setDiel (-1);

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

// Acquisition window changed
int CVICALLBACK onCheckMinMax (int panel, int control, int event,
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
			
			status = checkMinMax ();

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
			
			status = ZTDR_QuantizeTimescale ();

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
			
			break;
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
			
			ZTDR_CloseDevice ();
			
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

			status = ZTDR_QuantizeTimescale ();

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

			status = ZTDR_QuantizeTimescale ();

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

// Vertical calibration
void CVICALLBACK onCalibrate (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	// Force recalibration
	asyncCal = ASYNC_MSG;
				
	// Add to acquisition queue
	asyncAcqCount++;
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

// Clear stored waveform (from menu)
void CVICALLBACK onClearWaveform (int menuBar, int menuItem, void *callbackData,
								  int panel)
{
	int status;

	status = clearWaveform ();

	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;

		// Add to acquisition queue
		asyncAcqCount++;
	}
}

// Save CSV file
void CVICALLBACK onCSV (int menuBar, int menuItem, void *callbackData,
						int panel)
{
	int status;

	// Stop timers before action
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = storeWaveform (0);
	
	// Resume timers
	status = ResumeAsyncTimerCallbacks ();
	
	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
				
		// Add to acquisition queue
		asyncAcqCount++;
	}
}

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

	ZTDR_CloseDevice ();

	QuitUserInterface (0);
}

// Save settings
void CVICALLBACK onLoadSettings (int menuBar, int menuItem, void *callbackData,
								 int panel)
{
	int status;

	// Stop timers before action
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = loadSettings (0);
	
	// Resume timers
	status = ResumeAsyncTimerCallbacks ();
	
	// Force recalibration
	asyncCal = ASYNC_YES;
				
	// Add to acquisition queue
	asyncAcqCount++;
}

// Recall waveform
void CVICALLBACK onRecallWaveform (int menuBar, int menuItem, void *callbackData,
								   int panel)
{
	int status;

	// Stop timers before action
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = recallWaveform ();
	
	// Resume timers
	status = ResumeAsyncTimerCallbacks ();
	
	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
				
		// Add to acquisition queue
		asyncAcqCount++;
	}
}

// Restore default settings
void CVICALLBACK onResetSettings (int menuBar, int menuItem, void *callbackData,
								  int panel)
{
	int status;

	// Stop timers before action
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = resetSettings ();
	
	// Resume timers
	status = ResumeAsyncTimerCallbacks ();
	
	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
				
		// Add to acquisition queue
		asyncAcqCount++;
	}
}

// Save settings
void CVICALLBACK onSaveSettings (int menuBar, int menuItem, void *callbackData,
								 int panel)
{
	int status;

	// Stop timers before action
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = saveSettings (0);
	
	// Resume timers
	status = ResumeAsyncTimerCallbacks ();
	
	// Force recalibration
	asyncCal = ASYNC_YES;
				
	// Add to acquisition queue
	asyncAcqCount++;
}

// Set autoscale mode
int CVICALLBACK onSetAutoscale (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int status;

			status = setAutoscale (-1);

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

	status = ZTDR_QuantizeTimescale ();

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

	status = ZTDR_QuantizeTimescale ();

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

	status = ZTDR_QuantizeTimescale ();

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

// Print waveform
void CVICALLBACK onPrint (int menuBar, int menuItem, void *callbackData,
						  int panel)
{
	int status;
	
	// Stop timers before action
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}

	status = printWaveform ();
	
	// Resume timers
	status = ResumeAsyncTimerCallbacks ();
}

// Save waveform as PNG
void CVICALLBACK onPNG (int menuBar, int menuItem, void *callbackData,
						int panel)
{
	int status;

	// Stop timers before action
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = savePNG ();
	
	// Resume timers
	status = ResumeAsyncTimerCallbacks ();
}

// Store waveform
void CVICALLBACK onStoreWaveform (int menuBar, int menuItem, void *callbackData,
								  int panel)
{
	int status;

	// Stop timers before action
	status = SuspendAsyncTimerCallbacks ();
	
	// Don't interrupt calibration/acquisition
	while (getLED () == 1 || timerLock == 1)
	{
		// do nothing	
	}
	
	status = storeWaveform (1);
	
	// Resume timers
	status = ResumeAsyncTimerCallbacks ();
	
	if (getAutoAcq () != 1)
	{
		// Conditional calibrations
		asyncCal = ASYNC_COND;
				
		// Add to acquisition queue
		asyncAcqCount++;
	}
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
			status = ZTDR_AcquireData (numAvg);

			double wfmDataX[length];
			double wfmDataY[length];

			for (int i = 0; i < length; i++)
			{
				wfmDataX[i] = ZTDR_FetchDataX (i);
				wfmDataY[i] = ZTDR_FetchDataY (i);
			}

			dumpFile ("eggwool.csv");
			*/
		}

		break;
	}
	return 0;
}
