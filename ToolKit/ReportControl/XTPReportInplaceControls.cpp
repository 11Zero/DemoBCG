// XTPReportInplaceControls.cpp
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
#include "Common/XTPDrawHelpers.h"
#include "Common/XTPSystemHelpers.h"


#include "XTPReportControl.h"
#include "XTPReportDefines.h"
#include "XTPReportInplaceControls.h"
#include "XTPReportColumn.h"
#include "XTPReportRecordItemText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CXTPReportInplaceControl::CXTPReportInplaceControl()
{
}

CXTPReportInplaceControl::~CXTPReportInplaceControl()
{
	SetItemArgs(0);
}

void CXTPReportInplaceControl::SetItemArgs(XTP_REPORTRECORDITEM_ARGS* pItemArgs)
{
	if (pItemArgs)
	{
		pItemArgs->AddRef();
		Release();

		pItem = pItemArgs->pItem;
		pControl = pItemArgs->pControl;
		pRow = pItemArgs->pRow;
		pColumn = pItemArgs->pColumn;
		rcItem = pItemArgs->rcItem;
	}
	else
	{
		Release();

		pItem = NULL;
		pControl = NULL;
		pRow = NULL;
		pColumn = NULL;
	}
}

IMPLEMENT_DYNAMIC(CXTPReportInplaceEdit, CEdit)

CXTPReportInplaceEdit::CXTPReportInplaceEdit()
{
	m_pSelectedConstraint = NULL;
	m_clrText = 0;
	m_bSetWindowText = FALSE;
}

CXTPReportInplaceEdit::~CXTPReportInplaceEdit()
{
}


BEGIN_MESSAGE_MAP(CXTPReportInplaceEdit, CEdit)
	//{{AFX_MSG_MAP(CXTPReportInplaceEdit)
	ON_WM_MOUSEACTIVATE()
	ON_WM_CTLCOLOR_REFLECT()
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnEnKillfocus)
	ON_CONTROL_REFLECT(EN_CHANGE, OnEnChange)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CXTPReportInplaceEdit::HideWindow()
{
	if (m_hWnd)
	{
		ShowWindow(SW_HIDE);
		SetItemArgs(0);
	}
}

void CXTPReportInplaceEdit::Create(XTP_REPORTRECORDITEM_ARGS* pItemArgs)
{
	SetItemArgs(pItemArgs);
	m_pSelectedConstraint = NULL;

	XTP_REPORTRECORDITEM_METRICS* pMetrics = new XTP_REPORTRECORDITEM_METRICS;

	CXTPReportRecordItemText* pTextItem = DYNAMIC_DOWNCAST(CXTPReportRecordItemText, pItem);

	pItemArgs->pRow->FillMetrics(pColumn, pItem, pMetrics);

	if (pTextItem != NULL)
	{
		pMetrics->strText = pTextItem->GetValue();
	}
	else //not CXTPReportRecordItemText case!
	{
		pMetrics->strText = pItem->GetCaption(pColumn);
	}

	CXTPReportRecordItemEditOptions* pEditOptions = pItem->GetEditOptions(pColumn);
	ASSERT(pEditOptions);
	if (!pEditOptions)
		return;
	CRect rect = pItemArgs->rcItem;
	rect.DeflateRect(2, 1, 2, 2);

	m_clrText = pMetrics->clrForeground;
	m_strValue = pMetrics->strText;
	m_strText_prev = pMetrics->strText;

	DWORD dwEditStyle = WS_CHILD | pEditOptions->m_dwEditStyle;

	dwEditStyle &= ~(ES_LEFT | ES_RIGHT | ES_CENTER);

	if (pControl->GetPaintManager()->m_bUseEditTextAlignment)
	{
		if (pMetrics->nColumnAlignment & DT_RIGHT)
			dwEditStyle |= ES_RIGHT;
		else if (pMetrics->nColumnAlignment & DT_CENTER)
			dwEditStyle |= ES_CENTER;
	}

	if (m_hWnd)
	{
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | ES_READONLY;

		if ((GetStyle() & dwStyle) != (dwEditStyle & dwStyle))
			DestroyWindow();
	}

	if (!m_hWnd)
		CEdit::Create(dwEditStyle, rect, pControl, 0);

	if (pControl->GetExStyle() & WS_EX_RTLREADING)
		ModifyStyleEx(0, WS_EX_RTLREADING);


	SetLimitText(pEditOptions->m_nMaxLength);

	//SetFocus();
	SetFont(pMetrics->pFont);
	SetWindowText(m_strValue);

	pMetrics->InternalRelease();

	if (rect.right > rect.left)
	{
		SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
		SetFocus();
	}
	else
	{
		HideWindow();
	}

	SetMargins(0, 0);
}

