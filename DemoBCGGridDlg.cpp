// DemoBCGGridDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DemoBCGGrid.h"
#include "DemoBCGGridDlg.h"
#include "YahooQuote.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
void AddDate(CXTPChartSeries* pSeries, LPCTSTR arg, double low, double high, double open, double close)//
{
	CXTPChartSeriesPoint* pPoint;	
	pPoint = pSeries->GetPoints()->Add(new CXTPChartSeriesPoint(arg, low, high, open, close));//	
};
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
	DDX_Control(pDX, IDC_TOOLKITCHART, m_wndToolKitChart);
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
	//CreateChart();
	//CreateToolKitChart();
	UpdateHistory();
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

void CDemoBCGGridDlg::CreateToolKitChart()
{
	m_ToolKitChart.Attach(m_wndToolKitChart.GetSafeHwnd());
	// set chart title.
	//m_ToolKitChart.GetContent()->SetBackgroundColor(CXTPChartColor::Gray);
	CXTPChartTitleCollection* pTitles = m_ToolKitChart.GetContent()->GetTitles();
	CXTPChartTitle* pTitle = pTitles->Add(new CXTPChartTitle());
	pTitle->SetText(_T("Historical Stock Prices"));
	pTitle->SetTextColor(CXTPChartColor::White);

	// set chart subtitle.
	CXTPChartTitle* pSubTitle = pTitles->Add(new CXTPChartTitle());
	pSubTitle->SetText(_T("www.codejock.com"));
	pSubTitle->SetDocking(xtpChartDockBottom);
	pSubTitle->SetAlignment(xtpChartAlignFar);
	pSubTitle->SetFont(CXTPChartFont::GetTahoma8());
	pSubTitle->SetTextColor(CXTPChartColor::White);

	/*CXTPChartContent* pContent = m_ToolKitChart.GetContent();
		CXTPChartSeriesCollection* pCollection = pContent->GetSeries();
		if (pCollection)
		{
			CXTPChartSeries* pSeries = pCollection->Add(new CXTPChartSeries());
			if (pSeries)
			{
				CXTPChartSeriesPointCollection* pPoints = pSeries->GetPoints();
				if (pPoints)
				{
					pPoints->Add(new CXTPChartSeriesPoint(0, 0.5));
					pPoints->Add(new CXTPChartSeriesPoint(1, 2));
					pPoints->Add(new CXTPChartSeriesPoint(2, 1));
					pPoints->Add(new CXTPChartSeriesPoint(3, 1.5));
				}

				// ----------------------------------------------------------
				// SET THE CHART SERIES STYLE
				// ----------------------------------------------------------
				// You can call SetStyle to set the chart style such as bar, 
				// line or area chart. Here we set the style to line chart by 
				// instantiating a CXTPChartLineSeriesStyle object.

				pSeries->SetStyle(new CXTPChartLineSeriesStyle());
				CXTPChartDiagram2D* pDiagram = (CXTPChartDiagram2D*)pSeries->GetDiagram();
				
				pDiagram->GetAxisY()->GetGridLines()->SetMinorVisible(TRUE);
				pDiagram->GetAxisY()->GetGridLines()->GetMinorLineStyle()->SetDashStyle(xtpChartDashStyleDot);
				
				pDiagram->GetAxisY()->GetTitle()->SetText(_T("US Dollars"));
				pDiagram->GetAxisY()->GetTitle()->SetVisible(TRUE);
				pDiagram->GetAxisY()->SetColor(CXTPChartColor::White);
				pDiagram->GetAxisY()->GetTitle()->SetTextColor(CXTPChartColor::White);
				pDiagram->GetAxisY()->GetLabel()->SetTextColor(CXTPChartColor::White);

				pDiagram->GetAxisY()->GetRange()->SetShowZeroLevel(TRUE);
				
				pDiagram->GetAxisX()->GetLabel()->SetAngle(360-30);
				pDiagram->GetAxisX()->GetLabel()->SetAntialiasing(TRUE);
				pDiagram->GetAxisX()->SetColor(CXTPChartColor::White);
				pDiagram->GetAxisX()->GetLabel()->SetTextColor(CXTPChartColor::White);
				pDiagram->GetAxisX()->GetTitle()->SetTextColor(CXTPChartColor::White);
				pDiagram->GetAxisX()->GetLabel()->SetVisible(TRUE);
				pDiagram->GetAxisX()->GetCustomLabels()->RemoveAll();

			}
		}*/

	//pSeries->SetArgumentScaleType(xtpChartScaleQualitative);
		// turn off legend.
	/*m_ToolKitChart.GetContent()->GetLegend()->SetVisible(FALSE);
	CXTPChartSeries* pSeries = m_ToolKitChart.GetContent()->GetSeries()->Add(new CXTPChartSeries());
    CXTPChartFastLineSeriesStyle* pStyle = (CXTPChartFastLineSeriesStyle*)pSeries->SetStyle(new CXTPChartFastLineSeriesStyle());
	//pStyle->SetLineThickness(1);
	CXTPChartDiagram2D* pDiagram = (CXTPChartDiagram2D*)pSeries->GetDiagram();
	
	pDiagram->GetAxisY()->GetGridLines()->SetMinorVisible(TRUE);
	pDiagram->GetAxisY()->GetGridLines()->GetMinorLineStyle()->SetDashStyle(xtpChartDashStyleDot);
	
	pDiagram->GetAxisY()->GetTitle()->SetText(_T("US Dollars"));
	pDiagram->GetAxisY()->GetTitle()->SetVisible(TRUE);

	pDiagram->GetAxisY()->GetRange()->SetShowZeroLevel(FALSE);
	
	pDiagram->GetAxisX()->GetLabel()->SetAngle(360-30);
	pDiagram->GetAxisX()->GetLabel()->SetAntialiasing(TRUE);


	//pSeries->SetArgumentScaleType(xtpChartScaleQualitative);
	pDiagram->GetAxisX()->GetLabel()->SetVisible(TRUE);
	pDiagram->GetAxisX()->GetCustomLabels()->RemoveAll();
	AddDate(pSeries,"1",1);
	return;*/


	m_ToolKitChart.GetContent()->GetSeries()->RemoveAll();

	CXTPChartSeries* pSeries = m_ToolKitChart.GetContent()->GetSeries()->Add(new CXTPChartSeries());
	//pSeries->SetArgumentScaleType(xtpChartScaleDateTime);
	//Color(CXTPChartColor::White);
	CTime timeEnd = CTime::GetCurrentTime();

	CTime timeStart = timeEnd;
	int nDays=90;
	CString lpszSymbol="sohu";
	timeStart -= CTimeSpan(nDays,0,0,0);
	
	CYahooQuote	quote;
	CStringArray& arrQuote = quote.GetHistory(lpszSymbol, timeStart, timeEnd);

	
    CXTPChartHighLowSeriesStyle* pStyle = NULL;
    int bCandleStick=1;
    if (bCandleStick)
    {
        pStyle = (CXTPChartHighLowSeriesStyle*)pSeries->SetStyle(new CXTPChartCandleStickSeriesStyle());
    }
    else
    {
        pStyle = (CXTPChartHighLowSeriesStyle*)pSeries->SetStyle(new CXTPChartHighLowSeriesStyle());
    }
    int m_bThickLine=1;
	pStyle->SetLineThickness(m_bThickLine ? 2 : 1);
	int m_bCandleStick = bCandleStick;
	
	CXTPChartDiagram2D* pDiagram = (CXTPChartDiagram2D*)pSeries->GetDiagram();
	
	pDiagram->GetAxisY()->GetGridLines()->SetMinorVisible(TRUE);
	pDiagram->GetAxisY()->GetGridLines()->GetMinorLineStyle()->SetDashStyle(xtpChartDashStyleDot);
	//pDiagram->GetAxisY()->GetGridLines()->SetGridColor(CXTPChartColor::White);
	pDiagram->GetAxisY()->GetTitle()->SetText(_T("US Dollars"));
	pDiagram->GetAxisY()->GetTitle()->SetVisible(TRUE);
	pDiagram->GetAxisY()->SetColor(CXTPChartColor::White);
	pDiagram->GetAxisY()->GetTitle()->SetTextColor(CXTPChartColor::White);
	pDiagram->GetAxisY()->GetLabel()->SetTextColor(CXTPChartColor::White);

	pDiagram->GetAxisY()->GetRange()->SetShowZeroLevel(FALSE);
	
	pDiagram->GetAxisX()->GetLabel()->SetAngle(360-30);
	pDiagram->GetAxisX()->GetLabel()->SetAntialiasing(TRUE);
	pDiagram->GetAxisX()->SetColor(CXTPChartColor::White);
	pDiagram->GetAxisX()->GetTitle()->SetTextColor(CXTPChartColor::White);
	pDiagram->GetAxisX()->GetLabel()->SetTextColor(CXTPChartColor::White);


	pSeries->SetArgumentScaleType(xtpChartScaleQualitative);
	pDiagram->GetAxisX()->GetLabel()->SetVisible(TRUE);


	pDiagram->GetAxisX()->GetCustomLabels()->RemoveAll();

	CXTPChartSeriesPointCollection* pPoints = pSeries->GetPoints();
	for (int i = 20 - 1; i > 0; --i)
	{
		CString str="";
		str.Format("a-%d",i);
		pPoints->Add(new CXTPChartSeriesPoint(str, rand()%20));
		/*AddDate(pSeries,
			str,
			rand()%20,
			rand()%20,
			rand()%20,
			rand()%20);*/

		
		/*if ((i % 2) == 0)
		{
			CXTPChartAxisCustomLabel* pLabel = new CXTPChartAxisCustomLabel();
			pLabel->SetAxisValue(str);
			pLabel->SetText("label");
			pDiagram->GetAxisX()->GetCustomLabels()->Add(pLabel);
		}*/
	}
}

