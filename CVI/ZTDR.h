/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2014. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: onPanel */
#define  PANEL_WAVEFORM                   2       /* control type: graph, callback function: onWaveform */
#define  PANEL_TOGGLEBUTTON               3       /* control type: textButton, callback function: (none) */
#define  PANEL_RING                       4       /* control type: ring, callback function: onChangeUnitY */
#define  PANEL_NUM_YMAX                   5       /* control type: numeric, callback function: (none) */
#define  PANEL_NUM_YMIN                   6       /* control type: numeric, callback function: (none) */
#define  PANEL_COMMANDBUTTON              7       /* control type: command, callback function: onZoom */
#define  PANEL_STR_DELTA                  8       /* control type: string, callback function: (none) */
#define  PANEL_CHK_DOTS                   9       /* control type: radioButton, callback function: (none) */
#define  PANEL_CMD_ACQUIRE                10      /* control type: command, callback function: onAcquire */
#define  PANEL_CHK_CTSACQUIRE             11      /* control type: radioButton, callback function: (none) */
#define  PANEL_NUM_WFMPERSEC              12      /* control type: numeric, callback function: onChangeAverage */
#define  PANEL_CAL_STEP1                  13      /* control type: command, callback function: onCal */
#define  PANEL_COMMANDBUTTON_2            14      /* control type: command, callback function: onVertCal */
#define  PANEL_COMMANDBUTTON_7            15      /* control type: command, callback function: onSave */
#define  PANEL_COMMANDBUTTON_6            16      /* control type: command, callback function: onRecall */
#define  PANEL_COMMANDBUTTON_9            17      /* control type: command, callback function: onReset */
#define  PANEL_COMMANDBUTTON_10           18      /* control type: command, callback function: onPrint */
#define  PANEL_COMMANDBUTTON_11           19      /* control type: command, callback function: onQuit */
#define  PANEL_RING_HORIZONTAL            20      /* control type: ring, callback function: onChangeUnitX */
#define  PANEL_NUM_STARTTM                21      /* control type: numeric, callback function: onChangeStart */
#define  PANEL_NUM_WINDOWSZ               22      /* control type: numeric, callback function: onChangeWindow */
#define  PANEL_TIMER_ACQUIRE              23      /* control type: timer, callback function: onTimer */
#define  PANEL_NUM_DIELECTRIC             24      /* control type: numeric, callback function: onChangeK */
#define  PANEL_DECORATION_4               25      /* control type: deco, callback function: (none) */
#define  PANEL_RING_RECLEN                26      /* control type: ring, callback function: (none) */
#define  PANEL_MESSAGES                   27      /* control type: string, callback function: (none) */
#define  PANEL_STR_CURS2                  28      /* control type: textMsg, callback function: (none) */
#define  PANEL_STR_CURS1                  29      /* control type: textMsg, callback function: (none) */
#define  PANEL_COMMANDBUTTON_8            30      /* control type: command, callback function: (none) */
#define  PANEL_TEXTMSG                    31      /* control type: textMsg, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK onAcquire(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onCal(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeAverage(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeUnitX(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeUnitY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeWindow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onPrint(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onRecall(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onReset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onSave(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onTimer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onVertCal(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onWaveform(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onZoom(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
