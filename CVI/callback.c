//==============================================================================
//
// Title:		callback.c
// Purpose:		Definitions of all UIR callback functions utilized in ZTDR
//				This file contains only structural code, execution in ZTDR.c
//
// Created on:	7/23/2014 at 10:12:00 AM by Brian Doxey.
// Copyright:	HYPERLABS. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

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
// Global functions (sorted alphabetically, not by function)

// Acquire waveform manually
int CVICALLBACK onAcquire (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			extern acquire ();

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
			extern setupTimescale ();

			extern acquire ();

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
			extern setupTimescale ();

			extern acquire ();

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
			extern changeUnitX ();

			extern setupTimescale ();

			extern acquire ();

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
				extern changeUnitY ();
			
				extern acquire ();

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
			extern usbfifo_close();

			QuitUserInterface(0);

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
			GetCtrlVal (panel, PANEL_CHK_CTSACQUIRE, &i);

			if (i == 1)
			{
				extern acquire ();
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
