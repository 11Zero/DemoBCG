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
// BCGPAppointmentDS.cpp: implementation of the CBCGPAppointmentDrawStruct class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPAppointmentDS.h"

#ifndef BCGP_EXCLUDE_PLANNER

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPAppointmentDrawStruct::CBCGPAppointmentDrawStruct()
{
	Reset ();
}

CBCGPAppointmentDrawStruct::~CBCGPAppointmentDrawStruct()
{
}

void CBCGPAppointmentDrawStruct::Reset ()
{
	m_Rect.SetRectEmpty ();
	m_Border  = BCGP_APPOINTMENT_DS_BORDER_NONE;
	m_Visible = FALSE;
}

void CBCGPAppointmentDrawStruct::SetRect (const CRect& rect)
{
	m_Rect = rect;
}

void CBCGPAppointmentDrawStruct::SetBorder (BCGP_APPOINTMENT_DS_BORDER border)
{
	m_Border = border;
}

void CBCGPAppointmentDrawStruct::SetVisible (BOOL visible)
{
	m_Visible = visible;
}

const CBCGPAppointmentDrawStruct& CBCGPAppointmentDrawStruct::operator = 
	(const CBCGPAppointmentDrawStruct& rDS)
{
	m_Rect    = rDS.m_Rect;
	m_Border  = rDS.m_Border;
	m_Visible = rDS.m_Visible;

	return *this;
}



CBCGPAppointmentDrawStructEx::CBCGPAppointmentDrawStructEx()
{
	Reset ();
}

CBCGPAppointmentDrawStructEx::~CBCGPAppointmentDrawStructEx()
{
}

void CBCGPAppointmentDrawStructEx::Reset ()
{
	CBCGPAppointmentDrawStruct::Reset ();

	m_bToolTipNeeded = FALSE;
	m_rectEdit.SetRectEmpty ();
	m_rectEditHitTest.SetRectEmpty ();
}

void CBCGPAppointmentDrawStructEx::SetToolTipNeeded (BOOL bToolTipNeeded)
{
	m_bToolTipNeeded = bToolTipNeeded;
}

void CBCGPAppointmentDrawStructEx::SetRectEdit (const CRect& rect)
{
	m_rectEdit = rect;
}

void CBCGPAppointmentDrawStructEx::SetRectEditHitTest (const CRect& rect)
{
	m_rectEditHitTest = rect;
}

const CBCGPAppointmentDrawStructEx& CBCGPAppointmentDrawStructEx::operator = 
	(const CBCGPAppointmentDrawStructEx& rDS)
{
	CBCGPAppointmentDrawStruct::operator = ((const CBCGPAppointmentDrawStruct&)rDS);

	m_bToolTipNeeded  = rDS.m_bToolTipNeeded;
	m_rectEdit        = rDS.m_rectEdit;
	m_rectEditHitTest = rDS.m_rectEditHitTest;

	return *this;
}


CBCGPAppointmentDSMap::CBCGPAppointmentDSMap ()
	: m_bConcatenate (TRUE)
{
}

CBCGPAppointmentDSMap::~CBCGPAppointmentDSMap ()
{
	RemoveAll ();
}

void CBCGPAppointmentDSMap::RemoveAll ()
{
	for (int i = 0; i < m_DSMap.GetSize (); i++)
	{
		if (m_DSMap[i].m_pDS != NULL)
		{
			delete m_DSMap[i].m_pDS;
		}
	}

	m_DSMap.RemoveAll ();
}

BOOL CBCGPAppointmentDSMap::PointInRect (const CPoint& point) const
{
	return GetFromPoint (point) != NULL;
}

CBCGPAppointmentDrawStruct* CBCGPAppointmentDSMap::CreateStruct () const
{
	return new CBCGPAppointmentDrawStruct;
}

