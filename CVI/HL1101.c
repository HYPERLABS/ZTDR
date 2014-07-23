#include <windows.h>
#include <formatio.h>
#include <ansi_c.h>
#include <rs232.h>
#include <cvirte.h>		
#include <userint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// HL Header
#include "callback.h"

// Include OLD DLL 
#include "usbfifodll.h"

// Include USBFIFO functionality
#include "usbfifo.h"



#define VERT_HIST 100
#define FILTER_WIDTH 4
#define CAL_THRESHOLD_HYST 100

#define BUF_REC_LEN 64
#define MAX_SAVE_FILE 100 

#define MV 0
#define NORM 1
#define RHO 2 
#define OHM 3






// Vert Cal

#define CAL_WINDOW_START 10 /* in ns */
#define OFFSET_ACQ_POS 0 /* First sample in the record, no matter where the wfm is positioned */
#define CAL_GUARD 0.5e-9 /* Guard region from 50% of step ampl to determine vstart_coax and vend_coax */
#define STEP_AMPL 800

static double wfm_data[NPOINTS_MAX], wfm_data_ave[NPOINTS_MAX];
static double ymin, ymax; 
static  int delay_value;
static  int start_delay=0;		
static  int offset;

static startupCal = 1;

// Define functions
double mean_array(void);
void SetupTimescale(void);


// Values and ranges for X and Y axes
char *y_label[] =
     {
      "MVOLT",
      "NORM ",
      "RHO  ",
      "OHM  " 
	 };  
      
char *label_dist[] =
     {
      "(X2-X1)[m]  ",
      "(X2-X1)[ft] ",
      "(X2-X1)[nS]" 
	 };

char *label_start[] =
     {
      "Start [m]   ",
      "Start [ns]" ,
      "Start [ft]  "
	 };
      
char *label_window[] =
     {
      "Window [m]",
      "Window [ns]",
      "Window [ft]" 
	 };
      
char *x_label[] =
     {
      "Round Trip [m]   ",
      "Round Trip [ns]  ",
      "Round Trip [ft]" 
	 };    
      
float max_range[] =
     {
      400.0,
      2000.0,
      1332
	 };

float dflt_value[]  =
     {
      10.0,
      50.0,
      33.3
	 };		
      
float dflt_start_value[] =
     {
      0.0,
      0.0,
      0.0
	 };

/* TO DO */
char save_file[MAX_SAVE_FILE+160];

// Transaction Data
static UINT16 rec_len=1024;

static double timescale[NPOINTS_MAX];
static double dist_m[NPOINTS_MAX]; 
static double dist_ft[NPOINTS_MAX]; 
static double rise_time;
static int plotid = -1, plotid1 = -1;
static int recall_plotid = -1;
static double cal_threshold1;
static double cal_threshold2;
static double cal_threshold3;

static double levelRight;
static double levelLeft;
static double calDiscLevel;

static UINT16 calstart=540, calend=3870, calstart_save=540;
//static UINT16 calstart=575, calend=3880;
#define CALSTART_DEFAULT 540
#define CALEND_DEFAULT 3870


static UINT16 dac0val=0, dac1val=0, dac2val=0;
static int freerun_en=0;

static UINT16 stepcount=6, strobecount=2;	  // Aki 7 to 6
static UINT16 stepcountArray[5]={4, 5, 6, 7, 8};
static double calLevels[5];







int y_axis,x_axis;

int acquisition_nr =1, wfmpersec =1;

static int retrieve, norm_flag,auto_flag; 

static double vampl = 679.0; 


//TO DO array sizes
/* static */ double dist[NPOINTS_MAX];
/*static */ double wfm_ret[NPOINTS_MAX];
/*static */ double wfm_dist[NPOINTS_MAX]; 
/*static */ double wfm_ret_min;
/* static */ double wfm_ret_max;
/* static */ int wfm_ret_flag = 0;
//double x1, x2, y1, y2, r1, p1,  dist_x1x2, b1, b2, e1, e2; 
static double x1, x2, y1, y2;

void timebase_cal(void);
void vert_cal(void);
/* END TO DO */


/***************************/
/* Initialization routines */
/***************************/






/*******************/
/* Common routines */
/*******************/
 
