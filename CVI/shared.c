//==============================================================================
//
// Title:		shared.c
// Purpose:		Shared functionality used across multiple files
//
// Copyright:	(c) 2015, HYPERLABS INC. All rights reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include <ansi_c.h>
#include <userint.h>

#include "constants.h"
#include "interface.h"
#include "shared.h"

//==============================================================================
// Externs

// User interface
extern	int		panelHandle;
extern	int		menuHandle;
extern	int		plotType;


//==============================================================================
// Global variables

// Event timers
clock_t timerStart, timerEnd;
float timeSpent;


//==============================================================================
// Global functions

// Get whether vertical autoscaling active
int getAutoscale (void)
{
	int status, checked;
	
	status = GetCtrlVal (panelHandle, PANEL_AUTOSCALE, &checked);
	
	return checked;
}

// Set vertical autoscaling
int setAutoscale (int checked)
{
	int status;
	
	status = SetCtrlVal (panelHandle, PANEL_AUTOSCALE, &checked);
	
	// TODO #106: useful return
	return 1;
}

// Get number of acquisitions to average
int getNumAvg (void)
{
	int status, numAvg;
	
	status = GetCtrlVal (panelHandle, PANEL_AVERAGE, &numAvg);
	
	return numAvg;
}

// Set waveform averaging
int setNumAvg (int numAvg)
{
	int status;
	
	status = SetCtrlVal (panelHandle, PANEL_AVERAGE, &numAvg);
	
	// TODO #106: useful return
	return 1;
}

// Change between dots and line
int setPlot (int plot)
{
	int status;																			   
	
	// Change unit selection and update menu
	if (plot == 0)
	{   
		// Dots
		plotType = 2L;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_DOTS, ATTR_CHECKED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_THINLINE, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_FATLINE, ATTR_CHECKED, 0);
	}
	else if (plot == 1)
	{
		// Thin line
		plotType = 0L;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_DOTS, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_THINLINE, ATTR_CHECKED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_FATLINE, ATTR_CHECKED, 0);
	}
	else if (plot == 2)
	{
		// Thick line
		plotType = 5L;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_DOTS, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_THINLINE, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_FATLINE, ATTR_CHECKED, 1);
	}
	
	// TODO #106: useful return
	return 1;
}

// Start event timer
int startTimer (void)
{
	// Label and log used for in-code reference only
	timerStart = clock ();
	
	// TODO #106: useful retunr
	return 1;
}

// End event timer
int stopTimer (char label[16], int log)
{
	int status;
	
	timerEnd = clock ();
	timeSpent = (float) (timerEnd - timerStart) / CLOCKS_PER_SEC;

	// Log to message window if requested
	if (log == 1)
	{
		char msg[64];
		status = sprintf (msg, "%s: %3.3f\n", label, timeSpent);
		
		status = SetCtrlVal (panelHandle, PANEL_MESSAGE, msg);
	}
	
	// TODO #106: useful retunr
	return 1;
}

// Write message to UIR
int writeMessage (int code, char message[256], int field)
{
	int status;
	
	// TODO #140, #141: cleanup routine
	char fullMessage[384];
	
	// Display message output
	if (code == 0)
	{
		// Don't show error if first char is zero 
		status = sprintf (fullMessage, "> %s\n", message);
	}
	else
	{
		// Format to show error message
		status = sprintf (fullMessage, "> ERROR #%d: %s\n", code, message);
	}
	
	// Write to main message log
	if (field == MSG_MAIN)
	{
		status = SetCtrlVal (panelHandle, PANEL_MESSAGE, fullMessage);
	}
	// Don't write anywhere
	else
	{
		// do nothing
	}
	
	// TODO #202: better return
	return 1;
}
