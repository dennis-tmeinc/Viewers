#pragma once

#ifndef __SUBVIEWSCREEN_H__
#define __SUBVIEWSCREEN_H__

#include <windows.h>
#include "screen.h"

//  new API defined by Harrison for sub view (zoom-in/rotate/dewap ?)
enum e_plyview {
	PLYVIEW_FRONT,
	PLYVIEW_MIDDLE,
	PLYVIEW_BACK
} ;

class SubViewScreen : public Screen
{
protected:
	int    m_subview;

protected:
	virtual LRESULT OnRButtonUp(UINT nFlags, int x, int y) { return 0; }	// disable context menu

public:
	SubViewScreen(HWND hparent, int subview);
	virtual int attach_channel(decoder * pdecoder, int channel);
};

#endif // __SUBVIEWSCREEN_H__