// Write parameters to device
static int WriteParams(void)
{
	int ret;

	ret = usbfifo_setparams(
			  freerun_en,
			  calstart,
			  calend,
			  start_tm,
			  end_tm,
			  stepcount,
			  strobecount,
			  0,
			  rec_len,
			  dac0val,
			  dac1val,
			  dac2val );

	//	printf("WriteParams: calstart=%d calend=%d\n", calstart, calend);

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

// Write parameters for calibration 
static int WriteParamsVertCal()
{
	int ret;

	ret = usbfifo_setparams(
			  0,		   		// no free run
			  calstart, 		//set by timebase calibration
			  calend,
			  start_tm,
			  end_tm,
			  stepcount,
			  strobecount,
			  0,
			  1024,		   //add define
			  dac0val,
			  dac1val,
			  dac2val );

	//	printf("WriteParams: calstart=%d calend=%d\n", calstart, calend);

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

// Calculate offset from average 0
double mean_array()
{
	long temp;
	int i;
	temp = 0;
	for (i=24; i<1024; i++){
		temp += wfmf[i];
	}

	return((double)temp/(double)1000.0);
}

// Reconstruct data into useable form
static void ReconstructData(double offset)
{
	int i;
	timeinf curt;
	UINT32 incr;
	FILE *fp = NULL; // fopen("c:/wfm.txt", "w");
	double myWfm[1024];
	double vel;
	
	vel = (double)3E8/sqrt(HL1101_diel);

	incr = (end_tm.time - start_tm.time)/rec_len;
	
	curt.time = start_tm.time;
	for (i=0;i<rec_len;i++)
	{						 		
		if (fp) fprintf(fp, "%d %d\n",i, wfm[i]);
		wfmf[i] = (double)wfm[i] - offset;
		if (i < 1024)
			myWfm[i] = wfmf[i];
		timescale[i] =((double)curt.time)/((double)0xFFFF)*50.0;
		dist_m[i] = timescale[i] * vel * 1E-9;
		dist_ft[i] = timescale[i] * vel * 1E-9 * MtoFT;
		curt.time += incr;
	}  
	if (fp) fclose(fp);
}

// Reconstruct data for calibration
static void ReconstructDataCal(void)
{
	int i, j;
	timeinf curt;
	double val;
	UINT32 incr;
	FILE *fp = NULL; // fopen("c:/wfm.txt", "w");
	
	incr = (end_tm.time - start_tm.time)/rec_len;
	
	curt.time = start_tm.time;
	for (i=0;i<rec_len;i++)
	{						 		
		if (fp) fprintf(fp, "%d %d\n",i, wfm[i]);
		wfmf[i] = (double)wfm[i];
		timescale[i] =((double)curt.time)/((double)0xFFFF)*50.0;
		curt.time += incr;
	}
	
// Smooth data for better resolution
	for (i = FILTER_WIDTH/2; i < rec_len - FILTER_WIDTH/2; i++)
	{
		val = 0;
		for (j = i - FILTER_WIDTH/2; j < i + FILTER_WIDTH/2; j++)
		{
			val = val + wfmf[j];
		}
		wfmf[i] = val/FILTER_WIDTH;
	}
	if (fp) fclose(fp);
}

// Set vertical labels
void set_y_labels(void)
{
	int status;

	status = SetCtrlAttribute (panelHandle,PANEL_WAVEFORM,
							   ATTR_YNAME,y_label[y_axis]);
}

// Set vertical labels
void HL1101_y_axis_1 (int panel, int control)
{
	GetCtrlVal (panel,control,&y_axis);
	set_y_labels();
}

// Set horizontal labels
void set_x_labels(void)
{
	int status;

	status = SetCtrlAttribute (panelHandle,PANEL_NUM_STARTTM,
							   ATTR_LABEL_TEXT,label_start[x_axis]);
	status = SetCtrlAttribute (panelHandle,PANEL_NUM_WINDOWSZ,
							   ATTR_LABEL_TEXT,label_window[x_axis]);
	status = SetCtrlAttribute (panelHandle,PANEL_WAVEFORM,
							   ATTR_XNAME,x_label[x_axis]);
	status = SetCtrlAttribute (panelHandle,PANEL_NUM_STARTTM,
							   ATTR_MAX_VALUE,max_range[x_axis]);
	status = SetCtrlVal(panelHandle,PANEL_NUM_STARTTM,
						dflt_start_value[x_axis]);
	status = SetCtrlAttribute (panelHandle,PANEL_NUM_WINDOWSZ,
							   ATTR_MAX_VALUE,max_range[x_axis]);
	status = SetCtrlVal(panelHandle,PANEL_NUM_WINDOWSZ,
						dflt_value[x_axis]);
}

// Set horizontal labels
void HL1101_x_axis (int panel, int control)
{
	GetCtrlVal (panel,control,&x_axis);
	set_x_labels();
}


/************************/
/* Calibration routines */
/************************/

// Set parameters for calibration
static void SetupToCalibrate(void)
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

	ret = usbfifo_acquire(&acq_result, 0);

	if (ret < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
		return;
	}
}

// Set timescale for calibration
void SetupTimescaleCal()
{
	double val;
	UINT32 windowsz;
	
	val = 0;
	start_tm.time = (UINT32)(val/50.0*0xFFFF);
	
	val = 0;
	windowsz = (UINT32)(val/50.0*0xFFFF);
	
	end_tm.time = start_tm.time + windowsz;
}

// Set timescale for vert cal
void SetupTimescaleVertCal()
{
	double val;
	UINT32 windowsz;
	 // AKI HACK : Changed 12ns to 10ns
	val = 10;  // add defines
	start_tm.time = (UINT32)(val/50.0*0xFFFF);
	
	val = 10;//ns
	windowsz = (UINT32)(val/50.0*0xFFFF);
	
	end_tm.time = start_tm.time + windowsz;
}

// Set timescale for vert cal at 0 ns ONLY
void SetupTimescaleVertCal0(double windowStart)
{
	double val;
	UINT32 windowsz;
	
	start_tm.time = (UINT32)(windowStart/50.0*0xFFFF);
	
	val = 0;//ns
	windowsz = (UINT32)(val/50.0*0xFFFF);
	
	end_tm.time = start_tm.time + windowsz;
	val = val;
}

// Set step count
void SetupStepcount(int index)
{
	stepcount = stepcountArray[(UINT16)index];
}

// TO DO: function description
static void FindMeanCalWfm(int calStepIndex)
{
	int i;
	double val;

	val = 0;
	for (i=0; i < rec_len; i++)
	{
		val = val + wfmf[i];
	}
	val = val /rec_len;
	calLevels[calStepIndex] = val;
}

// TO DO: function description
static void FindCalDiscontLevel(void)
{
	int i, j;

	char dispStr[10];

	calDiscLevel = 0;

	j = 0;

	for (i = 0; i < rec_len; i++)
	{
		calDiscLevel = calDiscLevel + wfmf[i];
	}

	calDiscLevel = calDiscLevel/rec_len;

}

// Acquire waveform for calibration (1/2)
static void AcquireWaveformCal1(int calStepIndex)
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

	if (!usb_opened)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Comm failure.");
		return;
	}

	//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquiring...");

	// Write acquisition parameters
	if (WriteParams() <= 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Param error.");
		return;
	} 

	// Run acquisition
	ret = usbfifo_acquire(&acq_result, 0);

	if (ret < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
		return;
	}

	// Read blocks of data from block numbers 0-63 (16384 pts)	
	blocksok = 1;
	nblocks = rec_len / 256;
	for (i=0; i<nblocks; i++)
	{
		// Verify data integrity of block
		int ntries = 3;
		while ( (ret = usbfifo_readblock(i, (UINT16*)(wfm+256*i) )) < 0
				&& ntries--);

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

	GetCtrlVal(panelHandle, PANEL_NUM_YMIN, &ymin);
	GetCtrlVal(panelHandle, PANEL_NUM_YMAX, &ymax);
	GetCtrlVal(panelHandle, PANEL_CHK_DOTS, &dots);

	ReconstructDataCal();
	FindMeanCalWfm(calStepIndex);

	if (startupCal == 0)
	{
		if (plotid1 != -1)
			DeleteGraphPlot(panelHandle, PANEL_WAVEFORM, plotid1, VAL_DELAYED_DRAW);


		plotid1 = PlotXY(panelHandle, PANEL_WAVEFORM, timescale, wfmf, rec_len,
						 VAL_DOUBLE, VAL_DOUBLE, dots?VAL_SCATTER:VAL_FAT_LINE,
						 VAL_SOLID_DIAMOND, VAL_SOLID, 1,
						 dots? VAL_MAGENTA : VAL_MAGENTA );

		SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_XAXIS, VAL_AUTOSCALE, 0, 0);

		ymind = (double)ymin;
		ymaxd = (double)ymax;


		SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_LEFT_YAXIS, VAL_MANUAL, ymind, ymaxd);
		SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_RIGHT_YAXIS, VAL_MANUAL, ymind, ymaxd);

		RefreshGraph(panelHandle, PANEL_WAVEFORM);
	}

}

