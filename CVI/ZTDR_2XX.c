//==============================================================================
//
// Title:		ZTDR_2XX.c
// Purpose:		ZTDR driver module and DLL functionality (v2.x.x)
//
// Copyright:	(c) 2018, HYPERLABS INC. All rights reserved.
//
//==============================================================================


//==============================================================================
// Include files

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "ftd2xx.h"
#include "constants.h"
#include "ZTDR_2XX.h"


//==============================================================================
// Global variables

// Device status
int 	deviceOpen = 0;
FT_HANDLE 	fifoHandle;				// ADUC FIFO lane
FT_HANDLE 	serialHandle;			// ADUC serial lane

// Device characteristics
int			deviceBps = 256000;		// expected host comm speed
char 		deviceID[16];			// device ID
char 		deviceCommspeed[10];	// commspeed of device

// Calibration
double 	calLevels[5];
double 	calThreshold;
int 	calIncrement;
double	vampl = 679;

// Acquisition environment
double	dielK = 2.25; // coax
int 	yUnits = 0; // mV
int 	xUnits = 0; // m
double	xStart = 0.0; // m
double	xEnd = 10.0; // m
double	xZero = 0.0; //m

// Number of data points acquired
UINT16 	recLen	= 1024;

// Waveform storage
double 	wfmDistFt[NPOINTS_MAX]; // distance (ft)
double 	wfmDistM[NPOINTS_MAX]; // distance (m)
double 	wfmTime[NPOINTS_MAX]; // time (ns)
double 	wfmX[NPOINTS_MAX]; // converted to selected units

UINT16 	wfm[NPOINTS_MAX]; // raw data from device
double 	wfmFilter[NPOINTS_MAX];	// filtered data from device
double  wfmDataRaw[NPOINTS_MAX]; // raw unconverted data for debug
double  wfmData[NPOINTS_MAX]; // converted to selected units
double  wfmAvg[NPOINTS_MAX]; // waveform after averaging

// Start/end time for device
timeinf startTime, endTime;

// USBFIFO parameters
UINT16 	calstart = 540;
UINT16 	calend = 3870;
UINT16 	stepcount = 6;
UINT16 	stepcountArray[5] = {4, 5, 6, 7, 8};
UINT16 	dac0val = 0, dac1val = 0, dac2val = 0;
UINT16 	strobecount = 2;


//==============================================================================
// Global functions (user-facing)

