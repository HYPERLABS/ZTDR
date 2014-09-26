//==============================================================================
//
// Title:		main.c
// Purpose:		Main ZTDR functionality for LabWindows/CVI
//
// Copyright:	(c) 2014, HYPERLABS INC. All rights reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include <windows.h>
#include "toolbox.h"
#include <formatio.h>
#include <userint.h>

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
// External global variables (roughly grouped by function)

// Initialization
extern	int 	usb_opened;

// Calibration
extern 	int 	calIncrement;
extern	double	vampl;

// Acquisition environment
extern	double	dielK; // coax
extern	int 	yUnits; // mV
extern	int 	xUnits; // m
extern	double	xStart; // m
extern	double	xEnd; // m

// Number of data points acquired
extern	UINT16 	recLen;

// Waveform storage
extern	double 	wfmDistFt[]; // distance (ft)
extern	double 	wfmDistM[]; // distance (m)
extern	double 	wfmTime[]; // time (ns)
extern	double 	wfmX[]; // converted to selected units

extern	UINT16 	wfm[]; // raw data from device
extern	double 	wfmFilter[];	// filtered data from device
extern	double  wfmDataRaw[]; // raw unconverted data for debug
extern	double  wfmData[]; // converted to selected units
extern	double	wfmAvg[]; // waveform after averaging

// Start/end time for device
extern	timeinf start_tm, end_tm;


//==============================================================================
// Global variables (roughly grouped by function)

// TODO: better names for these

// Unit labels and ranges
char *labelY[] =
{
	"VOLTS (mV)",
	"NORMALIZED",
	"IMPEDANCE (Ohm)",
	"REFLECT COEFF (Rho)"
};

char *labelX[] =
{
	"ROUNDTRIP (m)",
	"ROUNDTRIP (ft)",
	"ROUNDTRIP (ns)"
};

char *shortY[] =
{
	"mV",
	"Norm",
	"Ohm",
	"Rho"
};

char *shortX[] =
{
	"m",
	"ft",
	"ns"
};

char *labelStartX[] =
{
	"START (m)",
	"START (ft)",
	"START (ns)"
};

char *labelEndX[] =
{
	"END (m)",
	"END (ft)",
	"END (ns)"
};

double defaultStart[] =
{
	0.0,
	0.0,
	0.0
};

double defaultEnd[]  =
{
	10.0,
	33.3,
	50.0
};

double maxRange[] =
{
	400.0,
	1332.0,
	2000.0
};

double minWidth[] =
{
	1.0,
	3.0,
	5.0
};

char *monthName[] =
{
	"JAN",
	"FEB",
	"MAR",
	"APR",
	"MAY",
	"JUN",
	"JUL",
	"AUG",
	"SEP",
	"OCT",
	"NOV",
	"DEC"
};

// Horizontal values for each unit
double wfmRecallX[NPOINTS_MAX]; 	// Recalled waveform

// Vertical values in different modes
double wfmRecall[NPOINTS_MAX]; 	// Recalled waveform

// Default plot type
int	plotType = 2L; // dots

// Waveform handles
int WfmActive; 	// current acquisition
int WfmStored;	// stored waveform

// UIR elements
int panelHandle, menuHandle;
int	rightHandle, bottomHandle;

// Panel size
int	windowWidth, windowHeight;

// Save to default directories
int defaultSaveCSV = 1;
int defaultSavePNG = 1;
int defaultSaveZTDR = 1;


//==============================================================================
// Global functions (roughly grouped by functionality, order of call)

// Main startup function
void main (int argc, char *argv[])
{
	int status;

	// Verify CVIRTE is running
	status = InitCVIRTE (0, argv, 0);
	
	// Load UI
	panelHandle = LoadPanel (0, "interface.uir", PANEL);
	
	// Load menu bar
	menuHandle = LoadMenuBar (panelHandle, "interface.uir", MENUBAR);
	
	// Load control arrays
	rightHandle = GetCtrlArrayFromResourceID (panelHandle, RIGHT);
	bottomHandle = GetCtrlArrayFromResourceID (panelHandle, BOTTOM);
	
	// Display panel and store size
	status = DisplayPanel (panelHandle);
	status = GetPanelAttribute(panelHandle, ATTR_WIDTH, &windowWidth);
	status = GetPanelAttribute(panelHandle, ATTR_HEIGHT, &windowHeight);
																	 
	// Make sure relevant output directories exist
	checkDirs ();
	
	// Show software version
	showVersion ();
	
	// Show startup message
	writeMsgCal (0);
	
	// Peform unified initialization and calibration
	int calStatus = initDevice ();
	
	// Indicate cal status
	writeMsgCal (calStatus);
	
	if (calStatus == 1)
	{ 
		// Run first acquisition
		acquire ();
	
		// Set initial cursor positions
		SetGraphCursor (panelHandle, PANEL_WAVEFORM, 1, 2.25, -250);
		SetGraphCursor (panelHandle, PANEL_WAVEFORM, 2, 3.25, 0);
	
		// Start event timers
		status = SetCtrlAttribute (panelHandle, PANEL_TIMER, ATTR_ENABLED, 1);
		status = SetCtrlAttribute (panelHandle, PANEL_CALTIMER, ATTR_ENABLED, 1);
	}
	else if (calStatus == -1)
	{
		// Instrument not connected or initial calibration failed
		status = MessagePopup ("Error", "Could not initialize the TDR device.");
		
		QuitUserInterface (0);
	}
	
	RunUserInterface ();	
	
	DiscardPanel (panelHandle);
}

