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

extern	int	panelHandle;

//==============================================================================
// Global variables

// Event timers
clock_t timerStart, timerEnd;
float timeSpent;


//==============================================================================
// Global functions

// Start event timer
int startTimer (char label[16], int log)
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