// Initialize and calibrate device (UIR agnostic)
__stdcall int ZTDR_Init (void)
{
	// Instrument initialization status
	int status;
	DWORD n;
	FT_STATUS serialStatus, fifoStatus;

	// If device already open, close it before re-opening
	if (deviceOpen)
	{
		ZTDR_CloseDevice ();

		deviceOpen = 0;
	}

	// Initialize individual FIFO and serial lanes
	fifoStatus = FT_OpenEx ("USBFIFOV1A", FT_OPEN_BY_SERIAL_NUMBER, &fifoHandle);
	serialStatus = FT_OpenEx ("USBFIFOV1B", FT_OPEN_BY_SERIAL_NUMBER, &serialHandle);

	// One or both lanes failed to initialize
	if (serialStatus != FT_OK || fifoStatus != FT_OK)
	{
		// Serial OK, FIFO failed
		if (serialStatus == FT_OK)
		{
			FT_Close (serialHandle);

			return -102;
		}
		// FIFO OK, serial failed
		else if (fifoStatus == FT_OK)
		{
			FT_Close (fifoHandle);

			return -103;
		}
		// Both serial and FIFO failed
		else
		{
			return -101;
		}
	}
	// Both serial and FIFO lanes initialized
	else
	{
		// Flag device as open
		deviceOpen = 1;

		// Set latency
		serialStatus = FT_SetLatencyTimer (serialHandle, 16);
		if (serialStatus != FT_OK)
		{
			return -110;
		}

		// Set device baud rate
		serialStatus = FT_SetBaudRate (serialHandle, deviceBps);
		if (serialStatus != FT_OK)
		{
			return -111;
		}

		// Set device data characteristics
		serialStatus = FT_SetDataCharacteristics (serialHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);

		if (serialStatus != FT_OK)
		{
			return -112;
		}

		// Set device flow control
		serialStatus = FT_SetFlowControl (serialHandle, FT_FLOW_NONE, 0, 0);

		if (serialStatus != FT_OK)
		{
			return -113;
		}

		// Set device timeouts
		serialStatus = FT_SetTimeouts (serialHandle, STD_TIMEOUT, STD_TIMEOUT);

		if (serialStatus != FT_OK)
		{
			return -114;
		}

		// Read device identification
		serialStatus = USBFIFO_WriteByte ('i');
		serialStatus = FT_Read (serialHandle, deviceID, 16, &n);

		if (strncmp (deviceID, "USBFIFO", 7) != 0)
		{
			return -115;
		}
		
		// Read device commspeed
		int numTries = 3;
		serialStatus = USBFIFO_WriteByte ('s');
		serialStatus = FT_Read (serialHandle, deviceCommspeed, 16, &n);
		
		// Retry up to 3 times if commspeed can't be read
		while (strncmp (deviceCommspeed, "256000", 6) != 0 && numTries > 0)
		{
			serialStatus = USBFIFO_WriteByte ('s');
			serialStatus = FT_Read (serialHandle, deviceCommspeed, 16, &n);	
			
			numTries--;
		}

		// Failed to get any commspeed
		if (numTries == 0)
		{
			return -116;
		}
		// Unexpected commspeed returned
		else if (strncmp (deviceCommspeed, "256000", 6) != 0)
		{
			return -117;
		}
	}

	// Dummy acquisition to ensure device initialization
	UINT8 acq_result;
	status = USBFIFO_Acquire (&acq_result, 0);
	
	// Initialization successful
	return 1;
}

// Full timebase calibration
__stdcall int ZTDR_CalTimebase (void)
{
	int status;
	int calStatus = 1;
	int i, j;

	// Set calibration window
	calstart = 0;
	calend = 4095;

	// Set increment for default 50 ns timescale
	calIncrement = (int) ((((double) CAL_WINDOW - (double) 0.0) *(double) 1.0 / (double) 1024.0 ) /
						  (((double) 50.0e-9) / (double) 65536.0));
	
	// Set start and end time to zero
	startTime.time = (UINT32) (0.0 / 50.0 * 0xFFFF);
	endTime.time = (UINT32) (0.0 / 50.0 * 0xFFFF);

	// Acquire data for each segment
	for (i = 0; i < 5; i++)
	{
		stepcount = stepcountArray[(UINT16) i];

		calStatus = ZTDR_PollDevice ();

		// Poll device failed
		if (calStatus < 0)
		{
			return calStatus;
		}

		status = ZTDR_ReconstructData (0, 1);

		// Find mean of waveform segment
		double val = 0.00;

		for (j = 0; j < recLen; j++)
		{
			val = val + wfmFilter[j];
		}

		calLevels[i] = val / recLen;
	}

	// Find optimal stepcount
	int idxMin = 0;
	int idxMax = 0;

	// Any valid data sets new max/min
	double max = 0.00;
	double min = 4095.0;

	// Cycle each data segment
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

	if ((min < 1) || (max > 4094))
	{
		// Timebase calibration fail
		return -201;
	}

	double val = ((max - min) / 4) + min;

	int idxOpt = 0;

	for (i = 4; i > 0; i--)
	{
		if (calLevels[i] < val)
		{
			idxOpt = i;
		}
	}

	// Update globals
	stepcount = stepcountArray[idxOpt];
	calThreshold = val;

	// Calibrate DAC
	status = ZTDR_CalDAC ();
	
	return calStatus;
}

