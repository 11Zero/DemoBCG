// XTPReportGroupRow.cpp : implementation of the CXTPReportGroupRow class.
//
// This file is a part of the XTREME REPORTCONTROL MFC class library.
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
#include "Resource.h"

#include "Common/XTPDrawHelpers.h"
#include "Common/XTPToolTipContext.h"
#include "Common/XTPMarkupRender.h"
#include "Common/XTPResourceManager.h"


#include "XTPReportControl.h"
#include "XTPReportColumns.h"
#include "XTPReportPaintManager.h"
#include "XTPReportRecordItemText.h"
#include "XTPReportInplaceControls.h"
#include "XTPReportGroupRow.h"

#include "XTPReportRecord.h"
#include "XTPReportRecords.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////////
// CXTPReportGroupItem


CXTPReportGroupRow::CXTPReportGroupRow()
{
	m_bExpanded = TRUE;
	m_pMarkupUIElement = NULL;

	m_strFormula.Empty();
	m_strFormatString.Empty();

}

CXTPReportGroupRow::~CXTPReportGroupRow()
{
	XTPMarkupReleaseElement(m_pMarkupUIElement);
}

void CXTPReportGroupRow::SetCaption(LPCTSTR lpszCaption)
{
	CString strGroupCaption(lpszCaption);

	if (strGroupCaption ==  _T("x"))
		strGroupCaption.Empty();

	if (m_strFormula.IsEmpty())
	{
		if (m_strGroupText == strGroupCaption)
			return;

		m_strGroupText = strGroupCaption;
		if (m_strGroupLabel.IsEmpty())
			m_strGroupLabel = m_strGroupText;

		XTPMarkupReleaseElement(m_pMarkupUIElement);

		if (m_pControl && (m_pControl->GetMarkupContext()))
			m_pMarkupUIElement = XTPMarkupParseText(m_pControl->GetMarkupContext(), m_strGroupText);
	}
	else
	{
		CString strCaption(strGroupCaption);
		double d(0.0);
		int m(0), M(0), n(0), N(0);
		CString s, t, u, v;
		CString sFormula(m_strFormula);
		sFormula.Replace(_T("(C"), _T("(R*C"));
		sFormula.Replace(_T(":C"), _T("(:R*C"));
		int pos = sFormula.Find(_T("SUMSUB("));
		int nextPos(-1);
//Simplified format for group row formulas - SUMSUB(C#:C#) as R# ignored and dynamically recreated
//For prev ver formulas - clean R# parts
//For multiple formulas case can be SUMSUB(R#Ci1:R#Cl1) SUMSUB(R#Ci2:R#Cl2) ... SUMSUB(R#Cim:R#Clm)
//ROW range ignored as used dynamic based on group row childs
//Space-separated string split between spaces and
//Instead "if (pos > -1)" can use loop "while (pos > -1)"
		//if (pos > -1)
		while (pos > -1)
		{
			CString sNextFormula, strCapt, strCol;
			if (sFormula.GetLength() > pos + 7)
				nextPos = sFormula.Find(_T("SUMSUB("), pos + 7);
			if (nextPos > -1)
				sNextFormula = sFormula.Mid(nextPos);

			if (GetChilds(FALSE) != NULL)
			{
				int mMax = m_pControl->GetRows()->GetCount();
				int nMax = m_pControl->GetColumns()->GetCount();
//CODE FOR RECALC - pattern SUMSUB(R#C#:R#C#) or * instead of # - means all row or column
//ROW range ignored as used dynamic based on group row childs
				sFormula.Replace(_T(" "), _T(""));
				if (sFormula.GetLength() > pos + 7)
					sFormula = sFormula.Mid(pos + 7);
				pos = sFormula.Find(_T(":"));
				if (pos > -1)
				{
					s = sFormula.Left(pos);
					t = sFormula.Mid(pos + 1);
					pos = s.Find(_T("C"));
					if (pos > -1)
					{
						u = s.Left(pos);
						s = s.Mid(pos + 1);
						u.Replace(_T("R"), _T(""));
						if (u == _T("*"))
							m = 0;
						else
							m = _ttoi(u);
						if (s == _T("*"))
							n = 0;
						else
							n = _ttoi(s);

						pos = t.Find(_T("C"));
						if (pos > -1)
						{
							u = t.Left(pos);
							t = t.Mid(pos + 1);
							u.Replace(_T("R"), _T(""));
							if (u == _T("*"))
								M = mMax;
							else
								M = _ttoi(u);
							t.Replace(_T(")"), _T(""));
							if (t == _T("*"))
								N = nMax;
							else
								N = _ttoi(t);

							if (n == N - 1)
								d = 0;
							CalculateByChilds(this, n, N, d);

							CString sFmt = GetFormatString();
							if (sFmt.IsEmpty() || sFmt == _T("%s"))
								sFmt = _T("%f");
							else if (sFmt == _T("%d"))
								sFmt = _T("%.0f");

							strCapt.Format(sFmt, d);
							strCol.Format(_T(" [%d]"), n);
							strCapt = strCol + strCapt;
						}
					}
				}
			}
			if (nextPos > -1)
			{
				sFormula = sNextFormula;
				pos = sFormula.Find(_T("SUMSUB("));
				sNextFormula.Empty();
			}
			strCaption += strCapt;
		}
		if (m_strGroupText == strCaption)
			return;

		m_strGroupText = m_strGroupLabel + strCaption;

		XTPMarkupReleaseElement(m_pMarkupUIElement);

		if (m_pControl && (m_pControl->GetMarkupContext()))
			m_pMarkupUIElement = XTPMarkupParseText(m_pControl->GetMarkupContext(), m_strGroupText);
	}
}

