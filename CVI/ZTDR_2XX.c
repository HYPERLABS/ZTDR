//==============================================================================
//
// Title:		ZTDR_2XX.c
// Purpose:		ZTDR driver module and DLL functionality (v2.x.x)
//
// Copyright:	(c) 2015, HYPERLABS INC. All rights reserved.
//
//==============================================================================


//==============================================================================
// Include files

#include <ansi_c.h>

#include "FTD2XX.h"

#include "constants.h"
#include "ZTDR_2XX.h"


//==============================================================================
// Global variables

// State of USB device
int 	usbOpened;




// TODO: clean these up

// USBFIFO functionality
FT_HANDLE 	dev_fifo_handle;		// ADUC FIFO lane
FT_HANDLE 	dev_serial_handle;		// ADUC serial lane

int			dev_hostbps = 256000;	// expectedhost comm speed

UINT8 	freerun_en = 0;				// unknown purpose, but always 0



// Debug results
char	dev_id[32];		// ID retrieved from device
int		dev_commspeed;	// Comm speed retrieved from device











// Calibration
double 	calDiscLevel;
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
timeinf start_tm, end_tm;


// USBFIFO functionality


// USBFIFO parameters
UINT16 	calstart = 540;
UINT16 	calend = 3870;
UINT16 	stepcount = 6;
UINT16 	stepcountArray[5] = {4, 5, 6, 7, 8};
UINT16 	dac0val = 0, dac1val = 0, dac2val = 0;
UINT16 	strobecount = 2;

char 	dev_comspdbuf[20];
char 	dev_idbuf[20];
int 	dev_opened = 0;


//==============================================================================
// Global functions (user-facing)

// Initialize and calibrate device (UIR agnostic)
__stdcall int initDevice (void)
{
	// Initial values for maximum length of array
	for (int i = 0; i < NPOINTS_MAX; i++)
	{
		wfm[i] = 0;
		wfmFilter[i] = 0.0;
	}

	// Open communication with USB device
	int initStatus = openDevice ();

	// Break on initilization failure
	if (initStatus < 0)
	{
		return initStatus;
	}
	// Continue with full timebase calibration
	else
	{
		// Set increment for default 50 ns timescale
		calIncrement = (int) ((((double) CAL_WINDOW - (double) 0.0) *(double) 1.0 / (double) 1024.0 )/
							  (((double) 50.0e-9) / (double) 65536.0));

		// TODO: remove if this is unnecessary
		// status = setupTimescale ();

		// Full timebase calibration
		int calStatus = calTimebase ();

		return calStatus;
	}
}

// Calibrate vertical axis
__stdcall int vertCal (void)
{
	int status, i;
	
	// Timescale for 50 averaging 1024 samples at 0 ns
	start_tm.time = 0.0;
	end_tm.time = start_tm.time;

	// Acquisition for offset calculation
	status = getData ();
	
	status = reconstructData (0, -1);
	
	// Find offset for acquisition
	double vstart;
	vstart = meanArray ();

	// Timescale for 50 ns calibration window
	double val = 0.0;
	start_tm.time = (UINT32) (val / 50.0 * 0xFFFF);

	val = 50.0;
	end_tm.time = (UINT32) (val / 50.0 * 0xFFFF);

	// Main calibration acquisition
	status = getData ();
	
	status = reconstructData (0, -1);

	// Find the 50% crossing from vstart to approx. vstart + 1200 (step size)
	i = 0;

	while (wfmFilter[i] < (vstart + 400.0) && (i <= 1022))
	{
		i = i + 1;
	}

	int i50 = i;

	// Compute a calibrated vstart as average of points from 0 to (i50 - CAL_GUARD) at calIncrement
	// Normalize calIncrement to waveform index
	int calInterval = (int) (CAL_GUARD * 0.5 / (CAL_WINDOW / 1024));

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

	tempID2 = i50 + 4 * calInterval;

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

	// TODO #106: useful return
	return 1;
} 

