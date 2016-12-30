// DemoBCGGridDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DemoBCGGrid.h"
#include "DemoBCGGridDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDemoBCGGridDlg dialog

CDemoBCGGridDlg::CDemoBCGGridDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDemoBCGGridDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDemoBCGGridDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDemoBCGGridDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDemoBCGGridDlg)
	DDX_Control(pDX, IDC_PLACEHOLDER, m_wndPlaceHolder);
	DDX_Control(pDX, IDC_BCGCHART, m_wndChart);
	DDX_Control(pDX, IDC_GRID_RECT, m_wndGridLocation);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDemoBCGGridDlg, CDialog)
	//{{AFX_MSG_MAP(CDemoBCGGridDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDemoBCGGridDlg message handlers

BOOL CDemoBCGGridDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	//CreateGridXTP();
	//CreateGrid();
	// TODO: Add extra initialization here
	CreateChart();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDemoBCGGridDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDemoBCGGridDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDemoBCGGridDlg::CreateGrid()
{	
	CWaitCursor wait;
	
	CRect rectGrid;
	//m_wndGridLocation.GetClientRect (&rectGrid);
	//m_wndGridLocation.MapWindowPoints (this, &rectGrid);
	m_wndGridLocation.GetWindowRect( &rectGrid );
	ScreenToClient( &rectGrid );

	m_wndGrid.Create (WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rectGrid, this, IDC_GRID_RECT);
	m_wndGrid.EnableHeader (TRUE, BCGP_GRID_HEADER_MOVE_ITEMS);
	m_wndGrid.EnableInvertSelOnCtrl ();

	// Set grid tab order (first):
	m_wndGrid.SetWindowPos (&CWnd::wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	//m_wndGrid.SetReadOnly ();
	m_wndGrid.InsertColumn (0, _T("姓名"), 75);
	m_wndGrid.InsertColumn (1, _T("电话"), 150);
	m_wndGrid.SetHeaderAlign(0,HDF_CENTER);
	m_wndGrid.SetHeaderAlign(1,HDF_CENTER);
	m_wndGrid.SetColumnAlign(0,HDF_CENTER);
	m_wndGrid.SetColumnAlign(1,HDF_CENTER);
	//m_wndGrid.AddRow()
	srand ((unsigned) time (NULL));
	for (int nRow = 0; nRow < 100; nRow++)
	{
		CBCGPGridRow* pRow = m_wndGrid.CreateRow (m_wndGrid.GetColumnCount ());
		ASSERT_VALID (pRow);
		for (int i = 0; i < m_wndGrid.GetColumnCount (); i++)
		{
			CString strItem;
			strItem.Format (_T("%c%d"), _T('A') + i, nRow + 1);
			pRow->GetItem (i)->SetValue ((_variant_t) strItem);
		}
		m_wndGrid.AddRow (pRow, FALSE);
	}
	m_wndGrid.LoadState (_T("MultiLineHeaderGrid"));
	m_wndGrid.AdjustLayout ();
	return;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE

}

void CDemoBCGGridDlg::CreateGridXTP()
{
	CRect rc;
	m_wndPlaceHolder.GetWindowRect( &rc );
	ScreenToClient( &rc );

	// create the property grid.
	if (m_wndPropertyGrid.Create( rc, this, IDC_PLACEHOLDER))
	{
		m_wndPropertyGrid.SetVariableItemsHeight(TRUE);

		LOGFONT lf;
		GetFont()->GetLogFont( &lf );

		// create document settings category.
		CXTPPropertyGridItem* pPhone        = m_wndPropertyGrid.AddCategory(_T("管理人员"));
		//m_wndPropertyGrid.SetTheme(xtpGridThemeNativeWinXP);

		pPhone->SetTooltip(_T("姓名电话"));

		// add child items to category.
		CXTPPropertyGridItem* pItemPhone  = pPhone->AddChildItem(
			new CXTPPropertyGridItem(_T("张三"), _T("123")));
		pPhone->Expand();
		//pItemSaveOnClose->Select();

	}
	else
	{AfxMessageBox("err");}
	//AutoLoadPlacement(_T("PropertyGridSample"));
	return;

}

void CDemoBCGGridDlg::OnOK() 
{
	CBCGPChartVisualObject* pChart = m_wndChart.GetChart();
	ASSERT_VALID(pChart);
	CBCGPChartSeries*	pSerie= pChart->GetSeries(0);
	CString tempstr="";
	/*if(count>=11)
	{
		pSerie->RemoveDataPoints(0);
		tempstr.Format("点%d",rand()%100);
		pSerie->SetDataPointCategoryName(tempstr,4);
	}
	for (int i = 0; i < 1; i++)
	{
		const double dblVal = rand()%20;
		
		tempstr.Format("点%d",rand()%100);
		pSerie->AddDataPoint(tempstr,dblVal);
	}*/
	int count=pSerie->GetDataPointCount();
	const double dblVal = rand()%20;
	tempstr.Format("点%d",rand()%20);
		COleDateTime today=COleDateTime::GetCurrentTime();
		COleDateTime date(2016, rand()%12+1, 1, 0, 0, rand()%60);
		//pSerie->MoveDataPoints(1,0,1);
		pSerie->RemoveDataPoints(1,0,1);
		pSerie->AddDataPoint(today.Format("%H-%M-%S"),dblVal);
		pSerie->EnableUpdateAxesOnNewData();
	//pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS)->SetScrollRange();
	pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS )->EnableScroll();
	if(pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS )->IsScrollEnabled()==0)
		AfxMessageBox("err");
	//pXAxis->SetAlwaysShowScrollBar();
	//pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS )->ShowScrollBar();
	//pXAxis->SetFixedDisplayRange(0., 10., 2.);
	//pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS )->SetAutoDisplayRange();

	pChart->SetDirty(TRUE,TRUE);
	pChart->Redraw();
	return;
}

