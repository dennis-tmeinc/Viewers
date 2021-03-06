
#pragma once

#include "../common/cwin.h"
#include "../common/util.h"
#include "../common/cstr.h"

class VolumeWindow : public Window {
protected:
	Image * m_backgroundimg ;
	Image * m_thumbimg ;
	Image *	m_muteimg ;
	Image *	m_spkimg ; 

	int     m_touchdown ;
	int     m_volume ;
	int     m_mute ;
	
	int		m_vid ;
	RECT    m_rect ;
	int     m_vbottom ;
	int     m_vtop ;

public:
	VolumeWindow(HWND hparent, int vid, int x, int y ) {
		DWORD v ;
		m_vid = vid ;
		waveOutGetVolume((HWAVEOUT) vid, &v );
		m_volume = (v&0xffff)/655 ;
		m_mute = 0 ;

		m_touchdown = 0 ;
		// load background/thumb image
		m_backgroundimg =  loadbitmap(_T("VOLUME_BACKGROUND") );
		m_thumbimg =  loadbitmap(_T("VOLUME_THUMB") );

		m_muteimg = loadbitmap( _T("BVOLUME_MUTE") );
		m_spkimg  = loadbitmap( _T("BVOLUME_SPEAKER") );

		RECT rt ;
		GetClientRect(hparent, &rt);
		MapWindowPoints(hparent, NULL, (LPPOINT)&rt, 2 );

//		HWND hwnd = CreateWindowEx(0, WinClass(), _T("Touch"), WS_POPUP|WS_THICKFRAME  , rt.right-220 , rt.bottom-400, 100, 300, 
		HWND hwnd = CreateWindowEx(0, WinClass(), _T("Touch"), WS_POPUP|WS_THICKFRAME  , x, y-300, 100, 300, 
			hparent, NULL, AppInst(), NULL );
		attach(hwnd);
		GetClientRect(hwnd,&m_rect);
		m_vbottom = m_rect.bottom - 40 ;
		m_vtop    = m_rect.top + 55 ;
	}

	~VolumeWindow() {
		delete m_backgroundimg ;
		delete m_thumbimg ;
		delete m_muteimg ;
		delete m_spkimg ; 
	}

protected:

	// use OnAttach() in place of WM_CREATE message
    virtual void OnAttach() {

		// set up our want / block settings
		DWORD dwPanWant  = GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY  | GC_PAN_WITH_INERTIA ;
		DWORD dwPanBlock = GC_PAN_WITH_GUTTER ;

		// set the settings in the gesture configuration
		GESTURECONFIG gc[] = {{ GID_ZOOM, GC_ZOOM, 0 },
							  { GID_PAN, dwPanWant , dwPanBlock}                     
							 };    
                     
		SetGestureConfig(m_hWnd, 0, 2, gc, sizeof(GESTURECONFIG));  

		ShowWindow( m_hWnd, SW_SHOW );
		SetCapture(m_hWnd);
	} ;

	int OnSet( int y )
	{
		y = 100-(y-m_vtop)*100/(m_vbottom-m_vtop) ;
		if( y<0 ) y = 0 ;
		if( y>100 ) y = 100 ;
		if( y!= m_volume ) {
			m_volume = y ;
			InvalidateRect( m_hWnd, NULL, FALSE );
		}
		return y ;
	}

	int OnLButtonDown( int x, int y ) 
	{
		if( x<m_rect.left || x>m_rect.right || y<m_rect.top || y>m_rect.bottom ) {
			ReleaseCapture();
		}
		else if( y>=m_vtop && y<m_vbottom ) {
			m_touchdown = 1;
			OnSet( y );
		}
		else if( y<m_vtop-15 ) {
			// mute/unmute
			if( m_mute ) {
				m_volume = m_mute ;
				m_mute = 0 ;
			}
			else {
				m_mute = m_volume ;
				m_volume = 0 ;
			}
			InvalidateRect( m_hWnd, NULL, FALSE );
		}

		return 0 ;
	}

	int OnLButtonUp( int x, int y ) 
	{
		m_touchdown = 0 ;
		return 0 ;
	}

	int OnMouseMove( int x, int y ) 
	{
		if( m_touchdown ) 
			OnSet( y );
		return 0 ;
	}

	void OnPan( int flags, int x, int y )
	{

		POINT npt ;
		npt.x = x  ;
		npt.y = y  ;
		MapWindowPoints(NULL, m_hWnd, &npt, 1);
		x=npt.x ;
		y=npt.y ;
		if( x>=m_rect.left && x<m_rect.right && y>=m_rect.top && y<m_rect.bottom ) {
			OnSet( y );
		}
		else if( flags & GF_BEGIN ) {		// first pan event
			ReleaseCapture();
		}
		else {
			OnSet( y );
		}
	}

