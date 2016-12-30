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
// BCGPTextFormatDlg.h : header file
//

#if !defined(AFX_BCGPTEXTFORMATDLG_H__EE284E9B_15A9_49E8_8AA7_108A73777F28__INCLUDED_)
#define AFX_BCGPTEXTFORMATDLG_H__EE284E9B_15A9_49E8_8AA7_108A73777F28__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPDialog.h"
#include "BCGPGraphicsManager.h"
#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
#include "BCGPFontComboBox.h"
#endif
#include "BCGPStatic.h"
#include "BCGPComboBox.h"
#include "BCGPButton.h"

class BCGCBPRODLLEXPORT CBCGPTextPreviewCtrl : public CBCGPStatic
{
	friend class CBCGPTextFormatDlg;

public:
	CBCGPTextPreviewCtrl()
	{
		m_pGM = NULL;
		m_pFormat = NULL;
		m_brFill.SetColor(CBCGPColor::White);
		m_brText.SetColor(CBCGPColor::Black);
		m_HorzAlign = CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING;
		m_VertAlign = CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING;
	}

	virtual ~CBCGPTextPreviewCtrl()
	{
		if (m_pGM != NULL)
		{
			delete m_pGM;
		}
	}

protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGPTextPreviewCtrl)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGPGraphicsManager*					m_pGM;
	CBCGPTextFormat*						m_pFormat;
	CBCGPBrush								m_brFill;
	CBCGPBrush								m_brText;
	CString									m_strText;
	CBCGPTextFormat::BCGP_TEXT_ALIGNMENT	m_HorzAlign;
	CBCGPTextFormat::BCGP_TEXT_ALIGNMENT	m_VertAlign;
};


/////////////////////////////////////////////////////////////////////////////
// CBCGPTextFormatDlgOptions

struct BCGCBPRODLLEXPORT CBCGPTextFormatDlgOptions
{
public:
	CBCGPTextFormatDlgOptions()
	{
		m_strPreviewText = _T("AaBbYyZz");
		m_bShowAngle = TRUE;
		m_bShowClipText = TRUE;
		m_bShowWordWrap = TRUE;
	}

	CString		m_strPreviewText;
	BOOL		m_bShowAngle;
	BOOL		m_bShowClipText;
	BOOL		m_bShowWordWrap;
	CBCGPBrush	m_brFill;
	CBCGPBrush	m_brText;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPTextFormatDlg dialog

class BCGCBPRODLLEXPORT CBCGPTextFormatDlg : public CBCGPDialog
{
// Construction
public:
	CBCGPTextFormatDlg(CBCGPTextFormat& textFormat, CBCGPTextFormatDlgOptions* pOptions = NULL, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBCGPTextFormatDlg)
	enum { IDD = IDD_BCGBARRES_TEXT_FORMAT };
	CBCGPButton	m_wndWordWrap;
	CBCGPButton	m_wndClipText;
	CBCGPStatic	m_wndAngleLabel;
	CBCGPStatic	m_wndLocaleLabel;
	CBCGPComboBox	m_wndLocaleCombo;
	CBCGPComboBox	m_wndTextSize;
	CBCGPComboBox	m_wndAngle;
	CBCGPTextPreviewCtrl	m_wndPreview;
#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
	CBCGPFontComboBox	m_wndFontComboBox;
#else
	CMFCFontComboBox	m_wndFontComboBox;
#endif
	BOOL	m_bClipText;
	CString	m_strTextAngle;
	int		m_nHorzAlignment;
	CString	m_strTextSize;
	int		m_nTextStyle;
	int		m_nVertAlignment;
	int		m_nTextWeight;
	BOOL	m_bWordWrap;
	int		m_nLocaleIndex;
	BOOL	m_bUnderline;
	BOOL	m_bStrikethrough;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPTextFormatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPTextFormatDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdateFont();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void SetupFormat(CBCGPTextFormat& textFormat, BOOL bForPreview);

	CBCGPTextFormat&			m_textFormat;
	CBCGPTextFormat				m_textFormatPreview;
	BOOL						m_bIsNegativeHeight;
	CBCGPTextFormatDlgOptions	m_Options;
	BOOL						m_bUseCharSet;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPTEXTFORMATDLG_H__EE284E9B_15A9_49E8_8AA7_108A73777F28__INCLUDED_)
