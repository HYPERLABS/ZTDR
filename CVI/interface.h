/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
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
#define  PANEL_AUTOSCALE                  4       /* control type: radioButton, callback function: onSetAutoscale */
#define  PANEL_YMIN                       5       /* control type: numeric, callback function: onCheckMinMax */
#define  PANEL_YMAX                       6       /* control type: numeric, callback function: onCheckMinMax */
#define  PANEL_CURSOR1                    7       /* control type: textMsg, callback function: (none) */
#define  PANEL_CURSOR2                    8       /* control type: textMsg, callback function: (none) */
#define  PANEL_DELTA                      9       /* control type: textMsg, callback function: (none) */
#define  PANEL_AVERAGE                    10      /* control type: numeric, callback function: onGeneric */
#define  PANEL_MESSAGE                    11      /* control type: textBox, callback function: (none) */
#define  PANEL_START                      12      /* control type: scale, callback function: onChangeWindow */
#define  PANEL_END                        13      /* control type: scale, callback function: onChangeWindow */
#define  PANEL_VC                         14      /* control type: numeric, callback function: (none) */
#define  PANEL_DIEL                       15      /* control type: numeric, callback function: onChangeDiel */
#define  PANEL_ZOOM                       16      /* control type: command, callback function: onZoom */
#define  PANEL_RESETZOOM                  17      /* control type: command, callback function: onResetZoom */
#define  PANEL_WAVEFORM                   18      /* control type: graph, callback function: onWaveform */
#define  PANEL_PANELCURSOR                19      /* control type: deco, callback function: (none) */
#define  PANEL_C1                         20      /* control type: textMsg, callback function: (none) */
#define  PANEL_C2                         21      /* control type: textMsg, callback function: (none) */
#define  PANEL_D2                         22      /* control type: textMsg, callback function: (none) */
#define  PANEL_PANELBOTTOM                23      /* control type: deco, callback function: (none) */
#define  PANEL_VERSION                    24      /* control type: textMsg, callback function: (none) */
#define  PANEL_DEBUG                      25      /* control type: command, callback function: onDebug */
#define  PANEL_LED                        26      /* control type: LED, callback function: (none) */
#define  PANEL_TIMESTAMP                  27      /* control type: textMsg, callback function: (none) */

#define  PANELSAVE                        2
#define  PANELSAVE_SERIAL                 2       /* control type: string, callback function: (none) */
#define  PANELSAVE_COMMENTS               3       /* control type: textBox, callback function: (none) */


     /* Control Arrays: */

#define  BOTTOM                           1
#define  RIGHT                            2

     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                          1
#define  MENUBAR_FILE                     2
#define  MENUBAR_FILE_PRINT               3       /* callback function: onPrint */
#define  MENUBAR_FILE_SEPARATOR_3         4
#define  MENUBAR_FILE_SAVECSV             5       /* callback function: onCSV */
#define  MENUBAR_FILE_SAVEPNG             6       /* callback function: onPNG */
#define  MENUBAR_FILE_SEPARATOR           7
#define  MENUBAR_FILE_STORE               8       /* callback function: onStoreWaveform */
#define  MENUBAR_FILE_RECALL              9       /* callback function: onRecallWaveform */
#define  MENUBAR_FILE_CLEAR               10      /* callback function: onClearWaveform */
#define  MENUBAR_FILE_SEPARATOR_6         11
#define  MENUBAR_FILE_SAVESETTINGS        12      /* callback function: onSaveSettings */
#define  MENUBAR_FILE_LOADSETTINGS        13      /* callback function: onLoadSettings */
#define  MENUBAR_FILE_DEFAULTSETTINGS     14      /* callback function: onResetSettings */
#define  MENUBAR_FILE_SEPARATOR_2         15
#define  MENUBAR_FILE_EXIT                16      /* callback function: onExit */
#define  MENUBAR_DISPLAY                  17
#define  MENUBAR_DISPLAY_DOTS             18      /* callback function: onSetPlotDots */
#define  MENUBAR_DISPLAY_THINLINE         19      /* callback function: onSetPlotThin */
#define  MENUBAR_DISPLAY_FATLINE          20      /* callback function: onSetPlotThick */
#define  MENUBAR_DISPLAY_SEPARATOR_4      21
#define  MENUBAR_DISPLAY_DARK             22      /* callback function: onSetBgDark */
#define  MENUBAR_DISPLAY_LIGHT            23      /* callback function: onSetBgLight */
#define  MENUBAR_XUNITS                   24
#define  MENUBAR_XUNITS_XUNITSM           25      /* callback function: onSetXM */
#define  MENUBAR_XUNITS_XUNITSFT          26      /* callback function: onSetXFt */
#define  MENUBAR_XUNITS_XUNITSNS          27      /* callback function: onSetXNs */
#define  MENUBAR_YUNITS                   28
#define  MENUBAR_YUNITS_YUNITSMV          29      /* callback function: onSetYMV */
#define  MENUBAR_YUNITS_YUNITSNORM        30      /* callback function: onSetYNorm */
#define  MENUBAR_YUNITS_YUNITSOHM         31      /* callback function: onSetYOhm */
#define  MENUBAR_YUNITS_YUNITSRHO         32      /* callback function: onSetYRho */
#define  MENUBAR_CALIBRATION              33
#define  MENUBAR_CALIBRATION_FULLCAL      34      /* callback function: onCalTimebase */
#define  MENUBAR_CALIBRATION_SEPARATOR_5  35
#define  MENUBAR_CALIBRATION_SETZERO      36      /* callback function: onSetZero */
#define  MENUBAR_CALIBRATION_RESETZERO    37      /* callback function: onResetZero */
#define  MENUBAR_CALIBRATION_SEPARATOR_7  38
#define  MENUBAR_CALIBRATION_VERTCAL      39      /* callback function: onCalibrate */


     /* Callback Prototypes: */

int  CVICALLBACK onAcquire(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onCalibrate(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onCalTimebase(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onChangeDiel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeWindow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onCheckMinMax(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onClearWaveform(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onCSV(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onDebug(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onExit(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onGeneric(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onLoadSettings(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onPNG(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onPrint(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onRecallWaveform(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onResetSettings(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onResetZero(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onResetZoom(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onSaveSettings(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onSetAutoscale(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK onSetBgDark(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onSetBgLight(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onSetPlotDots(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onSetPlotThick(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onSetPlotThin(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onSetXFt(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onSetXM(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onSetXNs(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onSetYMV(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onSetYNorm(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onSetYOhm(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onSetYRho(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onSetZero(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK onStoreWaveform(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK onWaveform(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onZoom(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
