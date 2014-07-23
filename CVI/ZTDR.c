//==============================================================================
//
// Title:		ZTDR.c
// Purpose:		A short description of the implementation.
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

#define NPOINTS_MAX 16384

#define CAL_WINDOW 10.0e-9

// Horizontal units
#define UNIT_M 0
#define UNIT_NS 1
#define UNIT_FT 2

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
int 	calIncrement;

// Transaction data
UINT16 	wfm[NPOINTS_MAX];
double 	wfmf[NPOINTS_MAX];

// Time window
timeinf end_tm;
timeinf start_tm;

// User interface and states
int 	HL1101_xaxis_val;
int 	HL1101_yaxis_val;

int		HL1101_start;
double 	HL1101_windowsz;

double 	HL1101_diel;

int 	panelHandle;


//==============================================================================
// Global functions (roughly grouped by functionality and order of call)

// Main startup function
int main (int argc, char *argv[])
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

	RunUserInterface ();
	DiscardPanel (panelHandle);
	
	return 0;
}

// Scale time range of acquisition window
void setupTimescale (void)
{
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
