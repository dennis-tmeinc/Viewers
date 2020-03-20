// DropTarget.cpp : implement drag and drop files
//

#include "stdafx.h"

#include <oleidl.h>
#include <math.h>

#include "cwin.h"
#include "cstr.h"
#include "cdir.h"
#include "util.h"
#include "dvrclient.h"
#include "dvrclientdlg.h"
#include "DropTarget.h"

HRESULT STDMETHODCALLTYPE DropTarget::DragEnter(
		/* [unique][in] */ __RPC__in_opt IDataObject* pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD* pdwEffect) {
		allowed = 0;
		FORMATETC fm = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		if (pDataObj != NULL) {
			if (pDataObj->QueryGetData(&fm) == S_OK) {
				allowed = 1;
			}
		}
		*pdwEffect = allowed ? DROPEFFECT_COPY : DROPEFFECT_NONE;
		return S_OK;
}

HRESULT STDMETHODCALLTYPE DropTarget::DragOver(
	/* [in] */ DWORD grfKeyState,
	/* [in] */ POINTL pt,
	/* [out][in] */ __RPC__inout DWORD* pdwEffect) {
	*pdwEffect = allowed ? DROPEFFECT_COPY : DROPEFFECT_NONE;
	return S_OK ;
}

HRESULT STDMETHODCALLTYPE DropTarget::DragLeave(void) {
	return S_OK;
}

HRESULT STDMETHODCALLTYPE DropTarget::Drop(
	/* [unique][in] */ __RPC__in_opt IDataObject* pDataObj,
	/* [in] */ DWORD grfKeyState,
	/* [in] */ POINTL pt,
	/* [out][in] */ __RPC__inout DWORD* pdwEffect) {

	if (allowed && pDataObj != NULL) {
		FORMATETC fm = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM med;
		if (pDataObj->GetData(&fm, &med) == S_OK) {
			if (med.tymed == TYMED_HGLOBAL && med.hGlobal != NULL) {

				LPDROPFILES files = (LPDROPFILES) GlobalLock(med.hGlobal);
				string * dropfile;
				if (files->fWide) {
					dropfile = new string( (WCHAR*)((char*)files + files->pFiles) );
				}
				else {
					dropfile = new string((char*)((char*)files + files->pFiles));
				}
				GlobalUnlock(med.hGlobal);
				if (med.pUnkForRelease == NULL) {
					GlobalFree(med.hGlobal);
					med.hGlobal = NULL;
				}
				PostMessage(g_maindlg->getHWND(), WM_OPENDVRFILE, NULL, (LPARAM)dropfile);

			}
		}
	}
	return S_OK;
}

// IUnknown 
HRESULT STDMETHODCALLTYPE DropTarget::QueryInterface(
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
{
	*ppvObject = this;
	return S_OK;
}

ULONG STDMETHODCALLTYPE DropTarget::AddRef(void) {
	return(1);
}

ULONG STDMETHODCALLTYPE DropTarget::Release(void)
{
	return(1);
}