// Main acquisition function
void acquire (void)
{
	int status;

	// Number of waveforms to average
	int numAvg;
	GetCtrlVal (panelHandle, PANEL_AVERAGE, &numAvg);
	
	// Call unified acquisition function
	acquireWaveform (numAvg);
	
	// Min/max of averaged waveform
	double ymax = 0.0;
	double ymin = 0.0;
	
	// Y Axis scaling based on selected unit
	switch (yUnits)
	{   
		case UNIT_MV:
		{   
			// Values certain to be overwritten immediately
			ymax = -500;
			ymin = 500;
			
			for (int i = 0; i < recLen; i++)
			{   
				if (wfmAvg[i] > ymax)
				{
					ymax = wfmAvg[i];
				}
				
				if (wfmAvg[i] < ymin)
				{
					ymin = wfmAvg[i];
				}
			}
			
			int ymax2, ymin2 = 0;
			
			// Round values to nearest 25
			ymax2 = (int) (RoundRealToNearestInteger (ymax / 25) * 25) + 25;
			ymin2 = (int) (RoundRealToNearestInteger (ymin / 25) * 25) - 25;
			
			ymax = ymax2;
			ymin = ymin2;
			
			break;
		}
		
		case UNIT_NORM:
		{   
			// Values certain to be overwritten immediately
			ymin = 2.00;
			ymax =  0.00;
			
			for (int i = 0; i < recLen; i++)
			{	
				if (wfmAvg[i] > ymax)
				{
					ymax = wfmAvg[i];
				}
				
				if (wfmAvg[i] < ymin)
				{
					ymin = wfmAvg[i];
				}
			}
			
			double ymax2, ymin2 = 0.0;
			
			// Round values to nearest 0.1
			ymax2 = (double) (RoundRealToNearestInteger (ymax / 0.1) * 0.1) + 0.1;
			ymin2 = (double) (RoundRealToNearestInteger (ymin / 0.1) * 0.1) - 0.1;
			
			ymax = ymax2;
			ymin = ymin2;

			break;
		}
		
		case UNIT_OHM:
		{	 
			 // Values certain to be overwritten immediately
			ymin = 500.00;
			ymax =  0.00;
			
			for (int i = 0; i < recLen; i++)
			{   
				if (wfmAvg[i] > ymax)
				{
					ymax = wfmAvg[i];
				}
				
				if (wfmAvg[i] < ymin)
				{
					ymin = wfmAvg[i];
				}
			}
			
			int ymax2, ymin2 = 0;
			
			// Round values to nearest 5
			ymax2 = (int) (RoundRealToNearestInteger (ymax / 5) * 5) + 5;
			ymin2 = (int) (RoundRealToNearestInteger (ymin / 5) * 5) - 5;
			
			ymax = ymax2;
			ymin = ymin2;

			break;
		}
		
		default: // RHO, data already in this unit
		{
			// Values certain to be overwritten immediately
			ymin = 1.00;
			ymax =  -1.00;
			
			for (int i = 0; i < recLen; i++)
			{  
				if (wfmAvg[i] > ymax)
				{
					ymax = wfmAvg[i];
				}
			
				if (wfmAvg[i] < ymin)
				{
					ymin = wfmAvg[i];
				}
			}
			
			double ymax2, ymin2 = 0.0;
			
			// Round values to nearest 0.1
			ymax2 = (double) (RoundRealToNearestInteger (ymax / 0.1) * 0.1) + 0.1;
			ymin2 = (double) (RoundRealToNearestInteger (ymin / 0.1) * 0.1) - 0.1;
			
			ymax = ymax2;
			ymin = ymin2;

			break;
		}
	}

	// Determine whether to autoscale
	int autoScale;
	status = GetCtrlVal (panelHandle, PANEL_AUTOSCALE, &autoScale);

	// YMAX/YMIN behavior
	if (autoScale == 1)
	{
		status = SetCtrlVal (panelHandle, PANEL_YMAX, ymax);
		status = SetCtrlVal (panelHandle, PANEL_YMIN, ymin);
		
	}
	else
	{
		status = GetCtrlVal (panelHandle, PANEL_YMAX, &ymax);
		status = GetCtrlVal (panelHandle, PANEL_YMIN, &ymin);

		// Compensate if min > max
		if((double) ymin >= (double) ymax)
		{
			ymin = (double) ymax - (double) 1.0;
		}
	}
	
	// Avoid crashes on partial waveforms
	if (ymax == ymin)
	{
		ymax += 1;
		ymin -= 1;
	}
	else if (ymax < ymin)
	{
		ymin = ymax -1;
	}

	// Set range if not constrained by recalled waveform
	if (!WfmStored)
	{
		SetAxisRange (panelHandle, PANEL_WAVEFORM, VAL_AUTOSCALE, 0.0, 0.0, VAL_MANUAL, ymin, ymax);
	}

	// Clear existing WfmActive, don't affect recalled waveforms
	if (WfmActive)
	{
		// Delay draw so there is no flicker before next waveform is plotted
		DeleteGraphPlot (panelHandle, PANEL_WAVEFORM, WfmActive, VAL_DELAYED_DRAW);
	}
	
	WfmActive = PlotXY (panelHandle, PANEL_WAVEFORM, wfmX, wfmAvg, recLen, VAL_DOUBLE, VAL_DOUBLE,
						plotType, VAL_SMALL_SOLID_SQUARE, VAL_SOLID, 1, MakeColor (113, 233, 70));
	
	// Trigger the DELAYED_DRAW
	RefreshGraph (panelHandle, PANEL_WAVEFORM);
	
	// Show timestamp of acquisition
	updateTimestamp ();
	
	// Position cursors and update control reading
	updateCursors ();
}

