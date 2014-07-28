//==============================================================================
//
// Title:		shared.c
// Purpose:		Main ZTDR functionality for HL11xx TDR instruments
//
// Created on:	7/22/2014 at 8:40:39 PM by Brian Doxey.
// Copyright:	HYPERLABS. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include <windows.h>
#include "toolbox.h"
#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>

#include "FTD2XX.h"

#include "driver.h"
#include "main.h"
#include "usbfifo.h"
#include "ZTDR.h"


//==============================================================================
// Constants

// General

// Calibration
#define CAL_WINDOW 10.0e-9
#define CAL_WINDOW_START 10 // ns
#define CAL_GUARD 0.5e-9
#define STEP_AMPL 800

// First sample in the record, no matter where the wfm is positioned
#define OFFSET_ACQ_POS 0





// File save/load
#define BUF_REC_LEN 64
#define MAX_SAVE_FILE 100


//==============================================================================
// Types


//==============================================================================
// Static global variables


//==============================================================================
// Static functions


//==============================================================================
// Global variables (roughly grouped by function)

// TO DO: significant cleanup of section





// Unit labels and ranges
char *y_label[] =
     {
      "VOLTS (mV)",
      "NORMALIZED",
      "IMPEDANCE (Ohm)",
      "REFLECT COEFF (Rho)" 
	 };

char *x_label[] =
     {
      "ROUNDTRIP (m)",
      "ROUNDTRIP (ft)",
      "ROUNDTRIP (ns)" 
	 };

char *y_short[] =
     {
      "mV",
      "Norm",
      "Ohm",
      "Rho" 
	 };

char *x_short[] =
     {
      "m",
      "ft",
      "ns" 
	 };

char *x_label_start[] =
     {
      "START (m)",
      "START (ft)" ,
      "START (ns)"
	 };

      
char *x_label_windowsz[] =
     {
      "WINDOW (m)",
      "WINDOW (ft)",
      "WINDOW (ns)" 
	 }; 		
      
float x_dflt_start[] =
     {
      0.0,
      0.0,
      0.0
	 };

float x_dflt_windowsz[]  =
     {
      10.0,
      33.3,
      50.0
	 };

float x_max_range[] =
     {
      400.0,
      1332,
      2000.0
	 };

char *label_dist[] =
     {
      "(X2-X1) (m) ",
      "(X2-X1) (ft)",
      "(X2-X1) (ns)" 
	 };


// Calibration
int 	calIncrement;
double 	vampl = 679.0;

// Calibration parameters
UINT16 	dac0val = 0, dac1val = 0, dac2val = 0;
int 	freerun_en = 0;
UINT16 	strobecount = 2;


// Horizontal values for each unit
double 	wfm_x[NPOINTS_MAX];			// Passed to graph
double 	wfm_dist[NPOINTS_MAX]; 		// Recalled waveform

// Vertical values in different modes
double  wfm_data[NPOINTS_MAX];		// Converted to selected units
double  wfm_data_ave[NPOINTS_MAX]; 	// After waveform averaging
double 	wfm_ret[NPOINTS_MAX]; 		// Recalled waveform

double	wfm_rho_data[NPOINTS_MAX];
double	wfm_z_data[NPOINTS_MAX];





// TO DO: START updated, organized variables


// TO DO: organize this section as extern (defined in .h)
// Control states needed outside UIR
int		diel = 2.25; // coax
int 	yUnits = 0; // mV
int 	xUnits = 0; // m
int		xStart = 0.0; // m
int		xEnd = 10.0; // m

// Number of data points acquired
UINT16 	rec_len	= 1024;

// Waveform storage
double 	wfmDistFt[NPOINTS_MAX]; // distance (ft)
double 	wfmDistM[NPOINTS_MAX]; // distance (m)
double 	wfmTime[NPOINTS_MAX]; // time (ns)

UINT16 	wfm[NPOINTS_MAX]; // raw data from device
double 	wfmFilter[NPOINTS_MAX];	// filtered data from device


// TO DO: below are not extern
int		plotType = 2L; // dots

// Waveform handles
int 	WfmActive; 	// current acquisition
int 	WfmStored;	// stored waveform