void CXTPReportInplaceEdit::SetWindowText(LPCTSTR lpszString)
{
	m_bSetWindowText = TRUE;

	CWnd::SetWindowText(lpszString);
	m_strText_prev = lpszString;

	m_bSetWindowText = FALSE;
}

void CXTPReportInplaceEdit::SetFont(CFont* pFont)
{
	m_fntEdit.DeleteObject();
	LOGFONT lf;
	pFont->GetLogFont(&lf);
	m_fntEdit.CreateFontIndirect(&lf);

	CEdit::SetFont(&m_fntEdit);
}

void CXTPReportInplaceEdit::OnEnKillfocus()
{
	if (pControl && pItem)
	{
		pItem->OnValidateEdit((XTP_REPORTRECORDITEM_ARGS*)this);
		//pItem->OnCancelEdit(pControl, TRUE);
		HideWindow();
	}
}

void CXTPReportInplaceEdit::OnEnChange()
{
	if (m_bSetWindowText || !pControl || !pItem)
		return;

	CString strValue, strValNew;
	GetWindowText(strValue);
	strValNew = strValue;

//recover previous selection by diff between old and new text - compare from the start and from the end
	int kO = strValue.GetLength();
	int kP = m_strText_prev.GetLength();
	int K = min(kO, kP);
	int kOld(0);
	int kOldEnd(kP);
	if (m_strText_prev != strValue)
	{
		for (int j = 0; j < K; j++)
		{
			if (m_strText_prev[j] != strValue[j])
			{
				kOld = j;
				break;
			}
		}
		for (int jB = 1; jB <= K; jB++)
		{
			if (m_strText_prev[kP - jB] != strValue[kO - jB])
			{
				kOldEnd = kP + 1 - jB;
				break;
			}
		}
	}
	BOOL bCommit = pItem->OnEditChanging((XTP_REPORTRECORDITEM_ARGS*)this, strValNew);

	if (!bCommit || strValNew != strValue)
	{
		int nSelStart = 0, nSelEnd = 0;
		GetSel(nSelStart, nSelEnd);

		int kN = strValNew.GetLength();
		int kk = min(kO, kN);
		if (strValNew != strValue) //it means that bCommit is TRUE
		{
			SetWindowText(strValNew);
			int k(kk - 1);
			for (int j = 0; j < kk; j++)
			{
				if (strValNew[j] != strValue[j])
				{
					k = j;
					break;
				}
			}
			SetSel(k, k);
		}
		else //it means that bCommit is FALSE - need rollback
		{
			SetWindowText(m_strText_prev);

			SetSel(kOld, kOldEnd);
			//Cursor position before 1st OnEditChanging - handler - modified symbol
			if (nSelStart == kO && nSelEnd == kO && kP < kO)
				SetSel(nSelStart, nSelEnd);

			if (nSelStart == 1 && nSelEnd == 1 && kP < kO)
				SetSel(0, 0);
		}
	}
	else
	{
		m_strText_prev = strValue;
		//Normal (default) cursor positioning
	}
}

