// DriveByReport.cpp : Drive By Report Dialog
//
#include "stdafx.h"

#include <setjmp.h>
#include <shlobj.h>
#include <wininet.h>

#include "dvrclient.h"
#include "DriveByReport.h"
#include "util.h"
#include "cdir.h"
#include "json.h"

#include "pdf/hpdf.h"

using namespace Gdiplus;

DlgDriveByReport::DlgDriveByReport(HWND hparent) {
	m_mapzoom = 15;
	mapw = 800;
	maph = 240;
	m_picID[0] = IDC_STATIC_PICTURE0;
	m_picID[1] = IDC_STATIC_PICTURE1;
	m_picID[2] = IDC_STATIC_PICTURE2;
	m_picID[3] = IDC_STATIC_PICTURE3;
	for (int i = 0; i < 4; i++) m_imagefile[i] = i;

	hinet = NULL;
	if (InternetCheckConnection(_T("https://dev.virtualearth.net/"), FLAG_ICC_FORCE_CONNECTION, 0)) {
		string	appname("WinINet");
		hinet = InternetOpen(appname, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	}

	if (hinet == NULL) {
		MessageBox(hparent, _T("This feature require internet connection, please check your network settings."), _T("ERROR"), MB_OK| MB_ICONERROR);
	}

	CreateDlg(IDD_DIALOG_DRIVEBYREPORT, hparent);
}

DlgDriveByReport::~DlgDriveByReport()
{
	if( hinet!=NULL)
		InternetCloseHandle(hinet);
}

int DlgDriveByReport::OnInitDialog()
{
	if (hinet == NULL) {
		EndDialog();
		return FALSE;
	}

	// init before createdlg
	string reportSessionFile;
	getTempFolder(reportSessionFile);
	reportSessionFile += "\\DBReportSess";

	drivebySession.loadFile(reportSessionFile);

	json * records = drivebySession.getLeaf("captures");

	if (records==NULL || records->itemSize() < 1) {
		MessageBox(m_hWnd, _T("At lease 1 image should be captured!"), NULL, 0);
		EndDialog();
		return TRUE;
	}

	// record: t2000, location, img
	// init known fields
	if (reg_read("db_autoopenpdf")) {
		CheckDlgButton(m_hWnd, IDC_CHECK_OPENPDF, BST_CHECKED);
	}

	m_busid = drivebySession.getLeafString("captures/0/dvrname");
	SetDlgItemText(m_hWnd, IDC_EDIT_BUSID, m_busid);

    return TRUE ;
}

const char bingmapkey[] = "ApzRPgKMicyrymrnEciz6pkDJNClpFK5rpYNIGDeKf_POOmBoYabc9KQuE8KS2HS";

void DlgDriveByReport::getAddr()
{
	// load addr form gps location
	string addrurl;
	addrurl.printf("https://dev.virtualearth.net/REST/v1/Locations/%s?key=%s",
		getGPS(), bingmapkey);

	// load addr form gps location
	HINTERNET hfile = InternetOpenUrl(hinet, addrurl, NULL, 0, 0, 0);
	if (hfile != NULL) {
		DWORD r;
		string addr_json;
		char * buf = new char[1024];
		while (InternetReadFile(hfile, buf, 1000, &r)) {
			if (r > 0) {
				buf[r] = 0;
				addr_json += buf;
			}
			else {
				break;
			}
		}

		delete[]buf;
		InternetCloseHandle(hfile);

		// parse json obj;
		json j;
		j.parse(addr_json);

		// address on j.resourceSets[0].resources[0].address.formattedAddress
		json * jaddr = j.getLeaf("resourceSets/0/resources/0/address/formattedAddress");
		if (jaddr) {
			m_addr = jaddr->getString();
		}
	}

	if (m_addr.isempty()) {
		// re-check Internet connection
		if (!InternetCheckConnection(addrurl, 0, 0)) {
			MessageBox(m_hWnd, _T("Internet connection is required for this feature!"), NULL, MB_ICONEXCLAMATION);
			EndDialog();
		}
	}
}

void DlgDriveByReport::getMap()
{
	// load map files
	string mapurl;
	mapurl.printf("https://dev.virtualearth.net/REST/v1/Imagery/Map/Road?pushpin=%s&mapSize=%d,%d&format=jpeg&key=%s",
		getGPS(), mapw, maph, bingmapkey);

	HINTERNET hfile = InternetOpenUrl(hinet, mapurl, NULL, 0, 0, 0);
	if (hfile != NULL) {
		mapurl.wcssize(500);
		DWORD blen = 1000;

		HttpQueryInfo(hfile, HTTP_QUERY_CONTENT_TYPE, mapurl.wcssize(500), &blen, NULL);
		if (strcmp(mapurl, "image/jpeg") == 0) {
			DWORD r;
			char* buf = new char[4096];
			mkTempFileName("DBMap", "JPG", m_mapfile);
			FILE* mapfile = fopen(m_mapfile, "wb");
			if (mapfile) {
				while (InternetReadFile(hfile, buf, 4096, &r)) {
					if (r > 0) {
						fwrite(buf, 1, r, mapfile);
					}
					else {
						break;
					}
				}
				fclose(mapfile);
			}
			delete[]buf;
		}
		else {
			MessageBox(m_hWnd, _T("Can not get map image from BING service"), _T("Error"), MB_OK | MB_ICONERROR);
		}

		InternetCloseHandle(hfile);
	}
}

const char * DlgDriveByReport::getGPS()
{
	return drivebySession.getLeafString(TMPSTR("captures/%d/location", m_imagefile[0]));
}

// captime=0: get current system time, 1: get capture Time
const char * DlgDriveByReport::getDateTimeString(int captime)
{
	SYSTEMTIME st;
	SYSTEMTIME* pDateTime = NULL;
	if (captime) {
		int t2000 = (int)drivebySession.getLeafNumber( TMPSTR("captures/%d/t2000", m_imagefile[0]));
		dvrtime dvrt = time_2000();
		time_add(dvrt, t2000);
		pDateTime = time_dvrtime(&dvrt, &st);
	}

	string datestr;
	string timestr;
	GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, pDateTime, NULL, datestr.tcssize(100), 100, NULL);
	GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, pDateTime, NULL, timestr.tcssize(100), 100);
	m_datetime = datestr;
	m_datetime += " ";
	m_datetime += timestr;
	return (const char *)m_datetime;
}