// UIR elements
int 	panelHandle, menuHandle;
int		rightHandle, bottomHandle;

// Panel size
int		width, height;

// TO DO: END updated, organized variables





//==============================================================================
// Global functions (roughly grouped by functionality, order of call)

// Main startup function
void main (int argc, char *argv[])
{
	int i;
	UINT8 acq_result;

	// Initial values for maximum length of array
	for (i=0; i < NPOINTS_MAX; i++)
	{
		wfm[i] = 0;
		wfmf[i] = 0.0;
	}

	// Verify instrument functionality
	if (InitCVIRTE (0, argv, 0) == 0)
	{
		return -1;	/* out of memory */
	}

	// TO DO: clean this routine up!
	// Load UI
	if ((panelHandle = LoadPanel (0, "ZTDR.uir", PANEL)) < 0)
	{
		return -1;
	}
	// Load menu bar
	menuHandle = LoadMenuBar (panelHandle, "ZTDR.uir", MENUBAR);
	
	// Display panel and store size
	DisplayPanel (panelHandle);
	GetPanelAttribute(panelHandle, ATTR_WIDTH, &width);
	GetPanelAttribute(panelHandle, ATTR_HEIGHT, &height);
	
	// Load control arrays
	rightHandle = GetCtrlArrayFromResourceID (panelHandle, RIGHT);
	bottomHandle = GetCtrlArrayFromResourceID (panelHandle, BOTTOM);
	
	// Make sure relevant output directories exist
	checkDirs ();
	
	// Show software version
	showVersion ();

	// Set 50 ns timescale
	calIncrement = (int) ((((double) CAL_WINDOW - (double) 0.0) *(double) 1.0 / (double) 1024.0 )/
						  (((double) 50e-9) / (double) 65536.0));

	// Set up and calibrate instrument
	setupTimescale ();
	openDevice ();
	
	writeMsgCal (0);
	calTimebase ();
	
	// Run first acquisition
	acquire ();				
	
	// Set initial cursor positions roughly to baseline and internal reference
	SetGraphCursor (panelHandle, PANEL_WAVEFORM, 1, 2.25, -250);
	SetGraphCursor (panelHandle, PANEL_WAVEFORM, 2, 3.25, 0);
	
	// Start timer for subsequent acquisitions
	SetCtrlAttribute (panelHandle, PANEL_TIMER, ATTR_ENABLED, 1);
	
	RunUserInterface ();	
	
	DiscardPanel (panelHandle);
	
	return 0;
}

