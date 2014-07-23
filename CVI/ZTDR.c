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

#include <ansi_c.h>
#include <userint.h>

#include "FTD2XX.h"

#include "callback.h"
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
#define UNIT_RHO 2 
#define UNIT_OHM 3

// Conversion
#define MtoFT 3.2808
#define FTtoM 0.3048


//==============================================================================
// Types


//==============================================================================
// Static global variables


//==============================================================================
// Static functions


//==============================================================================
// Global variables (roughly grouped by function)

// Initialization
int 	usb_opened = 0;

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

UINT16 	wfm[NPOINTS_MAX]; 			// Raw data
double 	wfmf[NPOINTS_MAX]; 			// Filtered data
double  wfm_data[NPOINTS_MAX];		// Converted to selected units
double  wfm_data_ave[NPOINTS_MAX]; 	// After waveform averaging

// Waveforms
static int WfmHandle;
static int WfmRecall;

// Time window
timeinf start_tm, end_tm;

// User interface and states
int 	panelHandle;


//==============================================================================
// Global functions (roughly grouped by functionality, order of call)

// Main startup function
void main (int argc, char *argv[])
{
	int i;

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

	// Set timescale prior to use
	setupTimescale ();

	// Set initial cursor positions
	SetGraphCursor (panelHandle, PANEL_WAVEFORM, 1, 33, 0);
	SetGraphCursor (panelHandle, PANEL_WAVEFORM, 2, 66, 0);

	openDevice ();
	
	// Initial time base calibration
	calTimebase();
	SetCtrlVal(panelHandle, PANEL_MESSAGES, "Calibration Done");

	RunUserInterface ();
	DiscardPanel (panelHandle);
}

// Scale time range of window for waveform acquisition
void setupTimescale (void)
{
	int HL1101_xaxis_val;
	int	HL1101_start;
	double 	HL1101_windowsz;
	double 	HL1101_diel;
	
	double val1, val2, vel;
	UINT32 windowsz;

	GetCtrlVal (panelHandle, PANEL_RING_HORIZONTAL, &HL1101_xaxis_val);
	GetCtrlVal (panelHandle, PANEL_NUM_STARTTM, &HL1101_start);
	GetCtrlVal (panelHandle, PANEL_NUM_WINDOWSZ, &HL1101_windowsz);
	GetCtrlVal (panelHandle, PANEL_NUM_WINDOWSZ, &HL1101_diel);

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
	vertCal ();
	setupTimescale ();
}


//==============================================================================
// Full time base calibration