// Calibrate amplitude
__stdcall int ZTDR_CalAmplitude (void)
{
	int status, i;

	// Timescale for 50 averaging 1024 samples at 0 ns
	startTime.time = 0;
	endTime.time = startTime.time;

	// Acquisition for offset calculation
	status = ZTDR_PollDevice ();
	status = ZTDR_ReconstructData (0, -1);

	// Find offset for acquisition
	double vstart = ZTDR_GetMean ();

	// Timescale for 50 ns calibration window
	startTime.time = (UINT32) (0.0 / 50.0 * 0xFFFF);
	endTime.time = (UINT32) (50.0 / 50.0 * 0xFFFF);

	// Main calibration acquisition
	status = ZTDR_PollDevice ();
	status = ZTDR_ReconstructData (0, -1);

	// Find the 50% crossing from vstart to approx. vstart + 1200 (step size)
	for (i = 0; wfmFilter[i] < (vstart + 400) && (i < 1024); i++)
	{
	}

	int i50 = i;

	// Could not locate edge
	if (i50 == 1023)
	{
		return -250;
	}

	// Calibrated vstart as average of points from 0 to (i50 - CAL_GUARD)
	int calInterval = (int) (CAL_GUARD / (CAL_WINDOW / 1024));

	int startIdx = i50 - calInterval;

	if (startIdx > 1)
	{
		double temp = 0;

		for (i = 0; i < startIdx; i++)
		{
			temp += wfmFilter[i];
		}

		vstart = temp / startIdx;
	}

	// Calibrated vend from reference cable
	double vend;
	int endIdx1 = i50 + calInterval;
	int endIdx2 = i50 + (calInterval * 2);

	// Reference goes beyond window
	if (endIdx1 >= 1024)
	{
		return -251;
	}

	double temp = 0;

	for (i = endIdx1; i < endIdx2; i++)
	{
		temp += wfmFilter[i];
	}

	vend = temp / (endIdx2 - endIdx1);

	vampl = vend - vstart;

	// Calibration successful
	return 1;
}

// Set acquisition environment
__stdcall int ZTDR_SetEnviron (int x, int y, double start, double end, double k, int rec)
{
	xUnits = x;
	yUnits = y;
	xStart = start;
	xEnd = end;
	dielK = k;
	recLen = (UINT16) rec;

	return 1;
}

// Set horizontal reference point
__stdcall int ZTDR_SetRefX (double x)
{
	int status;

	// Acquire reference point based on step to open
	if (x == -1.0)
	{
		// Switch to Rho to find 150% point
		int yUnitsPrev = yUnits;
		yUnits = 3;
		double idx150 = 0.6;

		// Acquire new waveform with no X offset
		xZero = 0.0;
		status = ZTDR_AcquireData (1);

		int i = 0;

		while (wfmAvg[i] < idx150 && i < 1024)
		{
			i++;
		}

		if (idx150 == 1024)
		{
			xZero = 0;

			return 0;
		}
		else
		{
			xZero = wfmX[i];
		}

		// Set to previous Y unit
		yUnits = yUnitsPrev;

		return 1;
	}
	// Set reference to absolute zero
	else if (x == 0.0)
	{
		xZero = 0.0;

		return 1;
	}
	// Set reference to specified value
	else if (x > 0.0)
	{
		xZero = x;

		return 1;
	}
	// Invalid horizontal reference specified
	else
	{
		return 0;
	}
}

