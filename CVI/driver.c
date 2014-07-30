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

#include "usbfifo.h"
#include "driver.h"

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

// Calibration
double 	calDiscLevel;
double 	calLevels[5];
double 	calThreshold;
int 	calIncrement;
double 	vampl = 679.0;

// TO DO: test scope; was a global in main.c, maybe needs to only be local
UINT16	calstart_save = 540;

//==============================================================================
// External global variables

// Control states needed outside UIR
double	diel = 2.25; // coax
int 	yUnits = 0; // mV
int 	xUnits = 0; // m
double	xStart = 0.0; // m
double	xEnd = 10.0; // m

// Number of data points acquired
UINT16 	recLen	= 1024;

// Waveform storage
double 	wfmDistFt[NPOINTS_MAX]; // distance (ft)
double 	wfmDistM[NPOINTS_MAX]; // distance (m)
double 	wfmTime[NPOINTS_MAX]; // time (ns)
double 	wfmX[NPOINTS_MAX]; // converted to selected units

UINT16 	wfm[NPOINTS_MAX]; // raw data from device
double 	wfmFilter[NPOINTS_MAX];	// filtered data from device
double  wfmData[NPOINTS_MAX]; // converted to selected units


//==============================================================================
// Global functions (sorted by function)

// Initialize device
int __stdcall initDevice (void)
{
	int i;

	// Initial values for maximum length of array
	for (i=0; i < NPOINTS_MAX; i++)
	{
		wfm[i] = 0;
		wfmFilter[i] = 0.0;
	}
	
	// Set increment for 50 ns timescale
	calIncrement = (int) ((((double) CAL_WINDOW - (double) 0.0) *(double) 1.0 / (double) 1024.0 )/
						  (((double) 50e-9) / (double) 65536.0));
	
	// Set up device
	setupTimescale ();
	openDevice ();
	
	// Calibration status
	// TO DO: use this
	int calStatus = 0; // Full timebase calibration
	calStatus = calTimebase ();
	
	return 1;
}

// Acquisition with no UIR
int __stdcall acquireWaveform (void)
{
	int status;
	int i;
	
	if (!usb_opened)
	{
		// Comm failure
		return -1;
	}
	
	// Set window to acquire offset at 0 ns
	vertCalZero (0);

	// Write the acquisition parameters
	if (vertCalWriteParams () <= 0)
	{
		// Write params failure (cal)
		return -2;
	}
	
	// Acquire data
	UINT8 acq_result;
	status = usbfifo_acquire (&acq_result, 0);
	
	if (status < 0)
	{
		// Acquisition failure (cal)
		return -3;
	}
	
	int blocksok;
	int nblocks;
	
	// Read blocks of data from block numbers 0-63 (max 64 blocks if 16384 pts)
	blocksok = 1;
	nblocks = recLen / 256;
	
	for (i=0; i < nblocks; i++)
	{
		// Verify data integrity of block
		int ntries = 3;
		
		while ((status = usbfifo_readblock ((UINT8) i, (UINT16*) (wfm + 256 * i))) < 0 && ntries--);
		
		if (status < 0)
		{
			blocksok = 0;
		}
	}
	
	if (blocksok == 0)
	{
		// Block read failure (cal)
		return -4;
	}
	
	// Reconstruct data and find offset for acquisition
	reconstructData (0);
	double offset;
	offset = meanArray();
	
	// Timescale and parameters for main acquisition
	setupTimescale ();
	
	if (writeParams () <= 0)
	{
		// Write params failure (main)
		return -5;
	}
	
	// Acquisition (no averaging)
	// TO DO: add averaging?
	status = usbfifo_acquire (&acq_result, 0);
		
	if (status < 0)
	{
		// Acquisition failure (main)
		return -6;
	}
	
	// Read blocks of data from block numbers 0-63 (max 64 blocks and 16384 pts)
	blocksok = 1;
	nblocks = recLen / 256;
	
	for (i = 0; i < nblocks; i++)
	{
		// Verify data integrity of block 
		int ntries = 3;
		while ((status = usbfifo_readblock((UINT8) i, (UINT16*) (wfm + 256 * i))) < 0 && ntries--);
	
		if (status < 0)
		{
			blocksok = 0;
		}
	}

	if (blocksok == 0)
	{
		// Block read failure (main)
		return -7;
	}
	
	// Reconstruct data and account for offset
	reconstructData (offset);
	
	// Store data, perform rho conversion
	for (i = 0; i < recLen; i++)
	{   
		// Convert first to Rho (baseline unit for conversions)
		wfmData[i] = (double) (wfmFilter[i]) / (double) vampl - 1.0;
	}
	
	// Y Axis scaling based on selected unit
	switch (yUnits)
	{   
		case UNIT_MV:
		{
			double ampl_factor = 250.0;
			
			for (i = 0; i < recLen; i++)
			{
				wfmData[i] *= ampl_factor;
			}
		}
		
		case UNIT_NORM:
		{
			for (i = 0; i < recLen; i++)
			{
				wfmData[i] += 1.0;
				
				if (wfmData[i] < 0)
				{
					wfmData[i] = 0;
				}
			}
		}
		
		case UNIT_OHM:
		{
			double impedance = 50;
			
			for (i = 0; i < recLen; i++)
			{   
				// Make sure Rho values are in range for conversion
				if (wfmData[i] <= -1)
				{
					wfmData[i] = -0.999;
				}
				else if (wfmData[i] >= 1)
				{
					wfmData[i] = 0.999;
				}
				
				// Convert to impedance from Rho
				wfmData[i] = (double) impedance * ((double) (1.0) + (double) (wfmData[i])) / ((double) (1.0) - (double) (wfmData[i]));
	   		    
				if(wfmData[i] >= 500)
				{ 
					wfmData[i] = 500.0;
				}
				else if(wfmData[i] < 0)
				{ 
					wfmData[i] = 0;
				}
			}
		}
		
		default: // RHO, data already in this unit
		{ 
			for (i=0; i < recLen; i++)
			{ 
				if (wfmData[i] <= -1)
				{
					wfmData[i] = -0.999;
				}
			
				if (wfmData[i] >= 1)
				{
					wfmData[i] = 0.999;
				}
			}
		}
	}
	
	// Horizontal units in time
	if (xUnits == UNIT_NS)
	{
		for (i = 0; i < recLen; i++)
		{
			wfmX[i] = wfmTime[i];
		}
	}
	// Horizontal units in meters
	else if (xUnits == UNIT_M) 
	{
		for (i = 0; i < recLen; i++)
		{
			wfmX[i] = wfmDistM[i];
		}
	}
	// Horizontal units in feet
	else 
	{
		for (i = 0; i < recLen; i++)
		{
			wfmX[i] = wfmDistFt[i];
		}
	}
	
	return 1;
}

