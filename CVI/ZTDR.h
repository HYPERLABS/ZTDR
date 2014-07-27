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
#define  PANEL_YUNITS                     2       /* control type: ring, callback function: onChangeUnitY */
#define  PANEL_AUTOSCALE                  3       /* control type: radioButton, callback function: onAuto */
#define  PANEL_YMIN                       4       /* control type: numeric, callback function: onGeneric */
#define  PANEL_YMAX                       5       /* control type: numeric, callback function: onGeneric */
#define  PANEL_AVERAGE                    6       /* control type: numeric, callback function: onChangeAverage */
#define  PANEL_ACQUIRE                    7       /* control type: command, callback function: onAcquire */
#define  PANEL_CONTINUOUS                 8       /* control type: radioButton, callback function: (none) */
#define  PANEL_CAL                        9       /* control type: command, callback function: onCal */
#define  PANEL_VERTCAL                    10      /* control type: command, callback function: onVertCal */
#define  PANEL_XUNITS                     11      /* control type: ring, callback function: onChangeUnitX */
#define  PANEL_START                      12      /* control type: numeric, callback function: onChangeStart */
#define  PANEL_WINDOW                     13      /* control type: scale, callback function: onChangeWindow */
#define  PANEL_DIEL                       14      /* control type: numeric, callback function: onChangeK */
#define  PANEL_ZOOM                       15      /* control type: command, callback function: onZoom */
#define  PANEL_DOTS                       16      /* control type: radioButton, callback function: (none) */
#define  PANEL_PRINT                      17      /* control type: command, callback function: onPrint */
#define  PANEL_PNG                        18      /* control type: command, callback function: onPNG */
#define  PANEL_STORE                      19      /* control type: command, callback function: onStore */
#define  PANEL_RECALL                     20      /* control type: command, callback function: onRecall */
#define  PANEL_RESET                      21      /* control type: command, callback function: onReset */
#define  PANEL_CSV                        22      /* control type: command, callback function: onStoreCSV */
#define  PANEL_MESSAGES                   23      /* control type: textBox, callback function: (none) */
#define  PANEL_TIMER                      24      /* control type: timer, callback function: onTimer */
#define  PANEL_PANECURSOR                 25      /* control type: deco, callback function: (none) */
#define  PANEL_C1                         26      /* control type: textMsg, callback function: (none) */
#define  PANEL_CURSOR1                    27      /* control type: textMsg, callback function: (none) */
#define  PANEL_C2                         28      /* control type: textMsg, callback function: (none) */
#define  PANEL_CURSOR2                    29      /* control type: textMsg, callback function: (none) */
#define  PANEL_D2                         30      /* control type: textMsg, callback function: (none) */
#define  PANEL_PANETOP                    31      /* control type: deco, callback function: (none) */
#define  PANEL_PANEBOTTOM                 32      /* control type: deco, callback function: (none) */
#define  PANEL_QUIT                       33      /* control type: command, callback function: onQuit */
#define  PANEL_DELTA                      34      /* control type: textMsg, callback function: (none) */
#define  PANEL_WAVEFORM                   35      /* control type: graph, callback function: onWaveform */


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
void CVICALLBACK onChangeX1(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeX2(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onChangeX3(int menubar, int menuItem, void *callbackData, int panel);
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