// Verify necessary folders
void checkDirs (void)
{
	int status;
	
	int existsDir;
	
	// Default .PNG output folder	
	status = FileExists ("images", &existsDir);
	
	if (status == 0) 
	{
		MakeDir ("images");
	}
	
	// Default .ZTDR output folder
	status = FileExists ("waveforms", &existsDir);
	
	if (status == 0) 
	{
		MakeDir ("waveforms");
	}
	
	// Default settings folder
	status = FileExists ("settings", &existsDir);
	
	if (status == 0) 
	{
		MakeDir ("settings");
	}
	
	// Default CSV output folder
	status = FileExists ("datalogs", &existsDir);
	
	if (status == 0) 
	{
		MakeDir ("datalogs");
	}
}

// Format and show current version and instrument
void showVersion (void)
{
	int status;
	
	// Get full version number
	char version[64];
	status = sprintf (version, "ZTDR v%s", _TARGET_PRODUCT_VERSION_);
	
	// Trim build number
	int len = strlen (version) - 2;
	version[len] = 0;
	
	// Append instrument model
	status = sprintf (version, "%s / HL1101", version);
	
	status = SetCtrlVal (panelHandle, PANEL_VERSION, version);
}

// Show timestamp of acquisition
void updateTimestamp (void)
{
	int status;
	
	char timestamp[64];
	int	month, day, year;
	int	hours, minutes, seconds;
	
	status = GetSystemDate (&month, &day, &year);
	status = GetSystemTime (&hours, &minutes, &seconds);
	
	status = sprintf (timestamp, "%s %02d %02d | %02d:%02d:%02d", monthName[month-1], day, year, hours, minutes, seconds);
	
	// Show version, timestamp
	status = SetCtrlVal (panelHandle, PANEL_TIMESTAMP, timestamp);
}

// Write calibration message to status
void writeMsgCal (int msg)
{
	int status;
	
	if (msg == 0)
	{   
		// Start initial calibration message
		status = SetCtrlVal (panelHandle, PANEL_MESSAGES, "> Calibration ...");
	}
	else if (msg == 1)
	{
		status = SetCtrlVal (panelHandle, PANEL_MESSAGES, " DONE!\n");
	}
	else
	{
		status = SetCtrlVal (panelHandle, PANEL_MESSAGES, " FAILED!\n");
	}
}

// Write VertCal message to status
void writeMsgVertCal (int msg)
{
	int status;
	
	if (msg == 1)
	{
		status = SetCtrlVal (panelHandle, PANEL_MESSAGES, "> VertCal ... DONE!\n");
	}
	else
	{
		status = SetCtrlVal (panelHandle, PANEL_MESSAGES, "> VertCal ... FAILED!\n");
	}
}

// Change between dots and line
void changePlot (int unit)
{
	int status;																			   
	
	// Change unit selection and update menu
	if (unit == 0)
	{   
		// Dots
		plotType = 2L;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_DOTS, ATTR_CHECKED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_THINLINE, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_FATLINE, ATTR_CHECKED, 0);
	}
	else if (unit == 1)
	{
		// Thin line
		plotType = 0L;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_DOTS, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_THINLINE, ATTR_CHECKED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_FATLINE, ATTR_CHECKED, 0);
	}
	else if (unit == 2)
	{
		// Thick line
		plotType = 5L;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_DOTS, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_THINLINE, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_DISPLAY_FATLINE, ATTR_CHECKED, 1);
	}
}

// Change graph background
void changeBg (int color)
{
	int status;
	
	// Set default dark background
	if (color == 0)
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
	else if (color == 1)
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
}

// Toggle autoscale
void changeAutoCal (void)
{
	int status;

	// Get status of timer
	int autoStatus;
	status = GetCtrlAttribute (panelHandle, PANEL_CALTIMER, ATTR_ENABLED, &autoStatus);
	
	if (autoStatus == 1)
	{
		// Turn off timer and uncheck menu item
		status = SetCtrlAttribute (panelHandle, PANEL_CALTIMER, ATTR_ENABLED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_CALIBRATION_AUTOCAL, ATTR_CHECKED, 0);
	}
	else
	{
		// Turn on timer and check menu item
		status = SetCtrlAttribute (panelHandle, PANEL_CALTIMER, ATTR_ENABLED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_CALIBRATION_AUTOCAL, ATTR_CHECKED, 1);
	}
}

// Toggle dimming of controls based on autoscale
void changeAutoScale (void)
{
	int status;
	
	// Get control value
	int val;
	status = GetCtrlVal (panelHandle, PANEL_AUTOSCALE, &val);
	
	if (val == 1)
	{
		status = SetCtrlAttribute (panelHandle, PANEL_YMAX, ATTR_DIMMED, 1);
		status = SetCtrlAttribute (panelHandle, PANEL_YMIN, ATTR_DIMMED, 1);
	}
	else
	{
		status = SetCtrlAttribute (panelHandle, PANEL_YMAX, ATTR_DIMMED, 0);
		status = SetCtrlAttribute (panelHandle, PANEL_YMIN, ATTR_DIMMED, 0);
	}
}

// Dielectric constant changed
void changeDiel (void)
{
	int status;
	
	status = GetCtrlVal (panelHandle, PANEL_DIEL, &dielK);
	
	double vc = 1 / sqrt (dielK);
	
	status = SetCtrlVal (panelHandle, PANEL_VC, vc);
}

