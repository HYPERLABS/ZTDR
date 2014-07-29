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
#define  PANEL_AUTOACQUIRE                2       /* control type: radioButton, callback function: (none) */
#define  PANEL_ACQUIRE                    3       /* control type: command, callback function: onAcquire */
#define  PANEL_AUTOSCALE                  4       /* control type: radioButton, callback function: onAuto */
#define  PANEL_YMIN                       5       /* control type: numeric, callback function: onGeneric */
#define  PANEL_YMAX                       6       /* control type: numeric, callback function: onGeneric */
#define  PANEL_CURSOR1                    7       /* control type: textMsg, callback function: (none) */
#define  PANEL_CURSOR2                    8       /* control type: textMsg, callback function: (none) */
#define  PANEL_DELTA                      9       /* control type: textMsg, callback function: (none) */
#define  PANEL_AVERAGE                    10      /* control type: numeric, callback function: onChangeAverage */
#define  PANEL_MESSAGES                   11      /* control type: textBox, callback function: (none) */
#define  PANEL_START                      12      /* control type: scale, callback function: onChangeStart */
#define  PANEL_END                        13      /* control type: scale, callback function: onChangeEnd */
#define  PANEL_DIEL                       14      /* control type: numeric, callback function: onChangeK */
#define  PANEL_ZOOM                       15      /* control type: command, callback function: onZoom */
#define  PANEL_RESET                      16      /* control type: command, callback function: onReset */
#define  PANEL_CLEAR                      17      /* control type: command, callback function: onClear */
#define  PANEL_WAVEFORM                   18      /* control type: graph, callback function: onWaveform */
#define  PANEL_PANELCURSOR                19      /* control type: deco, callback function: (none) */
#define  PANEL_C1                         20      /* control type: textMsg, callback function: (none) */
#define  PANEL_C2                         21      /* control type: textMsg, callback function: (none) */
#define  PANEL_D2                         22      /* control type: textMsg, callback function: (none) */
#define  PANEL_PANEBOTTOM                 23      /* control type: deco, callback function: (none) */
#define  PANEL_TIMER                      24      /* control type: timer, callback function: onTimer */
#define  PANEL_VERSION                    25      /* control type: textMsg, callback function: (none) */


     /* Control Arrays: */

#define  BOTTOM                           1
#define  RIGHT                            2

     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_FILE                     2
#define  MENUBAR_FILE_SAVESETTINGS        3
#define  MENUBAR_FILE_LOADSETTINGS        4
#define  MENUBAR_FILE_SEPARATOR_2         5
#define  MENUBAR_FILE_EXIT                6       /* callback function: onExit */
#define  MENUBAR_DATA                     7
#define  MENUBAR_DATA_PRINT               8       /* callback function: onPrint */
#define  MENUBAR_DATA_SEPARATOR_3         9
#define  MENUBAR_DATA_SAVECSV             10      /* callback function: onCSV */
#define  MENUBAR_DATA_SAVEPNG             11      /* callback function: onPNG */
#define  MENUBAR_DATA_SEPARATOR           12
#define  MENUBAR_DATA_STORE               13      /* callback function: onStore */
#define  MENUBAR_DATA_RECALL              14      /* callback function: onRecall */
#define  MENUBAR_DATA_CLEAR               15      /* callback function: onClearMenu */
#define  MENUBAR_PLOT                     16
#define  MENUBAR_PLOT_DOTS                17      /* callback function: onChangePlot1 */
#define  MENUBAR_PLOT_THINLINE            18      /* callback function: onChangePlot2 */
#define  MENUBAR_PLOT_FATLINE             19      /* callback function: onChangePlot3 */
#define  MENUBAR_XUNITS                   20
#define  MENUBAR_XUNITS_XUNITS1           21      /* callback function: onChangeX1 */
#define  MENUBAR_XUNITS_XUNITS2           22      /* callback function: onChangeX2 */
#define  MENUBAR_XUNITS_XUNITS3           23      /* callback function: onChangeX3 */
#define  MENUBAR_YUNITS                   24
#define  MENUBAR_YUNITS_YUNITS1           25      /* callback function: onChangeY1 */
#define  MENUBAR_YUNITS_YUNITS2           26      /* callback function: onChangeY2 */
#define  MENUBAR_YUNITS_YUNITS3           27      /* callback function: onChangeY3 */
#define  MENUBAR_YUNITS_YUNITS4           28      /* callback function: onChangeY4 */
#define  MENUBAR_CALIBRATION              29
#define  MENUBAR_CALIBRATION_VERTCAL      30      /* callback function: onVertCal */
#define  MENUBAR_CALIBRATION_FULLCAL      31      /* callback function: onTimeCal */
#define  MENUBAR_CALIBRATION_AUTOCAL      32


     /* Callback Prototypes: */

int  CVICALLBACK onAcquire(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onAuto(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeAverage(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeEnd(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onChangePlot1(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangePlot2(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangePlot3(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onChangeStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onChangeX1(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeX2(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeX3(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeY1(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeY2(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeY3(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeY4(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onClear(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onClearMenu(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onCSV(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onExit(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onGeneric(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onPNG(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onPrint(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onRecall(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onReset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onStore(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onTimeCal(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onTimer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onVertCal(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onWaveform(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onZoom(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
