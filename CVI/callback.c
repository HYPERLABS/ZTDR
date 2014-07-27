//==============================================================================
//
// Title:		callback.c
// Purpose:		Definitions of all UIR callback functions utilized in ZTDR
//				This file contains only structural code; execution in ZTDR.c
//
// Created on:	7/23/2014 at 10:12:00 AM by Brian Doxey.
// Copyright:	HYPERLABS. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include <ansi_c.h>
#include "shared.h"
#include "usbfifo.h"
#include "ZTDR.h"

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

extern int panelHandle;

//==============================================================================
// Global functions (sorted alphabetically, not by functionality)

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
int CVICALLBACK onAuto (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			setAuto ();
			
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

// Full time base calibration
int CVICALLBACK onCal (int panel, int control, int event,
					   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			calTimebase ();

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
int CVICALLBACK onChangeWindow (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
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

// Horizontal units changed
int CVICALLBACK onChangeUnitX (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			changeUnitX ();

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

// Vertical units changed
int CVICALLBACK onChangeUnitY (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			{
				changeUnitY ();
			
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
			int width, height;
			
			GetPanelAttribute(panel, ATTR_WIDTH, &width);
			GetPanelAttribute(panel, ATTR_HEIGHT, &height);
			
			SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_WIDTH, width - 100);
			SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_HEIGHT, height - 100);
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

// Print panel and waveform
int CVICALLBACK onPNG (int panel, int control, int event,
					   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
		{
			savePNG ();
			
			break;
		}
			
		case EVENT_RIGHT_CLICK:
		{
			break;
		}
	}
	return 0;
}

// Print panel and waveform
int CVICALLBACK onPrint (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
		{
			printWaveform ();
			
			break;
		}
			
		case EVENT_RIGHT_CLICK:
		{
			break;
		}
	}
	return 0;
}

// Close UI and exit
int CVICALLBACK onQuit (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			usbfifo_close ();

			QuitUserInterface (0);

			break;
		}

		case EVENT_RIGHT_CLICK:
		{

			break;
		}

	}
	
	return 0;
}

// Recall stored waveform
int CVICALLBACK onRecall (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
		{ 	
			recallWaveform ();
			
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
int CVICALLBACK onReset (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			resetWaveform ();
			
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

// Store waveform
int CVICALLBACK onStore (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{  	
			storeWaveform (1);
			
			break;
		}
		
		case EVENT_RIGHT_CLICK:
		{   	
			break;
		}
	}
	
	return 0;
}

// Store waveform
int CVICALLBACK onStoreCSV (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{  	
			storeWaveform (0);
			
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
int CVICALLBACK onTimer (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			int i;
			GetCtrlVal (panel, PANEL_CONTINUOUS, &i);

			if (i == 1)
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

// Vertical calibration only
int CVICALLBACK onVertCal (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			vertCal ();
			
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

// Zoom on selection /* TO DO */
int CVICALLBACK onZoom (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			zoom ();
			
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



// TO DO: Organize menu based commands
// Horizontal units changed to M
void CVICALLBACK onChangeX1 (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	changeX (1);
	
	setupTimescale ();

	acquire ();
	
	return 0;	
}

// Horizontal units changed to FT
void CVICALLBACK onChangeX2 (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	changeX (1);
	
	setupTimescale ();

	acquire ();
	
	return 0;	
}

// Horizontal units changed to NS
void CVICALLBACK onChangeX3 (int menuBar, int menuItem, void *callbackData, 
							int panel)
{
	changeX (2);
	
	setupTimescale ();

	acquire ();
	
	return 0;	
}