// Change horizontal units
void changeUnitX (int unit)
{
	int status;
	
	// Store previous unit and limits to perform conversion of current window values
	int prevUnit;
	double prevStart, prevEnd;
	
	prevUnit = xUnits;
	status = GetCtrlVal (panelHandle, PANEL_START, &prevStart);
	status = GetCtrlVal (panelHandle, PANEL_END, &prevEnd);
	
	xUnits = unit; 
	
	// Change unit selection and update menu
	if (unit == 0)
	{
		xUnits = UNIT_M;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_XUNITS_XUNITS1, ATTR_CHECKED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_XUNITS_XUNITS2, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_XUNITS_XUNITS3, ATTR_CHECKED, 0);
	}
	else if (unit == 1)
	{
		xUnits = UNIT_FT;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_XUNITS_XUNITS1, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_XUNITS_XUNITS2, ATTR_CHECKED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_XUNITS_XUNITS3, ATTR_CHECKED, 0);
	}
	else if (unit == 2)
	{
		xUnits = UNIT_NS;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_XUNITS_XUNITS1, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_XUNITS_XUNITS2, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_XUNITS_XUNITS3, ATTR_CHECKED, 1);
	}
	
	// Calculate V/C
	double vc = 1.0 / sqrt (dielK);
	
	// Keep same window and apply to new units
	if (xUnits == UNIT_M)
	{
		if (prevUnit == UNIT_M)
		{
			// Keep m as m
			xStart = prevStart;
			xEnd = prevEnd;
		}
		else if (prevUnit == UNIT_FT)
		{
			// Convert ft to m
			xStart = prevStart * FT_TO_M;
			xEnd = prevEnd * FT_TO_M;	
		}
		else if (prevUnit == UNIT_NS)
		{
			// Convert ns to m
			xStart = prevStart * (vc * 3e8 * 1e-9);
			xEnd = prevEnd * (vc * 3e8 * 1e-9);
		}
	}
	else if (xUnits == UNIT_FT)
	{
		if (prevUnit == UNIT_M)
		{
			// Convert m to ft
			xStart = prevStart / FT_TO_M;
			xEnd   = prevEnd / FT_TO_M;
		}
		else if (prevUnit == UNIT_FT)
		{
			// Keep ft as ft
			xStart = prevStart;
			xEnd = prevEnd;
		}
		else if (prevUnit == UNIT_NS)
		{
			// Convert ns to ft
			xStart = (prevStart * vc * 3E8 * 1e-9) / FT_TO_M;
			xEnd = (prevEnd * vc * 3E8 * 1e-9) / FT_TO_M;
		}
	}
	else if (xUnits == UNIT_NS)
	{
		if (prevUnit == UNIT_M)
		{
			// Convert m to ns
			xStart = prevStart / (vc * 3e8 * 1e-9);
			xEnd = prevEnd / (vc * 3e8 * 1e-9);
			
			// endTime = xEnd / (acqDiel * 3e8);
		}
		else if (prevUnit == UNIT_FT)
		{
			// Convert ft to ns
			xStart = (prevStart * FT_TO_M) / (vc * 3E8 * 1E-9);
			xEnd = (prevEnd * FT_TO_M) / (vc * 3E8 * 1E-9);
		}
		else if (prevUnit == UNIT_NS)
		{
			// Keep ns as ns
			xStart = prevStart;
			xEnd = prevEnd;
		}
	}
	
	// Update X labels and limits
	status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_XNAME, labelX[xUnits]);
	
	status = SetCtrlAttribute (panelHandle, PANEL_START, ATTR_LABEL_TEXT, labelStartX[xUnits]);
	status = SetCtrlAttribute (panelHandle, PANEL_END, ATTR_LABEL_TEXT, labelEndX[xUnits]);
	
	status = SetCtrlAttribute (panelHandle, PANEL_START, ATTR_MAX_VALUE, maxRange[xUnits]);
	status = SetCtrlAttribute (panelHandle, PANEL_END, ATTR_MAX_VALUE, maxRange[xUnits]);
	
	status = SetCtrlVal (panelHandle, PANEL_START, xStart);
	status = SetCtrlVal (panelHandle, PANEL_END, xEnd);
}

// Change vertical units
void changeUnitY (int unit)
{
	int status;
	
	// Change unit selection and update menu 
	if (unit == 0)
	{
		yUnits = UNIT_MV;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS1, ATTR_CHECKED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS2, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS3, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS4, ATTR_CHECKED, 0);
		
		// Set manual scale defaults 
		status = SetCtrlVal (panelHandle, PANEL_YMAX, 250.00);
		status = SetCtrlVal (panelHandle, PANEL_YMIN, -250.00);
		
		// Set precision of Y axis
		status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_YPRECISION, 0);
	}
	else if (unit == 1)
	{
		yUnits = UNIT_NORM;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS1, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS2, ATTR_CHECKED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS3, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS4, ATTR_CHECKED, 0);
		
		// Set manual scale defaults 
		status = SetCtrlVal (panelHandle, PANEL_YMAX, 2.00);
		status = SetCtrlVal (panelHandle, PANEL_YMIN, 0.00);
		
		// Set precision of Y axis
		status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_YPRECISION, 2);
	}
	else if (unit == 2)
	{
		yUnits = UNIT_OHM;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS1, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS2, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS3, ATTR_CHECKED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS4, ATTR_CHECKED, 0);
		
		// Set manual scale defaults 
		status = SetCtrlVal (panelHandle, PANEL_YMAX, 500.00);
		status = SetCtrlVal (panelHandle, PANEL_YMIN, 0.00);
		
		// Set precision of Y axis
		status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_YPRECISION, 0);
	}
	else if (unit == 3)
	{
		yUnits = UNIT_RHO;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS1, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS2, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS3, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS_YUNITS4, ATTR_CHECKED, 1);
		
		// Set manual scale defaults
		status = SetCtrlVal (panelHandle, PANEL_YMAX, 1.00);
		status = SetCtrlVal (panelHandle, PANEL_YMIN, -1.00);					   
		
		// Set precision of Y axis
		status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_YPRECISION, 2);
	}
	
	status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_YNAME, labelY[yUnits]);
}

