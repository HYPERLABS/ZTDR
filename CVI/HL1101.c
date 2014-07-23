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
/*static */ 
/*static */ double wfm_dist[NPOINTS_MAX]; 
/*static */ double wfm_ret_min;
/* static */ double wfm_ret_max;
/* static */ int wfm_ret_flag = 0;
//double x1, x2, y1, y2, r1, p1,  dist_x1x2, b1, b2, e1, e2; 
static double x1, x2, y1, y2;