int CXTPReportInplaceEdit::OnMouseActivate(CWnd* , UINT , UINT ) //(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return MA_NOACTIVATE;
}

UINT CXTPReportInplaceEdit::OnGetDlgCode()
{
	return DLGC_WANTALLKEYS;
}

void CXTPReportInplaceEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!pControl)
		return;

	if (nChar == VK_TAB) return;
	if (nChar == VK_ESCAPE)
	{
		pControl->EditItem(NULL);
		return;
	}
	if (nChar == VK_RETURN)
	{
		if ((GetStyle() & ES_WANTRETURN) == 0)
		{
			pControl->EditItem(NULL);
		}
		else
		{
			CEdit::OnChar(nChar, nRepCnt, nFlags);
		}
		return;
	}
	if (pItem && pColumn && pItem->GetEditOptions(pColumn)->m_bConstraintEdit)
	{
		CXTPReportRecordItemEditOptions* pEditOptions = pItem->GetEditOptions(pColumn);

		CXTPReportRecordItemConstraints* pConstraints = pEditOptions->GetConstraints();
		int nCount = pConstraints->GetCount();
		if (nCount > 0)
		{

			CString str, strActual;
			GetWindowText(str);
			strActual = str;

			CXTPReportRecordItemConstraint* pConstraint = (m_pSelectedConstraint == NULL) ?
				pEditOptions->FindConstraint(str): m_pSelectedConstraint;

			int nIndexStart, nIndex;
			nIndexStart = nIndex = (pConstraint == NULL ? nCount - 1 : pConstraint->GetIndex());

			CString strSeach ((TCHAR)nChar);

			do
			{
				nIndex = nIndex < nCount - 1 ? nIndex + 1 : 0;

				pConstraint = pConstraints->GetAt(nIndex);
				str = pConstraint->m_strConstraint;

				if (strSeach.CompareNoCase(str.Left(1)) == 0)
				{
					m_pSelectedConstraint = pConstraint;
					SetWindowText(str);
					SetSel(0, -1);

					if (strActual.CompareNoCase(str) != 0)
						((CXTPReportControl*)pControl)->OnConstraintSelecting(pRow, pItem, pColumn, pConstraint);

					return;
				}

			} while (nIndex != nIndexStart);

			return;
		}
	}
	if (nChar == 1 && nFlags == 30) //Ctrl+A case
	{
		SetSel(0, -1);
	}
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CXTPReportInplaceEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CXTPReportControl* _pControl = pControl;
	if (!_pControl)
		return;

	if (nChar == VK_TAB)
	{
		_pControl->SetFocus();
		_pControl->SendMessage(WM_CHAR, nChar);
		return;
	}
	if (nChar == VK_ESCAPE)
	{
		SetWindowText(m_strValue);
		return;
	}
	else if (nChar == VK_F5)
	{
		_pControl->Recalc(TRUE);
		return;
	}
	else if (nChar == VK_RETURN)
	{
		if ((GetStyle() & ES_WANTRETURN) == 0)
			return;

		if (GetKeyState(VK_CONTROL) < 0) // ES_WANTRETURN is set and Ctrl+Enter is pressed
		{
			pControl->EditItem(NULL);
			return;
		}
	}
	else if (nChar == VK_UP || nChar == VK_DOWN || nChar == VK_PRIOR || nChar == VK_NEXT)
	{
		if (pItem && pColumn && pItem->GetEditOptions(pColumn)->m_bConstraintEdit)
		{
			CXTPReportRecordItemConstraint* pConstraint;
			CXTPReportRecordItemEditOptions* pEditOptions = pItem->GetEditOptions(pColumn);
			CXTPReportRecordItemConstraints* pConstraints = pEditOptions->GetConstraints();

			int nCount = pConstraints->GetCount();
			if (nCount > 1)
			{
				CString strActual, str;
				GetWindowText(strActual);

				int nIndex = 0;          // the first item

				if (nChar == VK_NEXT)
				{
					nIndex = nCount - 1; // the last item
				}
				else if (nChar != VK_PRIOR)
				{
					// look for the actually selected item
					for (int i = 0; i < nCount; i++)
					{
						pConstraint = pConstraints->GetAt(i);

						if (strActual.CompareNoCase(pConstraint->m_strConstraint) == 0)
						{
							if (nChar == VK_UP)
								nIndex = max(0, i - 1);
							else if (nChar == VK_DOWN)
								nIndex = min(nCount-1, i + 1);

							break;
						}
					}
				}

				pConstraint = pConstraints->GetAt(max(0, min(nIndex, nCount-1)));
				str = pConstraint->m_strConstraint;
				m_pSelectedConstraint = pConstraint;

				// set the default font, because user could change the font (for ex. to striked one)
				SetFont(pControl->GetPaintManager()->GetTextFont());
				SetWindowText(str);
				SetSel(0, -1);

				if (strActual.CompareNoCase(str) != 0)
					((CXTPReportControl*)pControl)->OnConstraintSelecting(pRow, pItem, pColumn, pConstraint);

				return;
			}
		}
	}

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CXTPReportInplaceEdit::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nChar == VK_UP || nChar == VK_DOWN) && pControl)
	{
		if (pControl->GetInplaceButtons()->GetSize() > 0)
		{
			CXTPReportInplaceButton* pButton = pControl->GetInplaceButtons()->GetAt(0);

			if (pButton->GetItem() == pItem)
			{
				pItem->OnInplaceButtonDown(pButton);
			}
		}
	}

	CEdit::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CXTPReportInplaceEdit::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pControl)
	{
		if (!pControl->OnPreviewKeyDown((UINT&)pMsg->wParam, LOWORD(pMsg->lParam), HIWORD(pMsg->lParam)) )
			return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN && IsDialogMessage(pMsg))
		return TRUE;

	return CEdit::PreTranslateMessage(pMsg);
}