// Update cursor readings
void updateCursors (void)
{  	
	int status;
	
	double c1x, c1y, c2x, c2y;
	static char buf[128];

	c1x = c1y = c2x = c2y = 0;
	
	status = GetGraphCursor (panelHandle, PANEL_WAVEFORM, 1, &c1x, &c1y);
	status = GetGraphCursor (panelHandle, PANEL_WAVEFORM, 2, &c2x, &c2y);

	// Cursor 1
	status = sprintf (buf, " %.3f %s, %.3f %s", c1x, shortX[xUnits], c1y, shortY[yUnits]);
	status = SetCtrlVal (panelHandle, PANEL_CURSOR1,  buf);

	// Cursor 2
	status = sprintf (buf, " %.3f %s, %.3f %s", c2x, shortX[xUnits], c2y, shortY[yUnits]);
	status = SetCtrlVal (panelHandle, PANEL_CURSOR2, buf);

	// Delta
	status = sprintf(buf, " %.3f %s, %.3f %s", c2x-c1x, shortX[xUnits], c2y-c1y, shortY[yUnits]);
	status = SetCtrlVal (panelHandle, PANEL_DELTA, buf);
}

// Cursor-based zoom
void zoom (void)
{   
	int status;
	
	double c1x, c1y, c2x, c2y;
	
	status = GetGraphCursor (panelHandle, PANEL_WAVEFORM, 1, &c1x, &c1y);
	status = GetGraphCursor (panelHandle, PANEL_WAVEFORM, 2, &c2x, &c2y);

	// Update start and end controls
	if (c1x < c2x)
	{
		status = SetCtrlVal(panelHandle, PANEL_START, c1x);
		status = SetCtrlVal(panelHandle, PANEL_END, c2x);
	}
	else
	{
		status = SetCtrlVal(panelHandle, PANEL_START, c2x);
		status = SetCtrlVal(panelHandle, PANEL_END, c1x);
	}
}

// Reset to default window
void resetZoom (void)
{
	SetCtrlVal (panelHandle, PANEL_START, defaultStart[xUnits]);
	SetCtrlVal (panelHandle, PANEL_END, defaultEnd[xUnits]);	
}

// Resize acquisition window
void resizeWindow (void)
{
	int status;
	
	double x1, x2;
	
	status = GetCtrlVal (panelHandle, PANEL_START, &x1);
	status = GetCtrlVal (panelHandle, PANEL_END, &x2);

	// Start is less than end and window is wide enough
	if ((x1 + minWidth[xUnits]) < x2) 
	{
		xStart = x1;
		xEnd = x2;
	}
	// Adjustment if end less than start
	else
	{
		int adjust = 0;
		
		if (xUnits == UNIT_M)
		{
			adjust = 1;
		}
		else if (xUnits == UNIT_FT)
		{
			adjust = 3;
		}
		else if (xUnits == UNIT_NS)
		{
			adjust = 5;
		}
		
		status = SetCtrlVal (panelHandle, PANEL_START, x1);
		status = SetCtrlVal (panelHandle, PANEL_END, x1 + adjust);
		
		xStart = x1;
		xEnd = x1 + adjust;
	}
} 

// Print current waveform and controles
void printWaveform (void)
{
	int status;
	
	// Disable timers during action
	status = SuspendTimerCallbacks ();
	
	// Force light color scheme to save toner
	int color;
	status = GetMenuBarAttribute (menuHandle,MENUBAR_DISPLAY_DARK, ATTR_CHECKED, &color);
	
	if (color == 1)
	{
		changeBg (1);
	}
	
	// Set optimal printer settings
	status = SetPrintAttribute (ATTR_PRINT_AREA_HEIGHT, VAL_USE_ENTIRE_PAPER);
	status = SetPrintAttribute (ATTR_PRINT_AREA_WIDTH, VAL_INTEGRAL_SCALE);
	
	status = PrintPanel (panelHandle, "", 1, VAL_FULL_PANEL, 1);
	
	// Change back to dark scheme if necessary
	if (color == 1)
	{
		changeBg (0);
	}
	
	// Re-enable timers 
	status = ResumeTimerCallbacks ();
}

// Save both PNG and CSV
void saveMulti (void)
{
	int status;
	
	// Disable timers during action
	status = SuspendTimerCallbacks ();
	
	
	
	// Re-enable timers
	status = ResumeTimerCallbacks ();
}

