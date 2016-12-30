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
//
// BCGPTextFormatDlg.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include "bcgprores.h"
#include "BCGPTextFormatDlg.h"
#include "BCGPLocalResource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BYTE _char_sets[] =
{
	DEFAULT_CHARSET,
	ANSI_CHARSET,
	SHIFTJIS_CHARSET,
	HANGUL_CHARSET,
	GB2312_CHARSET,
	CHINESEBIG5_CHARSET,
	JOHAB_CHARSET,
	HEBREW_CHARSET,
	ARABIC_CHARSET,
	GREEK_CHARSET,
	TURKISH_CHARSET,
	VIETNAMESE_CHARSET,
	THAI_CHARSET,
	EASTEUROPE_CHARSET,
	RUSSIAN_CHARSET,
	BALTIC_CHARSET,
};

static int GetCharsetIndex(BYTE bCharSet)
{
	for (int i = 0; i < sizeof(_char_sets) / sizeof(BYTE); i++)
	{
		if (_char_sets[i] == bCharSet)
		{
			return i;
		}
	}

	return -1;
}

static BYTE GetCharset(int nIndex)
{
	if (nIndex < 0 || nIndex >= sizeof(_char_sets) / sizeof(BYTE))
	{
		return DEFAULT_CHARSET;
	}

	return _char_sets[nIndex];
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTextPreviewCtrl

BEGIN_MESSAGE_MAP(CBCGPTextPreviewCtrl, CBCGPStatic)
	//{{AFX_MSG_MAP(CBCGPTextPreviewCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBCGPTextPreviewCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CBCGPMemDC memDC (dc, this);
	CDC* pDC = &memDC.GetDC ();

	CRect rect;
	GetClientRect(rect);

	pDC->FillRect (rect, &globalData.brWindow);

	if (m_pGM == NULL || m_pFormat == NULL)
	{
		return;
	}

	m_pGM->BindDC(pDC, rect);

	if (!m_pGM->BeginDraw())
	{
		return;
	}

	m_pGM->FillRectangle(rect, m_brFill);

	int nVertMargin = rect.Height() / 4;
	int nHorzMargin = rect.Width() / 4;

	switch (m_HorzAlign)
	{
	case CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING:
		rect.right -= nHorzMargin;
		break;

	case CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_TRAILING:
		rect.left += nHorzMargin;
		break;
	}

	switch (m_VertAlign)
	{
	case CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING:
		rect.bottom -= nVertMargin;
		break;

	case CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_TRAILING:
		rect.top += nVertMargin;
		break;
	}

	m_pGM->DrawText(m_strText, rect, *m_pFormat, m_brText);

	m_pGM->EndDraw();
}