// Acquire waveform for calibration (2/2)
static void AcquireWaveformCal2(void)
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
	
	if (!usb_opened)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Comm failure.");
		return;
	}
	
	//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquiring...");
	
	// Write acquisition parameters
	if (WriteParams() <= 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Param error.");
		return;
	}
	
	// Run acquisition
	ret = usbfifo_acquire(&acq_result, 0);
	

	if (ret < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
		return;
	}

 	// Read blocks of data from block numbers 0-63 (16384 pts)	blocksok = 1;
	blocksok = 1;
	nblocks = rec_len / 256;
	for (i=0;i<nblocks;i++)
	{
		// Verify data integrity of block
		int ntries = 3;
		while ( (ret = usbfifo_readblock(i, (UINT16*)(wfm+256*i) )) < 0 
		 	&& ntries--);
		
		if (ret < 0)
			blocksok = 0;
	}
	
	if (blocksok == 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Read failure.");
		return;
	}
	
		
	GetCtrlVal(panelHandle, PANEL_NUM_YMIN, &ymin);
	GetCtrlVal(panelHandle, PANEL_NUM_YMAX, &ymax);
	GetCtrlVal(panelHandle, PANEL_CHK_DOTS, &dots);
	
	ReconstructDataCal();
	FindCalDiscontLevel();
	
	if (startupCal == 0)
	{
		if (plotid != -1)
		DeleteGraphPlot(panelHandle, PANEL_WAVEFORM, plotid, VAL_DELAYED_DRAW);
	
		plotid = PlotXY(panelHandle, PANEL_WAVEFORM, timescale, wfmf, rec_len,
			VAL_DOUBLE, VAL_DOUBLE, dots?VAL_SCATTER:VAL_FAT_LINE, 
			VAL_SOLID_DIAMOND, VAL_SOLID, 1, 
			dots? VAL_MAGENTA : VAL_MAGENTA );
	
		SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_XAXIS, VAL_AUTOSCALE, 0, 0);

		ymind = (double)ymin;
		ymaxd = (double)ymax;
	   
		SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_LEFT_YAXIS, VAL_MANUAL, ymind, ymaxd);
		SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_RIGHT_YAXIS, VAL_MANUAL, ymind, ymaxd);
	
		RefreshGraph(panelHandle, PANEL_WAVEFORM);
	}

}

// Find optimal step count
void FindOptimalStepCount(void)
{
	int i, idx_min, idx_max, opt_idx;
	double val, min, max;

	max = 0;
	min = (double)4095;

	idx_min = 0;
	idx_max = 0;

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
		SetCtrlVal(panelHandle, PANEL_MESSAGES, "Calibration failed");
	}

	val = (max - min)/4 + min;

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

	cal_threshold3 = val;

}