// Save waveform and controls to PNG
void savePNG (void)
{
	int status;
	
	// Disable timers during action
	status = SuspendTimerCallbacks ();

	// Select file to save
	char filename[64];
	char save_file[260];
	status = sprintf (filename, ".png");
	
	// Chose save folder
	if (defaultSavePNG == 1)
	{
		// Save to program default folder
		status = FileSelectPopup ("images", filename, "PNG (*.png)", "Select File to Save", VAL_SAVE_BUTTON, 0, 0, 1, 1, save_file);
		defaultSavePNG = 0;
	}
	else
	{
		// Save to last directory user was in
		status = FileSelectPopup ("", filename, "PNG (*.png)", "Select File to Save", VAL_SAVE_BUTTON, 0, 0, 1, 1, save_file);
	}

	// Don't attempt to save if user cancels
	if (status == VAL_NO_FILE_SELECTED)
	{
		// Re-enable timers 
		status = ResumeTimerCallbacks ();
		
		return;
	}
	
	// Prepare image file
	int imageFile;
	status = GetPanelDisplayBitmap (panelHandle, VAL_FULL_PANEL, VAL_ENTIRE_OBJECT, &imageFile);
	status = SaveBitmapToPNGFile (imageFile, save_file);
	
	// Re-enable timers
	status = ResumeTimerCallbacks ();
}

// Store waveform to file as ZTDR (format = 1) or CSV (= 0)
void storeWaveform (int format)
{   
	int status;	

	// Disable timers during action
	status = SuspendTimerCallbacks ();
	
	// File setup
	char save_file[512];
	char filename[64];
	
	// Save dialog
	if (format == 1)
	{
		status = sprintf (filename, ".ztdr");
		
		// Chose save folder
		if (defaultSaveZTDR == 1)
		{
			// Save in default location
			status = FileSelectPopup ("waveforms", filename, "ZTDR Waveform (*.ztdr)", "Select File to Save", VAL_SAVE_BUTTON, 0, 0, 1, 1, save_file);
			defaultSaveZTDR = 0;
		}
		else
		{
			// Save in last user directory
			status = FileSelectPopup ("", filename, "ZTDR Waveform (*.ztdr)", "Select File to Save", VAL_SAVE_BUTTON, 0, 0, 1, 1, save_file);
		}
	}
	else
	{
		status = sprintf (filename, ".csv");
		
		// Chose save folder
		if (defaultSaveCSV == 1)
		{
			// Save in default location
			status = FileSelectPopup ("datalogs", filename, "CSV (*.csv)", "Select File to Save", VAL_SAVE_BUTTON, 0, 0, 1, 1, save_file);
			defaultSaveCSV = 0;
		}
		else
		{
			// Save in last user directory
			status = FileSelectPopup ("", filename, "CSV (*.csv)", "Select File to Save", VAL_SAVE_BUTTON, 0, 0, 1, 1, save_file);
		}
	}

	// Don't attempt to save if user cancels
	if (status == VAL_NO_FILE_SELECTED)
	{
		// Re-enable timers
		status = ResumeTimerCallbacks ();
		
		return;
	}	
	
	// Open selected file for write
	int fd;
	fd = OpenFile (save_file, VAL_READ_WRITE, VAL_TRUNCATE, VAL_ASCII);
	
	// Set up data buffer;
	char buf[128];
	buf[0] = 0;

	// Create header row
	double ymin, ymax;
	
	GetCtrlVal (panelHandle, PANEL_YMIN, &ymin);
	GetCtrlVal (panelHandle, PANEL_YMAX, &ymax);
	
	// Write header row
	if (format == 1)
	{
		// Header for .ZTDR
		status = sprintf (buf + strlen(buf), "%d, %d, %3.10f, %3.10f, %3.3f, %3.3f, %3.3f\n", yUnits, xUnits, xStart, xEnd, ymin, ymax, dielK);
	}
	else
	{
		// Header for .CSV
		status = sprintf (buf + strlen(buf), "%s, %s\n", labelY[yUnits], labelX[xUnits]);
	}
	
	status = WriteFile (fd, buf, strlen (buf));
	
	// Log X/Y data
	for (int i = 0; i < recLen; i++)
	{
		// Reset buffer
		buf[0] = 0;
	
		status = sprintf (buf + strlen(buf), "%3.10f, %3.10f\n", wfmAvg[i], wfmX[i]);
		
		status = WriteFile (fd, buf, strlen(buf));
	}
	
	status = CloseFile (fd);
	
	// Re-enable timers 
	status = ResumeTimerCallbacks ();
}