// Acquisition (UIR agnostic)
__stdcall int ZTDR_AcquireData (int numAvg)
{
	int status, i;

	// Verify device
	if (deviceOpen != 1)
	{
		return -1;
	}

	// Timescale for averaging 1024 samples at 0 ns
	startTime.time = 0;
	endTime.time = startTime.time;

	// TODO #180: need dummy strobe or other fix for first-point dribble up

	// Acquisition for offset calculation
	status = ZTDR_PollDevice ();
	status = ZTDR_ReconstructData (0, -1);

	double offset = ZTDR_GetMean ();

	// Timescale for main acquisition
	status = ZTDR_QuantizeTimescale ();

	// Run once for each waveform
	for (int j = 0; j < numAvg; j++)
	{
		// Main acquisition
		status = ZTDR_PollDevice ();
		status = ZTDR_ReconstructData (offset, -1);

		// Store data, perform rho conversion
		for (i = 0; i < recLen; i++)
		{
			// Store raw data seperately
			wfmDataRaw[i] = (double) wfmFilter[i];

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

				break;
			}

			case UNIT_NORM:
			{
				for (i = 0; i < recLen; i++)
				{
					wfmData[i] += 1.0;
				}

				break;
			}

			case UNIT_OHM:
			{
				double impedance = 50;

				double tmp[1024];

				for (i = 0; i < recLen; i++)
				{
					tmp[i] = wfmData[i];

					// Constrain reflected value to 1.0 of incident step
					if (wfmData[i] >= 1.0)
					{
						wfmData[i] = 0.9999;
					}

					// Convert to impedance from Rho
					wfmData[i] = impedance * ((1.0 + wfmData[i]) / (1.0 - wfmData[i]));

					if(wfmData[i] >= 500)
					{
						wfmData[i] = 500.0;
					}
					else if(wfmData[i] < 0)
					{
						wfmData[i] = 0;
					}
				}

				break;
			}

			default: // RHO, data already in this unit
			{
				for (i = 0; i < recLen; i++)
				{
					// No further conversion necessary
				}

				break;
			}
		}

		// Average waveforms
		for (i = 0; i < recLen; i++)
		{
			wfmAvg[i] = (j * wfmAvg[i] + wfmData[i]) / (j + 1);
		}
	}

	// Horizontal units in time
	if (xUnits == UNIT_NS)
	{
		for (i = 0; i < recLen; i++)
		{
			wfmX[i] = wfmTime[i] - xZero;
		}
	}
	// Horizontal units in meters
	else if (xUnits == UNIT_M)
	{
		for (i = 0; i < recLen; i++)
		{
			wfmX[i] = wfmDistM[i] - xZero;
		}
	}
	// Horizontal units in feet
	else
	{
		for (i = 0; i < recLen; i++)
		{
			wfmX[i] = wfmDistFt[i] - xZero;
		}
	}

	return 1;
}

// Acquire horizontal value of specific point
__stdcall double ZTDR_FetchDataX (int idx)
{
	double val = wfmX[idx];

	return val;
}

// Acquire vertical value of specific point
__stdcall double ZTDR_FetchDataY (int idx)
{
	double val = wfmAvg[idx];

	return val;
}

// Dump data to CSV
__stdcall int ZTDR_DumpFile (char *filename)
{
	int status;

	// Open selected file for write
	FILE *fd;
	fd = fopen (filename, "w");

	// Set up data buffer;
	char buf[256];
	buf[0] = 0;

	// List unit names to avoid confusion
	char *nameY[] =
	{
		"mV",
		"Norm",
		"Ohm",
		"Rho"
	};

	char *nameX[] =
	{
		"m",
		"ft",
		"ns"
	};

	// Write header row
	status = sprintf (buf + strlen(buf), "%s, %s, %3.10f, %3.10f, %3.3f, %3.10f\n", nameX[xUnits], nameY[yUnits], xStart, xEnd, dielK, xZero);

	status = fwrite (buf, 1, strlen (buf), fd);

	// Log X/Y data
	for (int i = 0; i < recLen; i++)
	{
		// Reset buffer
		buf[0] = 0;

		status = sprintf (buf + strlen (buf), "%3.10f, %3.10f\n", wfmX[i], wfmAvg[i]);

		status = fwrite (buf, 1, strlen (buf), fd);
	}

	status = fclose(fd);

	return 1;
}


//==============================================================================
// Global functions (not user-facing)