	int OnGesture(  WPARAM wParam, LPARAM lParam )
	{
				// Create a structure to populate and retrieve the extra message info.
		GESTUREINFO gi;  
    
		ZeroMemory(&gi, sizeof(GESTUREINFO));
    
		gi.cbSize = sizeof(GESTUREINFO);

		BOOL bResult  = GetGestureInfo((HGESTUREINFO)lParam, &gi);
		BOOL bHandled = FALSE;

		if (bResult){
			// now interpret the gesture
			switch (gi.dwID){
			   case GID_ZOOM:
				   // Code for zooming goes here     
				   bHandled = TRUE;
				   break;
			   case GID_PAN:
				   // Code for panning goes here
				   OnPan( gi.dwFlags, gi.ptsLocation.x, gi.ptsLocation.y );
				   bHandled = TRUE;
				   break;
			   case GID_ROTATE:
				   // Code for rotation goes here
				   bHandled = TRUE;
				   break;
			   default:
				   // A gesture was not recognized
				   break;
			}
		}
		return DefWndProc( WM_GESTURE, wParam, lParam);
	}

	void OnTimer()
	{
		KillTimer(m_hWnd, 1);
		// set volume
		waveOutSetVolume((HWAVEOUT)m_vid, ((m_volume*655)<<16)+m_volume*655 );
	}

	void OnKeyDown( int k )
	{
		if( k==VK_UP ) {
			if( m_volume<100 ) {
				m_volume++ ;
				InvalidateRect( m_hWnd, NULL, FALSE );
			}
		}
		else if(  k==VK_DOWN ) {
			if( m_volume>0 ) {
				m_volume-- ;
				InvalidateRect( m_hWnd, NULL, FALSE );
			}
		}
		else {
			PostMessage(m_hWnd, WM_CLOSE,0,0);
		}
	}

	void OnPaint()
	{
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(m_hWnd, &ps);
		Graphics * gx = new Graphics(hdc);
		Bitmap * draw = new Bitmap( m_rect.right, m_rect.bottom, gx ) ;
		Graphics * g = new Graphics(draw);

		g->DrawImage( m_backgroundimg, 0,0, m_rect.right, m_rect.bottom );

		int v = m_vbottom - m_volume * (m_vbottom-m_vtop)/100 ;

		g->DrawImage( m_thumbimg, 24, v-16, 32, 32 );

		Gdiplus::Font        font(TEXT("Times New Roman"), 14, FontStyleRegular, UnitPixel);
		Gdiplus::PointF      pointF( (m_rect.left+m_rect.right)/2-10, m_rect.bottom-25 );
        Gdiplus::SolidBrush  solidBrush(Color(255, 0, 0, 255));
		string vol ;
		vol.printf("%d", m_volume );
        g->DrawString( (TCHAR *)vol, -1, &font, pointF, &solidBrush);

		// draw speaker icon
		if( m_volume == 0 ) {
			g->DrawImage( m_muteimg, (m_rect.left+m_rect.right)/2 - 21, 3 );
		}
		else {
			g->DrawImage( m_spkimg, (m_rect.left+m_rect.right)/2 - 21, 3 );
		}

		gx->DrawImage( draw, 0, 0 ) ;
		delete g ;
		delete draw ;
		delete gx ;
		EndPaint(m_hWnd, &ps);

		// Use Timer to delay set volume
		SetTimer(m_hWnd, 1, 10, NULL);
	}

	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		LRESULT res = 0 ;
		switch( uMsg ) {

		case WM_GESTURE:
			res = OnGesture( wParam, lParam );
			break;

		case WM_LBUTTONDOWN:
			res = OnLButtonDown(MOUSE_POSX(lParam), MOUSE_POSY(lParam) );
			break ;

		case WM_LBUTTONUP:
			res = OnLButtonUp(MOUSE_POSX(lParam), MOUSE_POSY(lParam) );
			break ;

		case WM_MOUSEMOVE:
			if( wParam&MK_LBUTTON )
				res = OnMouseMove(MOUSE_POSX(lParam), MOUSE_POSY(lParam) );
			break;

		case WM_CAPTURECHANGED :
			PostMessage(m_hWnd, WM_CLOSE, 0 , 0 );
			break;

		case WM_PAINT:
			OnPaint();
			break;

		case WM_TIMER:
			OnTimer();
			break;

		case WM_KEYDOWN :
			OnKeyDown( (int)wParam );
			break;

		default :
			return Window::WndProc(uMsg, wParam, lParam);
		}
        return res ;
    }

} ;