// Recall stored waveform
void recallWaveform (void)
{
	int status;

	// Disable timers during action
	SuspendTimerCallbacks ();
	
	// Select file
	char save_file[260];
	
	// Chose load folder
	if (defaultSaveZTDR == 1)
	{
		// Load from default location
		status = FileSelectPopup ("waveforms", "*.ztdr", "ZTDR Waveform (*.ztdr)", "Select File to Retrieve", VAL_SELECT_BUTTON, 0, 0, 1, 1, save_file);
	}
	else
	{
		// Load from last user directory
		status = FileSelectPopup ("", "*.ztdr", "ZTDR Waveform (*.ztdr)", "Select File to Retrieve", VAL_SELECT_BUTTON, 0, 0, 1, 1, save_file);
	}

	// Don't crash if user cancels
	if (status == VAL_NO_FILE_SELECTED)
	{
		// Re-enable timers 
		ResumeTimerCallbacks ();
		
		return;
	}
	
	// Open file for reading
	int fd = OpenFile (save_file, VAL_READ_WRITE, VAL_OPEN_AS_IS, VAL_ASCII);
	
	// Set up data buffer
	char buf[128];
	int buf_len = 128;
	buf[0] = 0;
												
	
	// Read header row for environmental variables
	int xStored, yStored;
	float startStored, endStored;
	float ymin, ymax;
	float dielStored;
	double vc;
	
	// Read header line
	status = ReadLine (fd, buf, buf_len - 1);
	sscanf (buf, "%d, %d, %f, %f, %f, %f, %f", &yStored, &xStored, &startStored, &endStored, &ymin, &ymax, &dielStored);
	vc = (double) 3E8 / sqrt (dielStored);
							   
	// Read X, Y values
	for (int i = 0; i < recLen; i++)
	{  
		float x, y;
		
		status = ReadLine (fd, buf, buf_len - 1);
		sscanf(buf,"%f, %f", &y, &x);

		wfmRecall[i] = (double) y;
		wfmRecallX[i] = (double) x;
	}
	
	// Data read finished
	status = CloseFile(fd);
								   
	// Set control values from stored waveform
	SetCtrlVal (panelHandle, PANEL_AUTOSCALE, 0);
	
	// Store globals
	changeUnitX (xStored);
	changeUnitY (yStored);

	// Update controls
	SetCtrlVal (panelHandle, PANEL_START, (double) startStored);
	SetCtrlVal (panelHandle, PANEL_END, (double) endStored);
	SetCtrlVal (panelHandle, PANEL_DIEL, (double) dielStored);
	SetCtrlVal (panelHandle, PANEL_YMAX, (double) ymax);
	SetCtrlVal (panelHandle, PANEL_YMIN, (double) ymin);
	SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_XNAME, labelX[xUnits]);
	
	// Change window and K
	resizeWindow ();
	setupTimescale ();
	changeDiel ();
	
	// Remove any other recalled waveforms
	if (WfmStored)
	{
		DeleteGraphPlot (panelHandle, PANEL_WAVEFORM, WfmStored, VAL_IMMEDIATE_DRAW);
		WfmStored = 0;
	}
	
	// Remove any active waveforms
	if (WfmActive)
	{
		DeleteGraphPlot (panelHandle, PANEL_WAVEFORM, WfmActive, VAL_IMMEDIATE_DRAW);
		WfmActive = 0;
	}
	
	// Scale waveform acquisition window
	SetAxisRange (panelHandle, PANEL_WAVEFORM, VAL_AUTOSCALE, 0.0, 0.0, VAL_MANUAL, (double) ymin, (double) ymax);
	
	// Plot waveform
	WfmStored = PlotXY (panelHandle, PANEL_WAVEFORM, wfmRecallX, wfmRecall, recLen, VAL_DOUBLE, VAL_DOUBLE, 
						VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, MakeColor (233, 113, 233));

	// Dim controls
	status = SetCtrlAttribute (panelHandle, PANEL_END, ATTR_DIMMED, 1);
	status = SetCtrlAttribute (panelHandle, PANEL_START, ATTR_DIMMED, 1);
	status = SetCtrlAttribute (panelHandle, PANEL_DIEL, ATTR_DIMMED, 1);
	status = SetCtrlAttribute (panelHandle, PANEL_ZOOM, ATTR_DIMMED, 1);
	status = SetCtrlAttribute (panelHandle, PANEL_YMAX, ATTR_DIMMED, 1);
	status = SetCtrlAttribute (panelHandle, PANEL_YMIN, ATTR_DIMMED, 1);
	status = SetCtrlAttribute (panelHandle, PANEL_AUTOSCALE, ATTR_DIMMED, 1);
	
	// Dim menus
	status = SetMenuBarAttribute (menuHandle, MENUBAR_DATA_STORE, ATTR_DIMMED, 1);
	status = SetMenuBarAttribute (menuHandle, MENUBAR_XUNITS, ATTR_DIMMED, 1);
	status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS, ATTR_DIMMED, 1);

	// Show clear button and menu item
	status = SetCtrlAttribute (panelHandle, PANEL_CLEAR, ATTR_VISIBLE, 1);
	status = SetMenuBarAttribute (menuHandle, MENUBAR_DATA_CLEAR, ATTR_DIMMED, 0);
	
	// Re-enable timers 
	ResumeTimerCallbacks ();
}

// Reset plot area and clear recalled waveform
void clearWaveform (void)
{
	int status;
	
	status = SetCtrlVal (panelHandle, PANEL_AUTOSCALE, 1);

	// Remove recalled waveform, if any
	if (WfmStored)
	{
		status = DeleteGraphPlot (panelHandle, PANEL_WAVEFORM, WfmStored, VAL_IMMEDIATE_DRAW);
		WfmStored = 0;
	}
	
	// Re-enable controls
	status = SetCtrlAttribute (panelHandle, PANEL_END, ATTR_DIMMED, 0);
	status = SetCtrlAttribute (panelHandle, PANEL_START, ATTR_DIMMED, 0);
	status = SetCtrlAttribute (panelHandle, PANEL_DIEL, ATTR_DIMMED, 0);
	status = SetCtrlAttribute (panelHandle, PANEL_ZOOM, ATTR_DIMMED, 0);
	status = SetCtrlAttribute (panelHandle, PANEL_YMAX, ATTR_DIMMED, 0);
	status = SetCtrlAttribute (panelHandle, PANEL_YMIN, ATTR_DIMMED, 0);
	status = SetCtrlAttribute (panelHandle, PANEL_AUTOSCALE, ATTR_DIMMED, 0);
	
	// Un-dim menus
	status = SetMenuBarAttribute (menuHandle, MENUBAR_DATA_STORE, ATTR_DIMMED, 0);
	status = SetMenuBarAttribute (menuHandle, MENUBAR_XUNITS, ATTR_DIMMED, 0);
	status = SetMenuBarAttribute (menuHandle, MENUBAR_YUNITS, ATTR_DIMMED, 0);
	
	// Hide clear button and dim menu
	status = SetCtrlAttribute (panelHandle, PANEL_CLEAR, ATTR_VISIBLE, 0);
	status = SetMenuBarAttribute (menuHandle, MENUBAR_DATA_CLEAR, ATTR_DIMMED, 1);
}