void CDemoBCGGridDlg::UpdateHistory()
{
/*	m_ToolKitChart.Attach(m_wndToolKitChart.GetSafeHwnd());
	CXTPChartContent* pContent = m_ToolKitChart.GetContent();

	pContent->GetLegend()->SetVisible(TRUE);

	CXTPChartTitle* pTitle = pContent->GetTitles()->Add(new CXTPChartTitle());
	pTitle->SetText(_T("ToolKit Chart"));

	CXTPChartSeriesCollection* pCollection = pContent->GetSeries();
	pCollection->RemoveAll();

	CXTPChartSeries* pSeries = pCollection->Add(new CXTPChartSeries());
	CXTPChartFastLineSeriesStyle* pStyle = new CXTPChartFastLineSeriesStyle();
	pSeries->SetStyle(pStyle);
	//pStyle->GetLabel()->SetVisible(1);
	pStyle->SetAntialiasing(0);				
	pSeries->SetArgumentScaleType(xtpChartScaleNumerical);
	CXTPChartDiagram2D* pDiagram = DYNAMIC_DOWNCAST(CXTPChartDiagram2D, pCollection->GetAt(0)->GetDiagram());
	ASSERT (pDiagram);

	/*pDiagram->SetAllowZoom(FALSE);
	pDiagram->GetAxisX()->SetAllowZoom(TRUE);
	pDiagram->GetAxisY()->SetAllowZoom(TRUE);
	pDiagram->GetAxisY()->GetRange()->SetZoomLimit(10);
	pDiagram->GetAxisX()->GetRange()->SetAutoRange(0);*/
/*	CXTPChartAxis* pAxisX = pDiagram->GetAxisX();
	if (pAxisX)
	{
		CXTPChartAxisTitle* pTitle = pAxisX->GetTitle();
		if (pTitle)
		{
			pTitle->SetText("X Axis");
			pTitle->SetVisible(TRUE);
		}
	}
	CXTPChartAxis* pAxisY = pDiagram->GetAxisY();
	if (pAxisY)
	{
		CXTPChartAxisTitle* pTitle = pAxisY->GetTitle();
		if (pTitle)
		{
			pTitle->SetText("Y Axis");
			pTitle->SetVisible(TRUE);
		}
	}
	if (pCollection)
	{
		AfxMessageBox("MARK");
		for(int i=0;i<100;i++)
		{
			CXTPChartSeries* pSeries = pCollection->GetAt(0);
			int nCount=0;
			if (pSeries)
			{
				int nValue = 50;

				nCount = pSeries->GetPoints()->GetCount();
				
				if (nCount)
					nValue = (int)pSeries->GetPoints()->GetAt(nCount - 1)->GetValue(0);
				
				nValue = nValue + (rand() % 20) - 10;
				
				if (nValue < 0) nValue = 0;
				if (nValue > 100) nValue = 100;
				
				pSeries->GetPoints()->Add(new CXTPChartSeriesPoint(nCount, nValue));

			}
			if (nCount > 100)
			{
				CXTPChartAxisRange* pRange = pDiagram->GetAxisX()->GetRange();
				
				BOOL bAutoScroll = pRange->GetViewMaxValue() == pRange->GetMaxValue();
				
				pRange->SetMaxValue(nCount);
				
				if (bAutoScroll)
				{
					double delta = pRange->GetViewMaxValue() - pRange->GetViewMinValue();
					
					pRange->SetViewAutoRange(FALSE);
					pRange->SetViewMaxValue(nCount);
					pRange->SetViewMinValue(nCount - delta);
				}
				
			}
		}
	}
	else
		AfxMessageBox("err");*/
	/*if (pCollection)
	{
		if (pSeries)
		{
			pSeries->SetName(_T("成田日上"));
			CXTPChartSeriesPointCollection* pPoints = pSeries->GetPoints();
			if (pPoints)
			{
				CXTPChartSeriesPoint* pPoint = NULL;
				for(int i=0;i<100;i++)
				{
					CString str="";
					str.Format("a-%d",i);
					pPoint = pPoints->Add(new CXTPChartSeriesPoint(i, rand()%100));
				}
			}

			//pSeries->SetStyle(new CXTPChartLineSeriesStyle());
		}
	}*/



	m_ToolKitChart.Attach(m_wndToolKitChart.GetSafeHwnd());	
	CXTPChartContent* pContent = m_ToolKitChart.GetContent();
	m_ToolKitChart.GetContent()->SetBackgroundColor(CXTPChartColor(255,255,255,255));

	CXTPChartTitleCollection* pTitles = m_ToolKitChart.GetContent()->GetTitles();
	CXTPChartTitle* pTitle = pTitles->Add(new CXTPChartTitle());
	pTitle->SetText(_T("Fast Line"));
	pTitle->SetTextColor(CXTPChartColor::Black);
	
	CXTPChartTitle* pSubTitle = m_ToolKitChart.GetContent()->GetTitles()->Add(new CXTPChartTitle());
	pSubTitle->SetText(_T("Use mouse wheel to zoom chart"));
	pSubTitle->SetDocking(xtpChartDockBottom);
	pSubTitle->SetAlignment(xtpChartAlignFar);
	pSubTitle->SetFont(CXTPChartFont::GetTahoma8());
	pSubTitle->SetTextColor(CXTPChartColor::Black);

	
	CXTPChartSeriesCollection* pCollection = pContent->GetSeries();
	pCollection->RemoveAll();
	CXTPChartSeries* pSeries = NULL;
	if (pCollection)
	{
		for (int s = 0; s < 1; s++)
		{
			pSeries = pCollection->Add(new CXTPChartSeries());
			if (pSeries)
			{
				pSeries->SetName(_T("Series"));				
				
				CXTPChartFastLineSeriesStyle* pStyle = new CXTPChartFastLineSeriesStyle();
				//pStyle->GetLabel()->SetVisible(0);
				//pStyle->GetMarker()->SetVisible(0);
				pSeries->SetStyle(pStyle);
				
				pStyle->SetAntialiasing(0);				
				
				pSeries->SetArgumentScaleType(xtpChartScaleNumerical);
			}
		}
	}
	// Set the X and Y Axis title for the series.
	//CXTPChartDiagram2D* pDiagram = DYNAMIC_DOWNCAST(CXTPChartDiagram2D, pCollection->GetAt(0)->GetDiagram());
	CXTPChartDiagram2D* pDiagram = (CXTPChartDiagram2D*)pSeries->GetDiagram();
	ASSERT (pDiagram);
	pDiagram->GetAxisY()->GetTitle()->SetText(_T("Y Axis"));
	pDiagram->GetAxisY()->GetTitle()->SetVisible(TRUE);
	pDiagram->GetAxisY()->SetColor(CXTPChartColor::Black);

	pDiagram->GetAxisX()->GetTitle()->SetText(_T("X Axis"));
	pDiagram->GetAxisX()->GetTitle()->SetVisible(TRUE);
	pDiagram->GetAxisX()->SetColor(CXTPChartColor::Black);

	pDiagram->SetAllowZoom(TRUE);
	
	pDiagram->GetAxisY()->GetRange()->SetMaxValue(100.1);
	pDiagram->GetAxisY()->GetRange()->SetAutoRange(FALSE);

	pDiagram->GetAxisY()->GetTitle()->SetText(_T("Y Axis"));
	pDiagram->GetAxisY()->GetTitle()->SetVisible(TRUE);
	pDiagram->GetAxisY()->SetColor(CXTPChartColor::Black);
	pDiagram->GetAxisY()->GetTitle()->SetTextColor(CXTPChartColor::Black);
	pDiagram->GetAxisY()->GetLabel()->SetTextColor(CXTPChartColor::Black);
	
	pDiagram->GetAxisX()->GetRange()->SetMaxValue(100.1);
	pDiagram->GetAxisX()->GetRange()->SetAutoRange(FALSE);
	pDiagram->GetAxisX()->GetRange()->SetZoomLimit(10);

	pDiagram->GetAxisX()->GetTitle()->SetText(_T("X Axis"));
	pDiagram->GetAxisX()->GetTitle()->SetVisible(TRUE);
	pDiagram->GetAxisX()->SetColor(CXTPChartColor::Black);
	pDiagram->GetAxisX()->GetTitle()->SetTextColor(CXTPChartColor::Black);
	pDiagram->GetAxisX()->GetLabel()->SetTextColor(CXTPChartColor::Black);
	pDiagram->GetAxisX()->SetAllowZoom(FALSE);
	
	pDiagram->GetAxisX()->SetInterlaced(FALSE);
	pDiagram->GetAxisY()->SetInterlaced(FALSE);	
	
	pDiagram->GetPane()->GetFillStyle()->SetFillMode(xtpChartFillSolid);



	for(int i=0;i<1000;i++)
	{
		CXTPChartContent* pContent = m_ToolKitChart.GetContent();
		
		CXTPChartSeriesCollection* pCollection = pContent->GetSeries();
		
		int nCount;
		
		if (pCollection)
		{
			for (int s = 0; s < pCollection->GetCount(); s++)
			{
				CXTPChartSeries* pSeries = pCollection->GetAt(s);
				if (pSeries)
				{
					int nValue = 50;
					
					nCount = pSeries->GetPoints()->GetCount();
					
					if (nCount)
						nValue = (int)pSeries->GetPoints()->GetAt(nCount - 1)->GetValue(0);
					
					nValue = nValue + (rand() % 20) - 10;
					
					if (nValue < 0) nValue = 0;
					if (nValue > 100) nValue = 100;
					CString str=_T("");
					str.Format(_T("a-%d"),i);
					
					pSeries->GetPoints()->Add(new CXTPChartSeriesPoint(str, nValue));
					
				}
			}
		}
					//CXTPChartAxisCustomLabel* pLabel = new CXTPChartAxisCustomLabel();
					//pLabel->SetAxisValue(2);
					//pLabel->SetText("00");
					//pDiagram->GetAxisX()->GetCustomLabels()->Add(pLabel);
		
		CXTPChartDiagram2D* pDiagram = DYNAMIC_DOWNCAST(CXTPChartDiagram2D, 
			m_ToolKitChart.GetContent()->GetPanels()->GetAt(0));
		ASSERT (pDiagram);
		
		
		if (nCount > 100)
		{
			CXTPChartAxisRange* pRange = pDiagram->GetAxisX()->GetRange();
			
			BOOL bAutoScroll = pRange->GetViewMaxValue() == pRange->GetMaxValue();
			
			pRange->SetMaxValue(nCount);
			
			if (bAutoScroll)
			{
				double delta = pRange->GetViewMaxValue() - pRange->GetViewMinValue();
				
				pRange->SetViewAutoRange(FALSE);
				pRange->SetViewMaxValue(nCount);
				pRange->SetViewMinValue(nCount - delta);
			}
			
		}
	}
}