BOOL CXTPReportGroupRow::CalculateByChilds(CXTPReportRow* pPassedRow, int col_start, int col_end, double& dPassedValue)
{
	BOOL bRet = FALSE;

	CXTPReportRow* pRow = NULL;
	CXTPReportRecord* pRec = NULL;
	CXTPReportRecordItem* pItem = NULL;

	for (int row = 0; row < pPassedRow->GetChilds(FALSE)->GetCount(); row++)
	{
		pRow = pPassedRow->GetChilds(FALSE)->GetAt(row);
		if (pRow && !pRow->IsGroupRow())
		{
			pRec = pRow->GetRecord();
			if (pRec)
			{
				bRet = TRUE;
				for (int col = col_start; col < col_end; col++)
				{
					if (col < pRec->GetItemCount())
					{
						pItem = pRec->GetItem(col);
						if (pItem)
						{
							CString s = pItem->GetCaption(NULL);
							dPassedValue += pItem->StringToDouble(s);
						}
					}
				}
			}
		}
		else if (pRow && pRow->IsGroupRow())
		{
			bRet = CalculateByChilds(pRow, col_start, col_end, dPassedValue);

			//CXTPReportRow* pChRow = NULL;
			//for (int row1 = 0; row1 < pRow->GetChilds(FALSE)->GetCount(); row1++)
			//{
			//  pChRow = pRow->GetChilds(FALSE)->GetAt(row1);
			//  if (pChRow && !pChRow->IsGroupRow())
			//  {
			//      pRec = pChRow->GetRecord();
			//      if (pRec)
			//      {
			//          for (int col = col_start; col < col_end; col++)
			//          {
			//              if (col < pRec->GetItemCount())
			//              {
			//                  pItem = pRec->GetItem(col);
			//                  if (pItem)
			//                  {
			//                      CString s = pItem->GetCaption(NULL);
			//                      dPassedValue += atof(XTP_CT2CA(s));
			//                  }
			//              }
			//          }
			//      }
			//  }
			//}
		}
	}
	return bRet;
}

CString CXTPReportGroupRow::GetCaption()
{
	return m_strGroupText;
}

void CXTPReportGroupRow::Draw(CDC* pDC, CRect rcRow, int nLeftOffset)
{
	CXTPReportPaintManager* pPaintManager = m_pControl->GetPaintManager();

	pDC->SetBkMode(TRANSPARENT);

	m_rcRow = rcRow;
	if (GetControl()->m_nFreezeColumnsCount == 0)
	{
		m_rcRow.left -= nLeftOffset;
		m_rcRow.right -= nLeftOffset;
	}

	XTP_REPORTRECORDITEM_DRAWARGS drawArgs;
	drawArgs.pDC = pDC;
	drawArgs.nTextAlign = DT_LEFT;

	drawArgs.pControl = m_pControl;
	drawArgs.pRow = this;
	drawArgs.pColumn = NULL;
	drawArgs.pItem = NULL;
	drawArgs.rcItem = m_rcRow;

	XTP_REPORTRECORDITEM_METRICS* pDrawMetrics = new XTP_REPORTRECORDITEM_METRICS;
	pDrawMetrics->strText = GetCaption();

	pPaintManager->FillGroupRowMetrics(this, pDrawMetrics, pDC->IsPrinting());

	ASSERT(m_pControl);
	if (m_pControl)
		m_pControl->GetItemMetrics(&drawArgs, pDrawMetrics);

	pPaintManager->DrawGroupRow(pDC, this, m_rcRow, pDrawMetrics);

	pDrawMetrics->InternalRelease();
}

void CXTPReportGroupRow::OnClick(CPoint ptClicked)
{
	// expand/collapse on single click at the collapse bitmap
	if (m_rcCollapse.PtInRect(ptClicked)
		&& m_rcCollapse.PtInRect(m_pControl->m_ptMouseDown))
	{
		BOOL bExp = IsExpanded();

		SetSelectedMostDeepChilds();
		SetExpanded(!bExp);
		UpdateSelectedMostDeepChilds();

		BOOL bControlKey = (GetKeyState(VK_CONTROL) < 0) || m_pControl->m_bControlKeyAlwaysOn;
		BOOL bShiftKey = (GetKeyState(VK_SHIFT) < 0);

		if ((bControlKey || bShiftKey) && bExp && m_pControl->m_bSelectionExcludeGroupRows)
			m_pControl->UnselectGroupRows();
	}
}