// Set acquisition environment
__stdcall int setEnviron (int x, int y, double start, double end, double k, int rec)
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
__stdcall int setRefX (double x)
{
	// Acquire reference point based on step to open
	if (x == -1.0)
	{
		// Switch to Rho to find 150% point
		int yUnitsPrev = yUnits;
		yUnits = 3;
		double idx150 = 0.6;

		// Acquire new waveform with no X offset
		xZero = 0.0;
		acquireWaveform (1);

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
__stdcall int acquireWaveform (int numAvg)
{
	int status, i;

	// Timescale for averaging 1024 samples at 0 ns
	start_tm.time = 0.0;
	end_tm.time = start_tm.time;

	// Acquisition for offset calculation
	status = getData ();
	
	status = reconstructData (0, -1);
	
	double offset = meanArray();

	// Timescale and parameters for main acquisition
	status = setupTimescale ();

	// Run once for each waveform
	for (int j = 0; j < numAvg; j++)
	{
		// TODO #175: need dummy strobe
		
		// Main acquisition
		status = getData ();
	
		status = reconstructData (offset, -1);
	
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
		for (int i = 0; i < recLen; i++)
		{
			wfmX[i] = wfmTime[i] - xZero;
		}
	}
	// Horizontal units in meters
	else if (xUnits == UNIT_M)
	{
		for (int i = 0; i < recLen; i++)
		{
			wfmX[i] = wfmDistM[i] - xZero;
		}
	}
	// Horizontal units in feet
	else
	{
		for (int i = 0; i < recLen; i++)
		{
			wfmX[i] = wfmDistFt[i] - xZero;
		}
	}

	return 1;
}

// Acquire horizontal data
__stdcall double fetchDataX (int idx)
{
	double val = wfmX[idx];

	return val;
}

// Acquire vertical data
__stdcall double fetchDataY (int idx)
{
	double val = wfmAvg[idx];

	return val;
}

// Dump data to CSV
__stdcall int dumpFile (char *filename)
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
	status = sprintf (buf + strlen(buf), "%s, %s, %3.10f, %3.10f, %3.3f, %3.10f\n", nameY[yUnits], nameX[xUnits], xStart, xEnd, dielK, xZero);

	status = fwrite (buf, 1, strlen (buf), fd);

	// Log X/Y data
	for (int i = 0; i < recLen; i++)
	{
		// Reset buffer
		buf[0] = 0;

		status = sprintf (buf + strlen (buf), "%3.10f, %3.10f\n", wfmAvg[i], wfmX[i]);

		status = fwrite (buf, 1, strlen (buf), fd);
	}

	status = fclose(fd);

	return 1;
}


//==============================================================================
// Global functions (not user-facing)

// Open communication with FTDI device
__stdcall int openDevice (void)
{
	char buf[32];

	usbOpened = usbfifo_open ();

	// USB communication initialized
	if (usbOpened)
	{
		// Get ID of USBFIFO device
		usbfifo_getid (buf, 32);
		strcpy (dev_id, buf);

		// TODO: error message (-102) if not as expected

		// Get comm speed of USBFIFO device
		usbfifo_getcomspd (buf, 32);
		int devbps = atoi (buf);

		// Compare retrieved with expected comm speed
		if (devbps != usbfifo_gethostbps ())
		{
			return -103;
		}

		// Inialization successful
		return 1;
	}
	// USB communication failed
	else
	{
		return -101;
	}
}

// Quantize acquisition timescale
__stdcall int setupTimescale (void)
{
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
		vel = (double) 3E8 / sqrt (dielK);
		val1 = xStart * 1E9 / vel;
		val2 = xEnd * 1E9 / vel;

		// Calculate distance in feet, if selected
		if (xUnits == UNIT_FT)
		{
			val1 = val1 / M_TO_FT;
			val2 = val2 / M_TO_FT;
		}
	}

	start_tm.time = (UINT32) (val1 / 50.0 * 0xFFFF);
	end_tm.time = (UINT32) (val2 / 50.0 * 0xFFFF);
	
	// TODO: useful return
	return 1;
}

