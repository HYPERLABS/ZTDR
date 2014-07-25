//==============================================================================
//
// Title:		ZTDR.c
// Purpose:		Main functionality for HL11xx TDR instruments
//
// Created on:	7/22/2014 at 8:40:39 PM by Brian Doxey.
// Copyright:	HYPERLABS. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include <windows.h>
#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>

#include "FTD2XX.h"

#include "shared.h"
#include "usbfifo.h"
#include "ZTDR.h"


//==============================================================================
// Constants

// General
#define NPOINTS_MAX 16384
#define FILTER_WIDTH 4

// Calibration
#define CAL_WINDOW 10.0e-9
#define CAL_WINDOW_START 10 // ns
#define CAL_GUARD 0.5e-9
#define STEP_AMPL 800

// First sample in the record, no matter where the wfm is positioned
#define OFFSET_ACQ_POS 0

#define CALSTART_DEFAULT 540
#define CALEND_DEFAULT 3870

// Horizontal units
#define UNIT_M 0
#define UNIT_NS 1
#define UNIT_FT 2

// Vertical units
#define UNIT_MV 0
#define UNIT_NORM 1  
#define UNIT_OHM 2
#define UNIT_RHO 3

// Conversion
#define MtoFT 3.2808
#define FTtoM 0.3048

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

// Initialization
int 	usb_opened = 0;

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
      "ROUNDTRIP (ns)",
      "ROUNDTRIP (ft)" 
	 };

char *x_label_start[] =
     {
      "START (m)",
      "START (ns)" ,
      "START (ft)"
	 };
      
char *x_label_windowsz[] =
     {
      "WINDOW (m)",
      "WINDOW (ns)",
      "WINDOW (ft)" 
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
      50.0,
      33.3
	 };

float x_max_range[] =
     {
      400.0,
      2000.0,
      1332
	 };

char *label_dist[] =
     {
      "(X2-X1) (m) ",
      "(X2-X1) (ft)",
      "(X2-X1) (ns)" 
	 };


// Calibration
double 	calDiscLevel;
int 	calIncrement;
double 	calLevels[5];
double 	cal_threshold;
UINT16 	stepcount = 6;
UINT16 	stepcountArray[5] = {4, 5, 6, 7, 8};
double 	vampl = 679.0;

// Calibration parameters
UINT16	calstart_save = 540;
UINT16 	calstart = 540; UINT16 calend = 3870;
UINT16 	dac0val = 0, dac1val = 0, dac2val = 0;
int 	freerun_en = 0;
UINT16 	strobecount = 2;

// Transaction data
UINT16 	rec_len=1024;

// Horizontal values for each unit
double 	dist_ft[NPOINTS_MAX]; 
double 	dist_m[NPOINTS_MAX];
double 	timescale[NPOINTS_MAX];
double 	wfm_x[NPOINTS_MAX];			// Passed to graph
double 	wfm_dist[NPOINTS_MAX]; 		// Recalled waveform

// Vertical values in different modes
UINT16 	wfm[NPOINTS_MAX]; 			// Raw data
double 	wfmf[NPOINTS_MAX]; 			// Filtered data
double  wfm_data[NPOINTS_MAX];		// Converted to selected units
double  wfm_data_ave[NPOINTS_MAX]; 	// After waveform averaging
double 	wfm_ret[NPOINTS_MAX]; 		// Recalled waveform

double	wfm_rho_data[NPOINTS_MAX];
double	wfm_z_data[NPOINTS_MAX];

// Waveform IDs
static int WfmHandle;
static int WfmRecall;

// Time window
timeinf start_tm, end_tm;

// User interface and states
int 	panelHandle;


// TO DO: better solution
int		firstAcq = 0;


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

	// Load UI
	if ((panelHandle = LoadPanel (0, "ZTDR.uir", PANEL)) < 0)
	{
		return -1;
	}

	DisplayPanel (panelHandle);

	// Set 50 ns timescale
	calIncrement = (int) ((((double) CAL_WINDOW - (double) 0.0) *(double) 1.0 / (double) 1024.0 )/
						  (((double) 50e-9) / (double) 65536.0));

	// Set initial cursor positions
	SetGraphCursor (panelHandle, PANEL_WAVEFORM, 1, 3, 0);
	SetGraphCursor (panelHandle, PANEL_WAVEFORM, 2, 6, 0);

	setupTimescale ();
	
	openDevice ();
	
	setupTimescale ();
	
	RunUserInterface ();
	
	DiscardPanel (panelHandle);
	
	return 0;
}

