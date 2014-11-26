/*
    ================
    misc.c
    ================
    */

#include "sbtm.h"

/*----------------------------------------------------------------------------*/
Err SndPlayResourceEx(int ResID)
{
    MemHandle h;

    if (!ExtendedSound)	return -1;

    h = DmGetResource('wave', ResID);

    if (h) {
        SndPtr x = MemHandleLock(h);
        SndPlayResource(x, sndGameVolume, sndFlagNormal);
        MemPtrUnlock(x);
        DmReleaseResource(h);
    }
    return 0;
}
/*---------------------------------------------------------------------------*/
Err WinDrawBitmapEx(int ResID, Coord x, Coord y)
{
    MemHandle h;

    h = DmGetResource('Tbmp', ResID);

    if (h) {
        BitmapPtr	bmp = MemHandleLock(h);
        WinDrawBitmap(bmp, x, y);
        MemPtrUnlock(bmp);
        DmReleaseResource(h);
    }
    return 0;
}

/*-------------------------------------------------------------*/
void * GetObjectPtr(UInt16 objectID)
{
    FormType * frmP;

    frmP = FrmGetActiveForm();
    return FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID));
}
/*-------------------------------------------------------------*/
Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
    UInt32 romVersion;

    /* See if we're on in minimum required version of the ROM or later. */
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
    if (romVersion < requiredVersion)
    {
        if ((launchFlags &
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
        {
            FrmAlert(RomIncompatibleAlert);

            /* Palm OS versions before 2.0 will continuously relaunch this
             * app unless we switch to another safe one. */
            if (romVersion < kPalmOS20Version) {
                AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
            }
        }

        return sysErrRomIncompatible;
    }

    return errNone;
}
/*-------------------------------------------------------------*/
void CheckSoundCaps()
{
    UInt32	version;
    Err		err;

    err = FtrGet(sysFileCSoundMgr, sndFtrIDVersion, &version);

    if (err) {
        ExtendedSound = false;
    }
    else {
        if (version == sndMgrVersionNum) {
            ExtendedSound = true;
        }
    }
}