// Write parameters to device
__stdcall int writeParams (void)
{
	int status;

	status = usbfifo_setparams (freerun_en, calstart, calend, start_tm, end_tm, stepcount,
								strobecount, 0, recLen, dac0val, dac1val, dac2val);

	// Parameters successfully written
	if (status == 1)
	{
		return 1;
	}
	// Parameters not written to device
	else
	{
		// TODO: differentiate between returned -1 (bad write) and -2 (wrong number of params)?
		return -401;
	}
}

// Get waveform for use in all functions
__stdcall int getData (void)
{
	int status = 0;
	
	// Device communication failure
	if (!usbOpened)
	{
		return -201;
	}
	
	// Write acquisition parameters to device
	if (writeParams () < 0)
	{   
		return -202;
	}
	
	// Acquire waveform
	// TODO: actual useful return of acquisition status
	
	UINT8 acq_result;
	status = usbfifo_acquire (&acq_result, 0);
	
	// Blocks of 256 data points (max 256 blocks, 16,384 data points)
	int numBlocks = recLen / 256;
	
	// Verify integrity of all data blocks
	for (int i = 0; i < numBlocks; i++)
	{
		// Number of read attempts before failure
		int nTries = 3;
		
		// Verify data integrity of block
		while ((status = usbfifo_readblock ((UINT8) i, (UINT16*) wfm + (256 * i))) < 0 && nTries--);

		if (status != 1)
		{
			// Indicate which block failed (-30n for nth block)
			return (300 - i);
		}
	}
	
	return 1;
}

// Reconstruct data into useable form
__stdcall int reconstructData (double offset, int filter)
{
	int i;
	
	// Increment between data points
	UINT32 incr;
	incr = (end_tm.time - start_tm.time) / recLen;
	
	timeinf curt;
	curt.time = start_tm.time;
	
	// Set dielectric constant
	double vDiel = (double) 3E8 / sqrt (dielK);

	for (i = 0; i < recLen; i++)
	{
		wfmFilter[i] = (double) wfm[i] - offset;

		wfmTime[i] = ((double) curt.time) / ((double) 0xFFFF) * 50.0;
		wfmDistM[i] = wfmTime[i] * vDiel * 1E-9;
		wfmDistFt[i] = wfmTime[i] * vDiel * 1E-9 * M_TO_FT;

		curt.time += incr;
	}
	
	// Smooth data for better resolution
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
	
	// TODO: useful return
	return 1;
}

// Calculate offset from average 0
__stdcall double meanArray (void)
{
	long val = 0;

	for (int i = 24; i < 1024; i++)
	{
		val += wfmFilter[i];
	}

	return ((double) val / (double) 1000.0);
}

// TODO: make calibrate timebase a user-facing function
// Full timebase calibration
__stdcall int calTimebase (void)
{
	int status;
	
	// Set calibration window
	calstart = 0;
	calend = 4095;
	
	// Dummy acquisition to ensure device initialization
	UINT8 acq_result;
	status = usbfifo_acquire (&acq_result, 0);
	
	// Set start and end time
	double val = 0;
	start_tm.time = (UINT32) (val / 50.0 * 0xFFFF);

	val = 0;
	end_tm.time = (UINT32) (val / 50.0 * 0xFFFF);
	
	// Acquire data for each of 4 data segments
	for (int i = 0; i < 5; i++)
	{
		stepcount = stepcountArray[(UINT16) i];

		status = getData ();
		
		status = reconstructData (0, 1);
		
		// Find mean of waveform segment
		calLevels[i] = calFindMean ();
	}

	int calStatus = calFindStepcount ();

	status = calDAC ();

	// TODO: is setupTimescale necessary here?
	// TODO: setupTimescale is done at each calibration/acquisition
	// status = setupTimescale ();
	
	// Amplitude calibration
	vertCal ();

	return calStatus;
}

// Find mean of waveform segment
__stdcall double calFindMean (void)
{
	double val = 0.00;

	for (int i = 0; i < recLen; i++)
	{
		val = val + wfmFilter[i];
	}

	val = val / recLen;

	return val;
}

