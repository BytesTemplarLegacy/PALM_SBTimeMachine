/*
 * PWav - Main program
 *
 *  $Id: main.c,v 1.1 2002/04/02 19:49:18 ldrolez Exp $ 
 *
 *  Copyright (C) 2000 Ludovic Drolez
 *
 *   This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  
 *
 */
 
#include <PalmOS.h>

#include <Unix/sys_types.h>

#include "main_rsrc.h"

#include <FloatMgr.h>
//#include "swav.h"

static Err StartApplication (void)
{
   FrmGotoForm (MainForm);
   return 0;
}

static void StopApplication (void)
{
}

// returns field that has the focus, if any, including in embedded tables
static FieldPtr GetFocusObjectPtr (void)
{
   FormPtr frm;
   UInt16  focus;
   FormObjectKind objType;

   frm = FrmGetActiveForm ();
   focus = FrmGetFocus (frm);
   if (focus == noFocus)
      return (NULL);

   objType = FrmGetObjectType (frm, focus);

   if (objType == frmFieldObj)
      return (FrmGetObjectPtr (frm, focus));

   else if (objType == frmTableObj)
      return (TblGetCurrentField (FrmGetObjectPtr (frm, focus)));

   return NULL;
}

static Boolean HandleCommonMenuItems (UInt16 menuID)
{
   FieldPtr fld;

   switch (menuID) {
    case EditUndo:
    case EditCut:
    case EditCopy:
    case EditPaste:
    case EditSelectAll:
      fld = (FieldPtr) GetFocusObjectPtr ();
      if (!fld)
	 return false;
      if (menuID == EditUndo)
	 FldUndo (fld);
      else if (menuID == EditCut)
	 FldCut (fld);
      else if (menuID == EditCopy)
	 FldCopy (fld);
      else if (menuID == EditPaste)
	 FldCopy (fld);
      else if (menuID == EditSelectAll)
	 FldSetSelection (fld, 0, FldGetTextLength (fld));
      return true;

    case EditKeyboard:
      SysKeyboardDialog (kbdDefault);
      return true;

    case EditGrafitti:
      SysGraffitiReferenceDialog (referenceDefault);
      return true;

    case OptionsAbout:
      FrmAlert (AboutBoxAlert);
      return true;

    default:
      return false;
   }
}

Boolean MyFormHandleMenuEvent (UInt16 menuID)
{
   if (HandleCommonMenuItems (menuID))
      return true;
   else
      switch (menuID) {
	 // other items here  
      }
   return false;
}

static Boolean MyFormHandleEvent (EventPtr event)
{
   Boolean handled = false;

   switch (event->eType) {
    case ctlSelectEvent:	// A control button was pressed and released.


      //FrmAlert(MainAlert);
      PlayTone ();
      handled = true;
      break;

    case frmOpenEvent:
      FrmDrawForm (FrmGetActiveForm ());
      handled = true;
      break;

    case menuEvent:
      handled = MyFormHandleMenuEvent (event->data.menu.itemID);
      break;
   }
   return handled;
}

/*
 * Simple debug function with scrolling
 */
void    Debug (char *msg)
{
   RectangleType rect;
   Int16   text_top = 20;
   Int16   text_lines = 10;
   Int16   text_height = FntLineHeight () * text_lines;

   WinHandle draw_win = WinGetDrawWindow ();

   /* scroll text up */
   rect.topLeft.x = 0;
   rect.topLeft.y = text_top + FntLineHeight ();
   rect.extent.x = 160;
   rect.extent.y = text_height - FntLineHeight ();
   WinCopyRectangle (draw_win, draw_win, &rect, 0, text_top, winPaint);

   /* clear new text area */
   rect.topLeft.x = 0;
   rect.topLeft.y = text_top + text_height - FntLineHeight ();
   rect.extent.x = 160;
   rect.extent.y = FntLineHeight ();
   WinEraseRectangle (&rect, 0);

   /* draw string */
   WinDrawChars (msg, StrLen (msg), 0,
		 text_top + text_height - FntLineHeight ());
}

