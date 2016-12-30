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
//
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_BCGPRIBBONBACKSTAGEPAGEPRINT_H__4FC7215B_DC6D_4370_9499_EE905AFD3B59__INCLUDED_)
#define AFX_BCGPRIBBONBACKSTAGEPAGEPRINT_H__4FC7215B_DC6D_4370_9499_EE905AFD3B59__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPBackStagePagePrint.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPPrintPreviewCtrl.h"
#include "BCGPDialog.h"
#include "BCGPButton.h"
#include "BCGPEdit.h"
#include "BCGPSpinButtonCtrl.h"
#include "BCGPURLLinkButton.h"
#include "BCGPStatic.h"
#include "BCGPSliderCtrl.h"
#include "BCGPRibbonPaletteButton.h"
#include "bcgprores.h"

class CBCGPRibbonBackstageView;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstagePagePrint dialog

class BCGCBPRODLLEXPORT CBCGPPrintInfo
{
public:
	CBCGPPrintInfo()
		: Flags    (0)
		, nFromPage(0)
		, nToPage  (0)
		, nCopies  (0)
		, bPrinting(FALSE)
		, lpParam  (NULL)
	{
	}

	virtual ~CBCGPPrintInfo()
	{
	}

	DWORD	Flags;
	WORD	nFromPage;
	WORD	nToPage;
	WORD	nCopies;

	BOOL	bPrinting;
	LPVOID	lpParam;
};

BOOL BCGCBPRODLLEXPORT BCGPPreparePrinting(CView* pView, CPrintInfo* pInfo);

class BCGCBPRODLLEXPORT CBCGPRibbonBackstagePagePrint : public CBCGPDialog
{
	DECLARE_DYNCREATE(CBCGPRibbonBackstagePagePrint)

public:
	static CBCGPPrintInfo	s_BCGPrintInfo;

// Construction
public:
	CBCGPRibbonBackstagePagePrint(UINT nIDTemplate = 0, CWnd* pParent = NULL);   // standard constructor
	virtual ~CBCGPRibbonBackstagePagePrint();

// Dialog Data
	//{{AFX_DATA(CBCGPRibbonBackstagePagePrint)
	enum { IDD = IDD_BCGBARRES_PRINT_FORM };
	CBCGPStatic	m_wndLabel3;
	CBCGPStatic	m_wndLabel2;
	CBCGPStatic	m_wndLabel1;
	CBCGPButton					m_btnPrint;
	CBCGPEdit					m_wndCopies;
	CBCGPSpinButtonCtrl			m_btnCopies;
	CBCGPRibbonComboGalleryCtrl	m_wndPrinter;
	CBCGPURLLinkButton			m_wndPrinterProperties;
	CBCGPRibbonComboGalleryCtrl	m_wndPage;
	CBCGPEdit					m_wndPageFrom;
	CBCGPEdit					m_wndPageTo;
	CBCGPRibbonComboGalleryCtrl	m_wndCollate;
	CBCGPRibbonComboGalleryCtrl	m_wndOrientation;
	CBCGPRibbonComboGalleryCtrl	m_wndPaper;
	CBCGPPrintPreviewCtrl*		m_wndPreview;
	CBCGPButton					m_btnPrev;
	CBCGPStatic					m_wndPageNum;
	CBCGPButton					m_btnNext;
	CBCGPStatic					m_wndZoomNum;
	CBCGPSliderCtrl				m_wndZoomSlider;
	short						m_nCopies;
	UINT						m_nPageFrom;
	UINT						m_nPageTo;
	//}}AFX_DATA

public:
	short GetCopies();
	void SetCopies(short nCopies, BOOL bEnable);

	CPrintInfo* GetPrintInfo()
	{
		return m_wndPreview != NULL ? m_wndPreview->GetPrintInfo() : NULL;
	}

	PRINTDLG* GetPrintDlg()
	{
		CPrintInfo* pInfo = GetPrintInfo();
		return (pInfo != NULL && pInfo->m_pPD != NULL) ? &pInfo->m_pPD->m_pd : NULL;
	}

	CBCGPPrintPreviewCtrl* GetPreviewWnd()
	{
		return m_wndPreview;
	}

	virtual void OnInitPrintControls();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonBackstagePagePrint)
	public:
	virtual BOOL OnInitDialog();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPRibbonBackstagePagePrint)
	afx_msg void OnSelectPrinter();
	afx_msg void OnPrintFile();
	afx_msg void OnUpdatePrintFile(CCmdUI* pCmdUI);
	afx_msg void OnPrinterProperties();
	afx_msg void OnSelectCollate();
	afx_msg void OnSelectPage();
	afx_msg void OnSelectPaper();
	afx_msg void OnPreviewPrint();
	afx_msg void OnPrevPage();
	afx_msg void OnPageChanged();
	afx_msg void OnNextPage();
	afx_msg void OnZoomChanged();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void LoadPrinterImages();
	void UpdatePrinters ();
	void UpdateLabels();
	void UpdatePapers (short nPaper = 0);
	int GetPrinterSelection () const;
	int GetPaperSelection () const;
	virtual void UpdatePrinterProperties (BOOL bSaveAndValidate, BOOL bNotify = FALSE);
	CString GetDeviceName ();
	HANDLE OpenPrinterByName (LPCTSTR lpDeviceName = NULL);

	CBCGPRibbonBackstageView* GetBackStageView();

	virtual CBCGPPrintPreviewCtrl* CreatePreviewWnd() const;
	virtual void InitializePrintInfo();
	virtual BOOL PreparePrintInfo();

protected:
	struct XPaperInfo
	{
		XPaperInfo() {}
		XPaperInfo(short paper, LPCTSTR name, const POINT& size)
			: nPaper (paper)
			, strName(name)
			, ptSize (size)
		{
		}

		CString GetDisplayName() const;

		short	nPaper;
		CString	strName;
		CPoint	ptSize;
	};
	typedef CArray<XPaperInfo, XPaperInfo&> XPapers;

	struct XPrinterInfo
	{
		XPrinterInfo()
			: dwAttributes(0)
			, dwStatus    (0)
		{
		}
		XPrinterInfo(LPCTSTR pServerName, LPCTSTR pPrinterName, LPCTSTR pPortName, 
					LPCTSTR pDriverName, DWORD attributes, DWORD status)
			: strServerName (pServerName)
			, strPrinterName(pPrinterName)
			, strPortName   (pPortName)
			, strDriverName (pDriverName)
			, dwAttributes  (attributes)
			, dwStatus      (status)
		{
		}

		CString GetDisplayName() const;
		CString GetStatusName() const;

		CString strServerName;
		CString strPrinterName;
		CString strPortName;
		CString strDriverName;
		DWORD	dwAttributes;
		DWORD	dwStatus;
	};
	typedef CArray<XPrinterInfo, XPrinterInfo&> XPrinters;

	CBCGPToolBarImages	m_PrinterImages;
	XPrinters			m_Printers;
	XPapers				m_Papers;
	
	CView*				m_pPrintView;
};

#endif // BCGP_EXCLUDE_RIBBON

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONBACKSTAGEPAGEPRINT_H__4FC7215B_DC6D_4370_9499_EE905AFD3B59__INCLUDED_)