// Open FTDI device
void __stdcall openDevice (void)
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
void __stdcall setEnviron (int x, int y, double start, double end, double k)
{
	xUnits = x;
	yUnits = y;
	xStart = start;
	xEnd = end;
	diel = k;
}

// Scale time range of window for waveform acquisition
void __stdcall setupTimescale (void)
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
	
	int egg =1 ;
}

// Reconstruct data into useable form
void __stdcall reconstructData (double offset)
{
	int i;
	
	double vel;
	
	vel = (double) 3E8 / sqrt (diel);

	UINT32 incr;
	timeinf curt;
	
	incr = (end_tm.time - start_tm.time) / recLen;
	
	curt.time = start_tm.time;
	
	for (i = 0; i < recLen; i++)
	{	
		wfmFilter[i] = (double) wfm[i] - offset;
		
		wfmTime[i] = ((double) curt.time) / ((double) 0xFFFF) * 50.0;
		wfmDistM[i] = wfmTime[i] * vel * 1E-9;
		wfmDistFt[i] = wfmTime[i] * vel * 1E-9 * MtoFT;
		
		curt.time += incr;
	}
}

// Calculate offset from average 0
double __stdcall meanArray (void)
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
int __stdcall calTimebase (void)
{
	int i;
	
	calSetParams ();
	
	// Acquire data for each of 5 data segments
	for (i=0; i<5; i++)
	{
		stepcount = stepcountArray[(UINT16) i];
		
		calAcquireWaveform (i);
	}

	int calStatus;
	calStatus = calFindStepcount ();

	calDAC ();
	
	// Set up time window and vertical calibration
	setupTimescale ();
	vertCal ();
	
	return calStatus;
}

// Set parameters for calibration
void __stdcall calSetParams (void)
{
	int status;
	
	// Set calibration window
	calstart = 0;
	calend = 4095;

	if (!usb_opened)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Comm failure."); 
		return;
	}

	UINT8 acq_result;
	
	// Acquire data to verify device is initialized
	status = usbfifo_acquire (&acq_result, 0); 
	
	if (status < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
		// TO DO: DO NOT RETURN HERE! THE FIRST ACQUISITION WILL FAIL EVERY TIME!
		// Need to avoid failing out of the next lines of code
		//return;
	}
	
	double val;
	
	// Set start and end time
	val = 0;
	start_tm.time = (UINT32) (val / 50.0*0xFFFF);
	
	val = 0;
	end_tm.time = (UINT32) (val / 50.0*0xFFFF);
	
	int egg = 1;
}

