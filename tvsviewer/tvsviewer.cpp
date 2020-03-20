// tvsviewer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "tvsviewer.h"
#include "dvrclient.h"
#include "dvrclientDlg.h"

struct btbar_ctl btbar_ctls[] = {
	{IDC_SLOW,     0, _T("PLAY_SLOW") },
	{IDC_BACKWARD, 3, _T("PLAY_BACKWARD") },
	{IDC_PAUSE,    1, _T("PLAY_PAUSE") },
	{IDC_PLAY,     -1000, NULL },
	{IDC_FORWARD,  0, _T("PLAY_FORWARD") },
	{IDC_FAST,     3, _T("PLAY_FAST") },
	{IDC_STEP,     3, _T("PLAY_STEP") },
	{IDC_MAP,      4, _T("MAP_ENABLE") },
	{IDC_MIXAUDIO, 1, _T("AUDIO_MIXALL") },
	{IDC_PIC_SPEAKER_VOLUME, 3, _T("VOLUME_0")},
	{IDC_SLIDER_VOLUME, 1, NULL },	
	{0, 0, NULL }
} ;

const int control_align = 0 ;       // control layout, 0: left, 1: right ;
const int btbar_refid = IDC_SLIDER_VOLUME;	// Volue Slider as ref
const int btbar_align = -5;		// >=0: left align, <0: right align

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);



	void testWin();
	//testWin();


	DvrclientDlg dlg ;
	dlg.DoModal();
	return 0 ;
}




//	D2D test program

#include <d2d1.h>
#include <d2d1_1.h>
class HelloMainWin : public Window
{

#ifdef IDC_APP
	HACCEL hAccelTable;
#endif

protected:

	ID2D1Factory* pD2DFactory;
	ID2D1HwndRenderTarget* pRT ;
	ID2D1SolidColorBrush* pBlackBrush ;
	ID2D1SolidColorBrush* pRedBrush ;

	void initPaintTool()
	{
		pD2DFactory = NULL;
		D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			&pD2DFactory
		);

		// Create a Direct2D render target          
		pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
				m_hWnd,
				D2D1::SizeU(
					10,10)
			),
			&pRT
		);

		pRT->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::White),
				&pBlackBrush
			);
		pRT->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Red),
				&pRedBrush
			);
	}

	void releasePaintTool()
	{
		pRT->Release();
		pRedBrush->Release();
		pBlackBrush->Release();
		pD2DFactory->Release();
	}

	LRESULT OnPaint();

	virtual LRESULT OnCommand(int Id, int Event)
	{
		switch (Id)
		{
#ifdef IDM_ABOUT
#ifdef IDD_ABOUTBOX
		case IDM_ABOUT:
			// Open about dialog
		{
			Dialog dlg;
			dlg.DoModal(IDD_ABOUTBOX, m_hWnd);
		}
		break;
#endif // IDD_ABOUTBOX
#endif // IDM_ABOUT
#ifdef IDM_EXIT
		case IDM_EXIT:
			PostMessage(m_hWnd, WM_CLOSE, 0, 0);
			break;
#endif
		default:
			return FALSE;
		}
		return TRUE;
	}

	virtual LRESULT OnNotify(LPNMHDR pnmhdr)
	{
		return FALSE;
	}

	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT res = FALSE;
		switch (message)
		{
		case WM_COMMAND:
			res = OnCommand(LOWORD(wParam), HIWORD(wParam));
			break;
		case WM_NOTIFY:
			res = OnNotify((LPNMHDR)lParam);
			break;
		case WM_PAINT:
			res = OnPaint();
			break;
		default:
			res = DefWndProc(message, wParam, lParam);
		}
		return res;
	}

#ifdef IDC_APP
	virtual BOOL PreProcessMessage(MSG* pmsg)
	{
		return TranslateAccelerator(m_hWnd, hAccelTable, pmsg);
	}
#endif

	virtual void OnAttach() {
		initPaintTool();
	}
	virtual void onDetach() {
		releasePaintTool();
	}

public:
	HelloMainWin()
	{
		HWND hwnd;
		HMENU hMenu = NULL;
		TCHAR title[256] = TEXT("Simple Window");
#ifdef IDC_APP
		LoadString(ResInst(), IDC_APP, title, 256);
		hAccelTable = LoadAccelerators(ResInst(), MAKEINTRESOURCE(IDC_APP));
		hMenu = LoadMenu(ResInst(), MAKEINTRESOURCE(IDC_APP));
#endif
		hwnd = CreateWindowEx(
			0,									  // no extended styles
			WinClass((HBRUSH)NULL), // class name, no background
			title,								  // window name
			WS_OVERLAPPEDWINDOW,				  // overlapped window
			CW_USEDEFAULT,						  // default horizontal position
			CW_USEDEFAULT,						  // default vertical position
			CW_USEDEFAULT,						  // default width
			CW_USEDEFAULT,						  // default height
			(HWND)NULL,							  // no parent or owner window
			(HMENU)hMenu,						  // menu
			AppInst(),							  // instance handle
			NULL);								  // no window creation data
		if (hwnd)
		{
			attach(hwnd);
#ifdef IDC_APP
			HICON hicon = LoadIcon(ResInst(), MAKEINTRESOURCE(IDC_APP));
			if (hicon)
			{
				SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
				SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hicon);
			}
#endif
			ShowWindow(hwnd, SW_SHOWNORMAL);
		}
	}
};

LRESULT HelloMainWin::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc;
	HRESULT hr;
	// hdc = BeginPaint(m_hWnd, &ps);
	// TextOut(hdc, 10, 10, TEXT("Hello world!"), 12);
	// EndPaint(m_hWnd, &ps);

	// Obtain the size of the drawing area.
	RECT rc;
	GetClientRect(m_hWnd, &rc);

	pRT->Resize(D2D1::SizeU(
		rc.right, rc.bottom));

	pRT->BeginDraw();
	pRT->Clear(D2D1::ColorF(D2D1::ColorF::Green));

	pRT->FillRectangle(
		D2D1::RectF(
			rc.left + 100.0f,
			rc.top + 100.0f,
			rc.right - 100.0f,
			rc.bottom - 100.0f),
		pBlackBrush);

	pRT->DrawLine(
		D2D1::Point2F(rc.left + 200.0f, rc.top + 200.0f),
		D2D1::Point2F(rc.right - 200.0f, rc.bottom - 200.0f),
		pRedBrush);

	pRT->DrawRectangle(
		D2D1::RectF(
			rc.left + 200.0f,
			rc.top + 200.0f,
			rc.right - 200.0f,
			rc.bottom - 200.0f),
		pRedBrush);

	hr = pRT->EndDraw();

	ValidateRect(m_hWnd, NULL);
	return 0;
}

void f()
{
	HelloMainWin mwin;
	mwin.run();
}