// Find optimal step count
__stdcall int calFindStepcount (void)
{
	int i;
	
	int idxMin, idxMax;
	idxMin = 0;
	idxMax = 0;

	// Set so any good data sets new max/min
	double max = 0.00;
	double min = 4095.0;

	// Cycle each of 4 data segments
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
		calStatus = -501;
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

	stepcount = stepcountArray[idxOpt];

	calThreshold = val;

	return calStatus;
}

// Calibrate DACs
__stdcall int calDAC (void)
{
	int status;
	
	// Set calibration window
	calstart = 0;

	// Start, end at 0 ns
	double val = 0;
	start_tm.time = (UINT32) (val / 50.0 * 0xFFFF);

	val = 0;
	end_tm.time = (UINT32) (val / 50.0 * 0xFFFF);;
	
	status = getData ();
	
	calDiscLevel = calFindDiscont ();

	int i = 0;

	while ((calDiscLevel < calThreshold) && (i < 10) && (calstart <= 1100))
	{
		calstart = calstart + 100;
		
		status = getData ();
	
		calDiscLevel = calFindDiscont ();
		
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
		
		status = getData ();
	
		calDiscLevel = calFindDiscont ();
		
		i++;
	}

	if (i == 16)
	{
		calstart = CALSTART_DEFAULT;
	}

	UINT16 calstart_save;
	calstart_save = calstart;

	calend = 4094;
	calstart = 2000;

	int stepcountSave;

	stepcountSave = stepcount;
	stepcount = stepcount + 4;
	
	// Start, end at 0 ns
	val = 0;
	start_tm.time = (UINT32) (val / 50.0 * 0xFFFF);

	val = 0;
	end_tm.time = (UINT32) (val / 50.0 * 0xFFFF);;

	status = getData ();
	
	calDiscLevel = calFindDiscont ();

	i = 0;

	while ((calDiscLevel < calThreshold) && (i < 25) && (calstart <= 4095))
	{
		calstart = calstart + 100;
		
		status = getData ();
	
		calDiscLevel = calFindDiscont ();
		
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
		
		status = getData ();
	
		calDiscLevel = calFindDiscont ();
		
		i++;
	}

	calend = calstart;

	if (i == 16)
	{
		calend = CALEND_DEFAULT;
	}

	calstart = calstart_save;

	stepcount = (UINT16) stepcountSave +1;
	
	return 1;
}

// TO DO: function description
__stdcall double calFindDiscont (void)
{
	double val = 0.0;

	for (int i = 0; i < recLen; i++)
	{
		val = val + wfmFilter[i];
	}

	val = val / recLen;
	
	return val;
}



// TODO: check errors based on response from usbfifo_acquire ()



//==============================================================================
// Global functions (USBFIFO)

// Read FTDI byte
__stdcall char ftrdbyte(void)
{
	char ch;
	int n;

	FT_Read (dev_serial_handle, &ch, 1, &n);

	return ch;
}

// Write FTDI byte
__stdcall void ftwrbyte(char ch)
{
	int n;

	FT_Write (dev_serial_handle, &ch, 1, &n);
}

// Acquire from FDTI device
__stdcall int usbfifo_acquire (UINT8 *ret_val, UINT8 arg)
{
	// NOTE: this is important crash point

	char ch;
	FT_STATUS stat;

	if (!dev_opened)
	{
		return 0;
	}

	// NOTE: Write 'a' (acquire) to ADUC
	ftwrbyte ('a');

	// NOTE: Write a '0' to ADUC (leave it)
	ftwrbyte (arg);

	// NOTE: Sets time for ADUC to respond
	stat = FT_SetTimeouts (dev_serial_handle, 1000, 1000);

	// NOTE: this actually returns the waveform?
	*ret_val = ftrdbyte ();

	// NOTE: '.' means the acquisition successful
	ch = ftrdbyte ();

	stat = FT_SetTimeouts (dev_serial_handle, STD_TIMEOUT, STD_TIMEOUT);

	if (ch != '.')
	{
		// NOTE: crashes software
		return -1;
	}
	else
	{
		return 1;
	}

}

