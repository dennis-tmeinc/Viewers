// DriveByReport.h
//    dialog to generate driveby report
#pragma once

#ifndef  __DRIVE_BY_REPORT__
#define  __DRIVE_BY_REPORT__

#include <wininet.h>

#include "cstr.h"
#include "cwin.h"
#include "json.h"

// Select Dvr dialog
class DlgDriveByReport : public Dialog
{

protected:
	int    m_mapzoom;
	int    mapw, maph;		// map size (720x200)
	string m_mapfile;
	string m_datetime;
	string m_addr;
	string m_busid;		// bus id
	string m_plate;		//	Plate of Violator
	string m_note;

	// internet connection handle
	HINTERNET hinet;

	json   drivebySession;
	int    m_picID[4];
	int    m_imagefile[4];

public:
	DlgDriveByReport( HWND hparent);
	~DlgDriveByReport();

	const char * getGPS();
	const char * getDateTimeString(int captime);
	void getMap();
	void getAddr();

protected:

	int OnInitDialog();
	int OnOK();
	int OnItemDraw(LPDRAWITEMSTRUCT itemStruct);
	int OnItemNotify(int Id, int Event);
	
	virtual int OnCommand(int Id, int Event)
	{
		// owner draw item notifications
		for (int i = 0; i < 4; i++) {
			if (Id == m_picID[i]) {
				return OnItemNotify(Id, Event);
			}
		}
		return Dialog::OnCommand(Id, Event);
	}


	virtual int DlgProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_DRAWITEM:
			return OnItemDraw( (LPDRAWITEMSTRUCT)lParam );

		default:
			return Dialog::DlgProc(message,  wParam,  lParam);
		}
		return FALSE;
	}
	int makepdf(char * reportfile);
};

#endif		// __DRIVE_BY_REPORT__
