#pragma once

class DropTarget : public IDropTarget {
private:
	int allowed;

public:
	virtual HRESULT STDMETHODCALLTYPE DragEnter(
		/* [unique][in] */ __RPC__in_opt IDataObject* pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD* pdwEffect);

	virtual HRESULT STDMETHODCALLTYPE DragOver(
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD* pdwEffect);

	virtual HRESULT STDMETHODCALLTYPE DragLeave(void);

	virtual HRESULT STDMETHODCALLTYPE Drop(
		/* [unique][in] */ __RPC__in_opt IDataObject* pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD* pdwEffect);

	// IUnknown 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);
};