void CXTPReportGroupRow::OnDblClick(CPoint ptClicked)
{
	// do not expand if double clicked on the collapse bitmap - different action (one level only)
	if (!m_rcCollapse.PtInRect(ptClicked))
	{
		// otherwise expand on double click
		SetSelectedChilds();

		SetExpanded(!IsExpanded());

		UpdateSelectedChilds();
	}
	// process parent
	CXTPReportRow::OnDblClick(ptClicked);
}

void CXTPReportGroupRow::SetSelectedChilds()
{
	for (int I = 0; I < m_pControl->m_pPlainTree->GetCount(); I++)
	{
		CXTPReportRow* pTRow = m_pControl->m_pPlainTree->GetAt(I);
		if (pTRow)
		{
			if (pTRow->IsGroupRow())
			{
				CXTPReportGroupRow* pTgRow = (CXTPReportGroupRow*) pTRow;
				if (pTgRow->GetChilds(FALSE) && pTgRow->IsExpanded())
				{
					for (int J = 0; J < pTgRow->GetChilds()->GetCount(); J++)
					{
						CXTPReportRow* pChRow = pTgRow->GetChilds(FALSE)->GetAt(J);
						if (pChRow)
						{
							CXTPReportRecord* pChRec = pChRow->GetRecord();
							if (pChRec)
								pChRec->m_bSelectedAsChildFlag = pChRow->IsSelected();
						}
					}
				}
			}
		}
	}
}

void CXTPReportGroupRow::UpdateSelectedChilds()
{
	for (int II = 0; II < m_pControl->m_pPlainTree->GetCount(); II++)
	{
		CXTPReportRow* pTRow = m_pControl->m_pPlainTree->GetAt(II);
		if (pTRow)
		{
			if (pTRow->IsGroupRow())
			{
				CXTPReportGroupRow* pTgRow = (CXTPReportGroupRow*) pTRow;
				if (pTgRow->GetChilds(FALSE) && pTgRow->IsExpanded())
				{
					for (int J = 0; J < pTgRow->GetChilds()->GetCount(); J++)
					{
						CXTPReportRow* pChRow = pTgRow->GetChilds(FALSE)->GetAt(J);
						if (pChRow)
						{
							CXTPReportRecord* pChRec = pChRow->GetRecord();
							if (pChRec)
							{
								CXTPReportRow* ptRow = m_pControl->m_pPlainTree->FindInTree(pChRec);
								if (ptRow && !ptRow->IsGroupRow())
									ptRow->SetSelected(pChRec->m_bSelectedAsChildFlag);
							}
						}
					}
				}
			}
		}
	}
}

void CXTPReportGroupRow::SetSelectedMostDeepChilds()
{
	for (int II = 0; II < m_pControl->GetRows()->GetCount(); II++)
	{
		CXTPReportRow* pRow = m_pControl->GetRows()->GetAt(II);
		if (pRow->GetChilds(FALSE)) continue;
		CXTPReportRecord* pChRec = pRow->GetRecord();
		if (pChRec)
			pChRec->m_bSelectedAsChildFlag = pRow->IsSelected();
	}
}

void CXTPReportGroupRow::UpdateSelectedMostDeepChilds()
{
	for (int II = 0; II < m_pControl->GetRows()->GetCount(); II++)
	{
		CXTPReportRow* pRow = m_pControl->GetRows()->GetAt(II);
		if (pRow->GetChilds(FALSE)) continue;
		CXTPReportRecord* pChRec = pRow->GetRecord();
		if (pChRec)
			pRow->SetSelected(pChRec->m_bSelectedAsChildFlag);
		//if (pChRec && !pRow->IsGroupRow())
		//  pRow->SetSelected(pChRec->m_bSelectedAsChildFlag);
		//else if (pRow->IsGroupRow())
		//  pRow->SetSelected(FALSE);
	}
}

INT_PTR CXTPReportGroupRow::OnToolHitTest(CPoint /*point*/, TOOLINFO* pTI)
{
	INT_PTR nHit = (INT_PTR)this;
	CString strTip = GetTooltip();

	if (strTip.IsEmpty())
		return -1;

//Clean markup
	CString s(strTip), u, v;
	int j = s.Find(_T("<"));
	int k = s.Find(_T(">"));
	while (j > -1 && k > -1)
	{
		u = s.Left(j);
		v = s.Mid(k + 1);
		s = u + v;
		strTip = s;
		j = s.Find(_T("<"));
		k = s.Find(_T(">"));
	}

	CXTPToolTipContext::FillInToolInfo(pTI, m_pControl->m_hWnd, m_rcRow, nHit, strTip);

	return nHit;
}

