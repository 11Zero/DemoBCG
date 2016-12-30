// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#if !defined(AFX_BCGPCALCULATOR_H__9B759C19_6B1C_48C4_A884_C0DEDD60DD77__INCLUDED_)
#define AFX_BCGPCALCULATOR_H__9B759C19_6B1C_48C4_A884_C0DEDD60DD77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPCalculator.h : header file
//

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)

#include "BCGCBPro.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPPopupMenu.h"
#include "BCGPToolbarImages.h"

class CBCGPPropList;

#endif

#include "BCGPVisualManager.h"

class CBCGPEdit;
class CBCGPRibbonComboBox;

/////////////////////////////////////////////////////////////////////////////
// CBCGPCalculator window

class BCGCBPRODLLEXPORT CBCGPCalculator : public CBCGPPopupMenuBar
{
	friend class CBCGPCalculatorPopup;
	friend class CCalculatorButton;
	friend class CCalculatorDisplay;

	DECLARE_SERIAL(CBCGPCalculator)

// Construction
public:
	CBCGPCalculator();
	CBCGPCalculator(double dblValue, UINT nID, CBCGPEdit* pParentEdit);

#ifndef BCGP_EXCLUDE_RIBBON
#ifndef _BCGSUITE_
	CBCGPCalculator(double dblValue, UINT nID, CBCGPRibbonComboBox* pParentRibbonEdit);
#endif
#endif

	virtual BOOL Create(
			CWnd*		pParentWnd,
			DWORD		dwStyle,
			UINT		nID);

	virtual BOOL CreateControl (
			CWnd*			pParentWnd,
			const CRect&	rect,
			UINT			nID);

protected:
	CBCGPCalculator (CBCGPCalculator& src, UINT uiCommandID);

// Operations:
public:
	void SetAdditionalCommands (const CStringList& lstCommands);
	void RemoveAdditionalCommands ();

	void SetExtendedCommands (const CList<UINT, UINT>& lstCommands);
	void RemoveExtendedCommands ();

// Attributes
public:
	enum CalculatorCommands
	{
		idCommandNone,
		idCommandClear,
		idCommandReset,
		idCommandMemClear,
		idCommandMemRead,
		idCommandMemAdd,
		idCommandAdd,
		idCommandSub,
		idCommandMult,
		idCommandDiv,
		idCommandSign,
		idCommandDot,
		idCommandSqrt,
		idCommandPercent,
		idCommandResult,
		idCommandRev,
		idCommandBackspace,
		idCommandAdvSin = 50,
		idCommandAdvCos,
		idCommandAdvTan,
		idCommandAdvX2,
		idCommandAdvX3,
		idCommandAdvExp,
		idCommandAdv10x,
		idCommandAdvLn,
		idCommandAdvLog,
		idCommandAdvPi,
		idCommandUser = 100
	};

	void SetValue (double dblValue);
	double GetValue () const
	{
		return m_dblValue;
	}

	void SetError (BOOL bIsError = TRUE)
	{
		m_bIsError = bIsError;
		UpdateDisplay ();
	}

	double GetSecondValue () const
	{
		return m_dblSecondValue;
	}

	double GetMemValue () const
	{
		return m_dblMemValue;
	}

	UINT GetCommandID () const
	{
		return m_nCommandID;
	}

	void SetCommandID (UINT nCommandID)
	{
		m_nCommandID = nCommandID;
	}

	virtual BOOL IsDrawGutter() const
	{
		return FALSE;
	}

	void EnableBorder(BOOL bEnable = TRUE);
	BOOL HasBorder() const
	{
		return m_bHasBorder;
	}

	static CString CommandToName(CalculatorCommands cmd);
	static CalculatorCommands NameToCommand(const CString& strName);

	void SetDisplayFormat(const CString& strDisplayFormat);
	const CString& GetDisplayFormat() const
	{
		return m_strDisplayFormat;
	}

protected:
	BOOL						m_bHasBorder;
	double						m_dblValue;
	double						m_dblSecondValue;
	double						m_dblMemValue;
	CString						m_strBuffer;
	UINT						m_uiLastCmd;
	UINT						m_nCommandID;
	BOOL						m_bIsEnabled;
	CBCGPPropList*				m_pWndPropList;
	CBCGPEdit*					m_pParentEdit;
	CBCGPRibbonComboBox*		m_pParentRibbonEdit;
	BOOL						m_bInternal;	// Used in customization
	int							m_nVertMargin;
	int							m_nHorzMargin;
	CRect						m_rectDisplay;
	BOOL						m_bIsClearBuffer;
	BOOL						m_bIsError;
	BOOL						m_bSeqResult;
	int							m_nRows;
	int							m_nColumns;
	CStringList					m_lstAdditionalCommands;
	CList<UINT, UINT>			m_lstExtCommands;
	CString						m_strDisplayFormat;