// TO DO: description of routine
void CalDAC()
{
	char dispStr[10];

	int i;
	int stepcount_save;
	
	calstart = 0;

	SetupTimescaleCal();
	AcquireWaveformCal2();
	
	i = 0;
	
	while ((calDiscLevel < cal_threshold3) && (i < 10) && (calstart <= 1100))
	{
		calstart = calstart + 100;
		AcquireWaveformCal2();
		i++;
	}
	
	if (i==10)
	{
		calstart = CALSTART_DEFAULT;
	}

	i = 0;

	while ((calDiscLevel > cal_threshold3) && (i < 16))
	{
		calstart = calstart - 10;
		AcquireWaveformCal2();
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

	SetupTimescaleCal();
	AcquireWaveformCal2();
	
	i = 0;
	
	while ((calDiscLevel < cal_threshold3) && (i < 25) && (calstart <= 4095))
	{
		calstart = calstart + 100;
		AcquireWaveformCal2();
		i++;
	}
	if (i == 25)
	{
		calend = CALEND_DEFAULT;
	}
	i = 0;

	while ((calDiscLevel > cal_threshold3) && (i < 16))
	{
		calstart = calstart - 10;
		AcquireWaveformCal2();
		i++;
	}

	calend = calstart;
	if (i == 16)
	{
		calend = CALEND_DEFAULT;
	}
	calstart = calstart_save;

	stepcount = stepcount_save +1;

}

// Main VertCal routine
static void PerformVertCal(void)
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
	SetupTimescaleVertCal0(CAL_WINDOW_START);

	// Write the acquisition parameters 
	if (WriteParamsVertCal() <= 0)
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
	for (i=0; i<nblocks; i++)
	{
		// Verify data integrity of block
		int ntries = 3;
		while ( (ret = usbfifo_readblock(i, (UINT16*)(wfm+256*i) )) < 0
				&& ntries--);

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
	ReconstructData(0);
	vstart = mean_array();

	// Timescale and parameters for main acquisition
	SetupTimescaleVertCal();

	// Write the acquisition parameters    
	if (WriteParamsVertCal() <= 0)
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
	for (i=0; i<nblocks; i++)
	{
		// Verify data integrity of block 
		int ntries = 3;
		while ( (ret = usbfifo_readblock(i, (UINT16*)(wfm+256*i) )) < 0
				&& ntries--);

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

	GetCtrlVal(panelHandle, PANEL_NUM_YMIN, &ymin);
	GetCtrlVal(panelHandle, PANEL_NUM_YMAX, &ymax);
	GetCtrlVal(panelHandle, PANEL_CHK_DOTS, &dots);

	ReconstructData(0);

	if (plotid != -1)
		DeleteGraphPlot(panelHandle, PANEL_WAVEFORM, plotid, VAL_DELAYED_DRAW);

	plotid = PlotXY(panelHandle, PANEL_WAVEFORM, timescale, wfmf, rec_len,
					VAL_DOUBLE, VAL_DOUBLE, dots?VAL_SCATTER:VAL_FAT_LINE,
					VAL_SOLID_DIAMOND, VAL_SOLID, 1,
					dots? VAL_MAGENTA : VAL_MAGENTA );

	// Find the 50% crossing from vstart to approx. vstart + 1200 (step size)
	i=0;

	while (wfmf[i] < (vstart + 400.0) && (i <= 1022))
	{
		i = i + 1;
	}

	i50 = i;

	// Compute a calibrated vstart as average of points from 0 to (i50 - CAL_GUARD(0.5ns)) at calIncrement
	// Normalize calIncrement to waveform index
	calInterval = (int)(CAL_GUARD/(CAL_WINDOW/1024));
	
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

	// Compute calibrated vend as average over 1ns at i50 + 2*CAL_GUARD (1ns) at calIncrement
	tempID = i50 + calInterval;
	if (tempID > 1023)		   /* Don't run out of acquired waveform */
		tempID = 1023;

	tempID2 = i50 + 3 * calInterval;
	if (tempID2 > 1023)
		tempID2 = 1023;

	temp = 0;
	for (i = tempID; i<tempID2; i++)
	{
		temp += wfmf[i];
	}
	vend = temp / (tempID2 - tempID);

	vampl = vend - vstart;
	
	RefreshGraph(panelHandle, PANEL_WAVEFORM);

}

// Call vertcal routines
void vert_cal()
{
	PerformVertCal();
	SetupTimescale();
}

/* NOT NEEDED 
// TO DO: function description
static void Reset_Calibrate(void)
{
	// Changes stimulus drive back from 80MHz to the CPLD
	UINT8 acq_result;
	int ret;

	if (!usb_opened)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Comm failure.");
		return;
	}

	ret = usbfifo_reset_calibrate(&acq_result, 0);

	if (ret < 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
		return;
	}
}
*/

// Callback for CALIBRATE button
int CVICALLBACK on_cal (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			timebase_cal();

			break;
	}
	return 0;
}

// Calibrate timebase
void timebase_cal()
{
	int i;

	SetCtrlVal(panelHandle, PANEL_MESSAGES, "Calibration in progress");
	
	SetupToCalibrate();
	SetupTimescaleCal();

	// Cycle through stepcount arrays
	for (i=0; i<5; i++)
	{
		SetupStepcount(i);
		AcquireWaveformCal1(i);
	}

	FindOptimalStepCount();

	CalDAC();
	
	//Reset_Calibrate();
	PerformVertCal(); // Aki very new cal
	SetupTimescale();
	SetCtrlVal(panelHandle, PANEL_MESSAGES, "Calibration Done");
}

// Callback for VERT CAL button
int CVICALLBACK on_vertcal (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			vert_cal();
			
			break;
	}
	return 0;
}


/************************/
/* Time window routines */
/************************/




/************************/
/* Acquisition routines */
/************************/

