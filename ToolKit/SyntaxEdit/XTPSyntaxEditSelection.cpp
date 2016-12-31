// XTPSyntaxEditSelection.cpp: implementation of the CXTPSyntaxEditAutoCompleteWnd class.
//
// This file is a part of the XTREME TOOLKIT PRO MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME SYNTAX EDIT LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"

// common includes
#include "Common/XTPNotifyConnection.h"
#include "Common/XTPSmartPtrInternalT.h"

#include "XTPSyntaxEditSelection.h"

// syntax editor includes
#include "XTPSyntaxEditDefines.h"
#include "XTPSyntaxEditLexPtrs.h"
#include "XTPSyntaxEditLexCfgFileReader.h"

#include "XTPSyntaxEditBufferManager.h"
#include "XTPSyntaxEditCtrl.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CXTPSyntaxEditSelection, CXTPCmdTarget)



/////////////////////////////////////////////////////////////////////////////
//class _XTP_EXT_CLASS CXTPImmWrapper
#ifdef _UNICODE
	#define XTP_PROC_NAME_AW(procName) procName "W"
#else
	#define XTP_PROC_NAME_AW(procName) procName "A"
#endif

#define XTP_IMM_FNCALL(fnName, errRetVal) \
	if (!m_pfn##fnName) \
		return errRetVal; \
	return m_pfn##fnName

CXTPImmWrapper::CXTPImmWrapper()
{
	m_hImmDll = ::LoadLibrary(_T("imm32.dll"));
	if (m_hImmDll)
	{
		m_pfnImmIsIME = (PFN_ImmIsIME)GetProcAddress(m_hImmDll, "ImmIsIME");
		m_pfnImmGetContext = (PFN_ImmGetContext)GetProcAddress(m_hImmDll, "ImmGetContext");
		m_pfnImmReleaseContext = (PFN_ImmReleaseContext)GetProcAddress(m_hImmDll, "ImmReleaseContext");
		m_pfnImmSetCompositionFont = (PFN_ImmSetCompositionFont)GetProcAddress(m_hImmDll, XTP_PROC_NAME_AW("ImmSetCompositionFont"));
		m_pfnImmSetCompositionWindow = (PFN_ImmSetCompositionWindow)GetProcAddress(m_hImmDll, "ImmSetCompositionWindow");
	}
	else
	{
		m_pfnImmIsIME = NULL;
		m_pfnImmGetContext = NULL;
		m_pfnImmReleaseContext = NULL;
		m_pfnImmSetCompositionFont = NULL;
		m_pfnImmSetCompositionWindow = NULL;
	}
}

CXTPImmWrapper::~CXTPImmWrapper()
{
	if (m_hImmDll)
	{
		::FreeLibrary(m_hImmDll);
	}
}

BOOL CXTPImmWrapper::ImmIsIME(HKL hKL)
{
	ASSERT(m_pfnImmIsIME);

	if (!m_pfnImmIsIME)
		return FALSE;

	if (hKL == NULL)
		hKL = ::GetKeyboardLayout(0);

	return m_pfnImmIsIME(hKL);
}

XTP_HIMC CXTPImmWrapper::ImmGetContext(HWND hWnd)
{
	ASSERT(m_pfnImmGetContext);
	XTP_IMM_FNCALL(ImmGetContext, NULL)(hWnd);
}

BOOL CXTPImmWrapper::ImmReleaseContext(HWND hWnd, XTP_HIMC hIMC)
{
	ASSERT(m_pfnImmReleaseContext);
	XTP_IMM_FNCALL(ImmReleaseContext, FALSE)(hWnd, hIMC);
}

BOOL CXTPImmWrapper::ImmSetCompositionWindow(XTP_HIMC hIMC, COMPOSITIONFORM* pCompForm)
{
	ASSERT(m_pfnImmReleaseContext);
	XTP_IMM_FNCALL(ImmSetCompositionWindow, FALSE)(hIMC, pCompForm);
}

BOOL CXTPImmWrapper::ImmSetCompositionFont(XTP_HIMC hIMC, LOGFONT* plfFont)
{
	ASSERT(m_pfnImmReleaseContext);
	XTP_IMM_FNCALL(ImmSetCompositionFont, FALSE)(hIMC, plfFont);
}



// class CXTPSyntaxEditSelection =======================================================
CXTPSyntaxEditSelection::CXTPSyntaxEditSelection()
{
	m_pOwnerCtrl = NULL;

	selStart_disp = selEnd_disp = XTP_EDIT_LINECOL::MakeLineCol(1, 1);
	selStart_str = selEnd_str = XTP_EDIT_LINECOL::MakeLineCol(1, 0);

	bSelectingRunning = FALSE;
	bWordSelectionMode = FALSE;
	bBlockSelectionMode = FALSE;
	nSelStartTextRowFromLeftBar = 0;

}

CXTPSyntaxEditSelection::CXTPSyntaxEditSelection(const CXTPSyntaxEditSelection& rSrc)
{

	m_pOwnerCtrl = NULL;

	*this = rSrc;
}

#define XTP_SE_AX_DETACH(pObject)   if (pObject) { \
										pObject->Detach(); \
										pObject->InternalRelease(); }

