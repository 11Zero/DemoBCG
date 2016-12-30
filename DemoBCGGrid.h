// DemoBCGGrid.h : main header file for the DEMOBCGGRID application
//

#if !defined(AFX_DEMOBCGGRID_H__7C1ED284_8FF1_4AAC_8660_BC12DCB18DFC__INCLUDED_)
#define AFX_DEMOBCGGRID_H__7C1ED284_8FF1_4AAC_8660_BC12DCB18DFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDemoBCGGridApp:
// See DemoBCGGrid.cpp for the implementation of this class
//

class CDemoBCGGridApp : public CWinApp
{
public:
	CDemoBCGGridApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDemoBCGGridApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDemoBCGGridApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEMOBCGGRID_H__7C1ED284_8FF1_4AAC_8660_BC12DCB18DFC__INCLUDED_)
