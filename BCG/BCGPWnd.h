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
//
// BCGPWnd.h : header file
//

#if !defined(AFX_BCGPWND_H__C745AB2B_1856_413D_98B2_32FFD103514B__INCLUDED_)
#define AFX_BCGPWND_H__C745AB2B_1856_413D_98B2_32FFD103514B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcbpro.h"
#include "oleacc.h"
#include "bcgpaccessibility.h"
#include "BCGPGestureManager.h"

struct tagBCG_GESTUREINFO;

#ifndef _BCGPCHART_STANDALONE

/////////////////////////////////////////////////////////////////////////////
// CBCGPWnd window

class BCGCBPRODLLEXPORT CBCGPWnd :	public CWnd,
									public CBCGPGestureBase
{
	DECLARE_DYNCREATE(CBCGPWnd)

// Construction
public:
	CBCGPWnd();

// Attributes
protected:
#if _MSC_VER < 1300
	IAccessible*	m_pStdObject;
#endif

	// Accessibility attributes:
	CBCGPAccessibilityData	m_AccData;

public:
	BOOL IsVisualManagerStyle () const
	{
		return m_bVisualManagerStyle;
	}

	BOOL IsOnGlass () const
	{
		return m_bOnGlass;
	}

protected:
	BOOL				m_bVisualManagerStyle;
	BOOL				m_bOnGlass;

// Operations
public:
	void SetControlVisualMode (CWnd* pParentWnd);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPWnd)
	//}}AFX_VIRTUAL

	virtual BOOL OnSetAccData (long /*lVal*/);
	virtual BOOL IsAccessibilityCompatible () { return TRUE; }
	virtual void AccNotifyObjectFocusEvent (int /*iButtton*/) {};

	virtual void OnRTLChanged(BOOL /*bIsRTL*/) {}

	virtual HRESULT get_accParent(IDispatch **ppdispParent);
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);
	virtual HRESULT get_accValue(VARIANT varChild, BSTR *pszValue);
	virtual HRESULT get_accDescription(VARIANT varChild, BSTR *pszDescription);
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);
	virtual HRESULT get_accHelp(VARIANT varChild, BSTR *pszHelp);
	virtual HRESULT get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic);
	virtual HRESULT get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut);
	virtual HRESULT get_accFocus(VARIANT *pvarChild);
	virtual HRESULT get_accSelection(VARIANT *pvarChildren);
	virtual HRESULT get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);
	virtual HRESULT accSelect(long flagsSelect, VARIANT varChild);
	virtual HRESULT accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
	virtual HRESULT accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual HRESULT accDoDefaultAction(VARIANT varChild);
	virtual HRESULT put_accName(VARIANT varChild, BSTR szName);
	virtual HRESULT put_accValue(VARIANT varChild, BSTR szValue);

// Implementation
protected:
	virtual ~CBCGPWnd();

	// Generated message map functions
	//{{AFX_MSG(CBCGPWnd)
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	//}}AFX_MSG
	afx_msg LRESULT OnGetObject(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlAero (WPARAM, LPARAM);
	afx_msg LRESULT OnGestureEvent(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnTabletQuerySystemGestureStatus(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

#if _MSC_VER < 1300

	DECLARE_OLECREATE(CBCGPWnd)

	BEGIN_INTERFACE_PART(Accessible, IAccessible)
		//IDispatch methods
		STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);
        STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
        STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, UINT cNames, LCID lcid, DISPID FAR* rgdispid);
        
		//IAccessible methods
		STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr);
		STDMETHOD(get_accParent)(THIS_ IDispatch * FAR* ppdispParent);
		STDMETHOD(get_accChildCount)(THIS_ long FAR* pChildCount);
		STDMETHOD(get_accChild)(THIS_ VARIANT varChildIndex, IDispatch * FAR* ppdispChild);

		STDMETHOD(get_accName)(THIS_ VARIANT varChild, BSTR* pszName);
		STDMETHOD(get_accValue)(THIS_ VARIANT varChild, BSTR* pszValue);
		STDMETHOD(get_accDescription)(THIS_ VARIANT varChild, BSTR FAR* pszDescription);
		STDMETHOD(get_accRole)(THIS_ VARIANT varChild, VARIANT *pvarRole);
		STDMETHOD(get_accState)(THIS_ VARIANT varChild, VARIANT *pvarState);
		STDMETHOD(get_accHelp)(THIS_ VARIANT varChild, BSTR* pszHelp);
		STDMETHOD(get_accHelpTopic)(THIS_ BSTR* pszHelpFile, VARIANT varChild, long* pidTopic);
		STDMETHOD(get_accKeyboardShortcut)(THIS_ VARIANT varChild, BSTR* pszKeyboardShortcut);
		STDMETHOD(get_accFocus)(THIS_ VARIANT FAR * pvarFocusChild);
		STDMETHOD(get_accSelection)(THIS_ VARIANT FAR * pvarSelectedChildren);
		STDMETHOD(get_accDefaultAction)(THIS_ VARIANT varChild, BSTR* pszDefaultAction);

		STDMETHOD(accSelect)(THIS_ long flagsSelect, VARIANT varChild);
		STDMETHOD(accLocation)(THIS_ long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild);
		STDMETHOD(accNavigate)(THIS_ long navDir, VARIANT varStart, VARIANT * pvarEndUpAt);
		STDMETHOD(accHitTest)(THIS_ long xLeft, long yTop, VARIANT * pvarChildAtPoint);
		STDMETHOD(accDoDefaultAction)(THIS_ VARIANT varChild);

		STDMETHOD(put_accName)(THIS_ VARIANT varChild, BSTR szName);
		STDMETHOD(put_accValue)(THIS_ VARIANT varChild, BSTR pszValue);
    END_INTERFACE_PART(Accessible)

	DECLARE_INTERFACE_MAP()
