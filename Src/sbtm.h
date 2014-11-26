/*
    ================
    sbtm.h
    ================
    */

#ifndef SBTIMEMACHINE_H_
#define SBTIMEMACHINE_H_

#include <PalmOS.h>
#include <PalmOSGlue.h>
#include <DateTime.h>
#include <TimeMgr.h>
#include <SoundMgr.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "SBTimeMachine_Rsc.h"

/*********************************************************************
 * Internal Structures
 *********************************************************************/


/*********************************************************************
 * Global variables
 *********************************************************************/

extern int	ExtendedSound;


/*********************************************************************
 * Internal Constants
 *********************************************************************/

/* Define the minimum OS version we support */
#define ourMinVersion    sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0)
#define kPalmOS20Version sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)

#define appFileCreator			'SBTM'
#define appName					"Strong Bad's Time Machine"
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x01

extern void * GetObjectPtr(UInt16 objectID);
extern Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags);

/* My shit. */
extern void CheckSoundCaps();
extern Err SndPlayResourceEx(int ResID);
extern Err WinDrawBitmapEx(int ResID, Coord x, Coord y);
extern Err SndPlayResourceEx(int ResID);

#endif /* SBTIMEMACHINE_H_ */
