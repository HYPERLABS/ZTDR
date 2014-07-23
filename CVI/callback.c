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

// Horizontal units changed
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