#endif
};

#endif

// CBCGPBaseAccessibleObject command target

class BCGCBPRODLLEXPORT CBCGPBaseAccessibleObject : public CCmdTarget
{
	DECLARE_DYNAMIC(CBCGPBaseAccessibleObject)
		
protected:
	CBCGPAccessibilityData	m_AccData;
	
public:
	CBCGPBaseAccessibleObject();
	virtual ~CBCGPBaseAccessibleObject();
	
public:
	DECLARE_INTERFACE_MAP()
		
	BEGIN_INTERFACE_PART(Accessible, IAccessible)
	//IDispatch methods
	STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, UINT cNames, LCID lcid, DISPID FAR* rgdispid);
	
	//IAccessible methods
	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr);
	STDMETHOD(get_accParent)(THIS_ IDispatch * FAR* ppdispParent);
	STDMETHOD(get_accChildCount)(THIS_ long FAR* pChildCount);
	STDMETHOD(get_accChild)(THIS_ VARIANT varChildIndex, IDispatch * FAR* ppdispChild);
	STDMETHOD(get_accName)(THIS_ VARIANT varChild, BSTR* pszName);
	STDMETHOD(get_accValue)(THIS_ VARIANT varChild, BSTR* pszValue);
	STDMETHOD(get_accDescription)(THIS_ VARIANT varChild, BSTR FAR* pszDescription);
	STDMETHOD(get_accRole)(THIS_ VARIANT varChild, VARIANT *pvarRole);
	STDMETHOD(get_accState)(THIS_ VARIANT varChild, VARIANT *pvarState);
	STDMETHOD(get_accHelp)(THIS_ VARIANT varChild, BSTR* pszHelp);
	STDMETHOD(get_accHelpTopic)(THIS_ BSTR* pszHelpFile, VARIANT varChild, long* pidTopic);
	STDMETHOD(get_accKeyboardShortcut)(THIS_ VARIANT varChild, BSTR* pszKeyboardShortcut);
	STDMETHOD(get_accFocus)(THIS_ VARIANT FAR * pvarFocusChild);
	STDMETHOD(get_accSelection)(THIS_ VARIANT FAR * pvarSelectedChildren);
	STDMETHOD(get_accDefaultAction)(THIS_ VARIANT varChild, BSTR* pszDefaultAction);
	STDMETHOD(accSelect)(THIS_ long flagsSelect, VARIANT varChild);
	STDMETHOD(accLocation)(THIS_ long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild);
	STDMETHOD(accNavigate)(THIS_ long navDir, VARIANT varStart, VARIANT * pvarEndUpAt);
	STDMETHOD(accHitTest)(THIS_ long xLeft, long yTop, VARIANT * pvarChildAtPoint);
	STDMETHOD(accDoDefaultAction)(THIS_ VARIANT varChild);
	STDMETHOD(put_accName)(THIS_ VARIANT varChild, BSTR szName);
	STDMETHOD(put_accValue)(THIS_ VARIANT varChild, BSTR pszValue);
    END_INTERFACE_PART(Accessible)
		
public:
	virtual HRESULT get_accParent(IDispatch **ppdispParent);
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);
	virtual HRESULT get_accValue(VARIANT varChild, BSTR *pszValue);
	virtual HRESULT get_accDescription(VARIANT varChild, BSTR *pszDescription);
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);
	virtual HRESULT get_accHelp(VARIANT varChild, BSTR *pszHelp);
	virtual HRESULT get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic);
	virtual HRESULT get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut);
	virtual HRESULT get_accFocus(VARIANT *pvarChild);
	virtual HRESULT get_accSelection(VARIANT *pvarChildren);
	virtual HRESULT get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);
	virtual HRESULT accSelect(long flagsSelect, VARIANT varChild);
	virtual HRESULT accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
	virtual HRESULT accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual HRESULT accDoDefaultAction(VARIANT varChild);
	virtual HRESULT put_accName(VARIANT varChild, BSTR szName);
	virtual HRESULT put_accValue(VARIANT varChild, BSTR szValue);
	
	virtual CWnd* GetParentWnd() const { return NULL; }
	virtual BOOL OnSetAccData(long lVal);
	virtual BOOL SetACCData(CWnd* pParent, CBCGPAccessibilityData& data);

	virtual BOOL IsAccessibilityCompatible() { return TRUE; }
	virtual void OnSetIAccessible(IAccessible*) {}
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPWND_H__C745AB2B_1856_413D_98B2_32FFD103514B__INCLUDED_)