// Acquire waveform for calibration
void __stdcall calAcquireWaveform (int calStepIndex)
{
	int status = 0;
	int i;
	
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
	
	UINT8 acq_result;
	
	// Run acquisition for calibration
	status = usbfifo_acquire (&acq_result, 0);

	/*
	if (ret < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
		return;
	}
	*/
	
	int blocksok;
	int nblocks;
	
	// Read blocks of data from block numbers (max 64, with 16384 pts)
	blocksok = 1;
	nblocks = recLen / 256;
	
	for (i = 0; i < nblocks; i++)
	{
		// Verify data integrity of block
		int ntries = 3;
		
		while ((status = usbfifo_readblock ((UINT8) i, (UINT16*) (wfm + 256 * i))) < 0 && ntries--);

		if (status < 0)
		{   
			blocksok = 0;
		}
	}
	
	/*
	if (blocksok == 0)
	{
		//setCtrlVal(panelHandle, PANEL_TXT_LOG, "Read failure.");
		return;
	}
	*/
	
	calReconstructData ();
	
	// If -1, calibrate DAC
	if (calStepIndex == -1)
	{
		calFindDiscont ();	
	}
	// If >0, proceed through step cal
	else
	{
		calFindMean (calStepIndex);
	}
}

// Reconstruct data segment for calibration
void __stdcall calReconstructData (void)
{
	int i, j;
	
	UINT32 incr;
	incr = (end_tm.time - start_tm.time) / recLen;
	
	timeinf curt;
	curt.time = start_tm.time;
	
	for (i = 0; i < recLen; i++)
	{						 		
		wfmFilter[i] = (double) wfm[i];
		wfmTime[i] = ((double) curt.time) / ((double) 0xFFFF) * 50.0;
		curt.time += incr;
	}
	
	// Smooth data for better resolution
	for (i = FILTER_WIDTH / 2; i < recLen - FILTER_WIDTH / 2; i++)
	{
		double val;
		
		val = 0.00;
		
		for (j = i - FILTER_WIDTH / 2; j < i + FILTER_WIDTH / 2; j++)
		{
			val = val + wfmFilter[j];
		}
		
		wfmFilter[i] = val / FILTER_WIDTH;
	}
}

// Find mean of waveform segment
void __stdcall calFindMean (int calStepIndex)
{
	int i;
	double val;

	val = 0.00;
	
	for (i=0; i < recLen; i++)
	{
		val = val + wfmFilter[i];
	}
	
	val = val / recLen;
	
	calLevels[calStepIndex] = val;
}

// Find optimal step count
int __stdcall calFindStepcount (void)
{
	int i;

	int idxMin, idxMax;
	idxMin = 0;
	idxMax = 0;
	
	// Set so any good data sets new max/min
	double min, max;
	max = 0.00;
	min = 4095.0;

	// Cycle each of 5 data segments
	for (i = 0; i < 5; i++)
	{
		if (calLevels[i] < min)
		{
			min = calLevels[i];
			idxMin = i;
		}
		if (calLevels[i] > max)
		{
			max = calLevels[i];
			idxMax = i;
		}
	}

	int calStatus = 0;
	
	if ((min < 1) || (max > 4094))
	{
		// Calibration failed
		calStatus = -1;
	}
	else
	{
		// Calibration success
		calStatus = 1;
	}

	double val;
	val = (max - min) / 4 + min;

	int idxOpt;
	idxOpt = 0;
	
	for (i = 4; i > 0; i--)
	{
		if (calLevels[i] < val)
		{
			idxOpt = i;
		}
	}
	
	if (idxOpt > 0)
	{
		idxOpt = idxOpt - 1;
	}
	
	stepcount = stepcountArray[idxOpt];

	stepcount = 6;

	calThreshold = val;
	
	return calStatus;
}

// Calibrate DACs
void __stdcall calDAC (void)
{
	int i;
	
	calstart = 0;

	calSetupTimescale ();
	
	calAcquireWaveform (-1);
	
	i = 0;
	
	while ((calDiscLevel < calThreshold) && (i < 10) && (calstart <= 1100))
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

	while ((calDiscLevel > calThreshold) && (i < 16))
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

	int stepcountSave;
	
	stepcountSave = stepcount;
	stepcount = stepcount + 4;

	calSetupTimescale ();
	
	calAcquireWaveform (-1);
	
	i = 0;
	
	while ((calDiscLevel < calThreshold) && (i < 25) && (calstart <= 4095))
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

	while ((calDiscLevel > calThreshold) && (i < 16))
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

	stepcount = (UINT16) stepcountSave +1;
}

