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

#if !defined(AFX_BCGPOLESERVERDOC_H__32D13298_EFD5_4A82_9B17_5D13414078F9__INCLUDED_)
#define AFX_BCGPOLESERVERDOC_H__32D13298_EFD5_4A82_9B17_5D13414078F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPOleServerDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBCGPOleServerDoc document

class BCGCBPRODLLEXPORT CBCGPOleServerDoc : public COleServerDoc
{
protected:
	CBCGPOleServerDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBCGPOleServerDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPOleServerDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	virtual COleServerItem* OnGetEmbeddedItem();
	//}}AFX_VIRTUAL
	virtual void OnResizeBorder( LPCRECT lpRectBorder, LPOLEINPLACEUIWINDOW lpUIWindow, BOOL bFrame );
	virtual void OnShowControlBars(CFrameWnd* pFrameWnd, BOOL bShow);
	virtual void OnDocWindowActivate( BOOL bActivate );


// Implementation
public:
	virtual ~CBCGPOleServerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPOleServerDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPOLESERVERDOC_H__32D13298_EFD5_4A82_9B17_5D13414078F9__INCLUDED_)