// Scale time range of window for waveform acquisition
void setupTimescale (void)
{
	int HL1101_xaxis_val;
	double	HL1101_start;
	double 	HL1101_windowsz;
	double 	HL1101_diel;
	
	double val1, val2, vel;
	UINT32 windowsz;

	GetCtrlVal (panelHandle, PANEL_XUNITS, &HL1101_xaxis_val);
	GetCtrlVal (panelHandle, PANEL_NUM_STARTTM, &HL1101_start);
	GetCtrlVal (panelHandle, PANEL_NUM_WINDOWSZ, &HL1101_windowsz);
	GetCtrlVal (panelHandle, PANEL_NUM_DIELECTRIC, &HL1101_diel);

	// If X Axis set to time
	if (HL1101_xaxis_val == UNIT_NS)
	{
		val1 = HL1101_start;
		val2 = HL1101_windowsz;
	}

	// If distance selected, calculate based on K
	else
	{
		// Calculate distance in meters
		vel = (double) 3E8 / sqrt (HL1101_diel);
		val1 = HL1101_start * 1E9 / vel;
		val2 = HL1101_windowsz * 1E9 / vel;

		// Calculate distance in feet, if selected
		if (HL1101_xaxis_val == UNIT_FT)
		{
			val1 = val1 / MtoFT;
			val2 = val2 / MtoFT;
		}
	}

	start_tm.time = (UINT32) (val1 / 50.0*0xFFFF);
	windowsz = (UINT32) (val2 / 50.0*0xFFFF);

	end_tm.time = start_tm.time + windowsz;
}

// Open FTDI device
void openDevice (void)
{
	char buf[32];
	int hostbps;
	int devbps;

	hostbps = usbfifo_gethostbps ();

	usb_opened = usbfifo_open ();
	
	if (usb_opened)
	{
		usbfifo_getid (buf, 32);
		usbfifo_getcomspd (buf, 32);
		devbps = atoi (buf);
	}
}

// Calibrate timebase
void calTimebase (void)
{
	int i;

	SetCtrlVal (panelHandle, PANEL_MESSAGES, "Calibration in progress");
	
	calSetParams ();
	
	// Acquire data for each of 5 data segments
	for (i=0; i<5; i++)
	{
		stepcount = stepcountArray[(UINT16) i];
		
		calAcquireWaveform (i);
	}

	calFindStepcount ();

	calDAC ();
	
	// Calibrate vertical and set up time window
	setupTimescale ();
	vertCal ();
}


//==============================================================================
// Full time base calibration

// Set parameters for calibration
void calSetParams (void)
{
	// Changes stimulus drive to 80MHz on the CPLD
	UINT8 acq_result;
	int ret;

	calstart = 0;
	calend = 4095;

	if (!usb_opened)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Comm failure.");
		
		return;
	}

	// Acquire data to verify instrument is working
	ret = usbfifo_acquire (&acq_result, 0);

	if (ret < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
		
		return;
	}
	
	double val;
	UINT32 windowsz;
	
	// Set start and end time of cal window
	val = 0;
	start_tm.time = (UINT32) (val / 50.0*0xFFFF);
	
	val = 0;
	windowsz = (UINT32) (val / 50.0*0xFFFF);
	
	end_tm.time = start_tm.time + windowsz;
}

// Acquire waveform for calibration
void calAcquireWaveform (int calStepIndex)
{
	int ret = 0;
	int i,n;
	unsigned char buf[24];
	char ch;
	UINT8 acq_result;
	static char cbuf[32];
	double ymin, ymax;
	int dots;
	int nblocks;
	int blocksok;

	if (!usb_opened)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Comm failure.");
		return;
	}

	//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquiring...");

	// Write acquisition parameters
	if (writeParams () <= 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Param error.");
		return;
	} 

	// Run acquisition for calibration
	ret = usbfifo_acquire (&acq_result, 0);

	if (ret < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
		return;
	}

	// Read blocks of data from block numbers (max 64, with 16384 pts)
	blocksok = 1;
	nblocks = rec_len / 256;
	
	for (i = 0; i < nblocks; i++)
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
	
	calReconstructData ();
	
	// If -1, calibrate DAC
	if (calStepIndex == -1)
	{
		calFindDiscont ();	
	}
	else
	{
		calFindMean (calStepIndex);
	}

	// TO DO: graph after calibration?
}