void    PlayTone (void)
{
   volatile UInt16	*pwmc;	/* pwm control */
   volatile UInt16	*pwms;
   volatile UInt8	*pwmp;
   UInt16			oldpwmc;
   UInt8			s1, s2;
   UInt16			n = 16000;
   UInt8			*ptr;
   UInt8 			*m[100];
   UInt16			c = 0, 
   					cmax = 0;	/* benchmarking */
   char				tmp[128];

   int				i, rec;

   //FlpCompDouble d;
   //d.d = 1.236346347234236634/10245;

   pwmc = 0xFFFFF500;
   pwms = 0xFFFFF502;
   pwmp = 0xFFFFF504;

   //SysTaskDelay (200); 
   rec = GetDBPtrs("sample.wav", m);
   
   // save    
   oldpwmc = *pwmc;
   StrPrintF (tmp, "Old pwmc: 0x%x", oldpwmc);
   Debug (tmp);
   
   //FlpFToA(d.fd, tmp);
   //Debug (tmp);

   //return;
   // configure pwm
   *pwmc = 0x001c;
   *pwmp = 0xFE;

   for (i = 0; i < (int) rec; i++) {     
     UInt8 *mi = m[i];
     n = (256 * *(mi + 6) + *(mi + 7)) / 2;
     //StrPrintF (tmp, "recs %d:  s:%d", i, n);
     //Debug (tmp);
     ptr = mi + 8;
     while (n) {
       // wait 
       c = 0;
       do  c++; while (!(*pwmc & 32));
       while (*pwmc & 32) {
	 // load 2 samples
	 s1 = (*ptr++);
	 s2 = (*ptr++);
	 *pwms = s2 | (s1 << 8);
       }
       n--;
       if (cmax < c)
	 cmax = c;
      }
   }

   *pwmc = oldpwmc;
   StrPrintF (tmp, "Loops: %d max: %d", c, cmax);
   Debug (tmp);
}

UInt16 GetDBPtrs(char *name, void **ptrs)
{
  // db stuff
  LocalID lid;
  UInt32  rec, bytes, datab;
  DmOpenRef dbref;
  char    tmp[128];
  int i;
  
  // get db 
  lid = DmFindDatabase (0, name);
  if (lid == 0) {
    Debug ("can't find sample.wav !");
    return;
  }
  DmDatabaseSize (0, lid, &rec, &bytes, &datab);
  StrPrintF (tmp, "recs %ld, tbytes %ld, bytes %ld", rec, bytes, datab);
  Debug (tmp);
  dbref = DmOpenDatabase (0, lid, dmModeReadOnly);
  for (i = 0; i < (int) rec; i++) {
    MemHandle h;
    
    h = DmQueryRecord (dbref, i);
    if (h == NULL) {
      StrPrintF (tmp, "recs %d pb", i);
      Debug (tmp);
    }
    *ptrs++ = MemHandleLock (h);	/* get pointers to data */
    MemHandleUnlock (h);
  }  
   DmCloseDatabase (dbref);
  return rec;
}

static Boolean ApplicationHandleEvent (EventPtr event)
{
   FormPtr frm;
   UInt16  formId;
   Boolean handled = false;

   if (event->eType == frmLoadEvent) {
      //Load the form resource specified in the event then activate it
      formId = event->data.frmLoad.formID;
      frm = FrmInitForm (formId);
      FrmSetActiveForm (frm);

      // Set the event handler for the form.  The handler of the currently 
      // active form is called by FrmDispatchEvent each time it is called 
      switch (formId) {
       case MainForm:
	 FrmSetEventHandler (frm, MyFormHandleEvent);
	 break;
      }
      handled = true;
   }

   return handled;
}

static void EventLoop (void)
{
   EventType event;
   UInt16  error;

   do {
      EvtGetEvent (&event, evtWaitForever);
      if (!SysHandleEvent (&event))
	 if (!MenuHandleEvent (0, &event, &error))
	    if (!ApplicationHandleEvent (&event))
	       FrmDispatchEvent (&event);
   } while (event.eType != appStopEvent);
}


UInt32  PilotMain (UInt16 launchCode, MemPtr cmdPBP, UInt16 launchFlags)
{
   Err     err;

   if (launchCode == sysAppLaunchCmdNormalLaunch) {
      if ((err = StartApplication ()) == 0) {
	 EventLoop ();
	 StopApplication ();
      }
   }

   return err;
}