CXTPSyntaxEditSelection::~CXTPSyntaxEditSelection()
{
}

void CXTPSyntaxEditSelection::SetStart_str(int nTextRow, int nStrPos)
{
	selStart_str.nLine = nTextRow;
	selStart_str.nCol = nStrPos;

	selStart_disp.nLine = nTextRow;

	if (m_pOwnerCtrl && m_pOwnerCtrl->GetEditBuffer())
	{
		selStart_disp.nCol = m_pOwnerCtrl->GetEditBuffer()->StrPosToCol(nTextRow, nStrPos);
	}
	else
	{
		ASSERT(FALSE);
		selStart_disp.nCol = nStrPos;
	}
}

void CXTPSyntaxEditSelection::SetEnd_str(int nTextRow, int nStrPos)
{
	selEnd_str.nLine = nTextRow;
	selEnd_str.nCol = nStrPos;

	selEnd_disp.nLine = nTextRow;

	if (m_pOwnerCtrl && m_pOwnerCtrl->GetEditBuffer())
	{
		selEnd_disp.nCol = m_pOwnerCtrl->GetEditBuffer()->StrPosToCol(nTextRow, nStrPos);
	}
	else
	{
		ASSERT(FALSE);
		selEnd_disp.nCol = nStrPos;
	}
}

void CXTPSyntaxEditSelection::SetStart_disp(int nTextRow, int nDispCol)
{
	ASSERT(nDispCol > 0);
	nDispCol = max(1, nDispCol);

	selStart_disp.nLine = nTextRow;
	selStart_disp.nCol = nDispCol;

	selStart_str.nLine = nTextRow;

	if (m_pOwnerCtrl && m_pOwnerCtrl->GetEditBuffer())
	{
		selStart_str.nCol = m_pOwnerCtrl->GetEditBuffer()->ColToStrPos(nTextRow, nDispCol);
	}
	else
	{
		ASSERT(FALSE);
		selStart_str.nCol = nDispCol;
	}
}

void CXTPSyntaxEditSelection::SetEnd_disp(int nTextRow, int nDispCol)
{
	ASSERT(nDispCol > 0);
	nDispCol = max(1, nDispCol);

	selEnd_disp.nLine = nTextRow;
	selEnd_disp.nCol = nDispCol;

	selEnd_str.nLine = nTextRow;

	if (m_pOwnerCtrl && m_pOwnerCtrl->GetEditBuffer())
	{
		selEnd_str.nCol = m_pOwnerCtrl->GetEditBuffer()->ColToStrPos(nTextRow, nDispCol);
	}
	else
	{
		ASSERT(FALSE);
		selEnd_str.nCol = nDispCol;
	}
}

