/*
    ================
    main.c
    ================
    Strong Bad's Time Machine
    PalmOS
    2003/05/27
    */

#include "sbtm.h"
#include <Font.h>
#include <FontSelect.h>

#define nSpeech	28

int Speech[3][nSpeech] =
{
    {
        Angry0, Angry1, Angry2, Angry3, Angry4, Angry5, Angry6, Angry7, Angry8, Angry9,
        Angry10, Angry11, Angry12, Angry13, Angry14, Angry15, Angry16, Angry17, Angry18, Angry19,
        Angry20, Angry30, Angry40, Angry50, AngryAM, AngryPM, AngryOClock, AngryCurrent
    },
    {
        Normal0, Normal1, Normal2, Normal3, Normal4, Normal5, Normal6, Normal7, Normal8, Normal9,
        Normal10, Normal11, Normal12, Normal13, Normal14, Normal15, Normal16, Normal17, Normal18, Normal19,
        Normal20, Normal30, Normal40, Normal50, NormalAM, NormalPM, NormalOClock, NormalCurrent
    },
    {
        Soothing0, Soothing1, Soothing2, Soothing3, Soothing4, Soothing5, Soothing6, Soothing7, Soothing8, Soothing9,
        Soothing10, Soothing11, Soothing12, Soothing13, Soothing14, Soothing15, Soothing16, Soothing17, Soothing18, Soothing19,
        Soothing20, Soothing30, Soothing40, Soothing50, SoothingAM, SoothingPM, SoothingOClock, SoothingCurrent
    },
};

enum { Angry, Plain, Soothing };

enum {
    Speech0, Speech1, Speech2, Speech3, Speech4, Speech5, Speech6, Speech7, Speech8, Speech9,
    Speech10, Speech11, Speech12, Speech13, Speech14, Speech15, Speech16, Speech17, Speech18, Speech19,
    Speech20, Speech30, Speech40, Speech50, SpeechAM, SpeechPM, SpeechOClock, SpeechCurrent
};

unsigned char Mood = Plain;

int	ExtendedSound = false;

static	top_o_hour_chime = false,
bottom_o_hour_chime = false;


#define err(x) SysFatalAlert(x)

/*----------------------------------------------------------------------------*/
static void MainFormInit(FormType *frmP)
{
    //FieldType	*field;
    //const char	*wizardDescription;
    //UInt16		fieldIndex;

    //fieldIndex = FrmGetObjectIndex(frmP, MainDescriptionField);
    //field = (FieldType *)FrmGetObjectPtr(frmP, fieldIndex);
    //FrmSetFocus(frmP, fieldIndex);

    //wizardDescription =
    //	"C application\n"
    //	"Creator Code: SBTM\n"
    //	"\n"
    //	"Other SDKs:\n"
    //	;

    /* dont stack FldInsert calls, since each one generates a
     * fldChangedEvent, and multiple uses can overflow the event queue */
    //FldInsert(field, wizardDescription, StrLen(wizardDescription));
}
/*----------------------------------------------------------------------------*/
static Boolean MainFormDoCommand(UInt16 command)
{
    Boolean handled = false;

    switch (command)
    {
    case MenuPlain:		Mood = Plain;		handled = true;	break;
    case MenuAngry:	Mood = Angry; 		handled = true;	break;
    case MenuSoothing:	Mood = Soothing;	handled = true;	break;

    case OptionsAboutSBTimeMachine:
    {
                                      FormType	*frmP;

                                      /* Clear the menu status from the display */
                                      MenuEraseStatus(0);

                                      /* Display the About Box. */
                                      frmP = FrmInitForm(AboutForm);
                                      FrmDoDialog(frmP);
                                      FrmDeleteForm(frmP);

                                      handled = true;
                                      break;
    }

    }

    return handled;
}
/*---------------------------------------------------------------------------*/
void Talk(int value)
{
    if (value > nSpeech) {
        return;
    }
    WinDrawBitmapEx(MouthOpen, 64, 98);
    SndPlayResourceEx(Speech[Mood][value]);
    WinDrawBitmapEx(MouthClosed, 64, 98);
}
/*---------------------------------------------------------------------------*/
void DoSayTime()
{
    DateTimeType dt;
    int ampm = 0;

    TimSecondsToDateTime(TimGetSeconds(), &dt);

    // Get AM/PM
    if (dt.hour > 12) {
        ampm = 1;		// 1 = PM
    }

    // "The Current Time is..."
    Talk(SpeechCurrent);
    SysTaskDelay(10);

    // Convert 24 to 12
    if (ampm) {
        Talk(dt.hour - 12);
    }
    else {
        if (dt.hour == 0) {
            Talk(Speech12);
        }
        else {
            Talk(dt.hour);
        }
    }

    SysTaskDelay(15);

    if (dt.minute == 0) {							// 0 - O'Clock
        Talk(SpeechOClock);
        SysTaskDelay(20);
    }
    else if (dt.minute < 20) {
        if (dt.minute > 9) {						// 10-19
            Talk(dt.minute);
        }
        else {								// 0-9
            Talk(Speech0); SysTaskDelay(10);
            Talk(dt.minute);
        }
    }
    else {									// 20-59
        SysTaskDelay(30);
        if (dt.minute > 19) {
            switch ((int)(dt.minute / 10)) {
            case 0: Talk(Speech0);	break;
            case 2:	Talk(Speech20); break;
            case 3:	Talk(Speech30); break;
            case 4:	Talk(Speech40); break;
            case 5:	Talk(Speech50); break;
            }
            if ((dt.minute % 10) > 0) {
                SysTaskDelay(10);
                Talk(dt.minute % 10);
            } // Else stay quiet, it ends in 0
        }
        else {
            Talk(dt.minute);		 // Remove this?
        }
    }
    SysTaskDelay(10);
    if (ampm) {
        Talk(SpeechPM);
    }
    else {
        Talk(SpeechAM);
    }
}
/*---------------------------------------------------------------------------*/
void MainLoop()
{
    DateTimeType	dt;
    ControlType	*ctrl;
    char			temp[80];
    FormPtr		F = FrmGetActiveForm();

    TimSecondsToDateTime(TimGetSeconds(), &dt);
    TimeToAscii(dt.hour, dt.minute, tfColonAMPM, temp);

    FntSetFont(largeBoldFont);
    WinDrawChars(temp, strlen(temp), 3, 16);

    //dt.minute = 30;

    if (top_o_hour_chime && (dt.minute == 0)) {		// Just chime once at the top of the hour...
        DoSayTime();
        top_o_hour_chime = false;
        bottom_o_hour_chime = true;
    }
    else if (bottom_o_hour_chime && (dt.minute == 30)) { // ...and again at the half-hour mark.
        DoSayTime();
        top_o_hour_chime = true;
        bottom_o_hour_chime = false;
    }

    //StrPrintF(temp, "[%d]", rand()%1024);
    //FrmSetTitle(FrmGetActiveForm(), temp);	
}
/*---------------------------------------------------------------------------*/
static Boolean MainFormHandleEvent(EventType * eventP)
{
    Boolean		handled = false;
    FormType	*frmP;

    switch (eventP->eType)
    {
    case nilEvent:
        MainLoop();
        handled = true;
        break;

    case menuEvent:
        return MainFormDoCommand(eventP->data.menu.itemID);

    case frmOpenEvent:
        frmP = FrmGetActiveForm();
        FrmDrawForm(frmP);
        MainFormInit(frmP);
        handled = true;
        break;

    case frmUpdateEvent:
        /*
         * To do any custom drawing here, first call
         * FrmDrawForm(), then do your drawing, and
         * then set handled to true.
         */
        break;

    case ctlSelectEvent:
    {
                           switch (eventP->data.ctlSelect.controlID) {
                           case SayTime: {
                                             //FormLabelType	*field = (FormLabelType*)GetObjectPtr(TimeText);
                                             DoSayTime();
                                             break;
                           }
                           }
    }
    }

    return handled;
}

