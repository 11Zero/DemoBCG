//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPRibbonBackstageViewPanel.h: interface for the CBCGPRibbonBackstageViewPanel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONBACKSTAGEVIEWPANEL_H__2EDD1A63_137D_4278_AD81_D0A2EB2CD816__INCLUDED_)
#define AFX_BCGPRIBBONBACKSTAGEVIEWPANEL_H__2EDD1A63_137D_4278_AD81_D0A2EB2CD816__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPRibbonButton.h"
#include "BCGPRibbonMainPanel.h"
#include "BCGPDialog.h"
#include "BCGPToolBarImages.h"
#include "BCGPListBox.h"
#include "BCGPPageTransitionManager.h"

///////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstageViewItemForm class

class BCGCBPRODLLEXPORT CBCGPRibbonBackstageViewItemForm : public CBCGPBaseRibbonElement
{
	friend class CBCGPRibbonBackstageViewPanel;

	DECLARE_DYNCREATE(CBCGPRibbonBackstageViewItemForm)

public:
	// pDlgClass should be derived from CBCGPDialog class:
	CBCGPRibbonBackstageViewItemForm(UINT nDlgTemplateID, CRuntimeClass* pDlgClass, CSize sizeWaterMark = CSize(0, 0));
	virtual ~CBCGPRibbonBackstageViewItemForm();

protected:
	CBCGPRibbonBackstageViewItemForm();

// Operations:
public:
	void SetWaterMarkImage(UINT uiWaterMarkResID, COLORREF clrBase = (COLORREF)-1 /* if clrBase is defined, the images will be addapted to the ribbon main button color */);

protected:
	void CommonConstruct();

// Overrides
protected:
	virtual void OnAfterChangeRect (CDC* pDC);
	virtual BOOL CanBeAddedToQAT () const
	{
		return FALSE;
	}

	virtual CWnd* OnCreateFormWnd();
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);
	virtual CSize GetRegularSize (CDC* /*pDC*/)	{	return m_sizeDlg;	}
	virtual void OnDraw (CDC* /*pDC*/) {}
	virtual void OnChangeVisualManager();

	virtual void OnSetBackstageWatermarkRect(CRect rectWatermark);
	virtual void SetLayoutReady(BOOL /*bReady*/ = TRUE)	{}

// Attributes
protected:
	CWnd*				m_pWndForm; // Form located on the right side
	UINT				m_nDlgTemplateID;
	CRuntimeClass*		m_pDlgClass;
	CSize				m_sizeDlg;
	CSize				m_sizeWaterMark;
	CBCGPToolBarImages	m_Watermark;
	CBCGPToolBarImages	m_WatermarkColorized;
	COLORREF			m_clrWatermarkBaseColor;
	BOOL				m_bImageMirror;
	UINT				m_nRecentFlags;
};

///////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstageViewItemPropertySheet class

class BCGCBPRODLLEXPORT CBCGPRibbonBackstageViewItemPropertySheet : public CBCGPRibbonBackstageViewItemForm
{
	DECLARE_DYNCREATE(CBCGPRibbonBackstageViewItemPropertySheet)

public:
	CBCGPRibbonBackstageViewItemPropertySheet(UINT nIconsListResID, int nIconWidth = 32);
	virtual ~CBCGPRibbonBackstageViewItemPropertySheet();

	void EnablePageTransitionEffect(CBCGPPageTransitionManager::BCGPPageTransitionEffect effect, int nTime = 300)
	{
		m_PageTransitionEffect = effect;
		m_nPageTransitionTime = nTime;
		m_bUseDefaultPageTransitionEffect = FALSE;
	}

protected:
	CBCGPRibbonBackstageViewItemPropertySheet();

// Operations:
public:
	void AddPage(CBCGPPropertyPage* pPage);
	void AddGroup(LPCTSTR lpszCaption);
	void RemoveAll();

// Overrides
protected:
	virtual CWnd* OnCreateFormWnd();
	virtual void CopyFrom (const CBCGPBaseRibbonElement& src);

	virtual void OnSetBackstageWatermarkRect(CRect rectWatermark);
	virtual void SetLayoutReady(BOOL bReady = TRUE);

// Attributes
protected:
	UINT													m_nIconsListResID;
	int														m_nIconWidth;
	CStringArray											m_arCaptions;
	CArray<CBCGPPropertyPage*, CBCGPPropertyPage*>			m_arPages;
	CBCGPPageTransitionManager::BCGPPageTransitionEffect	m_PageTransitionEffect;
	int														m_nPageTransitionTime;
	BOOL													m_bUseDefaultPageTransitionEffect;
};

//////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstageViewPanel class