// Reconstruct data segment for calibration
void calReconstructData (void)
{
	int i, j;
	timeinf curt;
	double val;
	UINT32 incr;
	
	incr = (end_tm.time - start_tm.time) / rec_len;
	
	curt.time = start_tm.time;
	
	for (i = 0; i < rec_len; i++)
	{						 		
		wfmf[i] = (double) wfm[i];
		timescale[i] = ((double) curt.time) / ((double) 0xFFFF) * 50.0;
		curt.time += incr;
	}
	
	// Smooth data for better resolution
	for (i = FILTER_WIDTH / 2; i < rec_len - FILTER_WIDTH / 2; i++)
	{
		val = 0;
		for (j = i - FILTER_WIDTH / 2; j < i + FILTER_WIDTH / 2; j++)
		{
			val = val + wfmf[j];
		}
		
		wfmf[i] = val / FILTER_WIDTH;
	}
}

// Find mean of waveform segment
void calFindMean (int calStepIndex)
{
	int i;
	double val;

	val = 0;
	
	for (i=0; i < rec_len; i++)
	{
		val = val + wfmf[i];
	}
	
	val = val / rec_len;
	
	calLevels[calStepIndex] = val;
}

// Find optimal step count
void calFindStepcount (void)
{
	int i, idx_min, idx_max, opt_idx;
	double val, min, max;

	// Set so any good data sets new max/min
	max = 0;
	min = (double) 4095;

	idx_min = 0;
	idx_max = 0;

	// Cycle each of 5 data segments
	for (i = 0; i < 5; i++)
	{
		if (calLevels[i] < min)
		{
			min = calLevels[i];
			idx_min = i;
		}
		if (calLevels[i] > max)
		{
			max = calLevels[i];
			idx_max = i;
		}
	}

	if ((min < 1) || (max > 4094))
	{
		// TO DO: better message here
		SetCtrlVal (panelHandle, PANEL_MESSAGES, "Calibration failed");
	}

	val = (max - min) / 4 + min;

	opt_idx = 0;
	
	for (i = 4; i > 0; i--)
	{
		if (calLevels[i] < val)
		{
			opt_idx = i;
		}
	}
	
	if (opt_idx > 0)
	{
		opt_idx = opt_idx - 1;
	}
	
	stepcount = stepcountArray[opt_idx];

	stepcount = 6;

	cal_threshold = val;
}

// TO DO: description of routine
void calDAC (void)
{
	char dispStr[10];

	int i;
	int stepcount_save;
	
	calstart = 0;

	calSetupTimescale ();
	
	calAcquireWaveform (-1);
	
	i = 0;
	
	while ((calDiscLevel < cal_threshold) && (i < 10) && (calstart <= 1100))
	{
		calstart = calstart + 100;
		calAcquireWaveform (-1);
		i++;
	}
	
	if (i==10)
	{
		calstart = CALSTART_DEFAULT;
	}

	i = 0;

	while ((calDiscLevel > cal_threshold) && (i < 16))
	{
		calstart = calstart - 10;
		calAcquireWaveform (-1);
		i++;
	}
	
	if (i == 16)
	{
		calstart = CALSTART_DEFAULT;
	}
	
	calstart_save = calstart;
	
	calend = 4094;
	calstart = 2000;

	stepcount_save = stepcount;
	stepcount = stepcount + 4;

	calSetupTimescale ();
	calAcquireWaveform (-1);
	
	i = 0;
	
	while ((calDiscLevel < cal_threshold) && (i < 25) && (calstart <= 4095))
	{
		calstart = calstart + 100;
		calAcquireWaveform (-1);
		i++;
	}
	if (i == 25)
	{
		calend = CALEND_DEFAULT;
	}
	i = 0;

	while ((calDiscLevel > cal_threshold) && (i < 16))
	{
		calstart = calstart - 10;
		calAcquireWaveform (-1);
		i++;
	}

	calend = calstart;
	if (i == 16)
	{
		calend = CALEND_DEFAULT;
	}
	calstart = calstart_save;

	stepcount = (UINT16) stepcount_save +1;

}

// Set timescale for full calibration
void calSetupTimescale (void)
{
	double val;
	UINT32 windowsz;
	
	// Start at 0 ns
	val = 0;
	start_tm.time = (UINT32) (val / 50.0*0xFFFF);

	// Window size of 0 ns
	val = 0;
	windowsz = (UINT32) (val / 50.0*0xFFFF);
	
	end_tm.time = start_tm.time + windowsz;
}

// TO DO: function description
void calFindDiscont (void)
{
	int i, j;

	char dispStr[10];

	calDiscLevel = 0;

	j = 0;

	for (i = 0; i < rec_len; i++)
	{
		calDiscLevel = calDiscLevel + wfmf[i];
	}

	calDiscLevel = calDiscLevel / rec_len;
}


//==============================================================================
// Calibrate vertical axis waveform data