/*===========================================================================*/
static Err AppStart(void)
{
    DateTimeType	dt;

    CheckSoundCaps();
    WinSetDrawMode(winPaint);

    TimSecondsToDateTime(TimGetSeconds(), &dt);

    if ((dt.minute == 0) || (dt.minute > 30)) {
        top_o_hour_chime = true;
    }
    else if (dt.minute <= 30) {
        bottom_o_hour_chime = true;
    }

    return errNone;
}
/*---------------------------------------------------------------------------*/
static void AppStop(void)
{
    /* Close all the open forms. */
    FrmCloseAllForms();
}
/*---------------------------------------------------------------------------*/
static Boolean AppHandleEvent(EventType *eventP)
{
    UInt16		formId;
    FormType	*frmP;

    if (eventP->eType == frmLoadEvent)
    {
        /* Load the form resource. */
        formId = eventP->data.frmLoad.formID;
        frmP = FrmInitForm(formId);
        FrmSetActiveForm(frmP);

        /*
         * Set the event handler for the form.  The handler of the
         * currently active form is called by FrmHandleEvent each
         * time is receives an event.
         */
        switch (formId)
        {
        case MainForm:
            FrmSetEventHandler(frmP, MainFormHandleEvent);
            break;

        }
        return true;
    }

    return false;
}
/*---------------------------------------------------------------------------*/
static void AppEventLoop(void)
{
    UInt16		error;
    EventType	event;

    do
    {
        /* change timeout if you need periodic nilEvents */
        EvtGetEvent(&event, 20);

        if (!SysHandleEvent(&event))
        {
            if (!MenuHandleEvent(0, &event, &error))
            {
                if (!AppHandleEvent(&event))
                {
                    FrmDispatchEvent(&event);
                }
            }
        }
    } while (event.eType != appStopEvent);
}
/*============================================================================*/
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    Err error;

    error = RomVersionCompatible(ourMinVersion, launchFlags);
    if (error) {
        return (error);
    }

    switch (cmd)
    {
    case sysAppLaunchCmdNormalLaunch:
        error = AppStart();
        if (error) {
            return error;
        }

        /*
         * start application by opening the main form
         * and then entering the main event loop
         */
        FrmGotoForm(MainForm);
        AppEventLoop();

        AppStop();
        break;
    }

    return errNone;
}
