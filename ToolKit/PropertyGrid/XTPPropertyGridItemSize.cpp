// XTPPropertyGridItemSize.cpp : implementation of the CXTPPropertyGridItemSize class.
//
// This file is a part of the XTREME PROPERTYGRID MFC class library.
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

#include "StdAfx.h"
#include "Common/XTPVC80Helpers.h"

#include "XTPPropertyGridInplaceEdit.h"
#include "XTPPropertyGridInplaceButton.h"
#include "XTPPropertyGridInplaceList.h"
#include "XTPPropertyGridItem.h"
#include "XTPPropertyGridItemSize.h"
#include "XTPPropertyGridItemNumber.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CXTPPropertyGridItemSize::CXTPPropertyGridItemSizeWidth

class CXTPPropertyGridItemSize::CXTPPropertyGridItemSizeWidth : public CXTPPropertyGridItemNumber
{
public:
	CXTPPropertyGridItemSizeWidth(LPCTSTR strCaption)
		: CXTPPropertyGridItemNumber(strCaption)
	{
	}

	virtual void OnValueChanged(CString strValue)
	{
		((CXTPPropertyGridItemSize*)m_pParent)->SetWidth(strValue);
	}
	virtual BOOL GetReadOnly() const
	{
		return m_pParent->GetReadOnly();
	}
};

/////////////////////////////////////////////////////////////////////////////
// CXTPPropertyGridItemSize::CXTPPropertyGridItemSizeHeight

class CXTPPropertyGridItemSize::CXTPPropertyGridItemSizeHeight : public CXTPPropertyGridItemNumber
{
public:
	CXTPPropertyGridItemSizeHeight(LPCTSTR strCaption)
		: CXTPPropertyGridItemNumber(strCaption)
	{
	}

	virtual void OnValueChanged(CString strValue)
	{
		((CXTPPropertyGridItemSize*)m_pParent)->SetHeight(strValue);
	}
	virtual BOOL GetReadOnly() const
	{
		return m_pParent->GetReadOnly();
	}
};

/////////////////////////////////////////////////////////////////////////////
// CXTPPropertyGridItemSize
IMPLEMENT_DYNAMIC(CXTPPropertyGridItemSize, CXTPPropertyGridItem)

CXTPPropertyGridItemSize::CXTPPropertyGridItemSize(LPCTSTR strCaption, CSize size, CSize* pBindSize)
	: CXTPPropertyGridItem(strCaption)
{
	m_pItemWidth = NULL;
	m_pItemHeight = NULL;
	m_szValue = size;
	BindToSize(pBindSize);
	m_strDefaultValue = m_strValue = SizeToString(size);
}

CXTPPropertyGridItemSize::CXTPPropertyGridItemSize(UINT nID, CSize size, CSize* pBindSize)
	: CXTPPropertyGridItem(nID)
{
	m_pItemWidth = NULL;
	m_pItemHeight = NULL;
	m_szValue = size;
	BindToSize(pBindSize);
	m_strDefaultValue = m_strValue = SizeToString(size);
}

CXTPPropertyGridItemSize::~CXTPPropertyGridItemSize()
{

}

/////////////////////////////////////////////////////////////////////////////
//

void CXTPPropertyGridItemSize::OnAddChildItem()
{
	if (m_pItemHeight && m_pItemWidth)
		return;

	m_pItemWidth = (CXTPPropertyGridItemSizeWidth*)AddChildItem(new CXTPPropertyGridItemSizeWidth(_T("Width")));
	m_pItemHeight = (CXTPPropertyGridItemSizeHeight*)AddChildItem(new CXTPPropertyGridItemSizeHeight(_T("Height")));

	UpdateChilds();

	m_pItemWidth->SetDefaultValue(m_pItemWidth->GetValue());
	m_pItemHeight->SetDefaultValue(m_pItemHeight->GetValue());
}

CString CXTPPropertyGridItemSize::SizeToString(CSize size)
{
	CString str;
	str.Format(_T("%i; %i"), size.cx, size.cy);
	return str;
}

CSize CXTPPropertyGridItemSize::StringToSize(LPCTSTR str)
{
	CString strWidth, strHeight;

	AfxExtractSubString(strWidth, str, 0, ';');
	AfxExtractSubString(strHeight, str, 1, ';');

	return CSize(_ttoi(strWidth), _ttoi(strHeight));
}

void CXTPPropertyGridItemSize::SetValue(CString strValue)
{
	SetSize(StringToSize(strValue));
}

void CXTPPropertyGridItemSize::SetSize(CSize size)
{
	m_szValue = size;

	if (m_pBindSize)
	{
		*m_pBindSize = m_szValue;
	}

	CXTPPropertyGridItem::SetValue(SizeToString(m_szValue));
	UpdateChilds();
}

void CXTPPropertyGridItemSize::BindToSize(CSize* pBindSize)
{
	m_pBindSize = pBindSize;
	if (m_pBindSize)
	{
		*m_pBindSize = m_szValue;
	}
}

void CXTPPropertyGridItemSize::OnBeforeInsert()
{
	if (m_pBindSize && *m_pBindSize != m_szValue)
	{
		SetSize(*m_pBindSize);
	}
}

void CXTPPropertyGridItemSize::UpdateChilds()
{
	m_pItemWidth->SetNumber(m_szValue.cx);
	m_pItemHeight->SetNumber(m_szValue.cy);
}

void CXTPPropertyGridItemSize::SetWidth(LPCTSTR strWidth)
{
	OnValueChanged(SizeToString(CSize(_ttoi(strWidth), m_szValue.cy)));
}

void CXTPPropertyGridItemSize::SetHeight(LPCTSTR strHeight)
{
	OnValueChanged(SizeToString(CSize(m_szValue.cx, _ttoi(strHeight))));
}