// Close FTDI device
__stdcall void ZTDR_CloseDevice (void)
{
	FT_STATUS status;

	if (!deviceOpen)
	{
		return;
	}

	status = FT_Close (serialHandle);
	status = FT_Close (fifoHandle);

	deviceOpen = 0;
}

// Get full waveform data for use in all functions
__stdcall int ZTDR_PollDevice (void)
{
	int status = 0;

	// Write acquisition parameters to device
	FT_STATUS stat;
	static UINT8 params[NPARAMS];
	char ch;
	DWORD n;

	// Full parameter list
	params[IDX_FREERUN] = 0;
	params[IDX_STEPCNT_UPPER] = stepcount >> 8;
	params[IDX_STEPCNT_LOWER] = (UINT8) stepcount;
	params[IDX_RECLEN_UPPER] = recLen >> 8;
	params[IDX_RECLEN_LOWER] = (UINT8) recLen;
	params[IDX_DAC0_UPPER] = dac0val >> 8;
	params[IDX_DAC0_LOWER] = (UINT8) dac0val;
	params[IDX_DAC1_UPPER] = dac1val >> 8;
	params[IDX_DAC1_LOWER] = (UINT8) dac1val;
	params[IDX_DAC2_UPPER] = dac2val >> 8;
	params[IDX_DAC2_LOWER] = (UINT8) dac2val;
	params[IDX_CALSTART_UPPER] = calstart >> 8;
	params[IDX_CALSTART_LOWER] = (UINT8) calstart;
	params[IDX_CALEND_UPPER] = calend >> 8;
	params[IDX_CALEND_LOWER] = (UINT8) calend;
	params[IDX_TMSTART_B3] = startTime.time_b.b3;
	params[IDX_TMSTART_B2] = startTime.time_b.b2;
	params[IDX_TMSTART_B1] = startTime.time_b.b1;
	params[IDX_TMSTART_B0] = startTime.time_b.b0;
	params[IDX_TMEND_B3] = endTime.time_b.b3;
	params[IDX_TMEND_B2] = endTime.time_b.b2;
	params[IDX_TMEND_B1] = endTime.time_b.b1;
	params[IDX_TMEND_B0] = endTime.time_b.b0;
	params[IDX_OVERSAMPLE] = 0;
	params[IDX_STROBECNT_UPPER] = strobecount >> 8;
	params[IDX_STROBECNT_LOWER] = (UINT8) strobecount;

	// Send parameters to device
	stat = USBFIFO_WriteByte ('p');
	stat = FT_Write (serialHandle, params, NPARAMS, &n);
	ch = USBFIFO_ReadByte ();

	// Errors
	if (ch != '.')
	{
		// No record received
		return -400 - ch;
	}

	else if (n != NPARAMS)
	{
		// Incorrect number of params passed
		return -399;
	}

	// Break if error
	if (status < 0)
	{
		return status;
	}

	// Acquire waveform
	UINT8 acq_result;
	status = USBFIFO_Acquire (&acq_result, 0);

	// Verify block integrity
	// Blocks of 256 data points (max 256 blocks, 16,384 data points)
	int numBlocks = recLen / 256;

	// Verify integrity of all data blocks
	for (int i = 0; i < numBlocks; i++)
	{
		// Number of read attempts before failure
		int nTries = 3;

		// Verify data integrity of block
		while ((status = USBFIFO_ReadBlock ((UINT8) i, (UINT16*) wfm + (256 * i))) < 0 && nTries--);

		if (status != 1)
		{
			// Indicate which block failed (-30n for nth block)
			return (-300 - i);
		}
	}

	// Acquisition successful
	return 1;
}

