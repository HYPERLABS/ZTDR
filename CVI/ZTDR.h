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

#define  PANEL                            1       /* callback function: on_panel_event */
#define  PANEL_WAVEFORM                   2       /* control type: graph, callback function: on_waveform */
#define  PANEL_TOGGLEBUTTON               3       /* control type: textButton, callback function: (none) */
#define  PANEL_RING                       4       /* control type: ring, callback function: onChangeUnitY */
#define  PANEL_NUM_YMAX                   5       /* control type: numeric, callback function: (none) */
#define  PANEL_NUM_YMIN                   6       /* control type: numeric, callback function: (none) */
#define  PANEL_STR_CURS1                  7       /* control type: string, callback function: (none) */
#define  PANEL_STR_CURS2                  8       /* control type: string, callback function: (none) */
#define  PANEL_COMMANDBUTTON              9       /* control type: command, callback function: HL1101_zoom */
#define  PANEL_STR_DELTA                  10      /* control type: string, callback function: (none) */
#define  PANEL_CHK_DOTS                   11      /* control type: radioButton, callback function: (none) */
#define  PANEL_CMD_ACQUIRE                12      /* control type: command, callback function: on_acquire */
#define  PANEL_CHK_CTSACQUIRE             13      /* control type: radioButton, callback function: (none) */
#define  PANEL_NUM_WFMPERSEC              14      /* control type: numeric, callback function: on_average */
#define  PANEL_CAL_STEP1                  15      /* control type: command, callback function: on_cal */
#define  PANEL_COMMANDBUTTON_2            16      /* control type: command, callback function: on_vertcal */
#define  PANEL_COMMANDBUTTON_7            17      /* control type: command, callback function: HL1101_save */
#define  PANEL_COMMANDBUTTON_6            18      /* control type: command, callback function: HL1101_recall */
#define  PANEL_COMMANDBUTTON_9            19      /* control type: command, callback function: HL1101_reset */
#define  PANEL_COMMANDBUTTON_10           20      /* control type: command, callback function: HL1101_print */
#define  PANEL_COMMANDBUTTON_11           21      /* control type: command, callback function: on_quit */
#define  PANEL_RING_HORIZONTAL            22      /* control type: ring, callback function: onChangeUnitX */
#define  PANEL_NUM_STARTTM                23      /* control type: scale, callback function: onChangeStart */
#define  PANEL_NUM_WINDOWSZ               24      /* control type: scale, callback function: onChangeWindow */
#define  PANEL_TIMER_ACQUIRE              25      /* control type: timer, callback function: on_timer_acquire */
#define  PANEL_NUM_DIELECTRIC             26      /* control type: scale, callback function: on_dielectric */
#define  PANEL_RING_CABLE_TYPE            27      /* control type: ring, callback function: on_cable_type */
#define  PANEL_DECORATION_4               28      /* control type: deco, callback function: on_average */
#define  PANEL_RING_RECLEN                29      /* control type: ring, callback function: on_reclen_change */
#define  PANEL_COMMANDBUTTON_8            30      /* control type: command, callback function: HL1101_setref */
#define  PANEL_MESSAGES                   31      /* control type: string, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK HL1101_print(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK HL1101_recall(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK HL1101_reset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK HL1101_save(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK HL1101_setref(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK HL1101_zoom(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_acquire(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_average(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_cable_type(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_cal(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_dielectric(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_panel_event(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_reclen_change(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_timer_acquire(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_vertcal(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK on_waveform(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeUnitX(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeUnitY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onChangeWindow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
