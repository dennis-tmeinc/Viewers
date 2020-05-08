#pragma once

#include "../common/cstr.h"
#include "../common/cwin.h"

#include "CallTmeDlg.h"

// Login dialog

class LoginDlg : public Dialog
{
public:
	enum { IDD = IDD_DIALOG_LOGIN };

	int m_usertype ;
    int m_retry ;

    LoginDlg( HWND hParent) {
        m_usertype = 0;
        m_retry = 3;
		CreateDlg(IDD_DIALOG_LOGIN, hParent);
	}

protected:
    virtual int OnInitDialog();
    virtual int OnOK();
    virtual void OnCbnEditchangeUsername() ;
    virtual int OnCommand( int Id, int Event ) 
    {
        if( Id == IDC_USERNAME && Event == CBN_EDITCHANGE ) 
        {
            OnCbnEditchangeUsername();
            return TRUE ;
        }
        return Dialog::OnCommand( Id, Event ) ;
    }
};

// User dialog
class UserDlg : public Dialog
{
public:
// Dialog Data
	enum { IDD = IDD_DIALOG_USER };

protected:
    virtual int OnInitDialog();
    virtual void OnBnClickedButtonNewuser();
	virtual void OnBnClickedButtonDeluser();
	virtual void OnBnClickedButtonChangepassword();
    virtual int OnCommand( int Id, int Event ) 
    {
        switch (Id) {
        case IDC_BUTTON_NEWUSER :
            OnBnClickedButtonNewuser();
            break;
        case IDC_BUTTON_DELUSER :
            OnBnClickedButtonDeluser();
            break;
        case IDC_BUTTON_CHANGEPASSWORD :
            OnBnClickedButtonChangepassword();
            break;
        }
        return Dialog::OnCommand( Id, Event ) ;
    }
};