// Main acquisition function /* TO DO */
static void AcquireWaveform(void)
{
	// Local variable definition
	int ret = 0;
	int i,n,k;		 //aki
	int status;
	
	unsigned char buf[24];
	char ch;
	UINT8 acq_result;
	static char cbuf[32];
	
	int dots;
	int nblocks;
	int blocksok;
	double ymind, ymaxd;
	
	double impedance = 50;
	double ampl_factor = 250.0;
	//double wfm_data[NPOINTS_MAX];
	double wfm_z_data[NPOINTS_MAX];
	double wfm_rho_data[NPOINTS_MAX];
	
	double wfmf_debug[1024];
	double wfm_data_debug[1024];
	
	double offset = 0;
	
	if (!usb_opened)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Comm failure.");
		return;
	}
	
	// Calculate offset of waveform by taking and averaging sample at 0 ns
	SetupTimescaleVertCal0(OFFSET_ACQ_POS);
	
	// Write the acquisition parameters
	if (WriteParamsVertCal() <= 0)
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
	for (i=0; i < nblocks; i++)
	{
		// Verify data integrity of block
		int ntries = 3;
		while ( (ret = usbfifo_readblock(i, (UINT16*)(wfm+256*i) )) < 0 
		 	&& ntries--);
		
		if (ret < 0)
			blocksok = 0;
	}
	
	if (blocksok == 0)
	{
		//setCtrlVal(panelHandle, PANEL_TXT_LOG, "Read failure.");
		return;
	}
	
	// Reconstruct data and find offset for acquisition
	ReconstructData(0); 
	offset = mean_array();
	
	// Timescale and parameters for main acquisition 
	SetupTimescale();
	
	// If first acquisition of software session, run timebase calibration
	if (startupCal)
	{
		timebase_cal();
		startupCal = 0;
		SetCtrlVal(panelHandle, PANEL_MESSAGES, "Calibration Done");
	}
	
	if (WriteParams() <= 0)
	{
		//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Param error.");
		return;
	}
	
	//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquiring...");
	
	// Acquire k waveforms, loop and average if k > 1
	for (k=0; k<acquisition_nr;k++) {
	
		ret = usbfifo_acquire(&acq_result, 0);
		
		if (ret < 0)
		{
			printf("Failed to run the acquisition sequence (did not get '.')");
			//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Acquire failure.");
			return;
		}
	
		// Read blocks of data from block numbers 0-63 (16384 pts)
		blocksok = 1;
		nblocks = rec_len / 256;
		for (i=0;i<nblocks;i++)
		{
			// Verify data integrity of block 
			int ntries = 3;
			while ( (ret = usbfifo_readblock(i, (UINT16*)(wfm+256*i) )) < 0 
			 	&& ntries--);
		
			if (ret < 0)
				blocksok = 0;
		}
	
		if (blocksok == 0)
		{
			//SetCtrlVal(panelHandle, PANEL_TXT_LOG, "Read failure.");
			return;
		}
		
		// Get axis limits and units
		GetCtrlVal(panelHandle, PANEL_NUM_YMIN, &ymin);
		GetCtrlVal(panelHandle, PANEL_NUM_YMAX, &ymax);
		GetCtrlVal(panelHandle, PANEL_CHK_DOTS, &dots);
	
		GetCtrlVal (panelHandle, PANEL_RING, &y_axis); 
		y_axis = HL1101_yaxis_val;
	
		// Reconstruct data and account for offset
		ReconstructData(offset);
 		
		// Store data, perform rho and impedance conversions
		for (i=0; i<rec_len; i++)
		{ 
			wfm_data[i] = (double)(wfmf[i])/(double)vampl - 1.0; 
			if (i < 1024)
			{
				wfmf_debug[i] = wfmf[i];
				wfm_data_debug[i] = wfm_data[i];
			}
		   		    	
			wfm_rho_data[i]=wfm_data[i]; 
		   				
			if(wfm_rho_data[i]<=-1){
				wfm_rho_data[i] =-0.999;
			}	  
			if(wfm_rho_data[i]>=1){
				wfm_rho_data[i] =0.999;
			}
		   		    	
			wfm_z_data[i] = (double)impedance * ((double)(1.0) + (double)(wfm_rho_data[i]))/((double)(1.0)-(double)(wfm_rho_data[i]));
		   		    	
			if(wfm_z_data[i]>=500){ 
				wfm_z_data[i]=500.0;
			}
		  
			if(wfm_data[i]>=500){ 
				wfm_data[i]=500.0;
			}

		}

		// Y Axis scaling based on selected unit
		switch (y_axis)
		{

			case MV:

				for (i=0; i<rec_len; i++)
				{
					wfm_data[i] *= ampl_factor;
				}

				ymin = -500.00;
				ymax =  500.00;

				break;

			case NORM:

				for (i=0; i<rec_len; i++)
				{
					wfm_data[i] += 1.0;
				}

				ymin = 0.00;
				ymax =  2.00;

				break;

			case OHM:

				for (i=0; i<rec_len; i++)
				{

					if(wfm_data[i]<=-1)
					{
						wfm_data[i] =-0.999;
					}
					if(wfm_data[i]>=1)
					{
						wfm_data[i] =0.999;
					}

					wfm_data[i] = (double)impedance * ((double)(1.0) + (double)(wfm_data[i]))/((double)(1.0)-(double)(wfm_data[i]));

					if(wfm_data[i]>=500)
					{
						wfm_data[i]=500.0;
					}

				}

				ymin =   0.00;
				ymax = 500.00;

				break;

			default: // RHO 

				ymin = -1.00;
				ymax =  1.00;

				break;

		}

		// Set Y Axis limits if manual scaling
		GetCtrlVal (panelHandle, PANEL_TOGGLEBUTTON, &auto_flag);
		if (auto_flag==0)
		{
			GetCtrlVal (panelHandle, PANEL_NUM_YMAX, &ymax);
			GetCtrlVal (panelHandle, PANEL_NUM_YMIN, &ymin);
				
			if((double)ymin>=(double)ymax)
			{
				ymin=(double)ymax-(double)1.0;
			}
		}

		if (plotid != -1)
		{
			DeleteGraphPlot(panelHandle, PANEL_WAVEFORM, -1, VAL_DELAYED_DRAW);
		}
		
		status = SetAxisRange(panelHandle, PANEL_WAVEFORM,VAL_AUTOSCALE,
							  0.0,0.0,VAL_MANUAL,ymin,ymax);
			
		ymind = (double)ymin;
		ymaxd = (double)ymax;		
					
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
			SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_LEFT_YAXIS, VAL_MANUAL, ymind, ymaxd);
			SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_RIGHT_YAXIS, VAL_MANUAL, ymind, ymaxd);
		}
	
		// Average waveforms
		for (i=0; i<1024; i++) {
			wfm_data_ave[i] = (k* wfm_data_ave[i] + wfm_data[i])/(k+1);
		}
	}	

	// Horizontal units in time
	if (HL1101_xaxis_val == T){
		plotid = PlotXY(panelHandle, PANEL_WAVEFORM, timescale, wfm_data_ave, rec_len,		   //aki
			VAL_DOUBLE, VAL_DOUBLE, dots?VAL_SCATTER:VAL_FAT_LINE, 
			VAL_SOLID_DIAMOND, VAL_SOLID, 1, 
			dots? VAL_MAGENTA : VAL_MAGENTA );
	}

	// Horizontal units in meters
	else if (HL1101_xaxis_val == M) {
		 plotid = PlotXY(panelHandle, PANEL_WAVEFORM, dist_m, wfm_data_ave, rec_len,			//aki
			VAL_DOUBLE, VAL_DOUBLE, dots?VAL_SCATTER:VAL_FAT_LINE, 
			VAL_SOLID_DIAMOND, VAL_SOLID, 1, 
			dots? VAL_MAGENTA : VAL_MAGENTA );
	}
	
	// Horizontal units in feet
	else {
		plotid = PlotXY(panelHandle, PANEL_WAVEFORM, dist_ft, wfm_data_ave, rec_len,		   //aki
			VAL_DOUBLE, VAL_DOUBLE, dots?VAL_SCATTER:VAL_FAT_LINE, 
			VAL_SOLID_DIAMOND, VAL_SOLID, 1, 
			dots? VAL_MAGENTA : VAL_MAGENTA );
	}
	
	RefreshGraph(panelHandle, PANEL_WAVEFORM);
	
	// Position cursors and update control reading
	double c1x, c1y, c2x, c2y;
	c1x = c1y = c2x = c2y = 0;
	
	GetGraphCursor (panelHandle, PANEL_WAVEFORM, 1, &c1x, &c1y);
    GetGraphCursor (panelHandle, PANEL_WAVEFORM, 2, &c2x, &c2y);
	
	sprintf(buf, "%.2f    %.2f", c1x, c1y);
	SetCtrlVal(panelHandle, PANEL_STR_CURS1,  buf);
	
	sprintf(buf, "%.2f    %.2f", c2x, c2y);
	SetCtrlVal(panelHandle, PANEL_STR_CURS2,  buf);
	
	sprintf(buf, "%.2f    %.2f", c2x-c1x, c2y-c1y);
	SetCtrlVal(panelHandle, PANEL_STR_DELTA, buf);

}

