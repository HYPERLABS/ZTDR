//==============================================================================
//
// Title:		interface.c
// Purpose:		Definitions of all UIR callback functions utilized in ZTDR
//				This file contains only structural code; execution in ZTDR.c
//
// Created on:	7/23/2014 at 10:12:00 AM by Brian Doxey.
// Copyright:	HYPERLABS. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include "ZTDR_1XX.h"
#include "interface.h"
#include "main.h"

//==============================================================================
// Constants

//==============================================================================
// Types

//==============================================================================
// Static global variables

//==============================================================================
// Static functions

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
			acquire ();

			break;
		}
		
		case EVENT_RIGHT_CLICK:
		{		 
			break;
		}
	}
	
	return 0;
}

// Switch between autoscale and manual
int CVICALLBACK onAutoScale (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			changeAutoScale ();
			
			acquire ();
			
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
			
			void acquire ();
			
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
			
			acquire ();
			
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

			acquire ();

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

			acquire ();

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
			
			acquire ();
			
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
			
			acquire ();

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
			acquire ();

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
			usbfifo_close ();
			
			QuitUserInterface (0);
			
			break;
		}
	}
	
	return 0;
}

// Timer-based acquisition, if set to auto-acquire
int CVICALLBACK onTimer (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			int status;
			GetCtrlVal (panel, PANEL_AUTOACQUIRE, &status);

			// Don't run if autoacquire disabled
			if (status == 1)
			{
				acquire ();
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

// Timer-based acquisition, if set to auto-acquire
int CVICALLBACK onTimerCal (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{   
			int status;
			GetCtrlVal (panel, PANEL_AUTOACQUIRE, &status);
			
			// Don't run if autoacquire disabled
			if (status == 1)
			{
				vertCal ();
				
				acquire ();
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

// Update cursors on waveform acquisition
int CVICALLBACK onWaveform (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			break;
		}

		case EVENT_VAL_CHANGED:
		{
			updateCursors ();

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
			
			acquire ();
			
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

// Auto-calibration
void CVICALLBACK onAutoCal (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	changeAutoCal ();
	
	return 0;	
}

// Change to dark background
void CVICALLBACK onChangeBg1 (int menuBar, int menuItem, void *callbackData,
								   int panel)
{
	changeBg (0);

	acquire ();
	
	return 0;	
}

// Change to light background
void CVICALLBACK onChangeBg2 (int menuBar, int menuItem, void *callbackData,
								   int panel)
{
	changeBg (1);

	acquire ();
	
	return 0;	
}

// Display changed to DOTS
void CVICALLBACK onChangePlot1 (int menuBar, int menuItem, void *callbackData,
								   int panel)
{
	changePlot (0);

	acquire ();
	
	return 0;	
}

// Display changed to THIN_LINE
void CVICALLBACK onChangePlot2 (int menuBar, int menuItem, void *callbackData,
								   int panel)
{
	changePlot (1);

	acquire ();
	
	return 0;	
}

// Display changed to FAT_LINE
void CVICALLBACK onChangePlot3 (int menuBar, int menuItem, void *callbackData,
								   int panel)
{
	changePlot (2);

	acquire ();
	
	return 0;	
}

// Horizontal units changed to M
void CVICALLBACK onChangeX1 (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	changeUnitX (0);
	
	resizeWindow ();
	
	setupTimescale ();

	acquire ();
	
	return 0;	
}

// Horizontal units changed to FT
void CVICALLBACK onChangeX2 (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	changeUnitX (1);
	
	resizeWindow ();
	
	setupTimescale ();

	acquire ();
	
	return 0;	
}

// Horizontal units changed to NS
void CVICALLBACK onChangeX3 (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	changeUnitX (2);
	
	resizeWindow ();
	
	setupTimescale ();

	acquire ();
	
	return 0;	
}

// Vertical units changed to mV
void CVICALLBACK onChangeY1 (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	changeUnitY (0);
	
	acquire ();
	
	return 0;	
}

// Vertical units changed to Norm
void CVICALLBACK onChangeY2 (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	changeUnitY (1);
	
	acquire ();
	
	return 0;	
}

// Vertical units changed to Ohm
void CVICALLBACK onChangeY3 (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	changeUnitY (2);
	
	acquire ();
	
	return 0;	
}

// Vertical units changed to Rho
void CVICALLBACK onChangeY4 (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	changeUnitY (3);
	
	acquire ();
	
	return 0;	
}

// Clear stored waveform (from menu)
void CVICALLBACK onClearMenu (int menuBar, int menuItem, void *callbackData,
							  int panel)
{
	clearWaveform ();
			
	setupTimescale ();
			
	acquire ();
	
	return 0;
}

// Save CSV file
void CVICALLBACK onCSV (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	storeWaveform (0);
	
	return 0;	
}

// Exit program
void CVICALLBACK onExit (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	usbfifo_close ();

	QuitUserInterface (0);
	
	return 0;	
}

// Save waveform as PNG
void CVICALLBACK onPNG (int menuBar, int menuItem, void *callbackData,
							 int panel)
{
	savePNG ();
	
	return 0;
}

// Print waveform
void CVICALLBACK onPrint (int menuBar, int menuItem, void *callbackData,
							 int panel)
{
	printWaveform ();
	
	return 0;
}

// Recall waveform
void CVICALLBACK onRecall (int menuBar, int menuItem, void *callbackData,
							 int panel)
{
	recallWaveform ();
	
	return 0;
}

// Store waveform
void CVICALLBACK onStore (int menuBar, int menuItem, void *callbackData,
							 int panel)
{
	storeWaveform (1);
	
	return 0;
}

// Timebase calibration
void CVICALLBACK onTimeCal (int menuBar, int menuItem, void *callbackData,
							 int panel)
{  
	writeMsgCal (0);
	
	calTimebase ();
	
	acquire ();
	
	return 0;
}

// Vertical calibration
void CVICALLBACK onVertCal (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	vertCal ();
	
	acquire ();
	
	return 0;	
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
				int status;
				
				int numAvg;
				
				status = GetCtrlVal (panel, PANEL_AVERAGE, &numAvg);
				
				status = acquireWaveform (numAvg);
				
				int egg = 1;
			}
			
			break;
	}
	return 0;
}