BOOL CXTPSyntaxEditSelection::IsSelExist()
{
	XTP_EDIT_LINECOL normStart = GetNormalStart_disp();
	XTP_EDIT_LINECOL normEnd = GetNormalEnd_disp();

	return normStart < normEnd;
}

BOOL CXTPSyntaxEditSelection::_IsInSel(BOOL bStr, int nTextRow, int nColX)
{
	XTP_EDIT_LINECOL normStart = bStr ? GetNormalStart_str() : GetNormalStart_disp();
	XTP_EDIT_LINECOL normEnd = bStr ? GetNormalEnd_str() : GetNormalEnd_disp();

	if (!(normStart < normEnd))
		return FALSE;

	XTP_EDIT_LINECOL pos = XTP_EDIT_LINECOL::MakeLineCol(nTextRow, nColX);

	if (bBlockSelectionMode)
	{
		return pos.nLine >= normStart.nLine && pos.nLine < normEnd.nLine &&
			pos.nCol >= normStart.nCol && pos.nCol < normEnd.nCol;
	}
	return pos >= normStart && pos < normEnd;
}

BOOL CXTPSyntaxEditSelection::_IsIntersectSel(BOOL bStr, int nTextRow, int nCol1, int nCol2)
{
	XTP_EDIT_LINECOL normStart = bStr ? GetNormalStart_str() : GetNormalStart_disp();
	XTP_EDIT_LINECOL normEnd = bStr ? GetNormalEnd_str() : GetNormalEnd_disp();

	if (!(normStart < normEnd))
		return FALSE;

	if (nTextRow != normEnd.nLine)
	{
		// infinit sel by end
		if (m_pOwnerCtrl->_IsVirtualSpaceActive())
		{
			normEnd.nCol = INT_MAX;
		}
		else
		{
			int nTextLenC = m_pOwnerCtrl->GetEditBuffer()->GetLineTextLengthC(nTextRow);
			if (bStr)
				normEnd.nCol = nTextLenC;
			else
				normEnd.nCol = m_pOwnerCtrl->GetEditBuffer()->StrPosToCol(nTextRow, nTextLenC);
		}
	}

	XTP_EDIT_LINECOL pos1 = XTP_EDIT_LINECOL::MakeLineCol(nTextRow, nCol1);
	XTP_EDIT_LINECOL pos2 = XTP_EDIT_LINECOL::MakeLineCol(nTextRow, nCol2);

	BOOL bIntersect = !(pos2 <= normStart || pos1 >= normEnd);

	if (bIntersect && bBlockSelectionMode)
	{
		// just condition by cols only
		bIntersect = !(pos2.nCol <= normStart.nCol || pos1.nCol >= normEnd.nCol);
	}

	return bIntersect;
}

int CXTPSyntaxEditSelection::GetSelStartForRow_str(int nTextRow, int nDispLine)
{
	XTP_EDIT_LINECOL normStart = GetNormalStart_str();
	XTP_EDIT_LINECOL normEnd = GetNormalEnd_str();

	if (nTextRow >= normStart.nLine && nTextRow <= normEnd.nLine)
	{
		if (bBlockSelectionMode)
		{
			int nDispPos = GetNormalStart_disp().nCol;
			ASSERT(m_pOwnerCtrl);
			if (m_pOwnerCtrl)
				return m_pOwnerCtrl->GetDrawTextProcessor().DispPosToStrPos(nDispLine, nDispPos - 1, TRUE);
				//m_pOwnerCtrl->_IsVirtualSpaceActive());

			return normStart.nCol;
		}
		else
		{
			if (nTextRow == normStart.nLine)
				return normStart.nCol;
			else
				return 0;
		}
	}

	return 0;
}