// Close FTDI device
__stdcall void usbfifo_close (void)
{
	FT_STATUS stat;

	if (!dev_opened)
	{
		return;
	}

	stat = FT_Close (dev_serial_handle);
	stat = FT_Close (dev_fifo_handle);

	dev_opened = 0;
}

// Get device comm speed
__stdcall void usbfifo_getcomspd (char *buf, int len)
{
	// NOTE: a failed comm speed indicates you need to unplug/replug the device

	for (UINT16 i = 0; i < strlen (dev_comspdbuf) + 1 && i < len && i < 19; i++)
	{
		*buf++ = dev_comspdbuf[i];
	}
}

// Get host comm speed
__stdcall int usbfifo_gethostbps (void)
{
	// Return value (hard-coded)
	return dev_hostbps;
}

// Get device ID
__stdcall void usbfifo_getid (char *buf, int len)
{
	// NOTE: make this avialable in the diag program

	for (UINT16 i = 0; i < strlen (dev_idbuf) + 1 && i < len && i < 19; i++)
	{
		*buf++ = dev_idbuf[i];
	}
}

// Open FTDI for use by software
__stdcall int usbfifo_open (void)
{
	char ch;
	int n;
	FT_STATUS stat, statfifo;

	// Don't perform operation if already done
	if (dev_opened)
	{
		return 1;
	}

	// NOTE: Possible that "dev_handle" is the serial, dev_fifo_handle is FIFO

	// NOTE: this tells Aki to either look at SERIAL or FIFO port soldering

	statfifo = FT_OpenEx ("USBFIFOV1A", FT_OPEN_BY_SERIAL_NUMBER, &dev_fifo_handle);
	stat = FT_OpenEx ("USBFIFOV1B", FT_OPEN_BY_SERIAL_NUMBER, &dev_serial_handle);

	if (stat != FT_OK || statfifo != FT_OK)
	{
		if (stat == FT_OK)
		{
			FT_Close(dev_serial_handle);
		}

		if (statfifo == FT_OK)
		{
			FT_Close(dev_fifo_handle);
		}

		dev_opened = 0;
	}

	else
	{
		dev_opened = 1;

		stat = FT_SetBaudRate (dev_serial_handle, dev_hostbps);
		if (stat != FT_OK)
		{
			return -1;
		}

		stat = FT_SetDataCharacteristics (dev_serial_handle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);

		if (stat != FT_OK)
		{
			return -2;
		}

		stat = FT_SetFlowControl (dev_serial_handle, FT_FLOW_NONE, 'o', 'p');  // o and p are bogus characters

		if (stat != FT_OK)
		{
			return -3;
		}

		stat = FT_SetTimeouts (dev_serial_handle, STD_TIMEOUT, STD_TIMEOUT);

		if (stat != FT_OK)
		{
			return -4;
		}

		// Read id string
		ftwrbyte ('i');
		FT_Read (dev_serial_handle, dev_idbuf, 16, &n);
		dev_idbuf[16] = '\0';
		ftwrbyte ('i');
		ch = ftrdbyte ();

		// NOTE: if no ".", failed to reed id

		if (ch != '.')
		{
			return -5;
		}

		// Read compspeed
		ftwrbyte('s');
		FT_Read( dev_serial_handle, dev_comspdbuf, 16, &n );
		dev_comspdbuf[16] = '\0';
		ftwrbyte('s');
		ch = ftrdbyte();

		if (ch != '.')
		{
			return -6;
		}



		if (strncmp (dev_idbuf, "USBFIFO", 7) != 0)
		{
			return -7;
		}

		// NOTE: FTDI comm lines; nothing important to debug

		FT_ClrDtr(dev_serial_handle);

		FT_SetRts(dev_serial_handle);
		FT_ClrRts(dev_serial_handle);
	}

	return dev_opened;
}

