#include "stdafx.h"
#include "dvrclient.h"
#include "dvrclientDlg.h"

#include "SubViewScreen.h"

SubViewScreen::SubViewScreen(HWND hparent, int subview)
	:Screen(hparent),
	m_subview(subview)
{
}

int SubViewScreen::attach_channel(decoder * pdecoder, int channel)
{
	if (pdecoder && pdecoder->supportattachview() && pdecoder->attachview(channel, m_hWnd, m_subview) >= 0) {
		m_decoder = pdecoder;
		m_channel = channel;
		m_get_chinfo_retry = 10;
		getchinfo();
		m_attached = 1;
		setaudio(m_audio);

		HWND hsurface = FindWindowEx(m_hWnd, NULL, NULL, NULL);
		if (hsurface != NULL) {
			m_surface.attach(hsurface);
		}

		return 1;
	}
	return 0;
}

