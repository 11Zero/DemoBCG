//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************

#if !defined(AFX_BCGFILEDIALOG_H__6975A857_62DC_11D2_8BF2_00A0C9B05590__INCLUDED_)
#define AFX_BCGFILEDIALOG_H__6975A857_62DC_11D2_8BF2_00A0C9B05590__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGFileDialog.h : header file
//

#include "BCGCBPro.h"

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
#include "BCGPTabWnd.h"
#endif

class CBCGPFileDialog;

class CBCGPFDTabWnd : public CBCGPTabWnd
{
public:
	CBCGPFDTabWnd ()
	{
		m_pParent = NULL;
		m_bIsDlgControl = TRUE;
	}

	virtual void FireChangeActiveTab (int nNewTab);
	CBCGPFileDialog*	m_pParent;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPFileDialog dialog

class BCGCBPRODLLEXPORT CBCGPFileDialog : public CFileDialog
{
	friend class CBCGPFDTabWnd;

	DECLARE_DYNAMIC(CBCGPFileDialog)

public:
	//----------------
	// Initialization:
	//----------------
	CBCGPFileDialog (LPCTSTR lpszCaption,
		BOOL bNewPage,
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);
	virtual ~CBCGPFileDialog ();

	void SetDlgIcon (HICON hIconBig, HICON hIconSmall = NULL);

	void SetNewImagesList (CImageList* pImagesNew)
	{
		m_pImagesNew = pImagesNew;
	}

	void SetLogoBitmap (CBitmap* pBmpLogo)
	{
		m_pBmpLogo = pBmpLogo;
	}

    //fdncred
    void SetExtraWidth(int iExtraWidth)
    {
        m_iExtraWidth = iExtraWidth;
    }

    void SetExtraHeight(int iExtraHeight)
    {
        m_iExtraHeight = iExtraHeight;
    }

	void AddNewItem (LPCTSTR lpszName, int iIconIndex);

	//---------
	// Results:
	//---------
	enum SelectedPage
	{
		BCGFileNew,
		BCGFileOpen,
		BCGFileRecent
	};

	SelectedPage GetPage () const
	{
		return m_nPage;
	}

	const CString& GetRecentFilePath () const
	{
		ASSERT (m_nPage == BCGFileRecent);
		return m_strRecentFilePath;
	}

	int GetNewItemIndex () const
	{
		ASSERT (m_nPage == BCGFileNew);
		return m_iNewItemIndex;
	}

protected:

	//------------------------
	// Internal notifications:
	//------------------------
	void OnTabSelchange ();
	void OnItemDblClick ();
	void CollectControls ();

	static LRESULT CALLBACK WindowProcNew(HWND hwnd,UINT message, WPARAM wParam, LPARAM lParam);

protected:
	//{{AFX_MSG(CBCGPFileDialog)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void OnInitDone();

// Attributes:
public:
	static WNDPROC m_wndProc;

protected:
	CBCGPFDTabWnd		m_wndTab;
	CStatic				m_wndDummy;
	CListCtrl			m_wndNewList;
	CListCtrl			m_wndRecentList;
	CList<HWND, HWND>	m_lstFDControls;

	CImageList*			m_pImagesNew;
	CImageList			m_ImagesRecent;

	CObList				m_lstNewItems;
	BOOL				m_bNewPage;
	SelectedPage		m_nPage;

	CString				m_strCaption;

	CString				m_strRecentFilePath;
	int					m_iNewItemIndex;

	CString				m_strFilter;

	CBitmap*			m_pBmpLogo;
	CRect				m_rectLogo;

	HICON				m_hIconBig;
	HICON				m_hIconSmall;

	int					m_iLogoAreaHeight;
    int                 m_iExtraWidth;
    int                 m_iExtraHeight;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGFILEDIALOG_H__6975A857_62DC_11D2_8BF2_00A0C9B05590__INCLUDED_)