void CXTPReportInplaceEdit::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (pRow && pItem)
	{
		MapWindowPoints(pControl, &point, 1);
		pRow->OnDblClick(point);
	}

	if (pItem)
	{
		CEdit::OnLButtonDblClk(nFlags, point);
	}
}

HBRUSH CXTPReportInplaceEdit::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetTextColor(m_clrText);

	return GetSysColorBrush(COLOR_WINDOW);
}

//////////////////////////////////////////////////////////////////////////
// CXTPReportInplaceButton

CXTPReportInplaceButton::CXTPReportInplaceButton(UINT nID)
{
	m_nID = nID;
	m_nWidth = 17;
	m_nFixedHeight = 19;
	m_bInsideCell = FALSE;
	m_nIconIndex = XTP_REPORT_NOICON;

	m_bPressed = m_bOver = FALSE;
	m_nState = 0;
	m_nSpinIncrement = 0;
	m_unSpinTimerCnt = 0;
	m_unSpinTimerId = 0;

	m_nSpinMin = INT_MIN;
	m_nSpinMax = INT_MAX;
	m_nSpinStep = 1;

	m_Items2Show = 10;
	m_bDraw = TRUE;

}

void CXTPReportInplaceButton::Create(XTP_REPORTRECORDITEM_ARGS* pItemArgs, CRect& rcButtons)
{
	m_bPressed = m_bOver = FALSE;
	m_nState = 0;
	m_nSpinIncrement = 0;
	m_unSpinTimerCnt = 0;
	m_unSpinTimerId = 0;
	SetItemArgs(pItemArgs);

	CRect rect(rcButtons);
	if (pControl->GetPaintManager()->IsFixedInplaceButtonHeight())
		rect.bottom = min(rect.bottom, rect.top + m_nFixedHeight);

	if (m_bInsideCell)
	{
		rect.right = rcButtons.left;
		rect.left = rect.right - m_nWidth;

		rcButtons.left = rect.left;
	}
	else
	{
		rect.left = rcButtons.right;
		rect.right = rect.left + m_nWidth;

		rcButtons.right = rect.right;
	}

	//to keep focused frame no touched
	rect.top++;
	rect.bottom -= 2;
	rect.left--;
	if (m_nID == XTP_ID_REPORT_COMBOBUTTON)
		rect.right--;

	if (!m_hWnd)
		CStatic::Create(NULL, SS_NOTIFY | WS_CHILD, rect, pItemArgs->pControl);

	SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
}

