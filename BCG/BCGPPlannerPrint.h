#if !defined(AFX_BCGPPLANNERPRINT_H__5B346DCD_33F3_4E8B_AA24_5D2015EF78D1__INCLUDED_)
#define AFX_BCGPPLANNERPRINT_H__5B346DCD_33F3_4E8B_AA24_5D2015EF78D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGPPlannerPrint.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPAppointment.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerPrint

class CBCGPPlannerPrint : public CObject
{
	friend class CBCGPAppointment;
	DECLARE_DYNCREATE(CBCGPPlannerPrint)

public:
	virtual ~CBCGPPlannerPrint();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// abstract
	virtual void SetCompressWeekend (BOOL /*bCompress*/){}
	// abstract
	virtual BOOL IsCompressWeekend () const
	{
		return FALSE;
	}

	// abstract
	virtual void SetDrawTimeFinish (BOOL /*bDraw*/){};
	// abstract
	virtual BOOL IsDrawTimeFinish () const
	{
		return TRUE;
	}

	int GetFirstViewHour () const
	{
		return (int)m_nFirstViewHour;
	}
	int GetLastViewHour () const
	{
		return (int)m_nLastViewHour;
	}

	const CString& GetCaptionFormat () const
	{
		return m_strCaptionFormat;
	}

	virtual void PrepareInfo (CPrintInfo* pInfo);

	virtual void PreparePrinting (CDC* pDC, CPrintInfo* pInfo,  
		CBCGPPlannerManagerCtrl* pPlanner);

	virtual void OnPaint (CDC* pDC, CPrintInfo* pInfo);

	COLORREF GetTextColor () const
	{
		return m_clrText;
	}

	COLORREF GetPenColor () const
	{
		return m_clrPen;
	}

	const CSize& GetOnePoint () const
	{
		return m_OnePoint;
	}

	virtual void SetDrawPageHeader (BOOL bDraw);
	BOOL IsDrawPageHeader () const
	{
		return m_bDrawPageHeader;
	}

	virtual void SetDrawPageFooter (BOOL bDraw);
	BOOL IsDrawPageFooter () const
	{
		return m_bDrawPageFooter;
	}

protected:
	CBCGPPlannerPrint ();

	const COleDateTime& GetDate () const
	{
		return m_Date;
	}

	const COleDateTime& GetDateStart () const
	{
		return m_DateStart;
	}

	const COleDateTime& GetDateEnd () const
	{
		return m_DateEnd;
	}

	int GetViewDuration () const	// Days
	{
		return (int)(m_DateEnd - m_DateStart).GetTotalDays () + 1;
	}

	// abstract
	virtual void CalculateDates (const COleDateTime& /*date*/){}

	// abstract
	virtual void GetCaptionFormatStrings (CStringArray& /*sa*/){}
	// abstract
	virtual CString GetPageHeaderText () const
	{
		return CString ();
	}
	// abstract
	virtual void AdjustLayout (CDC* /*pDC*/, const CRect& /*rectClient*/){}
	// abstract
	virtual void AdjustRects (){};
	// abstract
	virtual void AdjustAppointments (){};


	virtual void AdjustLayout (CDC* pDC, CPrintInfo* pInfo);

	virtual void AdjustPageHeader (CDC* pDC);

	virtual void AdjustPageFooter (CDC* pDC);

	virtual void AdjustCaptionFormat (CDC* pDC);

	virtual void CheckVisibleAppointments(const COleDateTime& date, const CRect& rect, 
		BOOL bFullVisible);

	virtual void OnDrawPageHeader (CDC* pDC);

	virtual void OnDrawPageFooter (CDC* pDC);

	// abstract
	virtual void OnDrawClient (CDC* /*pDC*/){}

	// 0 - all appointments in view; 
	// 1 - upper appointments not in view; 
	// 2 - lower appointments not in view
	// 3 - upper and lower not in view
	virtual BYTE OnDrawAppointments (CDC* pDC, const CRect& rect = CRect (0, 0, 0, 0), 
		const COleDateTime& date = COleDateTime ());

	virtual void DrawHeader (CDC* pDC, const CRect& rect, int dxColumn);

	virtual void DrawCaption (CDC* pDC, const CRect& rect, 
				const CString& strText,	BOOL bCenter, BOOL bExtended, 
				const CBrush& br);

	virtual void DrawCaption (CDC* pDC, const CRect& rect, 
				const COleDateTime& day, BOOL bCenter, BOOL bExtended, 
				const CBrush& br);

	virtual void DrawAppointment (CDC* pDC, CBCGPAppointment* pApp, CBCGPAppointmentDrawStruct* pDS);

	XBCGPAppointmentArray& GetQueryedAppointments ()
	{
		return m_arQuery;
	}

	const XBCGPAppointmentArray& GetQueryedAppointments () const
	{
		return m_arQuery;
	}

	DWORD GetDrawFlags () const
	{
		return m_dwDrawFlags;
	}

protected:
	XBCGPAppointmentArray m_arQuery;

	CRect				m_rectApps;
	int					m_nRowHeight;

	COleDateTime		m_Date;			// Planner base date
	COleDateTime		m_DateStart;	// Planner start date
	COleDateTime		m_DateEnd;		// Planner end date

	CArray<CRect, CRect&>
						m_ViewRects;

	CBrush				m_brWhite;
	CBrush				m_brGray;
	CBrush				m_brDarkGray;
	CPen				m_penBlack;

	COLORREF			m_clrPen;
	COLORREF			m_clrText;

	CSize				m_OnePoint;

	CRect				m_rectPageHeader;
	CRect				m_rectPageFooter;

	BOOL				m_bDrawPageHeader;
	BOOL				m_bDrawPageFooter;

	CFont				m_Font;
	CFont				m_FontBold;
	CFont				m_FontVert;
	CFont				m_FontHeader;
	CFont				m_FontFooter;

	COleDateTime		m_dtBegin;
	int					m_nCurrentPage;

	CString				m_strCaptionFormat;
	BOOL				m_bFirstDraw;

	DWORD				m_dwDrawFlags;

	double				m_nFirstViewHour;
	double				m_nLastViewHour;
};

#endif // BCGP_EXCLUDE_PLANNER

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPLANNERPRINT_H__5B346DCD_33F3_4E8B_AA24_5D2015EF78D1__INCLUDED_)
