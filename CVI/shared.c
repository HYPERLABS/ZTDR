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

 // Acquisition environment
extern	int		xUnits;
extern	double	xStart;
extern	double	xEnd;
extern	double	xZero;
extern	double	dielK;

// User interface
extern	int		panelHandle;
extern	int		menuHandle;
extern	int		plotType;


//==============================================================================
// Global variables

// Event timers
clock_t timerStart, timerEnd;
float timeSpent;

static double minWidth[] =
{
	1.0,
	3.0,
	5.0
};

//==============================================================================
// Global functions

// Get auto acquisition mode
int getAutoAcq (void)
{
	int status, checked;
	
	status = GetCtrlVal (panelHandle, PANEL_AUTOACQUIRE, &checked);
	
	return checked;
}

// Set auto acquisition mode
int setAutoAcq (int checked)
{
	int status;
	
	status = SetCtrlVal (panelHandle, PANEL_AUTOACQUIRE, checked);
	
	// TODO #106: useful return
	return 1;
}

// Get autoscale setting
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
	
	status = SetCtrlVal (panelHandle, PANEL_AUTOSCALE, checked);
	
	// TODO #106: useful return
	return 1;
}

// Get color scheme
int getBg (void)
{
	int status;
	
	int color;
	status = GetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_DARK, ATTR_CHECKED, &color);
	
	if (color == 1)
	{
		return COLOR_DARK;
	}
	else
	{
		return COLOR_LIGHT;
	}
}

// Change color scheme
int setBg (int color)
{
	int status;
	
	// Set default dark background
	if (color == COLOR_DARK)
	{ 
		// Set background and grid color
		status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_PLOT_BGCOLOR, MakeColor (32, 32, 32));
		status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_GRID_COLOR, MakeColor (80, 80, 80));
		status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_GRAPH_BGCOLOR, VAL_WHITE);

		// More visible cursors
		status = SetCursorAttribute (panelHandle, PANEL_WAVEFORM, 1, ATTR_CURSOR_COLOR, MakeColor (83, 200, 233));
		status = SetCursorAttribute (panelHandle, PANEL_WAVEFORM, 2, ATTR_CURSOR_COLOR, MakeColor (233, 83, 83));
		
		// Timestamp
		status = SetCtrlAttribute (panelHandle, PANEL_TIMESTAMP, ATTR_TEXT_COLOR , MakeColor (76, 157, 47));
		
		// Update checkmarks
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_DARK, ATTR_CHECKED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_LIGHT, ATTR_CHECKED, 0);	
	}
	
	// Set alternate light background
	else if (color == COLOR_LIGHT)
	{ 
		// Set background and grid color
		status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_PLOT_BGCOLOR, VAL_WHITE);
		status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_GRID_COLOR, VAL_DK_GRAY);
		status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_GRAPH_BGCOLOR, VAL_LT_GRAY);

		// More visible cursors
		status = SetCursorAttribute (panelHandle, PANEL_WAVEFORM, 1, ATTR_CURSOR_COLOR, VAL_BLUE);
		status = SetCursorAttribute (panelHandle, PANEL_WAVEFORM, 2, ATTR_CURSOR_COLOR, VAL_RED);
		
		// Timestamp
		status = SetCtrlAttribute (panelHandle, PANEL_TIMESTAMP, ATTR_TEXT_COLOR , VAL_BLACK);
		
		// Update checkmarks
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_DARK, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_LIGHT, ATTR_CHECKED, 1);
	}
	
	// TODO #106: useful return
	return 1;
}

// Get value of dielectric
double getDiel (void)
{
	int status;
	double k;
	
	status = GetCtrlVal (panelHandle, PANEL_DIEL, &k);
	
	return k;
}

// Update values of dielectric, VC
int setDiel (double k)
{
	int status;
	
	// Derive from UIR field
	if (k < 0)
	{
		dielK = getDiel ();
	}
	// Set to specific value
	else
	{
		dielK = k;
		
		status = SetCtrlVal (panelHandle, PANEL_DIEL, dielK);
	}
	
	// TODO #202: useful return
	return 1;
}

// Get LED status
int getLED (void)
{
	int status;
	
	int light;
	status = GetCtrlVal (panelHandle, PANEL_LED, &light);
	
	return light;
}

// Set LED illumination
int setLED (int light)
{
	int status;
	
	status = SetCtrlVal (panelHandle, PANEL_LED, light);
	
	// TODO #352: useful return
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

// Verify size and zero of acquisition window
int setWindow (void)
{
	int status;
	
	double x1 = getXStart ();
	double x2 = getXEnd ();
	
	// Verify start is less than end
	if (x1 > x2)
	{
		double xTmp = x2;
		x2 = x1;
		x1 = xTmp;
	}
	
	// Verify window is wide enough
	if ((x1 + minWidth[xUnits]) < x2)
	{
		// Pass absolute values
		status = setXStart (x1 + xZero);
		status = setXEnd (x2 + xZero);
	}
	// Adjustment if end less than start
	else
	{
		// Retrieve proper adjustment amount
		double adjust = minWidth[xUnits];
		
		status = setXStart (x1 + xZero);
		
		x2 = x1 + adjust;
		status = setXEnd (x2 + xZero);
	}
	
	// TODO #106: useful return
	return 1;
}

// Get horizontal window end
double getXEnd (void)
{
	int status;

	double x;
	status = GetCtrlVal (panelHandle, PANEL_END, &x);

	return x;
}

// Get horizontal window start
double getXStart (void)
{
	int status;

	double x;
	status = GetCtrlVal (panelHandle, PANEL_START, &x);

	return x;
}

// Set horizontal window end
int setXEnd (double x)
{
	int status;

	xEnd = x;
	status = SetCtrlVal (panelHandle, PANEL_END, x - xZero);

	// TODO #202: useful return
	return 1;
}

// Set horizontal window start
int setXStart (double x)
{
	int status;

	xStart = x;
	status = SetCtrlVal (panelHandle, PANEL_START, x - xZero);

	// TODO #202: useful return
	return 1;
}

// Get vertical min
double getYMin (void)
{
	int status;

	double y;
	status = GetCtrlVal (panelHandle, PANEL_YMIN, &y);

	return y;
}

// Get vertical max
double getYMax (void)
{
	int status;

	double y;
	status = GetCtrlVal (panelHandle, PANEL_YMAX, &y);

	return y;
}

// Set vertical min
int setYMin (double y)
{
	int status;
			  ;
	status = SetCtrlVal (panelHandle, PANEL_YMIN, y);

	// TODO #106: useful return
	return 1;
}

// Set vertical max
int setYMax (double y)
{
	int status;
			  ;
	status = SetCtrlVal (panelHandle, PANEL_YMAX, y);

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
