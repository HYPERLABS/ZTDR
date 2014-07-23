#include <windows.h>
#include <formatio.h>
#include <ansi_c.h>
#include <rs232.h>
#include <cvirte.h>		
#include <userint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define VERT_HIST 100
#define CAL_THRESHOLD_HYST 100

#define BUF_REC_LEN 64
#define MAX_SAVE_FILE 100 



static double wfm_data[NPOINTS_MAX], wfm_data_ave[NPOINTS_MAX];
static double ymin, ymax; 
static  int delay_value;
static  int start_delay=0;		
static  int offset;

static startupCal = 1;

// Define functions
double mean_array(void);
void SetupTimescale(void);

/* TO DO */
char save_file[MAX_SAVE_FILE+160];


static double rise_time;
static int plotid = -1, plotid1 = -1;
static int recall_plotid = -1;
static double cal_threshold1;
static double cal_threshold2;

static double levelRight;
static double levelLeft;



int y_axis,x_axis;

int wfmpersec =1;

static int retrieve, norm_flag,auto_flag; 

 


//TO DO array sizes
/* static */ double dist[NPOINTS_MAX];
/*static */ double wfm_ret[NPOINTS_MAX];
/*static */ double wfm_dist[NPOINTS_MAX]; 
/*static */ double wfm_ret_min;
/* static */ double wfm_ret_max;
/* static */ int wfm_ret_flag = 0;
//double x1, x2, y1, y2, r1, p1,  dist_x1x2, b1, b2, e1, e2; 
static double x1, x2, y1, y2;

// Call vertcal routines
void vert_cal()
{
	PerformVertCal();
	SetupTimescale();
}

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
