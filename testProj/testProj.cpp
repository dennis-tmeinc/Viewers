// testProj.cpp : Defines the entry point for the application.
//

#include "framework.h"

#include <d2d1.h>
#include <d2d1_1.h>

#include "testProj.h"

#include "../common/cwin.h"
#include "../common/json.h"

#define MAX_LOADSTRING 100

// Global Variables:
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

class MainWin : public Window
{

#ifdef IDC_APP
	HACCEL hAccelTable;
#endif

protected:

	ID2D1Factory* factory = NULL;
	ID2D1DCRenderTarget* target = NULL;
	float mdpi = USER_DEFAULT_SCREEN_DPI;

	// called after window been attached (to do some init after the window created)
	virtual void OnAttach() {
		HRESULT hr;
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);

		// Create a DC render target.
		D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(
				DXGI_FORMAT_B8G8R8A8_UNORM,
				D2D1_ALPHA_MODE_IGNORE),
			0,
			0,
			D2D1_RENDER_TARGET_USAGE_NONE,
			D2D1_FEATURE_LEVEL_DEFAULT
		);

		hr = factory->CreateDCRenderTarget(
			&props,
			&target
		);

		FLOAT dpix, dpiy;
		factory->GetDesktopDpi(&dpix, &dpiy);
		mdpi = dpiy;
		if (target)
			target->SetDpi(mdpi, mdpi);

		json j(JSON_Null);
		j.loadFile("pos.json");
		if (j.itemSize() > 0) {
			float left = j.getLeafNumber("left");
			float right = j.getLeafNumber("right");
			float top = j.getLeafNumber("top");
			float bottom = j.getLeafNumber("bottom");
			SetWindowPos(m_hWnd, HWND_TOP, (int)left, (int)top, (int)(right - left), (int)(bottom - top), SWP_ASYNCWINDOWPOS | SWP_NOZORDER );
		}
	}

	// called after window been attached (to do some init after the window created)
	virtual void OnDetach() {
		RECT rc;
		GetWindowRect(m_hWnd, &rc);

		json j(JSON_Object);
		j.addNumberItem("left", rc.left)
			.addNumberItem("right", rc.right)
			.addNumberItem("top", rc.top)
			.addNumberItem("bottom", rc.bottom);
		j.saveFile("pos.json");

		if( target )
			target->Release();
		if( factory )
			factory->Release();
	}

	virtual LRESULT OnDpiChanged(int dpi) {
		mdpi = dpi;
		if (target)
			target->SetDpi(mdpi, mdpi);
		return FALSE;
	}

	virtual LRESULT OnSize() {
		return FALSE;
	}

	virtual LRESULT OnPaint()
	{
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(m_hWnd, &ps);
		Gdiplus::Graphics g(hdc);
		
		RECT r;
		GetClientRect(m_hWnd, &r);

		HRESULT hr;

		hr = target->BindDC(hdc, &r);

		target->BeginDraw();

		// Create render target here...
		const D2D1_COLOR_F color = D2D1::ColorF(D2D1::ColorF::Red);
		ID2D1SolidColorBrush * m_brush;
		
		target->CreateSolidColorBrush(color, &m_brush);
		//m_brush->SetColor(differentColor);

		D2D1_SIZE_F size = target->GetSize();
		D2D1_RECT_F rect = D2D1::RectF(size.width /2, 0, size.width, size.height);
		target->FillRectangle(rect, m_brush);

		target->FillRectangle(D2D1::RectF( 0, 0, 96, 96), m_brush);

		target->EndDraw();

		//  g->TranslateTransform(100.0f, 50.0f);
		/*
		REAL dpix = g.GetDpiX();

		Gdiplus::Font font(FontFamily::GenericSansSerif(), 24, FontStyleRegular, UnitPoint);
		Gdiplus::PointF pointF( (r.left + r.right)/2.0f, 20.0f);
		Gdiplus::SolidBrush solidBrush(Color(255, 0, 0, 255));
		g.DrawString(TEXT("Hello world!"), -1, &font, pointF, &solidBrush);

		Gdiplus::Pen p(Color(100,100,0), 2.0f);
		g.DrawLine(&p, 0.0f, 0.0f, (r.left + r.right) / 2.0f, (r.top + r.bottom) / 2.0f);
		*/
		//delete g;

		// GDI
		POINT pt[2];
		pt[0].x = (r.left + r.right) / 2;
		pt[0].y = (r.top + r.bottom) / 2;
		pt[1].x = r.right ;
		pt[1].y = 0;

		HGDIOBJ hPen = CreatePen(PS_SOLID, 3, RGB(50,20,0));
		HGDIOBJ hPenOld = SelectObject(hdc, hPen);

		MoveToEx(hdc, pt[0].x, pt[0].y, NULL);
		LineTo(hdc, pt[1].x, pt[1].y);

		SelectObject(hdc, hPenOld);
		DeleteObject(hPen);

		EndPaint(m_hWnd, &ps);
		return 0;
	}

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
		case WM_DPICHANGED:
			res = OnDpiChanged(HIWORD(wParam));
			break;
		case WM_SIZE:
			res = OnSize();
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

public:
	MainWin()
	{
		HWND hwnd;
		HMENU hMenu = NULL;
		TCHAR title[256] = TEXT("Simple Window");

		// Initialize global strings
		LoadStringW(AppInst(), IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadStringW(AppInst(), IDC_TESTPROJ, szWindowClass, MAX_LOADSTRING);

		hMenu = LoadMenu(AppInst(), MAKEINTRESOURCE(IDC_TESTPROJ));

#ifdef IDC_APP
		LoadString(ResInst(), IDC_APP, title, 256);
		hAccelTable = LoadAccelerators(ResInst(), MAKEINTRESOURCE(IDC_APP));
		hMenu = LoadMenu(ResInst(), MAKEINTRESOURCE(IDC_APP));
#endif
		hwnd = CreateWindowEx(
			0,									  // no extended styles
			WinClass((HBRUSH)(COLOR_WINDOW + 1)), // class name
			szTitle,								  // window name
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
			ShowWindowAsync(hwnd, SW_SHOWNORMAL);
		}
	}
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MainWin  m;
	return	m.run();
}