void convertImage(const char * src, const char * dst)
{
	Bitmap * img;
	string srcimg;
	string dstimg;
	srcimg = src;
	dstimg = dst;
	img = Bitmap::FromFile(srcimg);
	if (img) {
		int xw = img->GetWidth();
		int xh = img->GetHeight();
		int w = xw;
		int h = xh;
		if (h < 600) {
			w = 800;
			h = 600;
		}
		Bitmap snapshot(w, h);
		Graphics g(&snapshot);
		g.DrawImage(img, Rect(0, 0, w, h), 0, 0, xw, xh, UnitPixel);
		savebitmap(dstimg, &snapshot);
		delete img;
	}
}

// draw captured pictures
int DlgDriveByReport::OnItemDraw(LPDRAWITEMSTRUCT itemStruct)
{
	int i;
	for (i = 0; i < 4; i++) {
		if (itemStruct->CtlID == m_picID[i]) {
			// switch picture
  			Graphics g(itemStruct->hDC);
			RECT rect;
			Bitmap * bmp;

			rect = itemStruct->rcItem;

			string limg;
			string img;
			limg.printf("captures/%d/img", m_imagefile[i]);
			img = drivebySession.getLeafString(limg);
			if (img.isempty()) {
				Brush *brush = new SolidBrush(Color::Wheat);
				g.FillRectangle(brush, (int)rect.left, (int)rect.top, (int)(rect.right - rect.left), (int)(rect.bottom - rect.top));
				delete brush;
			}
			else {
				bmp = Bitmap::FromFile(img);
				if (bmp) {
					g.DrawImage(bmp, (int)rect.left, (int)rect.top, (int)(rect.right - rect.left), (int)(rect.bottom - rect.top));
					delete bmp;
					// update address as first Image
					if (i == 0) {
						getAddr();
						SetDlgItemText(m_hWnd, IDC_EDIT_ADDRESS, m_addr);
					}
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

// when item clicked
int DlgDriveByReport::OnItemNotify(int Id, int Event)
{
	int i;
	for (i = 0; i < 4; i++) {
		if (Id == m_picID[i] && Event == STN_CLICKED) {
			// switch picture
			json * captures = drivebySession.getLeaf("captures");
			int ncap = captures->itemSize();
			if (i == 0) {
				m_imagefile[i] = (m_imagefile[i] + 1) % ncap;
			}
			else {
				m_imagefile[i] = (m_imagefile[i] + 1) % (ncap+1);
			}

			// redraw picture
			HWND hpic = GetDlgItem(m_hWnd, Id);
			InvalidateRect(hpic, NULL, FALSE);
			return TRUE;
		}
	}
	return FALSE;
}

int DlgDriveByReport::OnOK()
{
	// save some fields
	reg_save("db_autoopenpdf", IsDlgButtonChecked(m_hWnd, IDC_CHECK_OPENPDF));

	// open save file Dialog
	string  filename;
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.hInstance = ResInst();
	ofn.nMaxFile = 512;
	ofn.lpstrFile = filename.tcssize(ofn.nMaxFile+2);
	ofn.lpstrFilter = _T("PDF files\0*.PDF\0All files\0*.*\0\0");
	ofn.Flags = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = _T("PDF");
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR ;

	if (GetSaveFileName(&ofn)) {
		// convert all images into jpg
		GetDlgItemText(m_hWnd, IDC_EDIT_BUSID, m_busid.tcssize(500), 480);
		GetDlgItemText(m_hWnd, IDC_EDIT_PLATE, m_plate.tcssize(500), 480);
		GetDlgItemText(m_hWnd, IDC_EDIT_ADDRESS, m_addr.tcssize(500), 480);
		GetDlgItemText(m_hWnd, IDC_EDIT_NOTE, m_note.tcssize(1024), 1020);
		getMap();

		if (makepdf(filename)) {
			if (IsDlgButtonChecked(m_hWnd, IDC_CHECK_OPENPDF)) {
				ShellExecute(NULL, _T("open"),
					filename,
					NULL, NULL, SW_SHOWNORMAL);
			}
		}
		else {
			MessageBox(m_hWnd, _T("Can't create report file!"), _T("Error"), MB_OK| MB_ICONERROR);
		}

		return EndDialog(IDOK);
	}
	return 1;
}

static jmp_buf pdf_error_jmpenv;
static void pdf_error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
	printf("ERROR: error_no=%04X, detail_no=%d\n",
		(unsigned int)error_no, (int)detail_no);
	longjmp(pdf_error_jmpenv, 1); /* invoke longjmp() on error */
}

int DlgDriveByReport::makepdf( char * reportfile)
{
	int i;
	HPDF_Doc  pdf;
	HPDF_Page page;
	HPDF_Font font;
	HPDF_REAL height;
	HPDF_REAL width;
	HPDF_REAL x, y, w, h ;
	HPDF_Image image;
	HPDF_REAL iw, ih;
	HPDF_REAL edge;
	HPDF_STATUS status;

	pdf = HPDF_New(pdf_error_handler, NULL);
	if (!pdf) {
		printf("error: cannot create PdfDoc object\n");
		return 0;
	}
	if (setjmp(pdf_error_jmpenv)) {
		HPDF_Free(pdf);
		return 0;
	}

	// HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);

	/* Add a new page object. */
	page = HPDF_AddPage(pdf);
	HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_PORTRAIT);
	edge = 30;

	height = HPDF_Page_GetHeight(page);
	width = HPDF_Page_GetWidth(page);
	font = HPDF_GetFont(pdf, "Helvetica", NULL);
	HPDF_Page_SetLineWidth(page, 1);
	y = height - edge;

	// header
	h = 55;
	// draw logos
	string logofile;
	logofile = "DriveByLogo1.JPG";
	getfilepath(logofile);
	image = HPDF_LoadJpegImageFromFile(pdf, logofile);
	if (image) {
		iw = HPDF_Image_GetWidth(image);
		ih = HPDF_Image_GetHeight(image);
		w = h * iw / ih;
		HPDF_Page_DrawImage(page, image, edge, y - h, w, h);
	}

	logofile = "DriveByLogo2.JPG";
	getfilepath(logofile);
	image = HPDF_LoadJpegImageFromFile(pdf, logofile);
	if (image) {
		iw = HPDF_Image_GetWidth(image);
		ih = HPDF_Image_GetHeight(image);
		w = h * iw / ih;
		HPDF_Page_DrawImage(page, image, width - edge - w, y - h, w, h);
	}

	// Page Title
	HPDF_Page_SetFontAndSize(page, font, 18);
	HPDF_Page_BeginText(page);
	HPDF_Page_TextRect(page, edge+w, y, width- edge - w, y - h , "Stop-Arm Drive-By Violation Report", HPDF_TALIGN_CENTER, NULL);
	HPDF_Page_EndText(page);

	y -= h+8;

	HPDF_Page_MoveTo(page, edge, y);
	HPDF_Page_LineTo(page, width - edge, y);
	HPDF_Page_Stroke(page);
	y -= 8;

	json * records = drivebySession.getLeaf("captures");

	// print 4 drive images
	w = width / 2 - 32;
	for (i = 0; i < 4; i++) {
		string img;
		img.printf("%d/img", m_imagefile[i]);
		json * imgfile = records->getLeaf(img);
		if( imgfile!=NULL ) {
			string bmp;
			string jpg;
			bmp = imgfile->getString();
			jpg = bmp;
			jpg += ".JPG";

			convertImage(bmp, jpg);

			image = HPDF_LoadJpegImageFromFile(pdf, jpg);
			iw = HPDF_Image_GetWidth(image);
			ih = HPDF_Image_GetHeight(image);

			// unlink(jpg);

			x = i % 2 ? width / 2 + 2 : 30;
			if (ih > 600) {	// hd image, use image ratio (HD)
				h = w * ih / iw;
			}
			else {			// use 4:3
				h = w * 3 / 4;
			}
			if (i == 2) {
				y -= h + 4;
			}
			/* Draw image to the canvas. */
			HPDF_Page_DrawImage(page, image, x, y - h, w, h);

		}
	}
	y -= h + 10;

	// draw map ?
	if (!m_mapfile.isempty()) {
		image = HPDF_LoadJpegImageFromFile(pdf, m_mapfile);
		iw = HPDF_Image_GetWidth(image);
		ih = HPDF_Image_GetHeight(image);
		w = width - 60;
		if (iw > w) {
			h = w * ih / iw;
		}
		else {
			w = iw;
			h = ih;
		}
		HPDF_Page_DrawImage(page, image, width/2-w/2, y - h, w, h);
		y -= h + 10;
	}

	// draw notes 
	HPDF_Page_SetFontAndSize(page, font, 12);
	HPDF_Page_BeginText(page);

	w = width / 2 - 2 - edge;
	x = edge;
	HPDF_Page_MoveTextPos(page, x, y);
	HPDF_Page_ShowTextNextLine(page, "Date-Time: ");
	HPDF_Page_ShowText(page, getDateTimeString(1));
	
	HPDF_Page_ShowTextNextLine(page, "Vehicle ID: ");
	HPDF_Page_ShowText(page, m_busid);

	HPDF_Page_ShowTextNextLine(page, "Plate of Violator: ");
	HPDF_Page_ShowText(page, m_plate);

	HPDF_Page_ShowTextNextLine(page, "Coordinate: ");
	HPDF_Page_ShowText(page, getGPS());

	HPDF_Page_ShowTextNextLine(page, "Address: ");
	HPDF_Page_ShowText(page, m_addr);

	HPDF_Page_ShowTextNextLine(page, "Note: ");
	HPDF_Point tpoint = HPDF_Page_GetCurrentTextPos(page);
	x = tpoint.x + 10;
	y = tpoint.y + 10;
	HPDF_Page_TextRect(page,
		x, y,
		width - edge, edge ,
		m_note,
		HPDF_TALIGN_JUSTIFY, NULL);

	HPDF_Page_EndText(page);

	// footer 
	y = edge;
	h = 20;
	HPDF_Page_SetFontAndSize(page, font, 8);
	HPDF_Page_BeginText(page);

	HPDF_Page_TextRect(page, edge , y-h, width - edge, y , "247 Security Inc.", HPDF_TALIGN_LEFT, NULL);
	HPDF_Page_TextRect(page, edge , y-h, width - edge, y , getDateTimeString(0), HPDF_TALIGN_RIGHT, NULL);

	HPDF_Page_EndText(page);

	// save file
	status = HPDF_SaveToFile(pdf, reportfile);

	/* clean up */
	HPDF_Free(pdf);

	return status==HPDF_OK;
}