// Calibrate vertical axis
void vertCal (void)
{
	int ret = 0;
	int i,n;
	unsigned char buf[24];
	char ch;
	UINT8 acq_result;
	static char cbuf[32];
	double ymin, ymax;

	int dots;
	int nblocks;
	int blocksok;
	double ymind, ymaxd;
	double vstart, vend;
	int tempID, tempID2, calInterval, i50;
	double temp;

	if (!usb_opened)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Comm failure.");
		return;
	}

	// Calculate offset of waveform by taking and averaging sample at 0 ns 
	vertCalOffset (CAL_WINDOW_START);

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

	// Read blocks of data from block numbers 0-63 (max 64, with 16384 pts)
	blocksok = 1;
	nblocks = rec_len / 256;
	
	for (i = 0; i < nblocks; i++)
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
	vstart = mean_array ();

	// Timescale and parameters for main acquisition
	vertCalTimescale ();

	// Write the acquisition parameters    
	if (vertCalWriteParams () <= 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Param error.");
		return;
	}

	// Acquire data 
	ret = usbfifo_acquire(&acq_result, 0);
	
	if (ret < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
		return;
	}

	// Read blocks of data from block numbers 0-63 (16384 pts)
	blocksok = 1;
	nblocks = rec_len / 256;
	for (i = 0; i < nblocks; i++)
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

	GetCtrlVal (panelHandle, PANEL_YMIN, &ymin);
	GetCtrlVal (panelHandle, PANEL_YMAX, &ymax);
	GetCtrlVal (panelHandle, PANEL_CHK_DOTS, &dots);

	reconstructData (0);
	
	// Find the 50% crossing from vstart to approx. vstart + 1200 (step size)
	i=0;

	while (wfmf[i] < (vstart + 400.0) && (i <= 1022))
	{
		i = i + 1;
	}

	i50 = i;

	// Compute a calibrated vstart as average of points from 0 to (i50 - CAL_GUARD) at calIncrement
	// Normalize calIncrement to waveform index
	calInterval = (int) (CAL_GUARD / (CAL_WINDOW / 1024));
	
	tempID = i50 - calInterval;

	if (tempID > 1)
	{
		temp = 0;
		for (i=0; i<tempID; i++)
		{
			temp += wfmf[i];
		}
		vstart = temp / tempID;
	}

	// Compute calibrated vend as average over 1ns at i50 + 2 * CAL_GUARD at calIncrement
	tempID = i50 + calInterval;
	
	if (tempID > 1023)
	{
		tempID = 1023;
	}

	tempID2 = i50 + 3 * calInterval;
	
	if (tempID2 > 1023)
	{
		tempID2 = 1023;
	}

	temp = 0;
	
	for (i = tempID; i < tempID2; i++)
	{
		temp += wfmf[i];
	}
	
	vend = temp / (tempID2 - tempID);

	vampl = vend - vstart;
	
	// TO DO: plot waveform here? 
	// TO DO: don't do regular cal if doing it here
}

// Set timescale and calculate offset at 0 ns
void vertCalOffset (double windowStart)
{
	double val;
	UINT32 windowsz;
	
	start_tm.time = (UINT32) (windowStart / 50.0*0xFFFF);
	
	val = 0;
	windowsz = (UINT32) (val / 50.0*0xFFFF);
	
	end_tm.time = start_tm.time + windowsz;
}

// Set timescale for vert cal
void vertCalTimescale (void)
{
	double val;
	UINT32 windowsz;

	val = 10;
	start_tm.time = (UINT32) (val / 50.0*0xFFFF);
	
	val = 10;
	windowsz = (UINT32) (val / 50.0*0xFFFF);
	
	end_tm.time = start_tm.time + windowsz;
}

// Write parameters for vertCal
int vertCalWriteParams (void)
{
	int ret;

	ret = usbfifo_setparams (0, calstart, calend, start_tm, end_tm,
							 stepcount, strobecount, 0, 1024, dac0val, dac1val, dac2val);
	
	if (ret < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Params failed.");
		return 0;
	}
	else
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Params written.");
		return 1;
	}			 
}