BEGIN_MESSAGE_MAP(CXTPReportInplaceButton, CStatic)
	//{{AFX_MSG_MAP(CXTPReportInplaceButton)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CXTPReportInplaceButton::OnPaint()
{
	CPaintDC dc(this);

	if (pControl)
	{
		if (m_bDraw)
			pControl->GetPaintManager()->DrawInplaceButton(&dc, this);
		m_bDraw = !m_bDraw; //to eliminate flickering
	}
}

void CXTPReportInplaceButton::OnFinalRelease()
{
	if (m_hWnd != NULL)
		DestroyWindow();

	CCmdTarget::OnFinalRelease();
}

void CXTPReportInplaceButton::OnLButtonDown(UINT, CPoint point)
{
	switch (m_nID)
	{
		case XTP_ID_REPORT_COMBOBUTTON :
			m_bOver = m_bPressed = TRUE;
			break;
		case XTP_ID_REPORT_EXPANDBUTTON :
			m_bOver = m_bPressed = TRUE;
			break;
		case XTP_ID_REPORT_SPINBUTTON :
		{
			CXTPClientRect rect(this);
			m_bOver = TRUE;
			rect.bottom -= rect.Height() / 2;
			m_nState = rect.PtInRect(point) ? SPNP_UP : SPNP_DOWN;

			m_nSpinIncrement = m_nState == SPNP_UP ? m_nSpinStep : -m_nSpinStep;

			pItem->OnInplaceButtonDown(this);

			// start timer
			m_unSpinTimerCnt = 0;
			m_unSpinTimerId = SetTimer(1, 500, NULL);
			break;
		}
	}
	Invalidate(FALSE);
	SetCapture();
}

void CXTPReportInplaceButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ((m_bPressed || m_nState) && pItem)
	{
		if (m_unSpinTimerId)
		{
			KillTimer(1);
			m_unSpinTimerId = 0;
		}
		m_nSpinIncrement = 0;
		m_unSpinTimerCnt = 0;
		m_bPressed = FALSE;
		m_nState = 0;
		Invalidate(FALSE);
		ReleaseCapture();
		if (m_bOver && m_nID != XTP_ID_REPORT_SPINBUTTON)
		{
			pItem->OnInplaceButtonDown(this);
		}
	}
	CStatic::OnLButtonUp(nFlags, point);
}

void CXTPReportInplaceButton::Activate()
{
	m_bOver = m_bPressed = TRUE;
	Invalidate(FALSE);
	SetCapture();
}

void CXTPReportInplaceButton::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	UNREFERENCED_PARAMETER(nFlags);

	if (m_nID == XTP_ID_REPORT_SPINBUTTON)
	{
		CXTPClientRect rect(this);
		if (rect.PtInRect(point))
		{
			rect.bottom -= rect.Height() / 2;
			m_nState = rect.PtInRect(point) ? SPNP_UP : SPNP_DOWN;
			m_nSpinIncrement = m_nState == SPNP_UP ? m_nSpinStep : -m_nSpinStep;
			pItem->OnInplaceButtonDown(this);
			// start timer
			m_unSpinTimerCnt = 0;
			m_unSpinTimerId = SetTimer(1, 500, NULL);
		}
		Invalidate(FALSE);
		SetCapture();
	}
}