// Data post-processing
__stdcall int ZTDR_ReconstructData (double offset, int filter)
{
	int i;

	// Increment between data points
	UINT32 incr;
	incr = (endTime.time - startTime.time) / recLen;

	timeinf currentTime;
	currentTime.time = startTime.time;

	// Set dielectric constant
	double vDiel = (double) 3E8 / sqrt (dielK);

	for (i = 0; i < recLen; i++)
	{
		wfmFilter[i] = (double) wfm[i] - offset;

		wfmTime[i] = ((double) currentTime.time) / ((double) 0xFFFF) * 50.0;
		wfmDistM[i] = wfmTime[i] * vDiel * 1E-9;
		wfmDistFt[i] = wfmTime[i] * vDiel * 1E-9 * M_TO_FT;

		currentTime.time += incr;
	}

	// Smooth data for better resolution during DAC calibration
	if (filter == 1)
	{
		for (i = FILTER_WIDTH / 2; i < recLen - FILTER_WIDTH / 2; i++)
		{
			double val;

			val = 0.00;

			for (int j = i - FILTER_WIDTH / 2; j < i + FILTER_WIDTH / 2; j++)
			{
				val = val + wfmFilter[j];
			}

			wfmFilter[i] = val / FILTER_WIDTH;
		}
	}

	// TODO #106: useful return
	return 1;
}

// Calculate mean of data array
__stdcall double ZTDR_GetMean (void)
{
	long val = 0;

	for (int i = 24; i < 1024; i++)
	{
		val += wfmFilter[i];
	}

	return ((double) val / (double) 1000.0);
}

// Calibrate DACs
__stdcall int ZTDR_CalDAC (void)
{
	int status, i;

	// Set calibration window
	calstart = 0;

	// Start, end at 0 ns
	startTime.time = (UINT32) (0.0 / 50.0 * 0xFFFF);
	endTime.time = (UINT32) (0.0 / 50.0 * 0xFFFF);

	status = ZTDR_PollDevice ();
	status = ZTDR_ReconstructData (0, 1);

	double calDiscLevel = ZTDR_FindDiscont ();

	for (i = 0; i < 10 && calDiscLevel < calThreshold && calstart <= 1100; i++)
	{
		calstart += 100;

		status = ZTDR_PollDevice ();
		status = ZTDR_ReconstructData (0, 1);

		calDiscLevel = ZTDR_FindDiscont ();
	}

	if (i==10)
	{
		calstart = CALSTART_DEFAULT;
	}

	for (i = 0; i < 16 && calDiscLevel > calThreshold; i++)
	{
		calstart -= 10;

		status = ZTDR_PollDevice ();
		status = ZTDR_ReconstructData (0, 1);

		calDiscLevel = ZTDR_FindDiscont ();
	}

	if (i == 16)
	{
		calstart = CALSTART_DEFAULT;
	}

	UINT16 calstartStored = calstart;
	int stepcountStored = stepcount;

	calstart = 2000;
	calend = 4094;

	stepcount = stepcount + 4;

	// Start, end at 0 ns
	startTime.time = (UINT32) (0.0 / 50.0 * 0xFFFF);
	endTime.time = (UINT32) (0.0 / 50.0 * 0xFFFF);

	status = ZTDR_PollDevice ();
	status = ZTDR_ReconstructData (0, 1);

	calDiscLevel = ZTDR_FindDiscont ();

	for (i = 0; i < 25 && calDiscLevel < calThreshold && calstart <= 4095; i++)
	{
		calstart = calstart + 100;

		status = ZTDR_PollDevice ();
		status = ZTDR_ReconstructData (0, 1);

		calDiscLevel = ZTDR_FindDiscont ();

		i++;
	}
	if (i == 25)
	{
		calend = CALEND_DEFAULT;
	}

	for (i = 0; i < 16 && calDiscLevel > calThreshold; i++)
	{
		calstart = calstart - 10;

		status = ZTDR_PollDevice ();
		status = ZTDR_ReconstructData (0, 1);

		calDiscLevel = ZTDR_FindDiscont ();
	}

	calend = calstart;

	if (i == 16)
	{
		calend = CALEND_DEFAULT;
	}

	calstart = calstartStored;

	stepcount = (UINT16) stepcountStored +1;

	return 1;
}