//==============================================================================
// General acquisition and processing

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
	
	int dots;
	double ymax, ymin;
	int nblocks;
	int blocksok;
	
	double impedance = 50;
	double ampl_factor = 250.0;
	
	int	HL1101_xaxis_val, HL1101_yaxis_val;
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
	vertCalOffset (OFFSET_ACQ_POS);
	
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
			blocksok = 0;
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
	
	if (firstAcq == 0)
	{
		calTimebase ();
		
		firstAcq = 1;
		SetCtrlVal(panelHandle, PANEL_MESSAGES, "Calibration Done");
	}
	
	if (writeParams () <= 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Param error.");
		return;
	}
	
	//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquiring...");
	
	// Number of waveforms to average
	GetCtrlVal (panelHandle, PANEL_NUM_WFMPERSEC, &acquisition_nr);
		
	// Get axis limits and units
	
	// TO DO: are these overwritten later?
	GetCtrlVal (panelHandle, PANEL_YMIN, &ymin);
	GetCtrlVal (panelHandle, PANEL_YMAX, &ymax);
	GetCtrlVal (panelHandle, PANEL_CHK_DOTS, &dots);

	// Get selected units
	GetCtrlVal (panelHandle, PANEL_YUNITS, &HL1101_yaxis_val);
	GetCtrlVal (panelHandle, PANEL_XUNITS, &HL1101_xaxis_val);
	
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
		switch (HL1101_yaxis_val)
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
				ymin= (double) ymax - (double) 1.0;
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
	if (!WfmRecall)
	{
		SetAxisRange (panelHandle, PANEL_WAVEFORM, VAL_AUTOSCALE, 0.0, 0.0, VAL_MANUAL, ymin, ymax);
	}

	// Clear existing WfmHandle, don't affect recalled waveforms
	if (WfmHandle)
	{
		// Delay draw so there is no flicker before next waveform is plotted
		DeleteGraphPlot (panelHandle, PANEL_WAVEFORM, WfmHandle, VAL_DELAYED_DRAW);
	}
	
	// Horizontal units in time
	if (HL1101_xaxis_val == UNIT_NS)
	{
		for (i = 0; i < rec_len; i++)
		{
			wfm_x[i] = timescale[i];
		}
	}
	// Horizontal units in meters
	else if (HL1101_xaxis_val == UNIT_M) 
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
	
	WfmHandle = PlotXY (panelHandle, PANEL_WAVEFORM, wfm_x, wfm_data_ave, rec_len, VAL_DOUBLE, VAL_DOUBLE,
						dots? VAL_SCATTER : VAL_FAT_LINE, VAL_SOLID_DIAMOND, VAL_SOLID, 1, dots? VAL_MAGENTA : VAL_MAGENTA);
	
	// Trigger the DELAYED_DRAW
	RefreshGraph (panelHandle, PANEL_WAVEFORM);
	
	// Position cursors and update control reading
	updateCursors ();
}

// Write parameters to device
int writeParams (void)
{
	int ret;

	ret = usbfifo_setparams ((UINT8) freerun_en, calstart, calend, start_tm, end_tm, stepcount, 
							 strobecount, 0, rec_len, dac0val, dac1val, dac2val);
	
	if (ret < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Params failed.");
		return 0;
	}
	else
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Params written.");
		return 1;
	}
}

// Reconstruct data into useable form
void reconstructData (double offset)
{
	int i;
	timeinf curt;
	UINT32 incr;
	// TO DO: is myWfm used?
	double myWfm[1024];
	double vel;
	double HL1101_diel;
	
	GetCtrlVal (panelHandle, PANEL_NUM_DIELECTRIC, &HL1101_diel);
	
	vel = (double) 3E8 / sqrt (HL1101_diel);

	incr = (end_tm.time - start_tm.time) / rec_len;
	
	curt.time = start_tm.time;
	
	for (i=0;i<rec_len;i++)
	{	
		wfmf[i] = (double) wfm[i] - offset;
		
		if (i < 1024)
		{
			myWfm[i] = wfmf[i];
		}
		
		timescale[i] = ((double) curt.time) / ((double) 0xFFFF) * 50.0;
		dist_m[i] = timescale[i] * vel * 1E-9;
		dist_ft[i] = timescale[i] * vel * 1E-9 * MtoFT;
		curt.time += incr;
	}
}

// Calculate offset from average 0
double mean_array (void)
{
	long temp;
	int i;
	temp = 0;
	for (i = 24; i < 1024; i++)
	{
		temp += wfmf[i];
	}

	return((double) temp / (double) 1000.0);
}


//==============================================================================
// Functions triggered by UIR callbacks