// Update position of controls on resize
void updateSize (void)
{
	int status;
	int count;
	
	// Disable timers during action
	status = SuspendTimerCallbacks ();
	
	int newWidth, newHeight;
								
	status = GetPanelAttribute (panelHandle, ATTR_WIDTH, &newWidth);
	status = GetPanelAttribute (panelHandle, ATTR_HEIGHT, &newHeight);

	// Prevent sizing too small
	if (newWidth < 1024)
	{
		newWidth = 1024;
		status = SetPanelAttribute (panelHandle, ATTR_WIDTH, newWidth);
	}
	if (newHeight < 576)
	{
		newHeight = 576;
		status = SetPanelAttribute (panelHandle, ATTR_HEIGHT, newHeight);
	}
	
	int xOffset, yOffset;
	
	// Calculate size change
	xOffset = newWidth - windowWidth;
	yOffset = newHeight - windowHeight;

	// Control position and size
	int ctrlWidth, ctrlHeight;
	int ctrlLeft, ctrlTop;
	
	// Resize panel window
	status = GetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_WIDTH, &ctrlWidth);
	status = GetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_HEIGHT, &ctrlHeight);
	
	status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_WIDTH, ctrlWidth + xOffset);
	status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_HEIGHT, ctrlHeight + yOffset);
	
	// Move right-hand control panel
	status = GetNumCtrlArrayItems (rightHandle, &count);
   	
	// Define here to avoid redefine errors
	int i;
	
	for (i = 0; i < count; i++)
	{   
		status = GetCtrlAttribute (panelHandle, GetCtrlArrayItem (rightHandle, i), ATTR_LEFT, &ctrlLeft);
		status = SetCtrlAttribute (panelHandle, GetCtrlArrayItem (rightHandle, i), ATTR_LEFT, ctrlLeft + xOffset);
	}
	
	// Move bottom control pane
	status = GetNumCtrlArrayItems (bottomHandle, &count);
   		  
	for (i = 0; i < count; i++)
	{
		status = GetCtrlAttribute (panelHandle, GetCtrlArrayItem (bottomHandle, i), ATTR_TOP, &ctrlTop);
		status = SetCtrlAttribute (panelHandle, GetCtrlArrayItem (bottomHandle, i), ATTR_TOP, ctrlTop + yOffset);
	}
	
	// Resize bottom control pane									   ;
	status = GetCtrlAttribute (panelHandle, PANEL_PANELBOTTOM, ATTR_WIDTH, &ctrlWidth);
	status = SetCtrlAttribute (panelHandle, PANEL_PANELBOTTOM, ATTR_WIDTH, ctrlWidth + xOffset);
	
	// Resize window start control
	status = GetCtrlAttribute (panelHandle, PANEL_START, ATTR_WIDTH, &ctrlWidth);
	status = SetCtrlAttribute (panelHandle, PANEL_START, ATTR_WIDTH, ctrlWidth + (xOffset / 3));
	
	// Reposition zoom controls
	status = GetCtrlAttribute (panelHandle, PANEL_ZOOM, ATTR_LEFT, &ctrlLeft);
	status = SetCtrlAttribute (panelHandle, PANEL_ZOOM, ATTR_LEFT, ctrlLeft + xOffset / 2);
	status = GetCtrlAttribute (panelHandle, PANEL_RESET, ATTR_LEFT, &ctrlLeft);
	status = SetCtrlAttribute (panelHandle, PANEL_RESET, ATTR_LEFT, ctrlLeft + xOffset / 2);
	
	// Resize window end control
	status = GetCtrlAttribute (panelHandle, PANEL_END, ATTR_WIDTH, &ctrlWidth);
	status = SetCtrlAttribute (panelHandle, PANEL_END, ATTR_WIDTH, ctrlWidth + (xOffset / 3));
	
	// Reposition window end control
	status = GetCtrlAttribute (panelHandle, PANEL_END, ATTR_LEFT, &ctrlLeft);
	status = SetCtrlAttribute (panelHandle, PANEL_END, ATTR_LEFT, ctrlLeft + (xOffset * 2/3));
	
	// Reposition window size label
	status = GetCtrlAttribute (panelHandle, PANEL_END, ATTR_LABEL_LEFT, &ctrlLeft);
	status = SetCtrlAttribute (panelHandle, PANEL_END, ATTR_LABEL_LEFT, ctrlLeft + (xOffset / 3));
	
	// Reposition window size display
	status = GetCtrlAttribute (panelHandle, PANEL_END, ATTR_DIG_DISP_LEFT, &ctrlLeft);
	status = SetCtrlAttribute (panelHandle, PANEL_END, ATTR_DIG_DISP_LEFT, ctrlLeft + (xOffset / 3));
	
	// Reposition K control
	status = GetCtrlAttribute (panelHandle, PANEL_DIEL, ATTR_LEFT, &ctrlLeft);
	status = SetCtrlAttribute (panelHandle, PANEL_DIEL, ATTR_LEFT, ctrlLeft + xOffset);
	
	// Write new window size to globals
	windowWidth = newWidth;
	windowHeight = newHeight;
	
	// Re-enable timers
	status = ResumeTimerCallbacks ();
}
