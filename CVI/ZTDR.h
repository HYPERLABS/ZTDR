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
#define  PANEL_CAL                        8       /* control type: command, callback function: onCal */
#define  PANEL_VERTCAL                    9       /* control type: command, callback function: onVertCal */
#define  PANEL_START                      10      /* control type: numeric, callback function: onChangeStart */
#define  PANEL_WINDOW                     11      /* control type: scale, callback function: onChangeWindow */
#define  PANEL_DIEL                       12      /* control type: numeric, callback function: onChangeK */
#define  PANEL_ZOOM                       13      /* control type: command, callback function: onZoom */
#define  PANEL_DOTS                       14      /* control type: radioButton, callback function: (none) */
#define  PANEL_PRINT                      15      /* control type: command, callback function: onPrint */
#define  PANEL_PNG                        16      /* control type: command, callback function: onPNG */
#define  PANEL_STORE                      17      /* control type: command, callback function: onStore */
#define  PANEL_RECALL                     18      /* control type: command, callback function: onRecall */
#define  PANEL_RESET                      19      /* control type: command, callback function: onReset */
#define  PANEL_CSV                        20      /* control type: command, callback function: onStoreCSV */
#define  PANEL_MESSAGES                   21      /* control type: textBox, callback function: (none) */
#define  PANEL_TIMER                      22      /* control type: timer, callback function: onTimer */
#define  PANEL_PANECURSOR                 23      /* control type: deco, callback function: (none) */
#define  PANEL_C1                         24      /* control type: textMsg, callback function: (none) */
#define  PANEL_CURSOR1                    25      /* control type: textMsg, callback function: (none) */
#define  PANEL_C2                         26      /* control type: textMsg, callback function: (none) */
#define  PANEL_CURSOR2                    27      /* control type: textMsg, callback function: (none) */
#define  PANEL_D2                         28      /* control type: textMsg, callback function: (none) */
#define  PANEL_PANETOP                    29      /* control type: deco, callback function: (none) */
#define  PANEL_PANEBOTTOM                 30      /* control type: deco, callback function: (none) */
#define  PANEL_QUIT                       31      /* control type: command, callback function: onQuit */
#define  PANEL_DELTA                      32      /* control type: textMsg, callback function: (none) */
#define  PANEL_WAVEFORM                   33      /* control type: graph, callback function: onWaveform */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_FILE                     2
#define  MENUBAR_XUNITS                   3
#define  MENUBAR_XUNITS_XUNITS1           4       /* callback function: onChangeX1 */
#define  MENUBAR_XUNITS_XUNITS2           5       /* callback function: onChangeX2 */
#define  MENUBAR_XUNITS_XUNITS3           6       /* callback function: onChangeX3 */
#define  MENUBAR_YUNITS                   7
#define  MENUBAR_YUNITS_YUNITS1           8       /* callback function: onChangeY1 */
#define  MENUBAR_YUNITS_YUNITS2           9       /* callback function: onChangeY2 */
#define  MENUBAR_YUNITS_YUNITS3           10      /* callback function: onChangeY3 */
#define  MENUBAR_YUNITS_YUNITS4           11      /* callback function: onChangeY4 */


     /* Callback Prototypes: */

int  CVICALLBACK onAcquire(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onAuto(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onCal(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
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