void CBCGPAppointmentDSMap::Add (const COleDateTime& date, const CRect& rect)
{
	CBCGPAppointmentDrawStruct* pDS = NULL;
	
	BOOL bEmpty = IsEmpty ();

	if (!bEmpty)
	{
		pDS = Get (date);
	}
	
	if (pDS == NULL)
	{
		if (!bEmpty)
		{
			XDateInterval& interval = m_DSMap[m_DSMap.GetSize () - 1];
			CBCGPAppointmentDrawStruct* pDSPrev = interval.m_pDS;
			
			if (IsConcatenate () &&
				interval.m_date2 < date && (date - interval.m_date2).GetTotalDays () <= 1.0)
			{
				if (pDSPrev->GetRect ().top    == rect.top &&
					pDSPrev->GetRect ().bottom == rect.bottom)
				{
					CRect rt;
					rt.UnionRect (pDSPrev->GetRect (), rect);

					pDSPrev->SetRect (rt);
					pDSPrev->SetBorder((CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER)
						(pDSPrev->GetBorder () | 
						 CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH));

					interval.m_date2 = date;

					return;
				}
			}

			pDSPrev->SetBorder ((CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER)
				(pDSPrev->GetBorder () & ~CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH));

			pDS = CreateStruct ();
			pDS->SetBorder (CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH);
		}
		else
		{
			pDS = CreateStruct ();
			pDS->SetBorder (CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START);
		}

		XDateInterval interval(date, pDS);
		m_DSMap.Add(interval);
	}

	pDS->SetRect (rect);
}

CBCGPAppointmentDrawStruct*
CBCGPAppointmentDSMap::Get (const COleDateTime& date) const
{
	CBCGPAppointmentDrawStruct* pDS = NULL;

	for (int i = 0; i < m_DSMap.GetSize (); i++)
	{
		const XDateInterval& interval = m_DSMap[i];

		if (interval.m_date1 <= date && date <= interval.m_date2)
		{
			pDS = interval.m_pDS;
			break;
		}
	}

	return pDS;
}

CBCGPAppointmentDrawStruct*
CBCGPAppointmentDSMap::GetFromPoint (const CPoint& point) const
{
	CBCGPAppointmentDrawStruct* pDSres = NULL;

	for (int i = 0; i < m_DSMap.GetSize (); i++)
	{
		CBCGPAppointmentDrawStruct* pDS = m_DSMap[i].m_pDS;

		if (pDS != NULL)
		{
			if (pDS->IsVisible () && pDS->GetRect ().PtInRect (point))
			{
				pDSres = pDS;
				break;
			}
		}
	}

	return pDSres;
}

CRect CBCGPAppointmentDSMap::GetVisibleRect(BOOL bFirst) const
{
	CRect rect(0, 0, 0, 0);

	const int nSize = (int)m_DSMap.GetSize ();

	for (int i = 0; i < nSize; i++)
	{
		CBCGPAppointmentDrawStruct* pDS = m_DSMap[bFirst ? i : (nSize - i - 1)].m_pDS;
		if (pDS == NULL)
		{
			continue;
		}

		if (pDS->IsVisible ())
		{
			rect = pDS->GetRect ();
			break;
		}
	}

	return rect;
}


CBCGPAppointmentDSExMap::CBCGPAppointmentDSExMap ()
{
}

CBCGPAppointmentDSExMap::~CBCGPAppointmentDSExMap ()
{
}

CBCGPAppointmentDrawStruct* CBCGPAppointmentDSExMap::CreateStruct () const
{
	return new CBCGPAppointmentDrawStructEx;
}

BOOL CBCGPAppointmentDSExMap::PointInRectEdit (const CPoint& point) const
{
	BOOL bRes = FALSE;

	for (int i = 0; i < m_DSMap.GetSize (); i++)
	{
		CBCGPAppointmentDrawStructEx* pDS = 
			(CBCGPAppointmentDrawStructEx*)m_DSMap[i].m_pDS;

		if (pDS != NULL)
		{
			if (pDS->IsVisible () && pDS->GetRectEdit ().PtInRect (point))
			{
				bRes = TRUE;
				break;
			}
		}
	}

	return bRes;
}

BOOL CBCGPAppointmentDSExMap::PointInRectEditHitTest(const CPoint& point) const
{
	BOOL bRes = FALSE;

	for (int i = 0; i < m_DSMap.GetSize (); i++)
	{
		CBCGPAppointmentDrawStructEx* pDS = 
			(CBCGPAppointmentDrawStructEx*)m_DSMap[i].m_pDS;

		if (pDS != NULL)
		{
			if (pDS->IsVisible () && pDS->GetRectEditHitTest ().PtInRect (point))
			{
				bRes = TRUE;
				break;
			}
		}
	}

	return bRes;
}

#endif // BCGP_EXCLUDE_PLANNER