// Set timescale for full calibration
void __stdcall calSetupTimescale (void)
{
	double val;

	// Start at 0 ns
	val = 0;
	start_tm.time = (UINT32) (val / 50.0*0xFFFF);

	// End at 0 ns
	val = 0;
	end_tm.time = (UINT32) (val / 50.0*0xFFFF);
}

// TO DO: function description
void __stdcall calFindDiscont (void)
{
	int i;

	calDiscLevel = 0;

	for (i = 0; i < recLen; i++)
	{
		calDiscLevel = calDiscLevel + wfmFilter[i];
	}

	calDiscLevel = calDiscLevel / recLen;
}

// Write parameters to device
int __stdcall writeParams (void)
{
	int status;

	status = usbfifo_setparams ((UINT8) freerun_en, calstart, calend, start_tm, end_tm, stepcount,
								strobecount, 0, recLen, dac0val, dac1val, dac2val);
	
	int egg = 1;
	
	if (status < 0)
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
void __stdcall vertCal (void)
{
	int status;
	int i;
	
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
		// SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Param error.");
		// return;
	}

	// Acquire data 
	UINT8 acq_result;
	status = usbfifo_acquire (&acq_result, 0);
 
	if (status < 0)
	{
		// SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
		// return;
	}

	// Read blocks of data from block numbers 0-63 (max 64, with 16384 pts)
	int blocksok;
	int nblocks;
	blocksok = 1;
	nblocks = recLen / 256;
	
	for (i = 0; i < nblocks; i++)
	{
		// Verify data integrity of block
		int ntries = 3;
		while ((status = usbfifo_readblock ((UINT8) i, (UINT16*) (wfm + 256 * i))) < 0 && ntries--);

		if (status < 0)
		{
			blocksok = 0;
		}
	}

	if (blocksok == 0)
	{
		//setCtrlVal(panelHandle, PANEL_TXT_LOG, "Read failure.");
		return;
	}
	
	reconstructData (0);
	
	// Find offset for acquisition
	double vstart;
	vstart = meanArray ();

	// Timescale and parameters for main acquisition
	vertCalTimescale ();

	// Write the acquisition parameters    
	if (vertCalWriteParams () <= 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Param error.");
		//return;
	}

	// Acquire data 
	status = usbfifo_acquire(&acq_result, 0);
	
	if (status < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
		//return;
	}

	// Read blocks of data from block numbers 0-63 (16384 pts)
	blocksok = 1;
	nblocks = recLen / 256;
	for (i = 0; i < nblocks; i++)
	{
		// Verify data integrity of block 
		int ntries = 3;
		while ((status = usbfifo_readblock ((UINT8) i, (UINT16*) (wfm + 256 * i))) < 0 && ntries--);

		if (status < 0)
		{
			blocksok = 0;
		}
	}

	if (blocksok == 0)
	{
		//setCtrlVal(panelHandle, PANEL_TXT_LOG, "Read failure.");
		//return;
	}
	
	reconstructData (0);
	
	// Find the 50% crossing from vstart to approx. vstart + 1200 (step size)
	i=0;

	while (wfmFilter[i] < (vstart + 400.0) && (i <= 1022))
	{
		i = i + 1;
	}

	
	int i50;
	i50 = i;
	
	// Compute a calibrated vstart as average of points from 0 to (i50 - CAL_GUARD) at calIncrement
	// Normalize calIncrement to waveform index
	int calInterval;
	calInterval = (int) (CAL_GUARD / (CAL_WINDOW / 1024));
	
	int tempID;
	tempID = i50 - calInterval;
	
	double temp;

	if (tempID > 1)
	{
		temp = 0;
		
		for (i=0; i<tempID; i++)
		{
			temp += wfmFilter[i];
		}
		vstart = temp / tempID;
	}

	int tempID2;
	double vend;

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
		temp += wfmFilter[i];
	}
	
	vend = temp / (tempID2 - tempID);

	vampl = vend - vstart;
}

// Set timescale for vertCal at 0 ns
void __stdcall vertCalZero (double windowStart)
{
	double val;
	
	start_tm.time = (UINT32) (windowStart / 50.0*0xFFFF);

	val = 0;
	end_tm.time = start_tm.time + (UINT32) val;
}

// Set timescale for vert cal
void __stdcall vertCalTimescale (void)
{
	double val;

	val = 10;
	start_tm.time = (UINT32) (val / 50.0*0xFFFF);
	
	val = 10;
	end_tm.time = start_tm.time + ((UINT32) (val / 50.0*0xFFFF));
}

// Write parameters for vertCal 
int __stdcall vertCalWriteParams (void)
{
	int status;

	status = usbfifo_setparams (0, calstart, calend, start_tm, end_tm, stepcount, 
								strobecount, 0, recLen, dac0val, dac1val, dac2val);
	
	if (status < 0)
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