class BCGCBPRODLLEXPORT CBCGPRibbonBackstageViewPanel : public CBCGPRibbonMainPanel,
														public CBCGPPageTransitionManager
{
	friend class CBCGPRibbonBackstageViewItemForm;

	DECLARE_DYNCREATE(CBCGPRibbonBackstageViewPanel)

public:
	CBCGPRibbonBackstageViewPanel();
	virtual ~CBCGPRibbonBackstageViewPanel();

// Operations
public:
	CBCGPRibbonButton* AddCommand(UINT uiCommandID, LPCTSTR lpszLabel, int nImageIndex = -1)
	{
		ASSERT_VALID (this);

		CBCGPRibbonButton* pButton = new CBCGPRibbonButton (uiCommandID, lpszLabel, nImageIndex);
		ASSERT_VALID (pButton);

		pButton->SetBackstageViewMode();

		CBCGPRibbonMainPanel::Add(pButton);

		return pButton;
	}

	CBCGPRibbonButton* AddView(UINT uiCommandID, LPCTSTR lpszLabel, CBCGPRibbonBackstageViewItemForm* pView)
	{
		ASSERT_VALID(this);

		CBCGPRibbonButton* pButton = new CBCGPRibbonButton (uiCommandID, lpszLabel);
		ASSERT_VALID (pButton);

		pButton->SetBackstageViewMode();

		if (pView != NULL)
		{
			ASSERT_VALID(pView);
			pButton->AddSubItem(pView);
		}

		CBCGPRibbonMainPanel::Add(pButton);

		return pButton;
	}

	CBCGPRibbonButton* AddPrintPreview(UINT uiCommandID, LPCTSTR lpszLabel,
		UINT uiWaterMarkResID = 0, COLORREF clrBase = (COLORREF)-1);

	CBCGPRibbonButton* AddRecentView(UINT uiCommandID, LPCTSTR lpszLabel, UINT nFlags = 0xFFFF,
		UINT uiWaterMarkResID = 0, COLORREF clrBase = (COLORREF)-1);

	BOOL AttachViewToItem(UINT uiID, CBCGPRibbonBackstageViewItemForm* pView, BOOL bByCommand = TRUE);
	BOOL AttachPrintPreviewToItem(UINT uiID, BOOL bByCommand = TRUE,
		UINT uiWaterMarkResID = 0, COLORREF clrBase = (COLORREF)-1);
	BOOL AttachRecentViewToItem(UINT uiID, UINT nFlags = 0xFFFF, BOOL bByCommand = TRUE,
		UINT uiWaterMarkResID = 0, COLORREF clrBase = (COLORREF)-1);

	virtual void ReposActiveForm();

	CBCGPRibbonButton* FindFormParent (CRuntimeClass* pClass, BOOL bDerived = TRUE);
	int FindFormPageIndex (CRuntimeClass* pClass, BOOL bDerived = TRUE);

	int GetFormsCount() const;
	CBCGPRibbonBackstageViewItemForm* GetForm(int nIndex) const;

protected:
	// The following methods cannot be called for CBCGPRibbonBackstageViewPanel:
	virtual void Add (CBCGPBaseRibbonElement* /*pElem*/)						{	ASSERT(FALSE);	}
	void AddToBottom (CBCGPRibbonMainPanelButton* /*pElem*/)					{	ASSERT(FALSE);	}
	void AddToRight (CBCGPBaseRibbonElement* /*pElem*/, int /*nWidth*/ = 300)	{	ASSERT(FALSE);	}
	void EnableCommandSearch (BOOL /*bEnable*/, LPCTSTR /*lpszLabel*/, LPCTSTR /*lpszKeyTip*/, int /*nWidth*/ = 0)	{	ASSERT(FALSE);	}

	virtual int GetMenuElements () const
	{
		ASSERT_VALID (this);
		return (int) m_arElements.GetSize ();
	}

	virtual BOOL IsBackstageView() const
	{
		return TRUE;
	}

	virtual BOOL IsBackstageRightPaneActive() const
	{
		ASSERT_VALID(this);
		return m_pSelected != NULL;
	}

	virtual void Repos (CDC* pDC, const CRect& rect);
	virtual CBCGPBaseRibbonElement* MouseButtonDown (CPoint point);

	virtual BOOL OnKey (UINT nChar);

	virtual void SelectView(CBCGPBaseRibbonElement* pElem);
	void AdjustScrollBars();
	virtual void OnPageTransitionFinished();

	virtual void DoPaint (CDC* pDC);

// Attributes:
protected:
	CBCGPBaseRibbonElement*		m_pSelected;
	CBCGPBaseRibbonElement*		m_pNewSelected;
	CRect						m_rectRight;
	CSize						m_sizeRightView;
	BOOL						m_bInAdjustScrollBars;
	BOOL						m_bSelectedByMouseClick;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRecentFilesListBox window

class BCGCBPRODLLEXPORT CBCGPRecentFilesListBox : public CBCGPListBox
{
	DECLARE_DYNAMIC(CBCGPRecentFilesListBox)

// Construction
public:
	CBCGPRecentFilesListBox();

// Attributes
public:
	void SetFoldersMode(BOOL bSet = TRUE);
	BOOL IsFoldersMode() const
	{
		return m_bFoldersMode;
	}

// Operations
public:
	void FillList(LPCTSTR lpszSelectedPath = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRecentFilesListBox)
	//}}AFX_VIRTUAL

	virtual void OnClickPin(int nClickedItem);
	virtual void OnClickItem(int nClickedItem);
	virtual BOOL OnReturnKey();

	virtual void OnDrawItemContent(CDC* pDC, CRect rect, int nIndex);
	virtual void OnChooseRecentFile(UINT uiCmd);
	virtual void OnChooseRecentFolder(LPCTSTR lpszFolder);
	virtual void OnChoosePinnedFile(LPCTSTR lpszFile);

	int AddItem(const CString& strFilePath, UINT nCmd = 0, BOOL bPin = FALSE);
	CString GetItemPath(int nItem);

// Implementation
public:
	virtual ~CBCGPRecentFilesListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPRecentFilesListBox)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CArray<HICON, HICON>	m_arIcons;
	BOOL					m_bFoldersMode;
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONBACKSTAGEVIEWPANEL_H__2EDD1A63_137D_4278_AD81_D0A2EB2CD816__INCLUDED_)