	CBCGPToolBarImages			m_CalcImages;
	CBCGPToolBarImages			m_CalcImagesDigits;
	CBCGPToolBarImages			m_CalcImagesCommands;
	CBCGPToolBarImages			m_CalcImagesUser;

	static CMap<UINT, UINT, CString, const CString&>	m_mapCommandNames;

// Overrides

	virtual void OnUserCommand (UINT uiCmd	/* Start from idCommandUser */);

	virtual BOOL OnDrawButton (CDC* pDC, CRect rect,
		CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state, 
		int cmd /* CBCGPCalculator::CalculatorCommands */);
	virtual BOOL OnDrawDisplay (CDC* pDC, CRect rect, const CString& strText, BOOL bMem);

	virtual BOOL OnSendCommand (const CBCGPToolbarButton* pButton);
	virtual void AdjustLocations ();
	virtual CSize CalcSize (BOOL bVertDock);

	virtual void Serialize (CArchive& ar);
	virtual void ShowCommandMessageString (UINT uiCmdId);

	virtual BOOL AllowChangeTextLabels () const
	{
		return FALSE;
	}

	virtual BOOL AllowShowOnList () const
	{	
		return TRUE;	
	}

	virtual BOOL AllowShowOnControlMenu () const
	{	
		return FALSE;
	}

	virtual void Rebuild ();
	virtual void PushDigit (short nDigit);
	virtual BOOL OnProcessKey (int nKey, BOOL bIsVirtKey);

	virtual void DoPaint(CDC* pDC);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPCalculator)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

#if (defined _BCGSUITE_) || (defined _BCGSUITE_INC_)
	virtual void OnFillBackground (CDC* pDC);
#endif

// Implementation
public:
	virtual ~CBCGPCalculator();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPCalculator)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	afx_msg LRESULT OnChangeVisualManager (WPARAM, LPARAM);
#ifdef _BCGSUITE_
	afx_msg LRESULT OnSetControlVMMode(WPARAM, LPARAM);
#endif
	DECLARE_MESSAGE_MAP()

	void SetPropList (CBCGPPropList* pWndList)
	{
		m_pWndPropList = pWndList;
	}

	void CommonInit ();

	void UpdateDisplay ();
	void UpdateBuffer ();
	void CalcResult ();

	static void InitCommandNames();
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPCalculatorPopup window

class BCGCBPRODLLEXPORT CBCGPCalculatorPopup : public CBCGPPopupMenu
{
	friend class CBCGPEdit;

	DECLARE_DYNAMIC(CBCGPCalculatorPopup)

// Construction
public:
    CBCGPCalculatorPopup (
					double dblValue, 
					UINT uiCommandID,
					CBCGPEdit* pParentEdit)
		: m_wndCalculator (dblValue, uiCommandID, pParentEdit)
	{
	}

#ifndef BCGP_EXCLUDE_RIBBON
#ifndef _BCGSUITE_

    CBCGPCalculatorPopup (
					double dblValue, 
					UINT uiCommandID,
					CBCGPRibbonComboBox* pParentRibbonEdit)
		: m_wndCalculator (dblValue, uiCommandID, pParentRibbonEdit)
	{
	}

#endif
#endif

	virtual ~CBCGPCalculatorPopup();

// Attributes
protected:
	CBCGPCalculator	m_wndCalculator;

// Operations
public:

	// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CBCGPCalculatorPopup)
	//}}AFX_VIRTUAL

	virtual CBCGPPopupMenuBar* GetMenuBar ()
	{
		return &m_wndCalculator;
	}

	virtual CBCGPControlBar* CreateTearOffBar (CFrameWnd* pWndMain, UINT uiID, LPCTSTR lpszName);

    // Generated message map functions
protected:
    //{{AFX_MSG(CBCGPCalculatorPopup)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPCALCULATOR_H__9B759C19_6B1C_48C4_A884_C0DEDD60DD77__INCLUDED_)