// TO DO: move back to better place
// Main acquisition function
void acquire (void)
{
	int ret = 0;
	int i,n,k;
	int status;
	int acquisition_nr = 1;
	
	unsigned char buf[24];
	char ch;
	UINT8 acq_result;
	static char cbuf[32];
	
	double ymax, ymin;
	int nblocks;
	int blocksok;
	
	double impedance = 50;
	double ampl_factor = 250.0;
	
	int	auto_flag;

	double wfmf_debug[1024];
	double wfm_data_debug[1024];
	
	double offset = 0;
	
	if (!usb_opened)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Comm failure.");
		return;
	}
	
	// Set window to acquire offset at 0 ns
	vertCalZero (OFFSET_ACQ_POS);
	
	// Write the acquisition parameters
	if (vertCalWriteParams () <= 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Param error.");
		return;
	}
	
	// Acquire data
	ret = usbfifo_acquire (&acq_result, 0);
	
	if (ret < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
		return;
	}
	
	// Read blocks of data from block numbers 0-63 (max 64 blocks and 16384 pts)
	blocksok = 1;
	nblocks = rec_len / 256;
	
	for (i=0; i < nblocks; i++)
	{
		// Verify data integrity of block
		int ntries = 3;
		while ((ret = usbfifo_readblock ((UINT8) i, (UINT16*) (wfm + 256 * i))) < 0 && ntries--);
		
		if (ret < 0)
		{
			blocksok = 0;
		}
	}
	
	if (blocksok == 0)
	{
		//setCtrlVal(panelHandle, PANEL_TXT_LOG, "Read failure.");
		return;
	}
	
	// Reconstruct data and find offset for acquisition
	reconstructData (0); 
	offset = mean_array();
	
	// Timescale and parameters for main acquisition 
	setupTimescale ();
	
	if (writeParams () <= 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Param error.");
		return;
	}
	
	//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquiring...");
	
	// Number of waveforms to average
	GetCtrlVal (panelHandle, PANEL_AVERAGE, &acquisition_nr);
		
	// Get axis limits and units
	
	// TO DO: are these overwritten later?
	GetCtrlVal (panelHandle, PANEL_YMIN, &ymin);
	GetCtrlVal (panelHandle, PANEL_YMAX, &ymax);
	
	// Acquire k waveforms, loop and average if k > 1
	for (k = 0; k < acquisition_nr; k++) 
	{ 
		ret = usbfifo_acquire (&acq_result, 0);
		
		if (ret < 0)
		{
			//printf("Failed to run the acquisition sequence (did not get '.')");
			//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
			return;
		}
	
		// Read blocks of data from block numbers 0-63 (max 64 blocks and 16384 pts)
		blocksok = 1;
		nblocks = rec_len / 256;
		
		for (i = 0; i < nblocks; i++)
		{
			// Verify data integrity of block 
			int ntries = 3;
			while ((ret = usbfifo_readblock((UINT8) i, (UINT16*) (wfm + 256 * i))) < 0 && ntries--);
		
			if (ret < 0)
			{
				blocksok = 0;
			}
		}
	
		if (blocksok == 0)
		{
			//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Read failure.");
			return;
		}
	
		// Reconstruct data and account for offset
		reconstructData (offset);
 		
		// Store data, perform rho conversion
		for (i = 0; i < rec_len; i++)
		{ 
			if (i < 1024)
			{
				// Store pre-conversion values for debug purposes
				wfmf_debug[i] = wfmf[i];
				wfm_data_debug[i] = wfm_data[i];
			}
			
			// Convert first to Rho (baseline unit for conversions)
			wfm_data[i] = (double) (wfmf[i]) / (double) vampl - 1.0;
		}

		// Y Axis scaling based on selected unit
		switch (yUnits)
		{   
			case UNIT_MV:
			{
				// Values certain to be overwritten immediately
				ymax = -500;
				ymin = 500;
				
				for (i = 0; i < rec_len; i++)
				{
					wfm_data[i] *= ampl_factor;
					
					if (wfm_data[i] > ymax)
					{
						ymax = wfm_data[i];
					}
					
					if (wfm_data[i] < ymin)
					{
						ymin = wfm_data[i];
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
				
				for (i = 0; i < rec_len; i++)
				{
					wfm_data[i] += 1.0;
					
					if (wfm_data[i] < 0)
					{
						wfm_data[i] = 0;
					}
					
					if (wfm_data[i] > ymax)
					{
						ymax = wfm_data[i];
					}
					
					if (wfm_data[i] < ymin)
					{
						ymin = wfm_data[i];
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
				
				for (i = 0; i < rec_len; i++)
				{   
					// Make sure Rho values are in range for conversion
					if (wfm_data[i] <= -1)
					{
						wfm_data[i] = -0.999;
					}
					else if (wfm_data[i] >= 1)
					{
						wfm_data[i] = 0.999;
					}
					
					// Convert to impedance from Rho
					wfm_data[i] = (double) impedance * ((double) (1.0) + (double) (wfm_data[i])) / ((double) (1.0) - (double) (wfm_data[i]));
		   		    
					if(wfm_data[i] >= 500)
					{ 
						wfm_data[i] = 500.0;
					}
					else if(wfm_data[i] < 0)
					{ 
						wfm_data[i] = 0;
					}
					
					if (wfm_data[i] > ymax)
					{
						ymax = wfm_data[i];
					}
					
					if (wfm_data[i] < ymin)
					{
						ymin = wfm_data[i];
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
				
				for (i=0; i < rec_len; i++)
				{ 
					if (wfm_data[i] <= -1)
					{
						wfm_data[i] = -0.999;
					}
				
					if (wfm_data[i] >= 1)
					{
						wfm_data[i] = 0.999;
					}
					
					if (wfm_data[i] > ymax)
					{
						ymax = wfm_data[i];
					}
				
					if (wfm_data[i] < ymin)
					{
						ymin = wfm_data[i];
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

		// Set Y Axis limits if manual scaling
		GetCtrlVal (panelHandle, PANEL_AUTOSCALE, &auto_flag);

		// Manual scaling
		if (auto_flag == 0)
		{
			GetCtrlVal (panelHandle, PANEL_YMAX, &ymax);
			GetCtrlVal (panelHandle, PANEL_YMIN, &ymin);

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
		
		// Average waveforms
		for (i = 0; i< rec_len; i++)
		{
			wfm_data_ave[i] = (k* wfm_data_ave[i] + wfm_data[i])/(k+1);
		}
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
	
	// Horizontal units in time
	if (xUnits == UNIT_NS)
	{
		for (i = 0; i < rec_len; i++)
		{
			wfm_x[i] = timescale[i];
		}
	}
	// Horizontal units in meters
	else if (xUnits == UNIT_M) 
	{
		for (i = 0; i < rec_len; i++)
		{
			wfm_x[i] = dist_m[i];
		}
	}
	// Horizontal units in feet
	else 
	{
		for (i = 0; i < rec_len; i++)
		{
			wfm_x[i] = dist_ft[i];
		}
	}
	
	WfmActive = PlotXY (panelHandle, PANEL_WAVEFORM, wfm_x, wfm_data_ave, rec_len, VAL_DOUBLE, VAL_DOUBLE,
						plotType, VAL_SMALL_SOLID_SQUARE, VAL_SOLID, 1, MakeColor (113, 233, 70));
	
	// Trigger the DELAYED_DRAW
	RefreshGraph (panelHandle, PANEL_WAVEFORM);
	
	// Position cursors and update control reading
	updateCursors ();
}

//==============================================================================
// Other functions triggered by UIR callbacks



// Recall stored waveform
void recallWaveform (void)
{
	int status;
	
	// Disable timers during action
	SuspendTimerCallbacks ();
	
	// Select file
	char save_file[MAX_SAVE_FILE + 160];
	
	status = FileSelectPopup ("waveforms","*.ztdr","ZTDR Waveform (*.ztdr)","Select File to Retrieve", VAL_SELECT_BUTTON, 0, 0, 1, 1, save_file);

	// Don't crash if user cancels
	if (status == VAL_NO_FILE_SELECTED)
	{
		// Re-enable timers 
		ResumeTimerCallbacks ();
		
		return;
	}
	
	// Open file for reading
	int fd;
	
	fd = OpenFile (save_file, VAL_READ_WRITE, VAL_OPEN_AS_IS, VAL_ASCII);
	
	// Set up data buffer
	int n, i;
	char buf[128];
	int buf_len = 128;
	buf[0] = 0;
												
	
	// Read header row for environmental variables
	// TO DO: change names of local variables
	int xStored, yStored;
	float windowstart, windowsize;
	float ymin, ymax;
	float diel;
	double vc;
	
	// TO DO: why is it "n"?
	// Read header line
	n = ReadLine (fd, buf, buf_len - 1);
	sscanf (buf, "%d, %d, %f, %f, %f, %f, %f", &xStored, &yStored, &windowstart, &windowsize, &ymin, &ymax, &diel);
	vc = (double) 3E8 / sqrt (diel);
							   
	// Read X, Y values
	for(i = 0; i < rec_len; i++)
	{  
		float x, y;
		
		n = ReadLine (fd, buf, buf_len - 1);
		sscanf(buf,"%f, %f", &y, &x);

		wfm_ret[i] = (double) y;
		wfm_dist[i] = (double) x;
	}
	
	// Data read finished
	n = CloseFile(fd);
	
	// Convert data to usable values
	UINT32 windowsz;
	start_tm.time = (UINT32) windowstart;
	windowsz = (UINT32) windowsize;
	end_tm.time = start_tm.time + windowsz;
	
	// Set control values from stored waveform
	SetCtrlVal (panelHandle, PANEL_AUTOSCALE, 0);
	
	changeUnitX (xStored);
	
	SetCtrlVal (panelHandle, PANEL_START, (double) start_tm.time);
	SetCtrlVal (panelHandle, PANEL_WINDOW, (double) windowsz);
	SetCtrlVal (panelHandle, PANEL_DIEL, diel);
	SetCtrlVal (panelHandle, PANEL_YMAX, (double) ymax);
	SetCtrlVal (panelHandle, PANEL_YMIN, (double) ymin);
	SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_XNAME, x_label[xUnits]);
	
	// Remove any other recalled waveforms
	if (WfmStored)
	{
		DeleteGraphPlot (panelHandle, PANEL_WAVEFORM, WfmStored, VAL_IMMEDIATE_DRAW);
		WfmStored =0;
	}
	
	// Scale waveform acquisition window
	SetAxisRange (panelHandle, PANEL_WAVEFORM, VAL_AUTOSCALE, 0.0, 0.0, VAL_MANUAL, (double) ymin, (double) ymax);
	
	// Plot waveform
	WfmStored = PlotXY (panelHandle, PANEL_WAVEFORM, wfm_dist, wfm_ret, rec_len, VAL_DOUBLE, VAL_DOUBLE, 
						VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, MakeColor (233, 113, 233));

	// Dim controls
	SetCtrlAttribute (panelHandle, PANEL_WINDOW, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_START, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_ZOOM, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_YMAX, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_YMIN, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_AUTOSCALE, ATTR_DIMMED, 1);
	
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

// Store waveform to file as ZTDR (format = 1) or CSV (= 0)
void storeWaveform (int format)
{   
	// TO DO: finish cleaning this routine up!
	
	int status;

	// Disable timers during action
	status = SuspendTimerCallbacks ();
	
	// File setup
	char save_file[MAX_SAVE_FILE + 160];
	
	// TO DO: default filename, based on serial(?)
	char filename[40];
	
	// Save dialog
	if (format == 1)
	{
		status = sprintf (filename, ".ztdr");
		status = FileSelectPopup ("waveforms", filename, "ZTDR Waveform (*.ztdr)", "Select File to Save", VAL_SAVE_BUTTON, 0, 0, 1, 1, save_file);
	}
	else
	{
		status = sprintf (filename, ".csv");
		status = FileSelectPopup ("logs", filename, "CSV File (*.csv)", "Select File to Save", VAL_SAVE_BUTTON, 0, 0, 1, 1, save_file);
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
	
	// Set up data buffer	
	int i;
	char buf[128];
	buf[0] = 0;

	// Create header row (also includes unlabeled environmental variables)
	double windowstart, windowsize;
	double ymin, ymax;
	double diel;
	
	GetCtrlVal (panelHandle, PANEL_START, &windowstart);
	GetCtrlVal (panelHandle, PANEL_WINDOW, &windowsize);
	GetCtrlVal (panelHandle, PANEL_YMIN, &ymin);
	GetCtrlVal (panelHandle, PANEL_YMAX, &ymax);
	GetCtrlVal (panelHandle, PANEL_DIEL, &diel);;
	
	// Write header row
	if (format == 1)
	{
		// Header for .ZTDR
		status = sprintf (buf + strlen(buf), "%d, %d, %3.10f, %3.10f, %3.3f, %3.3f, %3.3f\n", yUnits, xUnits, windowstart, windowsize, ymin, ymax, diel);
	}
	else
	{
		// Header for .CSV
		status = sprintf (buf + strlen(buf), "%s, %s\n", y_label[yUnits], x_label[xUnits]);
	}
	
	status = WriteFile (fd, buf, strlen(buf));
	
	// Log X/Y data
	for (i = 0; i < rec_len; i++)
	{
		// Reset buffer
		buf[0] = 0;
	
		status = sprintf (buf + strlen(buf), "%3.10f, %3.10f\n", wfm_data[i], wfm_x[i]);
		
		status = WriteFile (fd, buf, strlen(buf));
	}
	
	status = CloseFile (fd);
	
	// Re-enable timers 
	status = ResumeTimerCallbacks ();
}
















// TO DO: WINDOWSIZE REFERENCES

// Cursor-based zoom
void zoom (void)
{   
	int status;
	
	double c1x, c1y, c2x, c2y;
	
	status = GetGraphCursor (panelHandle, PANEL_WAVEFORM, 1, &c1x, &c1y);
	status = GetGraphCursor (panelHandle, PANEL_WAVEFORM, 2, &c2x, &c2y);

	if (c1x < c2x)
	{
		status = SetCtrlVal(panelHandle, PANEL_START, c1x);
	}
	else
	{
		status = SetCtrlVal(panelHandle, PANEL_START, c2x);
	}
	
	// Update window size
	status = SetCtrlVal (panelHandle, PANEL_WINDOW, fabs (c2x - c1x));
}








// TO DO: below this, functions totally cleaned up and validated
// TO DO: these just need to be sorted

// Toggle dimming of controls based on autoscale
void changeAuto (void)
{
	int status;
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

// Change between dots and line
void changePlot (int unit)
{
	int status;																			   
	
	// Change unit selection and update menu
	if (unit == 0)
	{   
		// Dots
		plotType = 2L;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_PLOT_DOTS, ATTR_CHECKED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_PLOT_THINLINE, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_PLOT_FATLINE, ATTR_CHECKED, 0);
	}
	else if (unit == 1)
	{
		// Thin line
		plotType = 0L;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_PLOT_DOTS, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_PLOT_THINLINE, ATTR_CHECKED, 1);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_PLOT_FATLINE, ATTR_CHECKED, 0);
	}
	else if (unit == 2)
	{
		// Thick line
		plotType = 5L;
		
		status = SetMenuBarAttribute (menuHandle, MENUBAR_PLOT_DOTS, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_PLOT_THINLINE, ATTR_CHECKED, 0);
		status = SetMenuBarAttribute (menuHandle, MENUBAR_PLOT_FATLINE, ATTR_CHECKED, 1);
	}
}

// Change horizontal units
void changeUnitX (int unit)
{
	int status;																			   
	
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
	
	// Update X labels and limits
	status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_XNAME, x_label[xUnits]);
	
	status = SetCtrlAttribute (panelHandle, PANEL_START, ATTR_LABEL_TEXT, x_label_start[xUnits]);
	status = SetCtrlAttribute (panelHandle, PANEL_WINDOW, ATTR_LABEL_TEXT, x_label_windowsz[xUnits]);
	
	status = SetCtrlAttribute (panelHandle, PANEL_START, ATTR_MAX_VALUE, x_max_range[xUnits]);
	status = SetCtrlAttribute (panelHandle, PANEL_WINDOW, ATTR_MAX_VALUE, x_max_range[xUnits]);
	
	status = SetCtrlVal (panelHandle, PANEL_START, x_dflt_start[xUnits]);
	status = SetCtrlVal (panelHandle, PANEL_WINDOW, x_dflt_windowsz[xUnits]);
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
	
	status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_YNAME, y_label[yUnits]);
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

// Reset plot area and clear recalled waveform
void clearWaveform (void)
{
	int status;
	
	// TO DO: is this desired behavior?
	status = SetCtrlVal (panelHandle, PANEL_AUTOSCALE, 1);

	// Remove recalled waveform, if any
	if (WfmStored)
	{
		status = DeleteGraphPlot (panelHandle, PANEL_WAVEFORM, WfmStored, VAL_IMMEDIATE_DRAW);
		WfmStored = 0;
	}
	
	// Re-enable controls
	status = SetCtrlAttribute (panelHandle, PANEL_WINDOW, ATTR_DIMMED, 0);
	status = SetCtrlAttribute (panelHandle, PANEL_START, ATTR_DIMMED, 0);
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

// Print current waveform and controles
void printWaveform (void)
{
	int status;
	
	// Disable timers during action
	status = SuspendTimerCallbacks ();
	
	// Get timestamp of request
	int	month, day, year;
	int	hours, minutes, seconds;
	
	status = GetSystemDate (&month, &day, &year);
	status = GetSystemTime (&hours, &minutes, &seconds);
	
	char timestamp[64];
	
	status = sprintf (timestamp, "> DATE: %02d/%02d/%02d\n> TIME: %02d:%02d:%02d\n", month, day, year, hours, minutes, seconds);
	
	// Set optimal printer settings
	status = SetPrintAttribute (ATTR_PRINT_AREA_HEIGHT, VAL_USE_ENTIRE_PAPER);
	status = SetPrintAttribute (ATTR_PRINT_AREA_WIDTH, VAL_INTEGRAL_SCALE);

	// Show version, timestamp and print
	status = SetCtrlVal (panelHandle, PANEL_MESSAGES, timestamp);
	
	status = PrintPanel (panelHandle, "", 1, VAL_FULL_PANEL, 1);
	
	// Re-enable timers 
	status = ResumeTimerCallbacks ();
}

// Reset to default window
void resetZoom (void)
{
	SetCtrlVal (panelHandle, PANEL_START, x_dflt_start[xUnits]);
	SetCtrlVal (panelHandle, PANEL_WINDOW, x_dflt_windowsz[xUnits]);	
}

// Save waveform and controls to PNG
void savePNG (void)
{
	int status;
	
	// Disable timers during action
	status = SuspendTimerCallbacks ();

	// Select file to save
	char filename[64];
	char save_file[MAX_SAVE_FILE+160];
	status = sprintf (filename, ".png");
	status = FileSelectPopup ("images", filename, "PNG (*.png)", "Select File to Save", VAL_SAVE_BUTTON, 0, 0, 1, 1, save_file);

	// Don't attempt to save if user cancels
	if (status == VAL_NO_FILE_SELECTED)
	{
		// Re-enable timers 
		status = ResumeTimerCallbacks ();
		
		return;
	}

	// Get timestamp of request
	char timestamp[64];
	int	month, day, year;
	int	hours, minutes, seconds;
	
	status = GetSystemDate (&month, &day, &year);
	status = GetSystemTime (&hours, &minutes, &seconds);
	
	status = sprintf (timestamp, "> DATE: %02d/%02d/%02d\n> TIME: %02d:%02d:%02d\n", month, day, year, hours, minutes, seconds);
	
	// Show version, timestamp
	status = SetCtrlVal (panelHandle, PANEL_MESSAGES, timestamp);
	
	// Prepare image file
	int imageFile;
	status = GetPanelDisplayBitmap (panelHandle, VAL_FULL_PANEL, VAL_ENTIRE_OBJECT, &imageFile);
	status = SaveBitmapToPNGFile (imageFile, save_file);
	
	// Re-enable timers
	status = ResumeTimerCallbacks ();
	
	// TO DO: add some functionality for serial number?
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
	status = sprintf (buf, " %.2f %s, %.2f %s", c1x, x_short[xUnits], c1y, y_short[yUnits]);
	status = SetCtrlVal (panelHandle, PANEL_CURSOR1,  buf);

	// Cursor 2
	status = sprintf (buf, " %.2f %s, %.2f %s", c2x, x_short[xUnits], c2y, y_short[yUnits]);
	status = SetCtrlVal (panelHandle, PANEL_CURSOR2, buf);

	// Delta
	status = sprintf(buf, " %.2f %s, %.2f %s", c2x-c1x, x_short[xUnits], c2y-c1y, y_short[yUnits]);
	status = SetCtrlVal (panelHandle, PANEL_DELTA, buf);
}

// Update position of controls on resize
void updateSize (void)
{
	int status;
	int i, count;
	
	// Disable timers during action
	status = SuspendTimerCallbacks ();
	
	int newWidth, newHeight;
								
	status = GetPanelAttribute (panelHandle, ATTR_WIDTH, &newWidth);
	status = GetPanelAttribute (panelHandle, ATTR_HEIGHT, &newHeight);
	
	int xOffset, yOffset;
	
	// Calculate size change
	xOffset = newWidth - width;
	yOffset = newHeight - height;

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
   		  
	for (i = 0; i < count; i++)
	{   
		status = GetCtrlAttribute (panelHandle, GetCtrlArrayItem (rightHandle, i), ATTR_LEFT, &ctrlLeft);
		status = SetCtrlAttribute (panelHandle, GetCtrlArrayItem (rightHandle, i), ATTR_LEFT, ctrlLeft + xOffset);
	}
	
	// Resize message box									   ;
	status = GetCtrlAttribute (panelHandle, PANEL_MESSAGES, ATTR_HEIGHT, &ctrlHeight);
	status = SetCtrlAttribute (panelHandle, PANEL_MESSAGES, ATTR_HEIGHT, ctrlHeight + yOffset);
	
	// Move bottom control pane
	status = GetNumCtrlArrayItems (bottomHandle, &count);
   		  
	for (i = 0; i < count; i++)
	{
		status = GetCtrlAttribute (panelHandle, GetCtrlArrayItem (bottomHandle, i), ATTR_TOP, &ctrlTop);
		status = SetCtrlAttribute (panelHandle, GetCtrlArrayItem (bottomHandle, i), ATTR_TOP, ctrlTop + yOffset);
	}
	
	// Resize bottom control pane									   ;
	status = GetCtrlAttribute (panelHandle, PANEL_PANEBOTTOM, ATTR_WIDTH, &ctrlWidth);
	status = SetCtrlAttribute (panelHandle, PANEL_PANEBOTTOM, ATTR_WIDTH, ctrlWidth + xOffset);
	
	// Resize window start control
	status = GetCtrlAttribute (panelHandle, PANEL_START, ATTR_WIDTH, &ctrlWidth);
	status = SetCtrlAttribute (panelHandle, PANEL_START, ATTR_WIDTH, ctrlWidth + (xOffset / 3));
	
	// Reposition zoom controls
	status = GetCtrlAttribute (panelHandle, PANEL_ZOOM, ATTR_LEFT, &ctrlLeft);
	status = SetCtrlAttribute (panelHandle, PANEL_ZOOM, ATTR_LEFT, ctrlLeft + xOffset / 2);
	status = GetCtrlAttribute (panelHandle, PANEL_RESET, ATTR_LEFT, &ctrlLeft);
	status = SetCtrlAttribute (panelHandle, PANEL_RESET, ATTR_LEFT, ctrlLeft + xOffset / 2);
	
	// Resize window size control
	status = GetCtrlAttribute (panelHandle, PANEL_WINDOW, ATTR_WIDTH, &ctrlWidth);
	status = SetCtrlAttribute (panelHandle, PANEL_WINDOW, ATTR_WIDTH, ctrlWidth + (xOffset / 3));
	
	// Reposition window size control
	status = GetCtrlAttribute (panelHandle, PANEL_WINDOW, ATTR_LEFT, &ctrlLeft);
	status = SetCtrlAttribute (panelHandle, PANEL_WINDOW, ATTR_LEFT, ctrlLeft + (xOffset * 2/3));
	
	// Reposition window size label
	status = GetCtrlAttribute (panelHandle, PANEL_WINDOW, ATTR_LABEL_LEFT, &ctrlLeft);
	status = SetCtrlAttribute (panelHandle, PANEL_WINDOW, ATTR_LABEL_LEFT, ctrlLeft + (xOffset / 3));
	
	// Reposition window size display
	status = GetCtrlAttribute (panelHandle, PANEL_WINDOW, ATTR_DIG_DISP_LEFT, &ctrlLeft);
	status = SetCtrlAttribute (panelHandle, PANEL_WINDOW, ATTR_DIG_DISP_LEFT, ctrlLeft + (xOffset / 3));
	
	// Reposition K control
	status = GetCtrlAttribute (panelHandle, PANEL_DIEL, ATTR_LEFT, &ctrlLeft);
	status = SetCtrlAttribute (panelHandle, PANEL_DIEL, ATTR_LEFT, ctrlLeft + xOffset);
	
	// Write new window size to globals
	width = newWidth;
	height = newHeight;
	
	// Re-enable timers
	status = ResumeTimerCallbacks ();
}

// Write message to status
void writeMsgCal (int msg)
{
	if (msg == 0)
	{   
		// Start initial calibration message
		SetCtrlVal (panelHandle, PANEL_MESSAGES, "> Calibration ...");
	}
	else if (msg == 1)
	{
		// Indicate successful calibration
		
	}
}