static void DoubleToString(double val, CString& str)
{
	str.Format(_T("%.2f"), val);

	int nIndex = str.Find(_T(".00"));
	if (nIndex > 0)
	{
		str = str.Left(nIndex);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTextFormatDlg dialog

CBCGPTextFormatDlg::CBCGPTextFormatDlg(CBCGPTextFormat& textFormat, CBCGPTextFormatDlgOptions* pOptions, CWnd* pParent /*=NULL*/)
	: CBCGPDialog(CBCGPTextFormatDlg::IDD, pParent),
	m_textFormat(textFormat)
{
	//{{AFX_DATA_INIT(CBCGPTextFormatDlg)
	m_bClipText = m_textFormat.IsClipText();
	m_strTextAngle = _T("0");
	m_nHorzAlignment = (int)m_textFormat.GetTextAlignment();
	m_strTextSize = _T("");
	m_nTextStyle = m_textFormat.GetFontStyle();
	m_nVertAlignment = m_textFormat.GetTextVerticalAlignment();
	m_nTextWeight = m_textFormat.GetFontWeight() == 950 ? 9 : m_textFormat.GetFontWeight() / 100 - 1;
	m_bWordWrap = m_textFormat.IsWordWrap();
	m_nLocaleIndex = 0;
	m_bUnderline = textFormat.IsUnderline();
	m_bStrikethrough = textFormat.IsStrikethrough();
	//}}AFX_DATA_INIT

	if (pOptions != NULL)
	{
		m_Options = *pOptions;

		if (!m_Options.m_brFill.IsEmpty())
		{
			m_wndPreview.m_brFill = m_Options.m_brFill;
		}

		if (!m_Options.m_brText.IsEmpty())
		{
			m_wndPreview.m_brText = m_Options.m_brText;
		}
	}

	double dblTextSize = m_textFormat.GetFontSize();
	m_bIsNegativeHeight = FALSE;

	if (dblTextSize < 0.0)
	{
		CWindowDC dc(NULL);

		int nLogY = dc.GetDeviceCaps(LOGPIXELSY);
		if (nLogY != 0)
		{
			dblTextSize = MulDiv(72, (int)(-100 * dblTextSize), nLogY * 100);
			m_bIsNegativeHeight = TRUE;
		}
	}

	DoubleToString(m_textFormat.GetDrawingAngle(), m_strTextAngle);
	DoubleToString(dblTextSize, m_strTextSize);

	m_bIsLocal = TRUE;

	m_wndPreview.m_pGM = CBCGPGraphicsManager::CreateInstance();
	ASSERT_VALID(m_wndPreview.m_pGM);

	m_bUseCharSet = !m_wndPreview.m_pGM->IsSupported(BCGP_GRAPHICS_MANAGER_LOCALE);

	m_wndPreview.m_pFormat = &m_textFormatPreview;

	m_wndPreview.m_HorzAlign = (CBCGPTextFormat::BCGP_TEXT_ALIGNMENT)m_nHorzAlignment;
	m_wndPreview.m_VertAlign = (CBCGPTextFormat::BCGP_TEXT_ALIGNMENT)m_nVertAlignment;

#ifndef _BCGSUITE_
	EnableVisualManagerStyle (globalData.m_bUseVisualManagerInBuiltInDialogs, TRUE);
#endif
}


void CBCGPTextFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPTextFormatDlg)
	DDX_Control(pDX, IDC_BCGBARRES_WORD_WRAP, m_wndWordWrap);
	DDX_Control(pDX, IDC_BCGBARRES_CLIP_TEXT, m_wndClipText);
	DDX_Control(pDX, IDC_BCGBARRES_TEXT_ANGLE_LABEL, m_wndAngleLabel);
	DDX_Control(pDX, IDC_BCGBARRES_TEXT_LOCALE_LABEL, m_wndLocaleLabel);
	DDX_Control(pDX, IDC_BCGBARRES_TEXT_LOCALE, m_wndLocaleCombo);
	DDX_Control(pDX, IDC_BCGBARRES_TEXT_SIZE, m_wndTextSize);
	DDX_Control(pDX, IDC_BCGBARRES_TEXT_ANGLE, m_wndAngle);
	DDX_Control(pDX, IDC_BCGBARRES_IMAGE, m_wndPreview);
	DDX_Control(pDX, IDC_BCGBARRES_TEXT_FAMILY, m_wndFontComboBox);
	DDX_Check(pDX, IDC_BCGBARRES_CLIP_TEXT, m_bClipText);
	DDX_CBString(pDX, IDC_BCGBARRES_TEXT_ANGLE, m_strTextAngle);
	DDX_CBIndex(pDX, IDC_BCGBARRES_TEXT_HORZ_ALIGNMENT, m_nHorzAlignment);
	DDX_CBString(pDX, IDC_BCGBARRES_TEXT_SIZE, m_strTextSize);
	DDX_CBIndex(pDX, IDC_BCGBARRES_TEXT_STYLE, m_nTextStyle);
	DDX_CBIndex(pDX, IDC_BCGBARRES_TEXT_VERT_ALIGNMENT, m_nVertAlignment);
	DDX_CBIndex(pDX, IDC_BCGBARRES_TEXT_WEIGHT, m_nTextWeight);
	DDX_Check(pDX, IDC_BCGBARRES_WORD_WRAP, m_bWordWrap);
	DDX_CBIndex(pDX, IDC_BCGBARRES_TEXT_LOCALE, m_nLocaleIndex);
	DDX_Check(pDX, IDC_BCGBARRES_UNDERLINE_TEXT, m_bUnderline);
	DDX_Check(pDX, IDC_BCGBARRES_STRIKETHROUGH_TEXT, m_bStrikethrough);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGPTextFormatDlg, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPTextFormatDlg)
	ON_CBN_SELENDOK(IDC_BCGBARRES_TEXT_FAMILY, OnUpdateFont)
	ON_CBN_SELENDOK(IDC_BCGBARRES_TEXT_LOCALE, OnUpdateFont)
	ON_CBN_SELENDOK(IDC_BCGBARRES_TEXT_SIZE, OnUpdateFont)
	ON_CBN_SELENDOK(IDC_BCGBARRES_TEXT_STYLE, OnUpdateFont)
	ON_CBN_SELENDOK(IDC_BCGBARRES_TEXT_VERT_ALIGNMENT, OnUpdateFont)
	ON_CBN_SELENDOK(IDC_BCGBARRES_TEXT_WEIGHT, OnUpdateFont)
	ON_BN_CLICKED(IDC_BCGBARRES_WORD_WRAP, OnUpdateFont)
	ON_CBN_SELENDOK(IDC_BCGBARRES_TEXT_HORZ_ALIGNMENT, OnUpdateFont)
	ON_BN_CLICKED(IDC_BCGBARRES_CLIP_TEXT, OnUpdateFont)
	ON_CBN_SELENDOK(IDC_BCGBARRES_TEXT_ANGLE, OnUpdateFont)
	ON_CBN_EDITCHANGE(IDC_BCGBARRES_TEXT_ANGLE, OnUpdateFont)
	ON_CBN_EDITCHANGE(IDC_BCGBARRES_TEXT_SIZE, OnUpdateFont)
	ON_BN_CLICKED(IDC_BCGBARRES_UNDERLINE_TEXT, OnUpdateFont)
	ON_BN_CLICKED(IDC_BCGBARRES_STRIKETHROUGH_TEXT, OnUpdateFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPTextFormatDlg message handlers

void CBCGPTextFormatDlg::SetupFormat(CBCGPTextFormat& textFormat, BOOL bForPreview)
{
	UpdateData();

	textFormat.Destroy();

	CString strFamily;
	if (m_wndFontComboBox.GetCurSel() >= 0)
	{
		m_wndFontComboBox.GetLBText(m_wndFontComboBox.GetCurSel(), strFamily);
	}

	double dblAngle = 0.0;
	double dblSize = 0.0;

	CString strTextAngle = m_strTextAngle;
	if (m_wndAngle.GetCurSel() >= 0)
	{
		m_wndAngle.GetLBText(m_wndAngle.GetCurSel(), strTextAngle);
	}

	CString strTextSize = m_strTextSize;
	if (m_wndTextSize.GetCurSel() >= 0)
	{
		m_wndTextSize.GetLBText(m_wndTextSize.GetCurSel(), strTextSize);
	}

#if _MSC_VER < 1400
	_stscanf (strTextAngle, _T("%lf"), &dblAngle);
	_stscanf (strTextSize, _T("%lf"), &dblSize);
#else
	_stscanf_s (strTextAngle, _T("%lf"), &dblAngle);
	_stscanf_s (strTextSize, _T("%lf"), &dblSize);
#endif

	if (m_bIsNegativeHeight && !bForPreview && dblSize != 0.0)
	{
		CWindowDC dc(NULL);

		int nLogY = dc.GetDeviceCaps(LOGPIXELSY);
		if (nLogY != 0)
		{
			dblSize = -dblSize * nLogY / 72.0;
		}
	}

	int nWeight = min(950, (m_nTextWeight + 1) * 100);

	if (!m_bUseCharSet)
	{
		CString strLocaleName;
		
		if (m_nLocaleIndex > 0)
		{
			CString strLocale;
			m_wndLocaleCombo.GetLBText(m_nLocaleIndex, strLocale);

#ifndef _BCGSUITE_
			strLocaleName = globalData.GetLocaleName(strLocale);
#endif
		}

		CBCGPTextFormat tfNew(strFamily, (float)dblSize, nWeight, (CBCGPTextFormat::BCGP_FONT_STYLE)m_nTextStyle,
			strLocaleName.IsEmpty() ? NULL : (LPCTSTR)strLocaleName);

		textFormat = tfNew;
	}
	else
	{
		CBCGPTextFormat tfNew(GetCharset(m_nLocaleIndex), strFamily, (float)dblSize, nWeight, (CBCGPTextFormat::BCGP_FONT_STYLE)m_nTextStyle);

		textFormat = tfNew;
	}

	textFormat.SetClipText(m_bClipText);

	if (bForPreview)
	{
		textFormat.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
		textFormat.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	}
	else
	{
		textFormat.SetTextAlignment((CBCGPTextFormat::BCGP_TEXT_ALIGNMENT)m_nHorzAlignment);
		textFormat.SetTextVerticalAlignment((CBCGPTextFormat::BCGP_TEXT_ALIGNMENT)m_nVertAlignment);
	}

	textFormat.SetWordWrap(m_bWordWrap);
	textFormat.SetDrawingAngle(dblAngle);

	textFormat.SetUnderline(m_bUnderline);
	textFormat.SetStrikethrough(m_bStrikethrough);
}

void CBCGPTextFormatDlg::OnOK() 
{
	SetupFormat(m_textFormat, FALSE);

	CBCGPDialog::OnOK();
}

BOOL CBCGPTextFormatDlg::OnInitDialog() 
{
	CBCGPDialog::OnInitDialog();

	if (!m_Options.m_bShowAngle)
	{
		m_wndAngle.EnableWindow(FALSE);
		m_wndAngle.ShowWindow(SW_HIDE);
		m_wndAngleLabel.ShowWindow(SW_HIDE);
	}

	if (!m_Options.m_bShowWordWrap)
	{
		m_wndWordWrap.EnableWindow(FALSE);
		m_wndWordWrap.ShowWindow(SW_HIDE);
		m_wndWordWrap.ShowWindow(SW_HIDE);
	}

	if (!m_Options.m_bShowClipText)
	{
		m_wndClipText.EnableWindow(FALSE);
		m_wndClipText.ShowWindow(SW_HIDE);
		m_wndClipText.ShowWindow(SW_HIDE);
	}

	if (m_bUseCharSet)
	{
		CBCGPLocalResource locaRes;

		m_nLocaleIndex = GetCharsetIndex(m_textFormat.GetCharSet());

		CString strLabel;
		strLabel.LoadString(IDS_BCGBARRES_SCRIPT_LABEL);

		m_wndLocaleLabel.SetWindowText(strLabel);
	}
	else
	{
		m_wndLocaleCombo.ResetContent();
		m_wndLocaleCombo.AddString(_T("Default"));

#ifndef _BCGSUITE_
		const CStringList& list = globalData.GetLocaleList();

		for (POSITION pos = list.GetHeadPosition (); pos != NULL;)
		{
			m_wndLocaleCombo.AddString(list.GetNext(pos));
		}

		CString strLocale = globalData.GetLocaleByName(m_textFormat.GetFontLocale());
		if (strLocale.IsEmpty())
		{
			m_nLocaleIndex = 0;	// Default
		}
		else
		{
			m_nLocaleIndex = m_wndLocaleCombo.FindStringExact(-1, strLocale);
		}

		m_wndLocaleCombo.SetCurSel(m_nLocaleIndex);
#endif
	}
	
	m_wndFontComboBox.SelectFont(m_textFormat.GetFontFamily());

	m_wndPreview.m_strText = m_Options.m_strPreviewText;
	SetupFormat(m_textFormatPreview, TRUE);

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBCGPTextFormatDlg::OnUpdateFont() 
{
	SetupFormat(m_textFormatPreview, TRUE);

	m_wndPreview.m_HorzAlign = (CBCGPTextFormat::BCGP_TEXT_ALIGNMENT)m_nHorzAlignment;
	m_wndPreview.m_VertAlign = (CBCGPTextFormat::BCGP_TEXT_ALIGNMENT)m_nVertAlignment;

	m_wndPreview.RedrawWindow();
}

