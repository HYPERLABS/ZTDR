#include <userint.h>
//==============================================================================
//
// Title:		menu.c
// Purpose:		Menu-driven functionality for HL11xx TDR instruments. 
//
// Created on:	7/26/2014 at 6:29:19 PM by Brian Doxey.
// Copyright:	HYPERLABS. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include "FTD2XX.h"

#include "menu.h"
#include "shared.h"
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

// TO DO: Organize
extern int	panelHandle, menuHandle; 
extern int	xUnits, yUnits;

//==============================================================================
// Global functions

// Set horizontal environmental variables

