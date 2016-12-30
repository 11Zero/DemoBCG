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
// BCGPAnalogClock.h: interface for the CBCGPAnalogClock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPANALOGCLOCK_H__92241BC3_CF3B_4A4D_913E_014C394D3036__INCLUDED_)
#define AFX_BCGPANALOGCLOCK_H__92241BC3_CF3B_4A4D_913E_014C394D3036__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPCircularGaugeImpl.h"

class CBCGPNumericIndicatorImpl;

class BCGCBPRODLLEXPORT CBCGPAnalogClock : public CBCGPCircularGaugeImpl  
{
	friend class CBCGPVisualCollector;
	friend class CBCGPVisualConstructor;

	DECLARE_DYNCREATE(CBCGPAnalogClock)

public:
	CBCGPAnalogClock(CBCGPVisualContainer* pContainer = NULL);
	virtual ~CBCGPAnalogClock();

	virtual void CopyFrom(const CBCGPBaseVisualObject& src);

	void EnableSecondHand(BOOL bEnable = TRUE);
	BOOL IsSecondHandEnabled() const
	{
		return m_bSecondHand;
	}

	void EnableDate(BOOL bEnable = TRUE, BCGP_SUB_GAUGE_POS pos = CBCGPGaugeImpl::BCGP_SUB_GAUGE_RIGHT);
	BOOL IsDateEnabled() const
	{
		return m_pDate != NULL;
	}

	void SetTimeOffset(const COleDateTimeSpan& offset);
	const COleDateTimeSpan& GetTimeOffset() const
	{
		return m_Offset;
	}

	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);

protected:
	static VOID CALLBACK ClockTimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	virtual void OnSetClockTime(BOOL bRedraw);

	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole)
	{
		HRESULT hr = CBCGPCircularGaugeImpl::get_accRole(varChild, pvarRole);
		if (SUCCEEDED(hr))
		{
			pvarRole->lVal = ROLE_SYSTEM_CLOCK;
		}
		
		return hr;
	}
	
	virtual HRESULT get_accValue(VARIANT varChild, BSTR *pszValue)
	{
		if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
		{
			COleDateTime currTime = COleDateTime::GetCurrentTime() + m_Offset;

			CString strValue = currTime.Format();
			*pszValue = strValue.AllocSysString();
			return S_OK;
		}

		return S_FALSE;
	}

// Attributes:
public:

protected:
	UINT						m_nTimerID;
	BOOL						m_bSecondHand;
	CBCGPNumericIndicatorImpl*	m_pDate;
	COleDateTimeSpan			m_Offset;

	static CMap<UINT,UINT,CBCGPAnalogClock*,CBCGPAnalogClock*> m_mapClocks;
	static CCriticalSection g_cs;			// For multi-thread applications
};

class BCGCBPRODLLEXPORT CBCGPAnalogClockCtrl : public CBCGPCircularGaugeCtrl  
{
	DECLARE_DYNCREATE(CBCGPAnalogClockCtrl)

public:
	CBCGPAnalogClockCtrl() {}
	virtual ~CBCGPAnalogClockCtrl() {}

	virtual CBCGPAnalogClock* GetClock()
	{
		return DYNAMIC_DOWNCAST(CBCGPAnalogClock, m_pGauge);
	}

	virtual CBCGPCircularGaugeImpl*	GetGauge()
	{
		if (m_pGauge == NULL)
		{
			m_pGauge = new CBCGPAnalogClock;
		}

		return m_pGauge;
	}
};

#endif // !defined(AFX_BCGPANALOGCLOCK_H__92241BC3_CF3B_4A4D_913E_014C394D3036__INCLUDED_)
