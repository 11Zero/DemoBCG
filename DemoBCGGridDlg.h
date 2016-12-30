// DemoBCGGridDlg.h : header file
//

#if !defined(AFX_DEMOBCGGRIDDLG_H__3F52DB02_7A75_4B23_93B6_7257A9FECD43__INCLUDED_)
#define AFX_DEMOBCGGRIDDLG_H__3F52DB02_7A75_4B23_93B6_7257A9FECD43__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDemoBCGGridDlg dialog

class CDemoBCGGridDlg : public CDialog
{
// Construction
public:
	void CreateChart();
	void CreateGridXTP();
	void CreateGrid();
	CDemoBCGGridDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDemoBCGGridDlg)
	enum { IDD = IDD_DEMOBCGGRID_DIALOG };
	CBCGPChartCtrl	m_wndChart;
	CStatic	m_wndPlaceHolder;
	CStatic	m_wndGridLocation;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDemoBCGGridDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	CBCGPGridCtrl		m_wndGrid;
	CXTPPropertyGrid m_wndPropertyGrid;
	// Generated message map functions
	//{{AFX_MSG(CDemoBCGGridDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEMOBCGGRIDDLG_H__3F52DB02_7A75_4B23_93B6_7257A9FECD43__INCLUDED_)