// Reset plot area
void reset_plot()
{
	int status;

	status = SetCtrlAttribute(panelHandle,PANEL_RING,
							  ATTR_DIMMED,0);
	status = SetCtrlAttribute(panelHandle,PANEL_NUM_WINDOWSZ,
							  ATTR_DIMMED,0);
	status = SetCtrlAttribute(panelHandle,PANEL_NUM_STARTTM,
							  ATTR_DIMMED,0);

	status = SetCtrlAttribute(panelHandle,PANEL_COMMANDBUTTON,
							  ATTR_DIMMED,0);

	status = SetCtrlAttribute(panelHandle,PANEL_NUM_YMAX,
							  ATTR_DIMMED,0);
	status = SetCtrlAttribute(panelHandle,PANEL_NUM_YMIN,
							  ATTR_DIMMED,0);
	status = SetCtrlAttribute(panelHandle,PANEL_TOGGLEBUTTON,
							  ATTR_DIMMED,0);

	wfm_ret_flag = 0;

	norm_flag = 0;
	SetCtrlVal (panelHandle, PANEL_TOGGLEBUTTON,1);

	if (recall_plotid != -1)
		DeleteGraphPlot (panelHandle, PANEL_WAVEFORM, recall_plotid,
						 VAL_IMMEDIATE_DRAW);
	recall_plotid = -1;
}

// Timer-based acquisition, if set to automatic
int CVICALLBACK on_timer_acquire (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	int i;
	switch (event)
	{
		case EVENT_TIMER_TICK:
			GetCtrlVal(panel, PANEL_CHK_CTSACQUIRE, &i);
			if (i)
			{
				AcquireWaveform();
			}
			break;
	}
	return 0;
}

// Manual acquisition
int CVICALLBACK on_acquire (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			AcquireWaveform();
			break;
	}
	return 0;
}


/**********************************/
/* Waveform storage and retrieval */
/**********************************/

// Store waveform to file
void HL1101_savedata()
{   
	int fd,n,status,i;
	char buf[BUF_REC_LEN];
	int no_data = rec_len;
	double val1, val2;

	status = PromptPopup ("SAVE TO FILE","Enter File ID :",
						  save_file,MAX_SAVE_FILE);

	if (StringLength(save_file) == 0)
		return;

	fd = OpenFile(save_file,VAL_READ_WRITE,VAL_TRUNCATE,VAL_ASCII); 

	for(i=0; i<no_data; i+=4)
	{
		sprintf(buf," %e %e %e %e ", wfm_data[i],wfm_data[i+1],wfm_data[i+2],wfm_data[i+3]);
		buf[BUF_REC_LEN-1] = '\n';
		n=WriteFile(fd,buf,(unsigned)BUF_REC_LEN);
	}

	for(i=0; i<no_data; i+=4)
	{
		sprintf(buf," %e %e %e %e ", timescale[i],timescale[i+1],timescale[i+2],timescale[i+3]);
		buf[BUF_REC_LEN-1] = '\n';
		n=WriteFile(fd,buf,(unsigned)BUF_REC_LEN);
	}

	// Save timebase information
	GetCtrlVal(panelHandle, PANEL_NUM_STARTTM, &val1);
	GetCtrlVal(panelHandle, PANEL_NUM_WINDOWSZ, &val2);

	for(i=0; i<BUF_REC_LEN; i++)
		buf[i] = ' ';

	sprintf(buf," %e %e %d %e",val1, val2, HL1101_xaxis_val, HL1101_diel);
	buf[BUF_REC_LEN-1] = '\n';
	n=WriteFile(fd,buf,(unsigned)BUF_REC_LEN);

	for (i=0; i<BUF_REC_LEN; i++)
		buf[i] = ' ';

	sprintf(buf," %d %e %e ",HL1101_yaxis_val, ymin, ymax);
	buf[BUF_REC_LEN-1] = '\n';
	n=WriteFile(fd,buf,(unsigned)BUF_REC_LEN);

	n=CloseFile(fd);

}

// Set appropriate min/max of retrieved waveform
void minmax( double a[], int len, double *min, double *max)
{
	int i;
	for (i=0; i<len; i++)
	{
		if (a[i] < *min) *min = a[i];
		if (a[i] > *max) *max = a[i];
	}
	return;
}

