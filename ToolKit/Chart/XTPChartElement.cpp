// XTPChartElement.cpp
//
// This file is a part of the XTREME TOOLKIT PRO MFC class library.
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

#include "XTPChartElement.h"
#include "XTPChartContent.h"

IMPLEMENT_DYNAMIC(CXTPChartElement, CCmdTarget)

CXTPChartElement::CXTPChartElement()
{
	m_pOwner = NULL;
	m_dwRef = 1;

}

CXTPChartElement::~CXTPChartElement()
{

}

void CXTPChartElement::Release()
{
	m_pOwner = NULL;

	InternalRelease();
}


DWORD CXTPChartElement::InternalAddRef()
{
	return InterlockedIncrement(&m_dwRef);
}

DWORD CXTPChartElement::InternalRelease()
{
	if (m_dwRef == 0)
		return 0;

	LONG lResult = InterlockedDecrement(&m_dwRef);
	if (lResult == 0)
	{
		delete this;
	}
	return lResult;
}



void CXTPChartElement::OnChartChanged(XTPChartUpdateOptions updateOptions)
{
	if (m_pOwner)
	{
		m_pOwner->OnChartChanged(updateOptions);
	}
}

CXTPChartContent* CXTPChartElement::GetContent() const
{
	CXTPChartElement* pOwner = m_pOwner;
	if (!pOwner)
		return NULL;

	while (pOwner->m_pOwner != NULL)
	{
		pOwner = pOwner->m_pOwner;
	}

	if (pOwner->IsKindOf(RUNTIME_CLASS(CXTPChartContent)))
	{
		return (CXTPChartContent*)pOwner;
	}

	return NULL;
}

void CXTPChartElement::DoPropExchange(CXTPPropExchange* /*pPX*/)
{

}

//////////////////////////////////////////////////////////////////////////
// CXTPChartContainer

CXTPChartContainer::CXTPChartContainer()
{

}

CXTPChartContainer::~CXTPChartContainer()
{
}

void CXTPChartContainer::OnChartChanged(XTPChartUpdateOptions /*updateOptions*/)
{

}

void CXTPChartContainer::SetCapture(CXTPChartElementView* /*pView*/)
{

}

//////////////////////////////////////////////////////////////////////////
// CXTPChartTextElement

IMPLEMENT_DYNAMIC(CXTPChartTextElement, CXTPChartElement)


//////////////////////////////////////////////////////////////////////////
// CXTPChartElementCollection

IMPLEMENT_DYNAMIC(CXTPChartElementCollection, CXTPChartElement)

CXTPChartElementCollection::CXTPChartElementCollection()
{

}

CXTPChartElementCollection::~CXTPChartElementCollection()
{
	RemoveAll();
}

void CXTPChartElementCollection::Release()
{
	m_pOwner = NULL;

	RemoveAll();

	CXTPChartElement::Release();
}


void CXTPChartElementCollection::RemoveAll()
{
	if (m_arrElements.GetSize() == 0)
		return;

	for (int i = 0; i < m_arrElements.GetSize(); i++)
	{
		SAFE_RELEASE(m_arrElements[i]);
	}
	m_arrElements.RemoveAll();

	OnChartChanged();
}


void CXTPChartElementCollection::RemoveAt(int nIndex)
{
	if (nIndex < 0 || nIndex >= m_arrElements.GetSize())
		return;

	CXTPChartElement* pElement = m_arrElements[nIndex];

	m_arrElements.RemoveAt(nIndex);
	SAFE_RELEASE(pElement);

	OnChartChanged();
}

void CXTPChartElementCollection::Remove(CXTPChartElement* pElement)
{
	for (int i = 0; i < m_arrElements.GetSize(); i++)
	{
		CXTPChartElement* p = m_arrElements[i];
		if (p == pElement)
		{
			RemoveAt(i);
			break;
		}
	}
}

void CXTPChartElementCollection::InsertAt(int nIndex, CXTPChartElement* pElement)
{
	m_arrElements.InsertAt(nIndex, pElement);
	pElement->m_pOwner = this;

	OnChartChanged();
}

int CXTPChartElementCollection::IndexOf(CXTPChartElement* pPanel) const
{
	for (int i = 0; i < m_arrElements.GetSize(); i++)
		if (m_arrElements[i] == pPanel)
			return i;

	return -1;
}