// TODO #106: function description
__stdcall double ZTDR_FindDiscont (void)
{
	double val = 0.0;

	for (int i = 0; i < recLen; i++)
	{
		val = val + wfmFilter[i];
	}

	val = val / recLen;

	return val;
}

// Quantize acquisition timescale
__stdcall int ZTDR_QuantizeTimescale (void)
{
	double start, end, diel;

	// If X Axis set to time
	if (xUnits == UNIT_NS)
	{
		start = xStart;
		end = xEnd;
	}

	// If distance selected, calculate based on K
	else
	{
		// Calculate distance in meters
		diel = (double) 3E8 / sqrt (dielK);
		start = xStart * 1E9 / diel;
		end = xEnd * 1E9 / diel;

		// Calculate distance in feet, if selected
		if (xUnits == UNIT_FT)
		{
			start = start / M_TO_FT;
			end = end / M_TO_FT;
		}
	}

	startTime.time = (UINT32) (start / 50.0 * 0xFFFF);
	endTime.time = (UINT32) (end / 50.0 * 0xFFFF);

	// TODO #106: useful return
	return 1;
}


//==============================================================================
// Global functions (USBFIFO)

// Read first byte from FTDI
__stdcall char USBFIFO_ReadByte (void)
{
	char ch;
	DWORD n;

	FT_Read (serialHandle, &ch, 1, &n);

	return ch;
}

// Send command string to FTDI
__stdcall int USBFIFO_WriteByte (char ch)
{
	FT_STATUS status;
	DWORD n;

	status = FT_Write (serialHandle, &ch, 1, &n);

	return (int)status;
}

// Acquire waveform from FDTI device
__stdcall int USBFIFO_Acquire (UINT8 *ret_val, UINT8 arg)
{
	char ch;
	FT_STATUS status;

	if (!deviceOpen)
	{
		return 0;
	}

	// Acquire ('a') from ADUC
	status = USBFIFO_WriteByte ('a');

	// Dummy argument ('0') to ADUC
	status = USBFIFO_WriteByte (arg);

	// Sets ADUC timeouts
	status = FT_SetTimeouts (serialHandle, 1000, 1000);

	*ret_val = USBFIFO_ReadByte ();

	// Check handshake ('.') to ensure successful acquisition
	ch = USBFIFO_ReadByte ();

	status = FT_SetTimeouts (serialHandle, STD_TIMEOUT, STD_TIMEOUT);

	if (ch != '.')
	{
		// Failed to acquire data; crashes
		return -1;
	}
	else
	{
		return 1;
	}

}

// Read data blocks of acquisition
__stdcall int USBFIFO_ReadBlock (UINT8 block_no, UINT16 *buf)
{
	char ch;
	DWORD n;
	int ret,i;

	// Define block length
	int blockLength = 256;
	UINT8 rawbuf8[2*blockLength];

	// Verify that device is open
	if (!deviceOpen)
	{
		return 0;
	}

	// Prepare to read block
	USBFIFO_WriteByte ('b');

	// Specify block to pull from
	USBFIFO_WriteByte (block_no);

	// Read from buffer
	ret = FT_Read (fifoHandle, rawbuf8, blockLength * 2, &n);

	// Verify that good data read from buffer ('f')
	ch = USBFIFO_ReadByte();

	if (ch == 'f')
	{
		// handle failure, clean out FIFO and return error
		while (FT_Read (fifoHandle, rawbuf8, 1, &n) == FT_OK && n == 1);
		return -1;
	}

	if ((int)n != 2 * blockLength)
	{
		return -2;
	}

	// NOTE: write data to software buffer
	for (i = 0; i < blockLength; i++)
	{
		buf[i] = (UINT16) (((UINT16) rawbuf8[2 * i + 1]) << 8) | ((UINT16) rawbuf8[2 * i]);
	}

	return 1;
}