void CDemoBCGGridDlg::CreateChart()
{
	CBCGPChartVisualObject* pChart=m_wndChart.GetChart();
	//pChart->ShowAxisIntervalInterlacing(BCGP_CHART_X_PRIMARY_AXIS, FALSE);

	//pChart->SetChartType (BCGPChartHistoricalLine, BCGP_CT_SIMPLE, FALSE);
	pChart->SetLegendPosition(BCGPChartLayout::LP_TOPRIGHT);


	CBCGPChartAxis* pXAxis = pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS);
	CBCGPChartAxis* pYAxis = pChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);
	pXAxis->m_axisLabelType = CBCGPChartAxis::ALT_LOW;
	pXAxis->m_majorTickMarkType = CBCGPChartAxis::TMT_INSIDE;
	pXAxis->EnableScroll();
	pXAxis->EnableZoom();
	pXAxis->ShowScrollBar();
	pXAxis->SetFixedUnitCount(20,1);
	//pXAxis->SetAlwaysShowScrollBar();
	//pXAxis->SetFixedDisplayRange(0., 10., 2.);
	//pXAxis->SetAutoDisplayRange();
	//COleDateTime today1=COleDateTime::GetCurrentTime();
	//COleDateTime date1(2016, 11, 1, 0, 0, rand()%60);
	pXAxis->m_bFormatAsDate=1;
	//pXAxis->SetFixedDisplayRange(date1, today1, 0.000012);
	pXAxis->SetScrollBarSize(12);
	//pXAxis->SetFixedMinorUnit(0.000012);
	//pXAxis->SetFixedMaximumDisplayValue(today1);
		//pXAxis->SetInterlaceStep(2);
	//pXAxis->m_bAlwaysVisible=1;
	//pSecondaryXAxis->m_bAlwaysVisible=1;
	pXAxis->m_strDataFormat="%Y_%m_%d:%H-%M-%S";
	//pXAxis->m_strDataFormat=_T("%.3f");
	/*double major=pXAxis->GetMajorUnit();
	CString majorstr="";
	majorstr.Format("%lf",major);
	AfxMessageBox(majorstr);
		COleDateTime date0(2016, 12, 1, 0, 0, 0);
		COleDateTime date1(2016, 12, 1, 0, 0, 1);
		major=date1-date0;
	majorstr.Format("%lf",major);
	AfxMessageBox(majorstr);*/


	//CBCGPChartAxis* pYAxis = pChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);
	//ASSERT_VALID(pYAxis);

	pYAxis->SetFixedDisplayRange(0, 20, 2);
	pYAxis->m_bDisplayAxisName = FALSE;

	pChart->SetChartTitle(_T("Chart Title"));
	pChart->ShowChartTitle(1,0);
	pChart->ShowAxisIntervalInterlacing(BCGP_CHART_Y_PRIMARY_AXIS);
	pChart->SetLegendPosition(BCGPChartLayout::LP_NONE,FALSE);
	pChart->ShowDataMarkers(0);
	pChart->ShowDataLabels(0);
	pChart->SetAxisName(BCGP_CHART_X_PRIMARY_AXIS, _T("X Axis"));
	pChart->ShowAxisName(BCGP_CHART_X_PRIMARY_AXIS, 1);
	pChart->ShowAxisIntervalInterlacing(BCGP_CHART_X_PRIMARY_AXIS, 1);
	pChart->ShowAxisGridLines(BCGP_CHART_X_PRIMARY_AXIS, 1, FALSE);

	pChart->SetAxisName(BCGP_CHART_Y_PRIMARY_AXIS, _T("Y Axis"));
	pChart->ShowAxisName(BCGP_CHART_Y_PRIMARY_AXIS, 1);
	pChart->ShowAxisIntervalInterlacing(BCGP_CHART_Y_PRIMARY_AXIS, 1);
	pChart->ShowAxisGridLines(BCGP_CHART_Y_PRIMARY_AXIS, 1, FALSE);
	CString strLabel;
	strLabel.Format(_T("Series %d"), 1);
	//CBCGPChartSeries*	pSerie = pChart->CreateSeries(strLabel);
	CBCGPChartSeries* pSerie = pChart->CreateSeries(_T("Area"), CBCGPColor(), BCGP_CT_DEFAULT, BCGPChartHistoricalLine);
	((CBCGPChartHistoricalLineSeries*)pSerie)->SetMajorXUnit(1);
	strLabel = pXAxis->m_strDataFormat;
	//AfxMessageBox(strLabel);
	CString strXAxis=_T("");
	for (int i = 0; i < 60; i++)
	{
		//break;
		//const double d = MAX_VAL / 25;
		double dblVal = rand()%20;//min(MAX - 1, max(0., m_arLastVal[i] + dblDelta));
		strXAxis.Format("坐标");
		COleDateTime today=COleDateTime::GetCurrentTime();
		COleDateTime date(2016, rand()%12+1, 1, 0, 0, i);
		strXAxis.Format("%d",i+2);
		//pSerie->AddDataPoint(today.Format("%H-%M-%S"), dblVal);
		((CBCGPChartHistoricalLineSeries*)pSerie)->AddDataPoint(dblVal,date);
		//pChart->SetDataPointDataLabelText(strXAxis,i);
	}
	COleDateTime date1(1903, 9, 25, 0, 0, 0);
	strXAxis.Format("%f",date1);
	AfxMessageBox(strXAxis);
	pChart->SetDirty();
	pChart->Redraw();
	//const BCGPChartFormatSeries* pSerFormat = pSerie->GetDataPointFormat(1000,1);
	//const BCGPChartFormatDataLabel aaa=pSerFormat->m_dataLabelFormat;
}
