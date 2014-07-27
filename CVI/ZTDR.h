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
#define  PANEL_AUTOACQUIRE                7       /* control type: radioButton, callback function: (none) */
#define  PANEL_START                      8       /* control type: numeric, callback function: onChangeStart */
#define  PANEL_WINDOW                     9       /* control type: scale, callback function: onChangeWindow */
#define  PANEL_DIEL                       10      /* control type: numeric, callback function: onChangeK */
#define  PANEL_RESETZOOM                  11      /* control type: command, callback function: (none) */
#define  PANEL_ZOOM                       12      /* control type: command, callback function: onZoom */
#define  PANEL_DOTS                       13      /* control type: radioButton, callback function: (none) */
#define  PANEL_PRINT                      14      /* control type: command, callback function: onPrint */
#define  PANEL_PNG                        15      /* control type: command, callback function: onPNG */
#define  PANEL_STORE                      16      /* control type: command, callback function: onStore */
#define  PANEL_RECALL                     17      /* control type: command, callback function: onRecall */
#define  PANEL_RESET                      18      /* control type: command, callback function: onReset */
#define  PANEL_MESSAGES                   19      /* control type: textBox, callback function: (none) */
#define  PANEL_TIMER                      20      /* control type: timer, callback function: onTimer */
#define  PANEL_PANECURSOR                 21      /* control type: deco, callback function: (none) */
#define  PANEL_C1                         22      /* control type: textMsg, callback function: (none) */
#define  PANEL_CURSOR1                    23      /* control type: textMsg, callback function: (none) */
#define  PANEL_C2                         24      /* control type: textMsg, callback function: (none) */
#define  PANEL_CURSOR2                    25      /* control type: textMsg, callback function: (none) */
#define  PANEL_D2                         26      /* control type: textMsg, callback function: (none) */
#define  PANEL_WAVEFORM                   27      /* control type: graph, callback function: onWaveform */
#define  PANEL_PANEBOTTOM                 28      /* control type: deco, callback function: (none) */
#define  PANEL_TEXTMSG                    29      /* control type: textMsg, callback function: (none) */
#define  PANEL_DELTA                      30      /* control type: textMsg, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_FILE                     2
#define  MENUBAR_FILE_SAVESETTINGS        3
#define  MENUBAR_FILE_LOADSETTINGS        4
#define  MENUBAR_FILE_SEPARATOR_2         5
#define  MENUBAR_FILE_EXIT                6       /* callback function: onExit */
#define  MENUBAR_DATA                     7
#define  MENUBAR_DATA_SAVECSV             8       /* callback function: onCSV */
#define  MENUBAR_DATA_SAVEPNG             9
#define  MENUBAR_DATA_SEPARATOR           10
#define  MENUBAR_DATA_STORE               11
#define  MENUBAR_DATA_RECALL              12
#define  MENUBAR_DISPLAY                  13
#define  MENUBAR_DISPLAY_DOTS             14
#define  MENUBAR_DISPLAY_LINE             15
#define  MENUBAR_XUNITS                   16
#define  MENUBAR_XUNITS_XUNITS1           17      /* callback function: onChangeX1 */
#define  MENUBAR_XUNITS_XUNITS2           18      /* callback function: onChangeX2 */
#define  MENUBAR_XUNITS_XUNITS3           19      /* callback function: onChangeX3 */
#define  MENUBAR_YUNITS                   20
#define  MENUBAR_YUNITS_YUNITS1           21      /* callback function: onChangeY1 */
#define  MENUBAR_YUNITS_YUNITS2           22      /* callback function: onChangeY2 */
#define  MENUBAR_YUNITS_YUNITS3           23      /* callback function: onChangeY3 */
#define  MENUBAR_YUNITS_YUNITS4           24      /* callback function: onChangeY4 */
#define  MENUBAR_CALIBRATION              25
#define  MENUBAR_CALIBRATION_VERTCAL      26      /* callback function: onVertCal */
#define  MENUBAR_CALIBRATION_FULLCAL      27      /* callback function: onTimeCal */
#define  MENUBAR_CALIBRATION_AUTOCAL      28


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
