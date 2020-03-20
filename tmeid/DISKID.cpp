// DISKID.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>
#include <userenv.h>

static DWORD serialno=0;

DWORD getHardDriveID ()
{
   static DWORD serialno=0 ;
   TCHAR Alluserprofile[512] ;

   if (serialno == 0) {
	   serialno = 0x7788;
	   DWORD plen = 500;
	   GetAllUsersProfileDirectory(Alluserprofile, &plen);
	   Alluserprofile[3] = 0;
	   GetVolumeInformation(Alluserprofile, NULL, 0, &serialno, NULL, NULL, NULL, 0);
   }
   return serialno;
}
