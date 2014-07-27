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
#define  PANEL_AUTOSCALE                  2       /* control type: radioButton, callback function: onAuto */
#define  PANEL_YMIN                       3       /* control type: numeric, callback function: onGeneric */
#define  PANEL_YMAX                       4       /* control type: numeric, callback function: onGeneric */
#define  PANEL_AVERAGE                    5       /* control type: numeric, callback function: onChangeAverage */
#define  PANEL_ACQUIRE                    6       /* control type: command, callback function: onAcquire */
#define  PANEL_CONTINUOUS                 7       /* control type: radioButton, callback function: (none) */
#define  PANEL_START                      8       /* control type: numeric, callback function: onChangeStart */
#define  PANEL_WINDOW                     9       /* control type: scale, callback function: onChangeWindow */
#define  PANEL_DIEL                       10      /* control type: numeric, callback function: onChangeK */
#define  PANEL_ZOOM                       11      /* control type: command, callback function: onZoom */
#define  PANEL_DOTS                       12      /* control type: radioButton, callback function: (none) */
#define  PANEL_PRINT                      13      /* control type: command, callback function: onPrint */
#define  PANEL_PNG                        14      /* control type: command, callback function: onPNG */
#define  PANEL_STORE                      15      /* control type: command, callback function: onStore */
#define  PANEL_RECALL                     16      /* control type: command, callback function: onRecall */
#define  PANEL_RESET                      17      /* control type: command, callback function: onReset */
#define  PANEL_MESSAGES                   18      /* control type: textBox, callback function: (none) */
#define  PANEL_TIMER                      19      /* control type: timer, callback function: onTimer */
#define  PANEL_PANECURSOR                 20      /* control type: deco, callback function: (none) */
#define  PANEL_C1                         21      /* control type: textMsg, callback function: (none) */
#define  PANEL_CURSOR1                    22      /* control type: textMsg, callback function: (none) */
#define  PANEL_C2                         23      /* control type: textMsg, callback function: (none) */
#define  PANEL_CURSOR2                    24      /* control type: textMsg, callback function: (none) */
#define  PANEL_D2                         25      /* control type: textMsg, callback function: (none) */
#define  PANEL_PANELTOP                   26      /* control type: deco, callback function: (none) */
#define  PANEL_PANEBOTTOM                 27      /* control type: deco, callback function: (none) */
#define  PANEL_DELTA                      28      /* control type: textMsg, callback function: (none) */
#define  PANEL_WAVEFORM                   29      /* control type: graph, callback function: onWaveform */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_FILE                     2
#define  MENUBAR_FILE_EXIT                3       /* callback function: onExit */
#define  MENUBAR_DATA                     4
#define  MENUBAR_DATA_SAVECSV             5       /* callback function: onCSV */
#define  MENUBAR_DATA_SAVEPNG             6
#define  MENUBAR_DATA_SEPARATOR           7
#define  MENUBAR_DATA_STORE               8
#define  MENUBAR_DATA_RECALL              9
#define  MENUBAR_XUNITS                   10
#define  MENUBAR_XUNITS_XUNITS1           11      /* callback function: onChangeX1 */
#define  MENUBAR_XUNITS_XUNITS2           12      /* callback function: onChangeX2 */
#define  MENUBAR_XUNITS_XUNITS3           13      /* callback function: onChangeX3 */
#define  MENUBAR_YUNITS                   14
#define  MENUBAR_YUNITS_YUNITS1           15      /* callback function: onChangeY1 */
#define  MENUBAR_YUNITS_YUNITS2           16      /* callback function: onChangeY2 */
#define  MENUBAR_YUNITS_YUNITS3           17      /* callback function: onChangeY3 */
#define  MENUBAR_YUNITS_YUNITS4           18      /* callback function: onChangeY4 */
#define  MENUBAR_CALIBRATION              19
#define  MENUBAR_CALIBRATION_VERTCAL      20      /* callback function: onVertCal */
#define  MENUBAR_CALIBRATION_FULLCAL      21      /* callback function: onTimeCal */


     /* Callback Prototypes: */

int  CVICALLBACK onAcquire(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onAuto(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeAverage(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeWindow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onChangeX1(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeX2(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeX3(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeY1(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeY2(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeY3(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeY4(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onCSV(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onExit(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onGeneric(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onPNG(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onPrint(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onRecall(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onReset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onStore(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onTimeCal(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onTimer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onVertCal(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onWaveform(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onZoom(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