// Read data blocks of acquisition
__stdcall int usbfifo_readblock (UINT8 block_no, UINT16 *buf)
{
#define BLOCK_LEN 256

	char ch;
	int n, ret,i;
	UINT8 rawbuf8[2*BLOCK_LEN];

	if (!dev_opened)
	{
		return 0;
	}

	// NOTE: command to 'b', read from block

	ftwrbyte ('b');

	// NOTE: which block to pull from
	ftwrbyte (block_no);

	// NOTE: FTDI function, goes to FIFO handle, reads buffer

	ret = FT_Read (dev_fifo_handle, rawbuf8, BLOCK_LEN * 2, &n);

	ch = ftrdbyte();

	// NOTE: 'f' didn't get good data from buffer

	if (ch == 'f')
	{
		// handle failure, clean out FIFO and return error
		while (FT_Read (dev_fifo_handle, rawbuf8, 1, &n) == FT_OK && n == 1);
		return -1;
	}

	if (n != 2*BLOCK_LEN)
	{
		return -2;
	}

	// NOTE: write data to software buffer

	for (i=0; i<BLOCK_LEN; i++)
	{
		buf[i] = (UINT16) (((UINT16) rawbuf8[2 * i + 1]) << 8) | ((UINT16) rawbuf8[2 * i]);
	}

	return 1;
}

// Set parameters for acquisition
__stdcall int usbfifo_setparams (UINT8 freerun_en, UINT16 calstart, UINT16 calend, timeinf tmstart, timeinf tmend, UINT16 stepcount,
								 UINT16 strobecount, UINT8 noversample, UINT16 record_len, UINT16 dac0, UINT16 dac1, UINT16 dac2)
{
	static UINT8 params[NPARAMS];
	int ch;
	int n;
	int ret;

	if (!dev_opened)
	{
		return 0;
	}

	params[IDX_FREERUN] = freerun_en;
	params[IDX_STEPCNT_UPPER] = stepcount >> 8;
	params[IDX_STEPCNT_LOWER] = (UINT8) stepcount;
	params[IDX_RECLEN_UPPER] = record_len >> 8;
	params[IDX_RECLEN_LOWER] = (UINT8) record_len;
	params[IDX_DAC0_UPPER] = dac0 >> 8;
	params[IDX_DAC0_LOWER] = (UINT8) dac0;
	params[IDX_DAC1_UPPER] = dac1 >> 8;
	params[IDX_DAC1_LOWER] = (UINT8) dac1;
	params[IDX_DAC2_UPPER] = dac2 >> 8;
	params[IDX_DAC2_LOWER] = (UINT8) dac2;
	params[IDX_CALSTART_UPPER] = calstart >> 8;
	params[IDX_CALSTART_LOWER] = (UINT8) calstart;
	params[IDX_CALEND_UPPER] = calend >> 8;
	params[IDX_CALEND_LOWER] = (UINT8) calend;
	params[IDX_TMSTART_B3] = tmstart.time_b.b3;
	params[IDX_TMSTART_B2] = tmstart.time_b.b2;
	params[IDX_TMSTART_B1] = tmstart.time_b.b1;
	params[IDX_TMSTART_B0] = tmstart.time_b.b0;
	params[IDX_TMEND_B3] = tmend.time_b.b3;
	params[IDX_TMEND_B2] = tmend.time_b.b2;
	params[IDX_TMEND_B1] = tmend.time_b.b1;
	params[IDX_TMEND_B0] = tmend.time_b.b0;
	params[IDX_OVERSAMPLE] = noversample;
	params[IDX_STROBECNT_UPPER] = strobecount >> 8;
	params[IDX_STROBECNT_LOWER] = (UINT8) strobecount;

	ftwrbyte('p');
	ret = FT_Write (dev_serial_handle, params, NPARAMS, &n);
	ch = ftrdbyte();

	if (ch != '.')
	{
		// No record received
		return -1;
	}

	if (n != NPARAMS)
	{
		// Incorrect number of params passed
		return -2;
	}

	return 1;
}