// Set horizontal environmental variables
void changeUnitX (void)
{
	int status;
	int x_axis;
	
	GetCtrlVal (panelHandle, PANEL_XUNITS, &x_axis);
	
	status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_XNAME, x_label[x_axis]);
	
	status = SetCtrlAttribute (panelHandle, PANEL_NUM_STARTTM, ATTR_LABEL_TEXT, x_label_start[x_axis]);
	status = SetCtrlAttribute (panelHandle, PANEL_NUM_WINDOWSZ, ATTR_LABEL_TEXT, x_label_windowsz[x_axis]);
	
	status = SetCtrlAttribute (panelHandle, PANEL_NUM_STARTTM, ATTR_MAX_VALUE, x_max_range[x_axis]);
	status = SetCtrlAttribute (panelHandle, PANEL_NUM_WINDOWSZ, ATTR_MAX_VALUE, x_max_range[x_axis]);
	
	status = SetCtrlVal (panelHandle, PANEL_NUM_STARTTM, x_dflt_start[x_axis]);
	status = SetCtrlVal (panelHandle, PANEL_NUM_WINDOWSZ, x_dflt_windowsz[x_axis]);
	
	// TO DO: better label for DIST
	// status = SetCtrlAttribute (panelHandle, PANEL_*****, ATTR_LABEL_TEXT, label_dist[x_axis]);	
}

// Toggle dimming of controls based on autoscale
void setAuto (void)
{
	int v;
			
	GetCtrlVal (panelHandle, PANEL_AUTOSCALE, &v);
	
	if (v == 1)
	{
		SetCtrlAttribute (panelHandle, PANEL_YMAX, ATTR_DIMMED, 1);
		SetCtrlAttribute (panelHandle, PANEL_YMIN, ATTR_DIMMED, 1);
	}
	else
	{
		SetCtrlAttribute (panelHandle, PANEL_YMAX, ATTR_DIMMED, 0);
		SetCtrlAttribute (panelHandle, PANEL_YMIN, ATTR_DIMMED, 0);
	}
}

// Change vertical environmental variables
void changeUnitY (void)
{
	int status;
	int y_axis;
	
	GetCtrlVal (panelHandle, PANEL_YUNITS, &y_axis);

	// 
	status = SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_YNAME, y_label[y_axis]);
	
	// TO DO: move to definitions (like X)
	// Update environmental variables
	if (y_axis == UNIT_MV)
	{
		// Set manual scale defaults 
		SetCtrlVal (panelHandle, PANEL_YMAX, 250.00);
		SetCtrlVal (panelHandle, PANEL_YMIN, -250.00);
		
		// Set precision of Y axis
		SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_YPRECISION, 0);
	}
	else if (y_axis == UNIT_NORM)
	{
		// Set manual scale defaults 
		SetCtrlVal (panelHandle, PANEL_YMAX, 2.00);
		SetCtrlVal (panelHandle, PANEL_YMIN, 0.00);
		
		// Set precision of Y axis
		SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_YPRECISION, 2);
	}
	else if (y_axis == UNIT_OHM)
	{
		// Set manual scale defaults 
		SetCtrlVal (panelHandle, PANEL_YMAX, 500.00);
		SetCtrlVal (panelHandle, PANEL_YMIN, 0.00);
		
		// Set precision of Y axis
		SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_YPRECISION, 0);
	}
	else if (y_axis == UNIT_RHO)
	{
		// Set manual scale defaults
		SetCtrlVal (panelHandle, PANEL_YMAX, 1.00);
		SetCtrlVal (panelHandle, PANEL_YMIN, -1.00);					   
		
		// Set precision of Y axis
		SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_YPRECISION, 2);
	}
}