// Set parameters for calibration
void calSetParams (void)
{
	// Changes stimulus drive to 80MHz on the CPLD
	UINT8 acq_result;
	int ret;
	double val;
	UINT32 windowsz;

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
	if (writeParams () <= 0)
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

	// Read blocks of data from block numbers 0-63 (max 64, with 16384 pts)
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

	GetCtrlVal (panelHandle, PANEL_NUM_YMIN, &ymin);
	GetCtrlVal (panelHandle, PANEL_NUM_YMAX, &ymax);
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
// Calibrate vertical axis waveform data

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
	
	if (writeParams () <= 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Param error.");
		return;
	}
	
	//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquiring...");
	
	// Number of waveforms to average
	GetCtrlVal (panelHandle, PANEL_NUM_WFMPERSEC, &acquisition_nr);
		
	// Get axis limits and units
	GetCtrlVal (panelHandle, PANEL_NUM_YMIN, &ymin);
	GetCtrlVal (panelHandle, PANEL_NUM_YMAX, &ymax);
	GetCtrlVal (panelHandle, PANEL_CHK_DOTS, &dots);

	// Get selected units
	GetCtrlVal (panelHandle, PANEL_RING, &HL1101_yaxis_val);
	GetCtrlVal (panelHandle, PANEL_RING_HORIZONTAL, &HL1101_xaxis_val);
	
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

				for (i=0; i<rec_len; i++)
				{
					wfm_data[i] *= ampl_factor;
				}

				ymin = -500.00;
				ymax =  500.00;

				break;

			case UNIT_NORM:

				for (i=0; i<rec_len; i++)
				{
					wfm_data[i] += 1.0;
				}

				ymin = 0.00;
				ymax =  2.00;

				break;

			case UNIT_OHM:

				for (i=0; i<rec_len; i++)
				{   	
					wfm_data[i] = (double) impedance * ((double) (1.0) + (double) (wfm_data[i])) / ((double) (1.0) - (double) (wfm_data[i]));
		   		    	
					if(wfm_data[i] >= 500)
					{ 
						wfm_data[i]=500.0;
					}
		  
					if(wfm_data[i] >= 500)
					{ 
						wfm_data[i] = 500.0;
					}
				}

				ymin =   0.00;
				ymax = 500.00;

				break;

			default: // RHO, data already in this unit

				ymin = -1.00;
				ymax =  1.00;

				break;

		}

		// Set Y Axis limits if manual scaling
		GetCtrlVal (panelHandle, PANEL_TOGGLEBUTTON, &auto_flag);

		// Manual scaling
		if (auto_flag==0)
		{
			GetCtrlVal (panelHandle, PANEL_NUM_YMAX, &ymax);
			GetCtrlVal (panelHandle, PANEL_NUM_YMIN, &ymin);

			// Compensate if min > max
			if((double) ymin >= (double) ymax)
			{
				ymin= (double) ymax - (double) 1.0;
			}
		}

		// Clear graph area
		DeleteGraphPlot (panelHandle, PANEL_WAVEFORM, -1, VAL_IMMEDIATE_DRAW);
		
		// Set Y axis range to be used in waveform 
		status = SetAxisRange (panelHandle, PANEL_WAVEFORM, VAL_AUTOSCALE, 0.0, 0.0, VAL_MANUAL, ymin, ymax);
		
		// Set appropriate values in UIR limits
		SetCtrlVal (panelHandle, PANEL_NUM_YMAX, ymax);
		SetCtrlVal (panelHandle, PANEL_NUM_YMIN, ymin);
		
		if (auto_flag)
		{
			SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_XAXIS, VAL_AUTOSCALE, 0, 0);
			SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_LEFT_YAXIS, VAL_AUTOSCALE, 0, 0);
			SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_RIGHT_YAXIS, VAL_AUTOSCALE, 0, 0);
		}
		else 
		{
			SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_LEFT_YAXIS, VAL_MANUAL, ymin, ymax);
			SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_RIGHT_YAXIS, VAL_MANUAL, ymin, ymax);
		}
	
		// Average waveforms
		for (i=0; i<1024; i++)
		{
			wfm_data_ave[i] = (k* wfm_data_ave[i] + wfm_data[i])/(k+1);
		}
	}
	
	// Horizontal units in time
	if (HL1101_xaxis_val == UNIT_NS)
	{
		WfmHandle = PlotXY (panelHandle, PANEL_WAVEFORM, timescale, wfm_data_ave, rec_len, VAL_DOUBLE, VAL_DOUBLE, 
							dots? VAL_SCATTER : VAL_FAT_LINE, VAL_SOLID_DIAMOND, VAL_SOLID, 1, dots? VAL_MAGENTA : VAL_MAGENTA);
	}

	// Horizontal units in meters
	else if (HL1101_xaxis_val == UNIT_M) 
	{
		WfmHandle = PlotXY (panelHandle, PANEL_WAVEFORM, dist_m, wfm_data_ave, rec_len, VAL_DOUBLE, VAL_DOUBLE, 
							dots? VAL_SCATTER : VAL_FAT_LINE, VAL_SOLID_DIAMOND, VAL_SOLID, 1, dots? VAL_MAGENTA : VAL_MAGENTA);
	}
	
	// Horizontal units in feet
	else 
	{
		WfmHandle = PlotXY (panelHandle, PANEL_WAVEFORM, dist_ft, wfm_data_ave, rec_len, VAL_DOUBLE, VAL_DOUBLE,
							dots? VAL_SCATTER : VAL_FAT_LINE, VAL_SOLID_DIAMOND, VAL_SOLID, 1, dots? VAL_MAGENTA : VAL_MAGENTA);
	}
	
	RefreshGraph (panelHandle, PANEL_WAVEFORM);
	
	// Position cursors and update control reading
	double c1x, c1y, c2x, c2y;
	c1x = c1y = c2x = c2y = 0;
	
	GetGraphCursor (panelHandle, PANEL_WAVEFORM, 1, &c1x, &c1y);
    GetGraphCursor (panelHandle, PANEL_WAVEFORM, 2, &c2x, &c2y);
	
	sprintf (buf, "%.2f    %.2f", c1x, c1y);
	SetCtrlVal (panelHandle, PANEL_STR_CURS1, buf);
	
	sprintf (buf, "%.2f    %.2f", c2x, c2y);
	SetCtrlVal (panelHandle, PANEL_STR_CURS2, buf);
	
	sprintf (buf, "%.2f    %.2f", c2x-c1x, c2y-c1y);
	SetCtrlVal (panelHandle, PANEL_STR_DELTA, buf);

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

	incr = (end_tm.time - start_tm.time)/rec_len;
	
	curt.time = start_tm.time;
	for (i=0;i<rec_len;i++)
	{	
		if (i < 1024)
		{
			myWfm[i] = wfmf[i];
		}
		
		timescale[i] = ((double) curt.time) / ((double) 0xFFFF)*50.0;
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