// Retrieve stored waveform
void HL1101_retrievedata()
{
	int fd,n,status,i;
	float d1,d2,d3,d4, diel;
	int ld1;
	float ld2,ld3;
	unsigned int xaxis_val;
	double vel;

	double temp_start,temp_end,temp;
	char buf[1024];
	int no_data = rec_len;
	double horzscale[1024];

	UINT32 windowsz;

	status = FileSelectPopup ("","*.*","*.*","Select file to be RETRIEVED",
							  VAL_SELECT_BUTTON,0,0,1,1,save_file);

	if (status == VAL_NO_FILE_SELECTED)
		return;

	fd = OpenFile(save_file,VAL_READ_WRITE,VAL_OPEN_AS_IS,VAL_ASCII);

	for(i=0; i<no_data; i+=4)
	{
		n=ReadFile(fd,buf,(unsigned)BUF_REC_LEN);

		sscanf(buf," %f %f %f %f ", &d1,&d2,&d3,&d4);

		wfm_ret[i+0] = (double) d1;
		wfm_ret[i+1] = (double) d2;
		wfm_ret[i+2] = (double) d3;
		wfm_ret[i+3] = (double) d4;
	}

	for(i=0; i<no_data; i+=4)
	{
		n=ReadFile(fd,buf,(unsigned)BUF_REC_LEN);

		sscanf(buf," %f %f %f %f ", &d1,&d2,&d3,&d4);

		wfm_dist[i+0] = (double) d1;
		wfm_dist[i+1] = (double) d2;
		wfm_dist[i+2] = (double) d3;
		wfm_dist[i+3] = (double) d4;
	}

	n = ReadFile(fd,buf,(unsigned)BUF_REC_LEN);

	sscanf(buf," %f %f %d %e", &d1,&d2, &HL1101_xaxis_val, &diel);

	n = ReadFile(fd,buf,(unsigned)BUF_REC_LEN);

	sscanf(buf," %d %e %e ", &ld1, &ld2, &ld3);

	n=CloseFile(fd);

	x_axis = HL1101_xaxis_val;
	HL1101_diel = (double)diel;
	start_tm.time = (UINT32)d1;
	windowsz = (UINT32)d2;
	end_tm.time = start_tm.time + windowsz;
	HL1101_yaxis_val = ld1;
	ymin = (double)ld2;
	ymax = (double)ld3;
	auto_flag = 0;

	wfm_ret_flag = 1;
	wfm_ret_min  =  3000.00;
	wfm_ret_max  = -3000.00;
	minmax (wfm_ret,1024,&wfm_ret_min,&wfm_ret_max);
	xaxis_val = (unsigned int)HL1101_xaxis_val;

	status = SetCtrlVal(panelHandle,PANEL_NUM_STARTTM,
						(double)start_tm.time);
	status = SetCtrlVal(panelHandle,PANEL_NUM_WINDOWSZ,
						(double)windowsz);
	status = SetCtrlVal(panelHandle,PANEL_RING,
						HL1101_yaxis_val);
	status = SetCtrlVal (panelHandle, PANEL_TOGGLEBUTTON, auto_flag);
	status = SetCtrlVal (panelHandle, PANEL_NUM_YMAX, ymax);
	status = SetCtrlVal (panelHandle, PANEL_NUM_YMIN, ymin);
	SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_LEFT_YAXIS, VAL_MANUAL, (double)ymin, (double)ymax);
	SetAxisScalingMode( panelHandle, PANEL_WAVEFORM, VAL_RIGHT_YAXIS, VAL_MANUAL, (double)ymin, (double)ymax);
	status = SetCtrlVal(panelHandle, PANEL_RING_HORIZONTAL, xaxis_val);
	status = SetCtrlVal(panelHandle, PANEL_NUM_DIELECTRIC, HL1101_diel);

	if (recall_plotid != -1)
		DeleteGraphPlot (panelHandle, PANEL_WAVEFORM, -1,
						 VAL_IMMEDIATE_DRAW);

	vel = (double)3E8/sqrt(HL1101_diel);
	if (HL1101_xaxis_val == T)
	{
		for (i=0; i<no_data; i++)
		{
			horzscale[i] = wfm_dist[i];
		}
	}
	else if (HL1101_xaxis_val == M)
	{
		for (i=0; i<no_data; i++)
		{
			horzscale[i] = wfm_dist[i] * vel * 1E-9;
		}
	}
	else
	{
		for (i=0; i<no_data; i++)
		{
			horzscale[i] = wfm_dist[i] * vel* 1E-9 * MtoFT;
		}
	}


	recall_plotid = PlotXY (panelHandle, PANEL_WAVEFORM, horzscale, wfm_ret, 1024,
							VAL_DOUBLE, VAL_DOUBLE, VAL_FAT_LINE,
							VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_GREEN);
	status = SetCtrlAttribute (panelHandle,PANEL_WAVEFORM,
							   ATTR_XNAME,x_label[x_axis]);

	status = SetCtrlAttribute(panelHandle,PANEL_RING,ATTR_DIMMED,1);
	status = SetCtrlAttribute(panelHandle,PANEL_NUM_WINDOWSZ,ATTR_DIMMED,1);
	status = SetCtrlAttribute(panelHandle,PANEL_NUM_STARTTM,ATTR_DIMMED,1);

	status = SetCtrlAttribute(panelHandle,PANEL_COMMANDBUTTON,ATTR_DIMMED,1);

	status = SetCtrlAttribute(panelHandle,PANEL_NUM_YMAX, ATTR_DIMMED,1);
	status = SetCtrlAttribute(panelHandle,PANEL_NUM_YMIN,ATTR_DIMMED,1);
	status = SetCtrlAttribute(panelHandle,PANEL_TOGGLEBUTTON,ATTR_DIMMED,1);

	return;
}

// Callback to store waveform
int CVICALLBACK HL1101_save (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			HL1101_savedata();
			
			break;
	}
	return 0;
}

