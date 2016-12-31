// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__C2BCF929_E687_4E60_9D71_FDA7A33329E6__INCLUDED_)
#define AFX_STDAFX_H__C2BCF929_E687_4E60_9D71_FDA7A33329E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include "ToolKit/XTToolkitPro.h"
#pragma comment(lib, "ToolkitPro1501vc60.lib")

#include "BCG/BCGCBProInc.h"			// BCGControlBar Pro
#pragma comment(lib, "BCGCBPRO2210.lib")

#if _MSC_VER >= 1400

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#endif
#define CFrameWnd CBCGPFrameWnd
#define CMDIFrameWnd CBCGPMDIFrameWnd
#define CMDIChildWnd CBCGPMDIChildWnd
#define CToolBar CBCGPToolBar
#define CStatusBar CBCGPStatusBar
#define CDialog CBCGPDialog
#define CWinApp CBCGPWinApp
#define CProgressCtrl CBCGPProgressCtrl
#define CScrollBar CBCGPScrollBar
#define CButton CBCGPButton	
#define CEdit CBCGPEdit
#define CListBox CBCGPListBox
#define CComboBox CBCGPComboBox
#define CListCtrl CBCGPListCtrl
#define CScrollView CBCGPChartView
#define CTreeCtrl CBCGPShellTree
#define CToolTipCtrl CBCGPToolTipCtrl
#define CSpinButtonCtrl CBCGPSpinButtonCtrl
#define CPropertyPage CBCGPPropertyPage
#define CHeaderCtrl CBCGPHeaderCtrl
//#define CFileDialog CBCGPFileDialog
#define CFormView CBCGPFormView
//#define CBitmap CBCGPPngImage
#define CPropertySheet CBCGPPropertySheet
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C2BCF929_E687_4E60_9D71_FDA7A33329E6__INCLUDED_)
