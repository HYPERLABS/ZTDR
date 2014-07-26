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
#define  PANEL_YUNITS                     3       /* control type: ring, callback function: onChangeUnitY */
#define  PANEL_AUTOSCALE                  4       /* control type: radioButton, callback function: onAuto */
#define  PANEL_YMIN                       5       /* control type: numeric, callback function: onGeneric */
#define  PANEL_YMAX                       6       /* control type: numeric, callback function: onGeneric */
#define  PANEL_AVERAGE                    7       /* control type: numeric, callback function: onChangeAverage */
#define  PANEL_ACQUIRE                    8       /* control type: command, callback function: onAcquire */
#define  PANEL_CONTINUOUS                 9       /* control type: radioButton, callback function: (none) */
#define  PANEL_CAL                        10      /* control type: command, callback function: onCal */
#define  PANEL_VERTCAL                    11      /* control type: command, callback function: onVertCal */
#define  PANEL_XUNITS                     12      /* control type: ring, callback function: onChangeUnitX */
#define  PANEL_START                      13      /* control type: numeric, callback function: onChangeStart */
#define  PANEL_WINDOW                     14      /* control type: scale, callback function: onChangeWindow */
#define  PANEL_DIEL                       15      /* control type: numeric, callback function: onChangeK */
#define  PANEL_ZOOM                       16      /* control type: command, callback function: onZoom */
#define  PANEL_DOTS                       17      /* control type: radioButton, callback function: (none) */
#define  PANEL_PRINT                      18      /* control type: command, callback function: onPrint */
#define  PANEL_PNG                        19      /* control type: command, callback function: onPNG */
#define  PANEL_STORE                      20      /* control type: command, callback function: onStore */
#define  PANEL_RECALL                     21      /* control type: command, callback function: onRecall */
#define  PANEL_RESET                      22      /* control type: command, callback function: onReset */
#define  PANEL_CSV                        23      /* control type: command, callback function: onStoreCSV */
#define  PANEL_MESSAGES                   24      /* control type: textBox, callback function: (none) */
#define  PANEL_TIMER                      25      /* control type: timer, callback function: onTimer */
#define  PANEL_PANECURSOR                 26      /* control type: deco, callback function: (none) */
#define  PANEL_CURSOR1                    27      /* control type: textMsg, callback function: (none) */
#define  PANEL_CURSOR2                    28      /* control type: textMsg, callback function: (none) */
#define  PANEL_DELTA                      29      /* control type: textMsg, callback function: (none) */
#define  PANEL_QUIT                       30      /* control type: command, callback function: onQuit */
#define  PANEL_PANETOP                    31      /* control type: deco, callback function: (none) */
#define  PANEL_PANEBOTTOM                 32      /* control type: deco, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK onAcquire(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onAuto(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onCal(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeAverage(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeUnitX(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeUnitY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeWindow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onGeneric(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onPNG(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onPrint(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onRecall(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onReset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onStore(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onStoreCSV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onTimer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onVertCal(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onWaveform(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onZoom(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