// Callback to retrieve stored waveform
int CVICALLBACK HL1101_recall (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			
			reset_plot();
			HL1101_retrievedata();
			
			break;
		}
	return 0;
}
 

/******************************/
/* Control-triggered routines */
/******************************/

// Basic panel functionality
int CVICALLBACK on_panel_event (int panel, int event, void *callbackData,
								int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:
			
			break;
			
		case EVENT_LOST_FOCUS:
			
			break;
			
		case EVENT_CLOSE:
			
			usbfifo_close();
			QuitUserInterface(0);
			
			break;
	}
	return 0;
}

// On waveform acquisition
int CVICALLBACK on_waveform (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	double c1x, c1y, c2x, c2y;
	static char buf[128];

	c1x = c1y = c2x = c2y = 0;

	switch (event)
	{
		case EVENT_COMMIT:

			break;

		case EVENT_VAL_CHANGED:

			GetGraphCursor(panel, control, 1, &c1x, &c1y);
			GetGraphCursor(panel, control, 2, &c2x, &c2y);

			sprintf(buf, "%.2f    %.2f", c1x, c1y);
			SetCtrlVal(panel, PANEL_STR_CURS1,  buf);

			sprintf(buf, "%.2f    %.2f", c2x, c2y);
			SetCtrlVal(panel, PANEL_STR_CURS2, buf);

			sprintf(buf, "%.2f    %.2f", c2x-c1x, c2y-c1y);
			SetCtrlVal(panel, PANEL_STR_DELTA, buf);

			break;
	}
	return 0;
}

// Window start changed
int CVICALLBACK on_starttm_change (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
			
			SetupTimescale();
			AcquireWaveform();
			
			break;
	}
	return 0;
}

// Window width changed
int CVICALLBACK on_windowsz_change (int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
			
			SetupTimescale();
			AcquireWaveform();
			
			break;
	}
	return 0;
}

// Cable type changed
int CVICALLBACK on_cable_type (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			GetCtrlVal(panelHandle, PANEL_RING_CABLE_TYPE, &HL1101_diel);
			SetCtrlVal(panelHandle, PANEL_NUM_DIELECTRIC, HL1101_diel);
			SetupTimescale();
			AcquireWaveform();
			
			break;
	}
	return 0;
}

// Dielectric (K) changed
int CVICALLBACK on_dielectric (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			GetCtrlVal(panelHandle, PANEL_NUM_DIELECTRIC, &HL1101_diel);
			SetupTimescale();
			AcquireWaveform();
			
			break;
	}
	return 0;
}

// Horizontal units changed
int CVICALLBACK on_horizontal (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			GetCtrlVal(panel, control, &HL1101_xaxis_val);
			HL1101_x_axis (panel, control);				  
			SetupTimescale();
			AcquireWaveform();
			
			break;
			
		case EVENT_RIGHT_CLICK:
			
			break;
	}
	return 0;
}

// Vertical units changed
int CVICALLBACK HL1101_y_axis (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			HL1101_y_axis_1 (panel, control);
			GetCtrlVal(panel, control, &HL1101_yaxis_val);
			AcquireWaveform();

			break;
	}
	return 0;
}

// UI reset
int HL1101_reset (int panel, int control, int event,
				  void *callbackData, int eventData1, int eventData2)
{
	int status;

	switch (event)
	{
		case EVENT_COMMIT:

			reset_plot();
			AcquireWaveform();
			
			break;
			
		case EVENT_RIGHT_CLICK:

			break;
	}
	return 0;
}

// Zoom on selection /* TO DO */
int CVICALLBACK HL1101_zoom (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			GetGraphCursor (panelHandle, PANEL_WAVEFORM, 1, &x1, &y1);
			GetGraphCursor (panelHandle, PANEL_WAVEFORM, 2, &x2, &y2);

			if (x1 < x2)
			{
				SetCtrlVal(panelHandle, PANEL_NUM_STARTTM, x1);
			}
			else
			{
				SetCtrlVal(panelHandle, PANEL_NUM_STARTTM, x2);
			}
			SetCtrlVal(panelHandle, PANEL_NUM_WINDOWSZ, fabs(x2-x1));
			SetupTimescale();
			AcquireWaveform();

			//SetCtrlVal (panelHandle, PANEL_SLIDE_2, x2);
			//GetCtrlVal(panelHandle, PANEL_SLIDE_2, &HL2100_end_val);
			//GetCtrlVal (panelHandle, PANEL_SLIDE_1, &start);
			//GetCtrlVal (panelHandle, PANEL_SLIDE_2, &end);

			//acquire();

			break;
			
		case EVENT_RIGHT_CLICK:
			
			break;
	}
	return 0;
}

// Waveform averaging changed
int CVICALLBACK on_average (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			GetCtrlVal(panel, control, &acquisition_nr);
			AcquireWaveform();
			
			break;
	}
	return 0;
}

// Print panel and waveform
int CVICALLBACK HL1101_print (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			
			(void) PrintPanel(panel, "", 1, VAL_FULL_PANEL, 1);
			
			break;
			
		case EVENT_RIGHT_CLICK:

			break;
		}
	return 0;
}

// Change record length (temporarily depricated)
int CVICALLBACK on_reclen_change (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	UINT32 val;
	switch (event)
	{
		case EVENT_VAL_CHANGED:
			
			GetCtrlVal(panelHandle, PANEL_RING_RECLEN, &val);
			rec_len = val;

			break;
	}
	return 0;
}

// SetRef functionality (temporarily depricated)
int CVICALLBACK HL1101_setref (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			
			break;
		}
	return 0;
}

// Close UI and exit
int CVICALLBACK on_quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			usbfifo_close();
			QuitUserInterface(0);
			break;
		}
	return 0;
}




// ABOVE: to organize
// BELOW: to optimize/troubleshoot

/***************************/
/* INITIALIZATION ROUTINES */
/***************************/


