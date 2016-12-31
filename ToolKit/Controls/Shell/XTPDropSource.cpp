// XTPDropSource.cpp : implementation file
//
// This file is a part of the XTREME CONTROLS MFC class library.
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

#include "Controls/Shell/XTPDropSource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CXTPDropSource

CXTPDropSource::CXTPDropSource()
{
	m_uiRefCount = 1;
}

CXTPDropSource::~CXTPDropSource()
{
}

/////////////////////////////////////////////////////////////////////////////
// IUnknown Methods

STDMETHODIMP CXTPDropSource::QueryInterface(REFIID riid, LPVOID* ppReturn)
{
	*ppReturn = NULL;

	//IUnknown
	if (IsEqualIID(riid, IID_IUnknown))
	{
		*ppReturn = this;
	}
	//IDropTarget
	else if (IsEqualIID(riid, IID_IDropSource))
	{
		*ppReturn = (IDropSource*)this;
	}
	if (*ppReturn)
	{
		(*(LPUNKNOWN*)ppReturn)->AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CXTPDropSource::AddRef()
{
	return ++m_uiRefCount;
}


STDMETHODIMP_(ULONG) CXTPDropSource::Release()
{
	if (--m_uiRefCount == 0)
	{
		delete this;
		return 0;
	}
	return m_uiRefCount;
}

/////////////////////////////////////////////////////////////////////////////
// IDropSource Methods

STDMETHODIMP CXTPDropSource::QueryContinueDrag(BOOL bEsc, DWORD dwKeyState)
{
	if (bEsc)
	{
		return DRAGDROP_S_CANCEL;
	}

	if ((dwKeyState & MK_LBUTTON) == 0x0000)
	{
		return DRAGDROP_S_DROP;
	}
	return S_OK;
}

STDMETHODIMP CXTPDropSource::GiveFeedback(DWORD)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}
