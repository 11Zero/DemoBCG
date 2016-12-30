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
// BCGPTextGaugeImpl.h: interface for the CBCGPTextGaugeImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTEXTGAUGEIMPL_H__364A65AC_3BDC_45E5_BE02_B18A45EEF97E__INCLUDED_)
#define AFX_BCGPTEXTGAUGEIMPL_H__364A65AC_3BDC_45E5_BE02_B18A45EEF97E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPStaticGaugeImpl.h"

class BCGCBPRODLLEXPORT CBCGPTextGaugeImpl : public CBCGPStaticGaugeImpl  
{
	DECLARE_DYNCREATE(CBCGPTextGaugeImpl)

public:
	CBCGPTextGaugeImpl(LPCTSTR lpszText = NULL, const CBCGPColor& color = CBCGPColor::Black, CBCGPVisualContainer* pContainer = NULL);
	CBCGPTextGaugeImpl(LPCTSTR lpszText, const CBCGPBrush& brush = CBCGPBrush(CBCGPColor::Black), CBCGPVisualContainer* pContainer = NULL);
	virtual ~CBCGPTextGaugeImpl();

	virtual void CopyFrom(const CBCGPBaseVisualObject& src);

// Overrides:
public:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual CBCGPSize GetDefaultSize(CBCGPGraphicsManager* pGM, const CBCGPBaseVisualObject* pParentGauge);

protected:
	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld);

	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName)
	{
		if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
		{
			CString strName = GetName();
			if (strName.IsEmpty())
			{
				strName = m_strText;
			}
			
			if (!strName.IsEmpty())
			{
				*pszName = strName.AllocSysString();
				return S_OK;
			}
		}
		
		return S_FALSE;
	}

	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole)
	{
		HRESULT hr = CBCGPStaticGaugeImpl::get_accRole(varChild, pvarRole);
		if (SUCCEEDED(hr))
		{
			pvarRole->lVal = ROLE_SYSTEM_STATICTEXT;
		}
		
		return hr;
	}

// Attributes:
public:
	void SetText(const CString& str, const CBCGPColor& clrText = CBCGPColor(), BOOL bRedraw = FALSE);
	void SetText(const CString& str, const CBCGPBrush& brText, BOOL bRedraw = FALSE);

	const CString& GetText() const
	{
		return m_strText;
	}

	const CBCGPColor& GetTextColor() const
	{
		return m_brText.GetColor ();
	}

	void SetTextColor(const CBCGPColor& color);

	const CBCGPBrush& GetTextBrush() const
	{
		return m_brText;
	}

	void SetTextBrush(const CBCGPBrush& brush);

	const CBCGPTextFormat& GetTextFormat() const
	{
		return m_textFormat;
	}

	void SetTextFormat(const CBCGPTextFormat& textFormat);

protected:
	void CreateResources(const CBCGPBrush& brText, BOOL bAllowEmptyBrush);

	CString			m_strText;
	CBCGPBrush		m_brText;
	CBCGPBrush		m_brTextLight;
	CBCGPTextFormat	m_textFormat;
};

#endif // !defined(AFX_BCGPTEXTGAUGEIMPL_H__364A65AC_3BDC_45E5_BE02_B18A45EEF97E__INCLUDED_)
