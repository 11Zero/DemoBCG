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
// BCGPAnalogClock.cpp: implementation of the CBCGPAnalogClock class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPAnalogClock.h"
#include "BCGPNumericIndicatorImpl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGPAnalogClock, CBCGPCircularGaugeImpl)
IMPLEMENT_DYNCREATE(CBCGPAnalogClockCtrl, CBCGPCircularGaugeCtrl)

CMap<UINT,UINT,CBCGPAnalogClock*,CBCGPAnalogClock*> CBCGPAnalogClock::m_mapClocks;
CCriticalSection CBCGPAnalogClock::g_cs;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPAnalogClock::CBCGPAnalogClock(CBCGPVisualContainer* pContainer) :
	CBCGPCircularGaugeImpl(pContainer)
{
	m_nTimerID = 0;
	m_bSecondHand = TRUE;
	m_pDate = NULL;

	SetColors(CBCGPCircularGaugeColors::BCGP_CIRCULAR_GAUGE_WHITE);
	SetFrameSize(6);

	SetClosedRange(0, 12);	// 0 - 12 hours
    SetStep(.2);			// 1/5
    SetMajorTickMarkStep(5);
	SetTickMarkStyle(CBCGPCircularGaugeScale::BCGP_TICKMARK_CIRCLE, TRUE, 7.);
	SetTickMarkSize(3, FALSE);

	AddPointer(
		CBCGPCircularGaugePointer(CBCGPCircularGaugePointer::BCGP_GAUGE_NEEDLE_TRIANGLE, .6));
    AddPointer(
		CBCGPCircularGaugePointer(CBCGPCircularGaugePointer::BCGP_GAUGE_NEEDLE_RECT, 0.0, 1.0, TRUE)); 
}
//*******************************************************************************
CBCGPAnalogClock::~CBCGPAnalogClock()
{
	::KillTimer(NULL, m_nTimerID);

	g_cs.Lock ();
	m_mapClocks.RemoveKey(m_nTimerID);
	g_cs.Unlock ();
}
//*******************************************************************************
void CBCGPAnalogClock::EnableSecondHand(BOOL bEnable)
{
	if (bEnable)
	{
		if (m_bSecondHand)
		{
			return;
		}

		AddPointer(
			CBCGPCircularGaugePointer(CBCGPCircularGaugePointer::BCGP_GAUGE_NEEDLE_RECT, 0.0, 1.0, TRUE));
	}
	else
	{
		if (!m_bSecondHand)
		{
			return;
		}

		RemovePointer(2);
	}

	m_bSecondHand = bEnable;
	OnSetClockTime(TRUE);
}
//*******************************************************************************
void CBCGPAnalogClock::EnableDate(BOOL bEnable, BCGP_SUB_GAUGE_POS pos)
{
	if (bEnable)
	{
		if (m_pDate != NULL)
		{
			return;
		}

		m_pDate = new CBCGPNumericIndicatorImpl;
		m_pDate->SetCells(2);
		m_pDate->SetDecimals(0);
		m_pDate->SetSeparatorWidth(0);

		CBCGPNumericIndicatorColors colors;
		colors.m_brFill = CBCGPBrush(CBCGPColor::White, CBCGPColor::LightGray, CBCGPBrush::BCGP_GRADIENT_HORIZONTAL);
		colors.m_brDigit = CBCGPBrush(CBCGPColor::Gray);

		m_pDate->SetColors(colors);

		AddSubGauge(m_pDate, pos, CBCGPSize(20, 15), CBCGPPoint(-5, 0));
	}
	else
	{
		if (m_pDate == NULL)
		{
			return;
		}

		ASSERT_VALID(m_pDate);
		
		RemoveSubGauge(m_pDate);
		m_pDate = NULL;
	}

	OnSetClockTime(TRUE);
}
//*******************************************************************************
void CBCGPAnalogClock::SetTimeOffset(const COleDateTimeSpan& offset)
{
	m_Offset = offset;
}
//*******************************************************************************
VOID CALLBACK CBCGPAnalogClock::ClockTimerProc(	HWND /*hwnd*/, UINT /*uMsg*/,
												UINT_PTR idEvent, DWORD /*dwTime*/)
{
	CBCGPAnalogClock* pObject = NULL;

	g_cs.Lock ();

	if (!m_mapClocks.Lookup((UINT)idEvent, pObject))
	{
		g_cs.Unlock ();
		return;
	}

	ASSERT_VALID(pObject);

	g_cs.Unlock ();

	pObject->OnSetClockTime(TRUE);
}
//*******************************************************************************
void CBCGPAnalogClock::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags)
{
	if (m_nTimerID == 0)
	{
		m_nTimerID = (UINT)::SetTimer (NULL, 0, 1000, ClockTimerProc);

		g_cs.Lock ();
		m_mapClocks.SetAt (m_nTimerID, this);
		g_cs.Unlock ();

		OnSetClockTime(FALSE);
	}

	CBCGPCircularGaugeImpl::OnDraw(pGM, rectClip, dwFlags);
}
//*******************************************************************************
void CBCGPAnalogClock::OnSetClockTime(BOOL bRedraw)
{
	COleDateTime currTime = COleDateTime::GetCurrentTime() + m_Offset;

	// Set date:
	if (m_pDate != NULL)
	{
		ASSERT_VALID(m_pDate);

		int day = currTime.GetDay();
		m_pDate->SetDrawLeadingZeros(day < 10);
		m_pDate->SetValue(day);

		SetDirty(FALSE);
	}

	// Set hours:
	double hour = (currTime.GetHour() % 12) + (double)currTime.GetMinute() / 60;
	SetValue(hour, 1, 0, FALSE);

	// Set minutes:
	double minute = .2 * currTime.GetMinute();
	SetValue(minute, 0, 0, FALSE);

	// Set seconds:
	if (m_bSecondHand)
	{
		double sec = currTime.GetSecond() / 5.;
		SetValue(sec, 2, GetValue(2) != 0. ? 500 : 0, FALSE);
	}

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPAnalogClock::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPCircularGaugeImpl::CopyFrom(srcObj);

	const CBCGPAnalogClock& src = (const CBCGPAnalogClock&)srcObj;

	m_bSecondHand = src.m_bSecondHand;
}
