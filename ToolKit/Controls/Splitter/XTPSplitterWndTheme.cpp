// XTPSplitterWndTheme.cpp: implementation of the XTPSplitterWndTheme class.
//
// This file is a part of the XTREME CONTROLS MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Common/XTPWinThemeWrapper.h"
#include "Common/XTPColorManager.h"
#include "Common/XTPDrawHelpers.h"
#include "Common/XTPResourceImage.h"

#include "Controls/Util/XTPControlTheme.h"
#include "Controls/Splitter/XTPSplitterWnd.h"
#include "Controls/Splitter/XTPSplitterWndTheme.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXTPSplitterWndTheme

CXTPSplitterWndTheme::CXTPSplitterWndTheme()
{
}

void CXTPSplitterWndTheme::RefreshMetrics(CXTPSplitterWnd* /*pSplitter*/)
{
	m_clrSplitterFace.SetStandardValue(GetSysColor(COLOR_3DFACE));
	m_clrSplitterBorders.SetStandardValue(GetSysColor(COLOR_3DSHADOW));
}

/////////////////////////////////////////////////////////////////////////////
// CXTPSplitterWndThemeOfficeXP

CXTPSplitterWndThemeOfficeXP::CXTPSplitterWndThemeOfficeXP()
{
	m_nTheme = xtpControlThemeOfficeXP;
}

void CXTPSplitterWndThemeOfficeXP::RefreshMetrics(CXTPSplitterWnd* pSplitter)
{
	CXTPSplitterWndTheme::RefreshMetrics(pSplitter);
}

/////////////////////////////////////////////////////////////////////////////
// CXTPSplitterWndThemeOffice2003

CXTPSplitterWndThemeOffice2003::CXTPSplitterWndThemeOffice2003()
{
	m_nTheme = xtpControlThemeOffice2003;
}

void CXTPSplitterWndThemeOffice2003::RefreshMetrics(CXTPSplitterWnd* pSplitter)
{
	CXTPSplitterWndThemeOfficeXP::RefreshMetrics(pSplitter);

	if (!XTPColorManager()->IsLunaColorsDisabled())
	{
		XTPCurrentSystemTheme systemTheme = XTPColorManager()->GetCurrentSystemTheme();

		switch (systemTheme)
		{
		case xtpSystemThemeBlue:
		case xtpSystemThemeRoyale:
		case xtpSystemThemeAero:
			m_clrSplitterFace.SetStandardValue(RGB(216, 231, 252));
			m_clrSplitterBorders.SetStandardValue(RGB(158, 190, 245));
			break;

		case xtpSystemThemeOlive:
			m_clrSplitterFace.SetStandardValue(RGB(226, 231, 191));
			m_clrSplitterBorders.SetStandardValue(RGB(171, 192, 138));
			break;

		case xtpSystemThemeSilver:
			m_clrSplitterFace.SetStandardValue(RGB(223, 223, 234));
			m_clrSplitterBorders.SetStandardValue(RGB(177, 176, 195));
			break;
		}

	}
}


/////////////////////////////////////////////////////////////////////////////
// CXTPSplitterWndThemeResource

CXTPSplitterWndThemeResource::CXTPSplitterWndThemeResource()
{
	m_nTheme = xtpControlThemeResource;
}

void CXTPSplitterWndThemeResource::RefreshMetrics(CXTPSplitterWnd* pSplitter)
{
	CXTPSplitterWndThemeOffice2003::RefreshMetrics(pSplitter);

	CXTPResourceImages* pImages = XTPResourceImages();

	m_clrSplitterFace.SetStandardValue(pImages->GetImageColor(_T("Window"), _T("ButtonFace")));
	m_clrSplitterBorders.SetStandardValue(pImages->GetImageColor(_T("Window"), _T("WindowFrame")));
}