void CXTPReportInplaceButton::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bPressed || m_nState)
	{
		CXTPClientRect rect(this);
		BOOL bOver;
		if (m_nID != XTP_ID_REPORT_SPINBUTTON)
			bOver = rect.PtInRect(point);
		else
		{
			bOver = rect.PtInRect(point) &&
				(point.y < (rect.bottom - rect.Height() / 2) && m_nState == SPNP_UP ||
				point.y >= (rect.bottom - rect.Height() / 2) && m_nState == SPNP_DOWN);
		}
		if ((bOver && !m_bOver) || (!bOver && m_bOver))
		{
			m_bOver = bOver;
			if (m_nID == XTP_ID_REPORT_SPINBUTTON)
			{
				if (m_bOver)
				{
					m_nSpinIncrement = m_nState == SPNP_UP ? 1 : -1;
					m_unSpinTimerCnt = 0;
					m_unSpinTimerId = SetTimer(1, 500, NULL); // start timer
				}
				else if (m_unSpinTimerId)
				{
					// stop timer
					KillTimer(1);
					m_nSpinIncrement = 0;
					m_unSpinTimerCnt = 0;
					m_unSpinTimerId = 0;
				}
			}
			Invalidate(FALSE);
		}
	}

	CStatic::OnMouseMove(nFlags, point);
}

void CXTPReportInplaceButton::OnCaptureChanged(CWnd* pWnd)
{
	m_bPressed = FALSE;
	m_nState = 0;
	Invalidate(FALSE);

	CStatic::OnCaptureChanged(pWnd);
}

void CXTPReportInplaceButton::OnTimer(UINT_PTR nIDEvent)
{
	if (m_unSpinTimerCnt == 0 && abs(m_nSpinIncrement) < 10)
	{
		// first timer event, reset timer
		KillTimer(1);
		m_unSpinTimerId = SetTimer(1, 100, NULL);
	}
	m_unSpinTimerCnt++;
	if (m_unSpinTimerCnt >= 20 && abs(m_nSpinIncrement < 100000))
	{
		m_nSpinIncrement *= 10;
		m_unSpinTimerCnt = 0;
	}
	pItem->OnInplaceButtonDown(this);

	CStatic::OnTimer(nIDEvent);
}

//////////////////////////////////////////////////////////////////////////
// CXTPReportInplaceList

CXTPReportInplaceList::CXTPReportInplaceList()
{
	m_bApply = FALSE;
	m_dwLastKeyDownTime = 0;
	m_Items2Show = 10;
}