// Recall stored waveform
void recallWaveform (void)
{
	int status;
	
	// Select file
	char save_file[MAX_SAVE_FILE + 160];
	
	status = FileSelectPopup ("waveforms","*.ztdr","ZTDR Waveform (*.ztdr)","Select File to Retrieve", VAL_SELECT_BUTTON, 0, 0, 1, 1, save_file);

	// Don't crash if user cancels
	if (status == VAL_NO_FILE_SELECTED)
	{
		return;
	}
	
	// Open file for reading
	int fd;
	
	fd = OpenFile (save_file, VAL_READ_WRITE, VAL_OPEN_AS_IS, VAL_ASCII);
	
	// Set up data buffer
	int n, i;
	char buf[1024];
	int no_data = rec_len;
	
	// Read vertical values (4 per row)
	for(i = 0; i < no_data; i += 4)
	{
		n = ReadFile(fd, buf,(unsigned) BUF_REC_LEN);

		float y1, y2, y3, y4;
		
		sscanf(buf," %f %f %f %f ", &y1, &y2, &y3, &y4);

		wfm_ret[i + 0] = (double) y1;
		wfm_ret[i + 1] = (double) y2;
		wfm_ret[i + 2] = (double) y3;
		wfm_ret[i + 3] = (double) y4;
	}
	
	// Read horizontal values (4 per row)
	for(i = 0; i < no_data; i += 4)
	{
		n = ReadFile (fd, buf,(unsigned) BUF_REC_LEN);

		float x1, x2, x3, x4;
		
		sscanf(buf," %f %f %f %f ", &x1, &x2, &x3, &x4);

		wfm_dist[i + 0] = (double) x1;
		wfm_dist[i + 1] = (double) x2;
		wfm_dist[i + 2] = (double) x3;
		wfm_dist[i + 3] = (double) x4;
	}

	// Read X limits, units, K
	float windowstart, windowsize;
	int x_axis;
	float diel;
	double vc;
	n = ReadFile (fd, buf, (unsigned) BUF_REC_LEN);
	sscanf (buf," %f %f %d %e", &windowstart, &windowsize, &x_axis, &diel);
	vc = (double) 3E8 / sqrt (diel);
	
	// Read Y limits, units
	int y_axis;
	float ymin, ymax;
	n = ReadFile (fd,buf,(unsigned) BUF_REC_LEN);
	sscanf (buf," %d %e %e ", &y_axis, &ymin, &ymax); 
	
	// Data read finished
	n=CloseFile(fd);
	
	// Convert data to usable values
	UINT32 windowsz;
	start_tm.time = (UINT32) windowstart;
	windowsz = (UINT32) windowsize;
	end_tm.time = start_tm.time + windowsz;
	
	// Set control values from stored waveform
	SetCtrlVal (panelHandle, PANEL_AUTOSCALE, 0);
	SetCtrlVal (panelHandle, PANEL_XUNITS, x_axis);
	SetCtrlVal (panelHandle, PANEL_NUM_STARTTM, (double) start_tm.time);
	SetCtrlVal (panelHandle, PANEL_NUM_WINDOWSZ, (double) windowsz);
	SetCtrlVal (panelHandle, PANEL_NUM_DIELECTRIC, diel);
	SetCtrlVal (panelHandle, PANEL_YUNITS, y_axis);
	SetCtrlVal (panelHandle, PANEL_YMAX, (double) ymax);
	SetCtrlVal (panelHandle, PANEL_YMIN, (double) ymin);
	SetCtrlAttribute (panelHandle, PANEL_WAVEFORM, ATTR_XNAME, x_label[x_axis]);
	
	// Remove any other recalled waveforms
	if (WfmRecall)
	{
		DeleteGraphPlot (panelHandle, PANEL_WAVEFORM, WfmRecall, VAL_IMMEDIATE_DRAW);
	}
	
	// Scale waveform acquisition window
	SetAxisRange (panelHandle, PANEL_WAVEFORM, VAL_AUTOSCALE, 0.0, 0.0, VAL_MANUAL, (double) ymin, (double) ymax);
	
	// Plot waveform
	WfmRecall = PlotXY (panelHandle, PANEL_WAVEFORM, wfm_dist, wfm_ret, rec_len, VAL_DOUBLE, VAL_DOUBLE, 
						VAL_FAT_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_GREEN);

	// Dim controls
	SetCtrlAttribute (panelHandle, PANEL_YUNITS, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_NUM_WINDOWSZ, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_NUM_STARTTM, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_COMMANDBUTTON, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_YMAX, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_YMIN, ATTR_DIMMED, 1);
	SetCtrlAttribute (panelHandle, PANEL_AUTOSCALE, ATTR_DIMMED, 1);

	return;
}