int CXTPSyntaxEditSelection::GetSelEndForRow_str(int nTextRow, int nDispLine, BOOL* pbInfinitSelEnd)
{
	if (pbInfinitSelEnd)
		*pbInfinitSelEnd = FALSE;


	ASSERT(m_pOwnerCtrl);
	if (!m_pOwnerCtrl)
		return 0;

	XTP_EDIT_LINECOL normStart = GetNormalStart_str();
	XTP_EDIT_LINECOL normEnd = GetNormalEnd_str();

	if (nTextRow >= normStart.nLine && nTextRow <= normEnd.nLine)
	{
		if (bBlockSelectionMode)
		{
			int nDispPos = GetNormalEnd_disp().nCol;
			return m_pOwnerCtrl->GetDrawTextProcessor().DispPosToStrPos(nDispLine, nDispPos - 1, TRUE);
				//m_pOwnerCtrl->_IsVirtualSpaceActive());
		}
		else
		{
			if (nTextRow == normEnd.nLine)
			{
				if (pbInfinitSelEnd && nSelStartTextRowFromLeftBar)
					*pbInfinitSelEnd = m_pOwnerCtrl->_IsVirtualSpaceActive();

				return normEnd.nCol;
			}
			else
			{
				int nDispPos = GetNormalEnd_disp().nCol;

				int nPos = m_pOwnerCtrl->GetDrawTextProcessor().DispPosToStrPos(nDispLine, nDispPos - 1,
						m_pOwnerCtrl->_IsVirtualSpaceActive());
				if (pbInfinitSelEnd)
					*pbInfinitSelEnd = m_pOwnerCtrl->_IsVirtualSpaceActive();

				int nRowTextLen = m_pOwnerCtrl->GetEditBuffer()->GetLineTextLengthC(nTextRow);
				nPos = max(nPos, nRowTextLen + 1);

				return nPos;
			}
		}
	}

	return 0;
}

void CXTPSyntaxEditSelection::Reset_str(int nTextRow, int nStrPos)
{
	bSelectingRunning = FALSE;
	bWordSelectionMode = FALSE;
	bBlockSelectionMode = FALSE;
	nSelStartTextRowFromLeftBar = 0;

	SetStart_str(nTextRow, nStrPos);
	SetEnd_str(nTextRow, nStrPos);
}

void CXTPSyntaxEditSelection::Reset_disp(int nTextRow, int nDispCol)
{
	bSelectingRunning = FALSE;
	bWordSelectionMode = FALSE;
	bBlockSelectionMode = FALSE;
	nSelStartTextRowFromLeftBar = 0;

	SetStart_disp(nTextRow, nDispCol);
	SetEnd_disp(nTextRow, nDispCol);
}

const CXTPSyntaxEditSelection& CXTPSyntaxEditSelection::operator=(const CXTPSyntaxEditSelection& rSrc)
{
	if (!m_pOwnerCtrl)
		m_pOwnerCtrl = rSrc.m_pOwnerCtrl;

	bSelectingRunning = rSrc.bSelectingRunning;
	bWordSelectionMode = rSrc.bWordSelectionMode;
	bBlockSelectionMode = rSrc.bBlockSelectionMode;
	nSelStartTextRowFromLeftBar = rSrc.nSelStartTextRowFromLeftBar;

	selStart_disp = rSrc.selStart_disp;
	selEnd_disp = rSrc.selEnd_disp;

	selStart_str = rSrc.selStart_str;
	selEnd_str = rSrc.selEnd_str;

	return *this;
}

BOOL CXTPSyntaxEditSelection::operator==(const CXTPSyntaxEditSelection& rSrc) const
{
	return  bSelectingRunning == rSrc.bSelectingRunning &&
		bWordSelectionMode == rSrc.bWordSelectionMode &&
		bBlockSelectionMode == rSrc.bBlockSelectionMode &&
		nSelStartTextRowFromLeftBar == rSrc.nSelStartTextRowFromLeftBar &&

		selStart_disp == rSrc.selStart_disp &&
		selEnd_disp == rSrc.selEnd_disp &&

		selStart_str == rSrc.selStart_str &&
		selEnd_str == rSrc.selEnd_str;
}

BOOL CXTPSyntaxEditSelection::operator!=(const CXTPSyntaxEditSelection& rSrc) const
{
	return !operator==(rSrc);
}