void CXTPReportInplaceList::Create(XTP_REPORTRECORDITEM_ARGS* pItemArgs, CXTPReportRecordItemConstraints* pConstaints)
{
	SetItemArgs(pItemArgs);

	CRect rect(pItemArgs->rcItem);

	if (!m_hWnd)
	{
		CListBox::CreateEx(WS_EX_TOOLWINDOW | (pControl->GetExStyle() & WS_EX_LAYOUTRTL), _T("LISTBOX"), _T(""), LBS_NOTIFY | WS_CHILD | WS_BORDER | WS_VSCROLL, CRect(0, 0, 0, 0), pControl, 0);
		SetOwner(pControl);
	}

	SetFont(pControl->GetPaintManager()->GetTextFont());
	ResetContent();

	int dx = rect.right - rect.left + 1;

	CWindowDC dc(pControl);
	CXTPFontDC font(&dc, GetFont());
	int nThumbLength = GetSystemMetrics(SM_CXHTHUMB);

	CString strCaption = pItem->GetCaption(pColumn);
	DWORD dwData = pItem->GetSelectedConstraintData(pItemArgs);

	for (int i = 0; i < pConstaints->GetCount(); i++)
	{
		CXTPReportRecordItemConstraint* pConstaint = pConstaints->GetAt(i);
		CString str = pConstaint->m_strConstraint;
		int nIndex = AddString(str);
		SetItemDataPtr(nIndex, pConstaint);

		dx = max(dx, dc.GetTextExtent(str).cx + nThumbLength);

		if ((dwData == (DWORD)-1 && strCaption == str) || (dwData == pConstaint->m_dwData))
			SetCurSel(nIndex);
	}

	int nHeight = GetItemHeight(0);
	rect.top = rect.bottom;
	//rect.bottom += nHeight * min(10, GetCount()) + 2;
	rect.bottom += nHeight * min(m_Items2Show, GetCount()) + 2;
	rect.left = rect.right - dx;

	pControl->ClientToScreen(&rect);

	CRect rcWork = XTPMultiMonitor()->GetWorkArea(rect);
	if (rect.bottom > rcWork.bottom && rect.top > rcWork.CenterPoint().y)
		rect.OffsetRect(0, - rect.Height() - pItemArgs->rcItem.Height());

	if (rect.left < rcWork.left) rect.OffsetRect(rcWork.left - rect.left, 0);
	if (rect.right > rcWork.right) rect.OffsetRect(rcWork.right - rect.right, 0);

	SetFocus();

	if (!m_hWnd) // Can be destroyed after focus set
		return;

	SetWindowLongPtr(m_hWnd, GWLP_HWNDPARENT, 0);
	ModifyStyle(WS_CHILD, WS_POPUP);
	SetWindowLongPtr(m_hWnd, GWLP_HWNDPARENT, (LONG_PTR)pControl->m_hWnd);

	SetWindowPos(&CWnd::wndTopMost, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

	CXTPMouseMonitor::SetupHook(this);
}

BEGIN_MESSAGE_MAP(CXTPReportInplaceList, CListBox)
	//{{AFX_MSG_MAP(CXTPReportInplaceList)
	ON_WM_MOUSEACTIVATE()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONUP()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CXTPReportInplaceList::OnMouseActivate(CWnd* /*pDesktopWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	return MA_NOACTIVATE;
}

UINT CXTPReportInplaceList::OnGetDlgCode()
{
	return DLGC_WANTALLKEYS;
}

void CXTPReportInplaceList::SetItemArgs(XTP_REPORTRECORDITEM_ARGS* pItemArgs)
{
	m_bApply = FALSE;
	CXTPReportInplaceControl::SetItemArgs(pItemArgs);

	m_dwLastKeyDownTime = 0;
	m_strHotSearchContext.Empty();
}

void CXTPReportInplaceList::PostNcDestroy()
{
	CXTPMouseMonitor::SetupHook(NULL);
	SetItemArgs(NULL);

	CListBox::PostNcDestroy();
}


void CXTPReportInplaceList::OnKillFocus(CWnd* pNewWnd)
{
	//ASSERT(pItem || m_bApply);

	if (pItem && !m_bApply)
		pItem->OnEditCanceled(this);

	CListBox::OnKillFocus(pNewWnd);
	DestroyWindow();
}

void CXTPReportInplaceList::OnLButtonUp(UINT, CPoint point)
{
	CXTPClientRect rc(this);

	if (rc.PtInRect(point))
		Apply();
	else
		Cancel();
}


void CXTPReportInplaceList::Cancel()
{
	m_bApply = FALSE;

	GetOwner()->SetFocus();
}

void CXTPReportInplaceList::Apply()
{
	if (!pControl)
		return;

	CXTPReportControl* pReportControl = pControl;

	int nIndex = GetCurSel();
	if (nIndex != LB_ERR)
	{
		m_bApply = TRUE;

		CXTPReportRecordItemConstraint* pConstraint = (CXTPReportRecordItemConstraint*)GetItemDataPtr(nIndex);

		XTP_REPORTRECORDITEM_ARGS itemArgs = *((XTP_REPORTRECORDITEM_ARGS*)this);
		itemArgs.AddRef();

		pItem->OnConstraintChanged(&itemArgs, pConstraint);
		pReportControl->RedrawControl();

		pReportControl->SendMessageToParent(itemArgs.pRow, itemArgs.pItem, itemArgs.pColumn, XTP_NM_REPORT_VALUECHANGED, 0);

		itemArgs.Release();
	}

	pReportControl->SetFocus();
}

void CXTPReportInplaceList::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	UNREFERENCED_PARAMETER(nRepCnt); UNREFERENCED_PARAMETER(nFlags);

	const DWORD cwdHotSearchTomeOut_ms = 1300;

	DWORD dwTime = GetTickCount();

	if (dwTime - m_dwLastKeyDownTime > cwdHotSearchTomeOut_ms)
		m_strHotSearchContext.Empty();

	m_dwLastKeyDownTime = dwTime;

	//----------------------------------------------
	m_strHotSearchContext += (TCHAR)nChar;

	int nIndex = GetCurSel();
	if (nIndex == LB_ERR)
		nIndex = 0;

	int nFindIdx = FindString(nIndex, m_strHotSearchContext);

	if (nFindIdx == LB_ERR && nIndex > 0)
		nFindIdx = FindString(0, m_strHotSearchContext);

	if (nFindIdx != LB_ERR)
	{
		SetCurSel(nFindIdx);

		if (nIndex != nFindIdx)
			OnSelectionChanged(nFindIdx);
	}
}

void CXTPReportInplaceList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_UP:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_HOME:
	case VK_END:
		m_strHotSearchContext.Empty();
	}

	//----------------------------------------------
	if (nChar == VK_ESCAPE)
	{
		Cancel();
	}
	else if (nChar == VK_RETURN || nChar == VK_F4)
	{
		Apply();
	}
	else
	{
		int nPrevSel = CListBox::GetCurSel();
		CListBox::OnKeyDown(nChar, nRepCnt, nFlags);
		int nActualSel = CListBox::GetCurSel();

		if (nPrevSel != nActualSel)
			OnSelectionChanged(nActualSel);
	}
}

BOOL CXTPReportInplaceList::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pControl)
	{
		if (!pControl->OnPreviewKeyDown((UINT&)pMsg->wParam, LOWORD(pMsg->lParam), HIWORD(pMsg->lParam)) )
			return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN && IsDialogMessage(pMsg))
		return TRUE;

	return CListBox::PreTranslateMessage(pMsg);
}

void CXTPReportInplaceList::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_DOWN || nChar == VK_UP)
	{
		Apply();
		return;
	}

	CListBox::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CXTPReportInplaceList::OnSelectionChanged(int nLBIndex)
{
	CString strActual, str;
	CListBox::GetText(nLBIndex,strActual);

	CXTPReportRecordItemEditOptions* pEditOptions = pItem->GetEditOptions(pColumn);
	CXTPReportRecordItemConstraints* pConstraints = pEditOptions->GetConstraints();

	int nCount = pConstraints->GetCount();
	for (int nIndex = 0; nIndex < nCount; nIndex++)
	{
		CXTPReportRecordItemConstraint* pConstraint = pConstraints->GetAt(nIndex);
		str = pConstraint->m_strConstraint;

		if (strActual.CompareNoCase(str) == 0)
		{
			((CXTPReportControl*) pControl)->OnConstraintSelecting(pRow, pItem, pColumn, pConstraint);
			break;
		}
	}

	//BOOL bAllowEdit = pEditOptions->m_bAllowEdit;
	BOOL bConstraintEdit = pEditOptions->m_bConstraintEdit;

	CXTPReportInplaceButton* pButton = pEditOptions->GetButton(0);
	if (pButton)
	{
		if (bConstraintEdit)
			pButton->m_bDraw = TRUE; //to eliminate flickering
		else
			pButton->m_bDraw = FALSE;
	}
}