// Store waveform to file
void storeWaveform (void)
{   
	int status;
	
	// File setup
	char save_file[MAX_SAVE_FILE + 160];
	
	// TO DO: default filename, based on serial(?)
	char filename[40];
	sprintf (filename, ".ztdr");
	
	// Save dialog
	status = FileSelectPopup ("waveforms", filename, "ZTDR Waveform (*.ztdr)", "Select File to Save", VAL_SAVE_BUTTON, 0, 0, 1, 1, save_file);

	// Don't attempt to save if user cancels
	if (status == VAL_NO_FILE_SELECTED)
	{
		return;
	}	
	
	// Open selected file for write
	int fd;
	fd = OpenFile (save_file, VAL_READ_WRITE, VAL_TRUNCATE, VAL_ASCII);
	

	// Set up data buffer	
	int i, n;
	char buf[BUF_REC_LEN];
	int no_data = rec_len;
	
	// Store Y values of waveform (4 per row)
	for (i = 0; i < no_data; i += 4)
	{
		sprintf (buf," %e %e %e %e ", wfm_data[i], wfm_data[i+1], wfm_data[i+2], wfm_data[i+3]);
		buf[BUF_REC_LEN-1] = '\n';
		n = WriteFile (fd, buf, (unsigned) BUF_REC_LEN);
	}
	
	// Store X values of waveform (4 per row)
	for(i = 0; i < no_data; i += 4)
	{
		sprintf (buf," %e %e %e %e ", wfm_x[i], wfm_x[i+1], wfm_x[i+2], wfm_x[i+3]);
		buf[BUF_REC_LEN-1] = '\n';
		n = WriteFile (fd, buf, (unsigned) BUF_REC_LEN);
	}
	
	// Get control information to be saved
	double windowstart, windowsize;
	int x_axis, y_axis;
	double ymin, ymax;
	double diel;
	
	GetCtrlVal (panelHandle, PANEL_NUM_STARTTM, &windowstart);
	GetCtrlVal (panelHandle, PANEL_NUM_WINDOWSZ, &windowsize);
	GetCtrlVal (panelHandle, PANEL_XUNITS, &x_axis);
	GetCtrlVal (panelHandle, PANEL_YUNITS, &y_axis);
	GetCtrlVal (panelHandle, PANEL_YMIN, &ymin);
	GetCtrlVal (panelHandle, PANEL_YMAX, &ymax);
	GetCtrlVal (panelHandle, PANEL_NUM_DIELECTRIC, &diel);
	
	// Store Y limits, units, K
	sprintf (buf," %e %e %d %e",windowstart, windowsize, x_axis, diel);
	buf[BUF_REC_LEN-1] = '\n';
	n = WriteFile (fd, buf,(unsigned) BUF_REC_LEN);

	// Store Y limits, units
	sprintf(buf, " %d %e %e ", y_axis, ymin, ymax);
	buf[BUF_REC_LEN-1] = '\n';
	n = WriteFile (fd, buf,(unsigned) BUF_REC_LEN);

	n = CloseFile (fd);
}

// Reset plot area and clear recalled waveform
void resetWaveform (void)
{
	SetCtrlAttribute (panelHandle, PANEL_YUNITS, ATTR_DIMMED, 0);
	SetCtrlAttribute (panelHandle, PANEL_NUM_WINDOWSZ, ATTR_DIMMED, 0);
	SetCtrlAttribute (panelHandle, PANEL_NUM_STARTTM, ATTR_DIMMED, 0);
	SetCtrlAttribute (panelHandle, PANEL_COMMANDBUTTON, ATTR_DIMMED, 0);
	SetCtrlAttribute (panelHandle, PANEL_YMAX, ATTR_DIMMED, 0);
	SetCtrlAttribute (panelHandle, PANEL_YMIN, ATTR_DIMMED, 0);
	SetCtrlAttribute (panelHandle, PANEL_AUTOSCALE, ATTR_DIMMED, 0);

	// TO DO: is this desired behavior?
	SetCtrlVal (panelHandle, PANEL_AUTOSCALE, 1);

	// Remove any other recalled waveforms
	if (WfmRecall)
	{
		DeleteGraphPlot (panelHandle, PANEL_WAVEFORM, WfmRecall, VAL_IMMEDIATE_DRAW);
	}
}

// Update cursor readings
void updateCursors (void)
{
	double c1x, c1y, c2x, c2y;
	static char buf[128];

	c1x = c1y = c2x = c2y = 0;

	GetGraphCursor (panelHandle, PANEL_WAVEFORM, 1, &c1x, &c1y);
	GetGraphCursor (panelHandle, PANEL_WAVEFORM, 2, &c2x, &c2y);

	sprintf (buf, "%.2f    %.2f", c1x, c1y);
	SetCtrlVal (panelHandle, PANEL_STR_CURS1,  buf);

	sprintf (buf, "%.2f    %.2f", c2x, c2y);
	SetCtrlVal (panelHandle, PANEL_STR_CURS2, buf);

	sprintf(buf, "%.2f    %.2f", c2x-c1x, c2y-c1y);
	SetCtrlVal(panelHandle, PANEL_STR_DELTA, buf);
}

// Cursor-based zoom
void zoom (void)
{
	double c1x, c1y, c2x, c2y;
	
	GetGraphCursor (panelHandle, PANEL_WAVEFORM, 1, &c1x, &c1y);
	GetGraphCursor (panelHandle, PANEL_WAVEFORM, 2, &c2x, &c2y);

	if (c1x < c2x)
	{
		SetCtrlVal(panelHandle, PANEL_NUM_STARTTM, c1x);
	}
	else
	{
		SetCtrlVal(panelHandle, PANEL_NUM_STARTTM, c2x);
	}
	
	// Update window size
	SetCtrlVal (panelHandle, PANEL_NUM_WINDOWSZ, fabs (c2x - c1x));
}
