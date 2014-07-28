//==============================================================================
//
// Title:		driver.c
// Purpose:		ZTDR calibration and initialization routines
//
// Created on:	7/28/2014 at 9:10:43 AM by Brian Doxey.
// Copyright:	HYPERLABS. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include <ansi_c.h>

#include "FTD2XX.h"

#include "driver.h"
#include "main.h"
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

// Initialization
int 	usb_opened = 0;

//==============================================================================
// Global functions (sorted by function)

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

// Pass to global enviornmental variables
void setEnviron (int x, int y, double start, double end, double k)
{
	xUnits = x;
	yUnits = y;
	xStart = start;
	xEnd = end;
	diel = k;
}

// Scale time range of window for waveform acquisition
void setupTimescale (void)
{   
	int status;
	
	double val1, val2, vel;
	
	// If X Axis set to time
	if (xUnits == UNIT_NS)
	{
		val1 = xStart;
		val2 = xEnd;
	}

	// If distance selected, calculate based on K
	else
	{
		// Calculate distance in meters
		vel = (double) 3E8 / sqrt (diel);
		val1 = xStart * 1E9 / vel;
		val2 = xEnd * 1E9 / vel;

		// Calculate distance in feet, if selected
		if (xUnits == UNIT_FT)
		{
			val1 = val1 / MtoFT;
			val2 = val2 / MtoFT;
		}
	}

	start_tm.time = (UINT32) (val1 / 50.0*0xFFFF);
	end_tm.time = (UINT32) (val2 / 50.0*0xFFFF);
}

// Reconstruct data into useable form
void reconstructData (double offset)
{
	int i;
	
	double vel;
	
	vel = (double) 3E8 / sqrt (diel);

	UINT32 incr;
	timeinf curt;
	
	incr = (end_tm.time - start_tm.time) / rec_len;
	
	curt.time = start_tm.time;
	
	for (i=0;i<rec_len;i++)
	{	
		wfmFilter[i] = (double) wfm[i] - offset;
		
		wfmTime[i] = ((double) curt.time) / ((double) 0xFFFF) * 50.0;
		wfmDistM[i] = wfmTime[i] * vel * 1E-9;
		wfmDistFt[i] = wfmTime[i] * vel * 1E-9 * MtoFT;
		
		curt.time += incr;
	}
}

// Calculate offset from average 0
double meanArray (void)
{
	int i;
	
	long val;
	val = 0;
	
	for (i = 24; i < 1024; i++)
	{
		val += wfmFilter[i];
	}

	return ((double) val / (double) 1000.0);
}

// Calibrate timebase ("full" calibration)
void calTimebase (void)
{
	int i;
	
	calSetParams ();
	
	// Acquire data for each of 5 data segments
	for (i=0; i<5; i++)
	{
		stepcount = stepcountArray[(UINT16) i];
		
		calAcquireWaveform (i);
	}

	calFindStepcount ();

	calDAC ();
	
	// Set up time window and vertical calibration
	setupTimescale ();
	vertCal ();
	
	// TO DO: use return to determine whether cal was successful
	// return 1;
}

// TO DO: validate functions below







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
	int nblocks;
	int blocksok;

	if (!usb_opened)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Comm failure.");
		return;
	}

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
	min = 4095.0;

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
		SetCtrlVal (panelHandle, PANEL_MESSAGES, " FAILED.\n");
	}
	else
	{
		// TO DO: make more meaningful (i.e. cover more scenarios)
		SetCtrlVal (panelHandle, PANEL_MESSAGES, " DONE.\n");
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

	// Calculate offset of waveform by averaging samples at 0 ns 
	vertCalZero (CAL_WINDOW_START);

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

// Set timescale for vertCal at 0 ns
void vertCalZero (double windowStart)
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
