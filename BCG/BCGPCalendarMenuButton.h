// BCGCalendarMenuButton.h: interface for the CBCGPCalendarMenuButton class.
//
//////////////////////////////////////////////////////////////////////

// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#if !defined(AFX_BCGPCALENDARMENUBUTTON_H__B5E358A5_9FBF_40EF_94DA_983CEEECB6C1__INCLUDED_)
#define AFX_BCGPCALENDARMENUBUTTON_H__B5E358A5_9FBF_40EF_94DA_983CEEECB6C1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
#include "BCGPToolbarMenuButton.h"
#endif

class BCGCBPRODLLEXPORT CBCGPCalendarMenuButton : public CBCGPToolbarMenuButton  
{
	DECLARE_SERIAL(CBCGPCalendarMenuButton)

public:
	CBCGPCalendarMenuButton();
	CBCGPCalendarMenuButton(UINT uiCmdID, LPCTSTR lpszText);
	virtual ~CBCGPCalendarMenuButton();

	virtual void SetDate (const COleDateTime& date, BOOL bNotify = TRUE);
	COleDateTime GetDate () const
	{
		return m_Calendar;
	}

	static const COleDateTime GetCalendarByCmdID (UINT uiCmdID);
	static void SetCalendarByCmdID (UINT uiCmdID, const COleDateTime& date)
	{
		m_CalendarsByID.SetAt (uiCmdID, date);
	}

/*	void SetColumnsNumber (int nColumns)
	{
		m_nColumns = nColumns;
	}*/

	void EnableTearOff (UINT uiID, 
						int nVertDockColumns = -1,
						int nHorzDockRows = -1);

	virtual void OnChangeParentWnd (CWnd* pWndParent);

protected:
	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual CBCGPPopupMenu* CreatePopupMenu ();
	virtual BOOL IsEmptyMenuAllowed () const
	{
		return TRUE;
	}

	virtual void Serialize (CArchive& ar);
	virtual void CopyFrom (const CBCGPToolbarButton& src);
	virtual int OnDrawOnCustomizeList (CDC* pDC, const CRect& rect, BOOL bSelected);
	virtual BOOL OnClick (CWnd* pWnd, BOOL bDelay);
	void Initialize ();

protected:
	COleDateTime				m_Calendar;	// Currently selected day
	int							m_nColumns;
	int							m_nVertDockColumns;
	int							m_nHorzDockRows;

	BOOL						m_bStdCalendarDlg;

	static CMap<UINT,UINT,COleDateTime, COleDateTime>	m_CalendarsByID;
};

#endif // !defined(AFX_BCGPCALENDARMENUBUTTON_H__B5E358A5_9FBF_40EF_94DA_983CEEECB6C1__INCLUDED_)
