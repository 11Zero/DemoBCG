//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPImageProcessing.cpp: implementation of the image processing classes.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPImageProcessing.h"
#include "BCGPDrawManager.h"
#include "BCGPMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const long c_BrightnessLimit = 150;
static const long c_ContrastLimit   = 100;
static const long c_OpacityLimit    = 100;

static double SinC(double dValue)
{
	if (dValue != 0.0)
	{
		dValue *= M_PI;
		return sin (dValue) / dValue;
	}

	return 1.0;
}

static double Filter_Box(double dValue)
{
	if ((dValue > -0.5) && (dValue <= 0.5))
	{
		return 1.0;
	}

	return 0.0;
}

static double Filter_Bilinear(double dValue)
{
	if (dValue < 0.0)
	{
		dValue = -dValue;
	}

	if (dValue < 1.0)
	{
		return 1.0 - dValue;
	}

	return 0.0;
}

static double Filter_Bicubic(double dValue)
{
	// f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1
	if (dValue < 0.0)
	{
		dValue = -dValue;
	}

	if (dValue < 1.0)
	{
		return (2.0 * dValue - 3.0) * dValue * dValue + 1.0;
	}

	return 0.0;
}

static double Filter_Bell(double dValue)
{
	if (dValue < 0.0)
	{
		dValue = -dValue;
	}

	if (dValue < 0.5)
	{
		return 0.75 - dValue * dValue;
	}
	else
	{
		if (dValue < 1.5)
		{
			dValue = dValue - 1.5;
			return 0.5 * dValue * dValue;
		}
	}

	return 0.0;
}

static double Filter_BSpline(double dValue)
{
	if (dValue < 0.0)
	{
		dValue = -dValue;
	}

	if (dValue < 1.0)
	{
		double tt = dValue * dValue;
		return 0.5 * tt * dValue - tt + 2.0 / 3.0;
	}
	else
	{
		if (dValue < 2.0)
		{
			dValue = 2.0 - dValue;
			return dValue * dValue * dValue / 6.0;
		}
	}

	return 0.0;
}

static double Filter_Lanczos3(double dValue)
{
	if (dValue < 0.0)
	{
		dValue = -dValue;
	}

	if (dValue < 3.0)
	{
		return SinC(dValue) * SinC(dValue / 3.0);
	}

	return 0.0;
}

static double Filter_Mitchell(double dValue)
{
	static double B = 1.0 / 3.0;
	static double C = B;

	if (dValue < 0.0)
	{
		dValue = -dValue;
	}

	const double tt = dValue * dValue;
	if (dValue < 1.0)
	{
		return ((12.0 - 9.0 * B - 6.0 * C) * (dValue * tt) + 
				(-18.0 + 12.0 * B + 6.0 * C) * tt + 
				(6.0 - 2.0 * B)) / 6.0;
	}
	else
	{
		if (dValue < 2.0)
		{
			return ((-1.0 * B - 6.0 * C) *(dValue * tt) +
					(6.0 * B + 30.0 * C) * tt + 
					(-12.0 * B - 48.0 * C) * dValue +
					(8.0 * B + 24.0 * C)) / 6.0;
		}
	}

	return 0.0;
}


struct CBCGPImageResizeFilter
{
	CBCGPZoomKernel::XLPFilterProc Proc;
	double Width;
};

static const CBCGPImageResizeFilter Filters[7] =
{
	{ &Filter_Box	  , 0.5},
	{ &Filter_Bilinear, 1.0},
	{ &Filter_Bicubic , 1.0},
	{ &Filter_Bell	  , 1.5},
	{ &Filter_BSpline , 2.0},
	{ &Filter_Lanczos3, 3.0},
	{ &Filter_Mitchell, 2.0}
};


CBCGPScanliner::CBCGPScanliner()
{
	Empty();
}

CBCGPScanliner::CBCGPScanliner
(
	LPBYTE data,
	const CSize& size,
	size_t height/* = 0*/,
	size_t pitch/*  = 0*/,
	BYTE channels/* = 1*/,
	BOOL invert/*   = FALSE*/
)
{
	Attach(data, size, height, pitch, channels, invert);
}

CBCGPScanliner::CBCGPScanliner
(
	LPBYTE data,
	const CRect& rect,
	size_t height/* = 0*/,
	size_t pitch/*  = 0*/,
	BYTE channels/* = 1*/,
	BOOL invert/*   = FALSE*/
)
{
	Attach(data, rect, height, pitch, channels, invert);
}

CBCGPScanliner::~CBCGPScanliner()
{
	Empty();
}

BOOL CBCGPScanliner::Attach
(
	LPBYTE data,
	const CSize& size,
	size_t height/* = 0*/,
	size_t pitch/*  = 0*/,
	BYTE channels/* = 1*/,
	BOOL invert/*   = FALSE*/
)
{
	return Attach(data, CRect(CPoint(0, 0), size), height, pitch, channels, invert);
}

BOOL CBCGPScanliner::Attach
(
	LPBYTE data,
	const CRect& rect,
	size_t height/* = 0*/,
	size_t pitch/*  = 0*/,
	BYTE channels/* = 1*/,
	BOOL invert/*   = FALSE*/
)
{
	Empty();

	if (data == NULL)
	{
		ASSERT(data != NULL);
		return FALSE;
	}

	CPoint point(rect.TopLeft());
	CSize size(rect.Size());
	if(pitch == 0)
	{
		pitch = size.cx;
	}
	if(height == 0)
	{
		height = point.y + size.cy;
	}

	if (pitch < (size_t)size.cx)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (height < (size_t)(point.y + size.cy))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_rows	   = size.cy;
	m_cols	   = size.cx * channels;
	m_pitch    = (DWORD)pitch;
	m_offset   = (int)m_pitch;
	if (invert)
	{
		m_offset = -m_offset;
	}
	m_channels = channels;
	m_height   = (DWORD)height;

	m_start_row = point.y;
	m_start_col = point.x;

	m_line_begin = _begin(data);
	m_line_end	 = _end(data);
	m_line		 = m_line_begin;

	return TRUE;
}

void CBCGPScanliner::Empty()
{
	m_line		= NULL;
	m_pitch 	= 0;
	m_start_row = 0;
	m_start_col = 0;
	m_rows		= 0;
	m_cols		= 0;
	m_offset	= 0;
	m_height	= 0;

	m_line_begin = NULL;
	m_line_end	 = NULL;
}

LPBYTE CBCGPScanliner::_begin(LPBYTE data) const
{
	LPBYTE line = data;

	if(m_offset > 0)
	{
		line += m_start_row * m_pitch;
	}
	else
	{
		line += (m_height - m_start_row - 1) * m_pitch;
	}

	if(m_start_col != 0)
	{
		line += m_start_col * m_channels;
	}

	return line;
}

LPBYTE CBCGPScanliner::_end(LPBYTE data) const
{
	LPBYTE line = data;

	if(m_offset > 0)
	{
		line += (m_start_row + m_rows - 1) * m_pitch;
	}
	else
	{
		line += (m_height - m_start_row - m_rows) * m_pitch;
	}

	if(m_start_col != 0)
	{
		line += m_start_col * m_channels;
	}

	return line;
}


CBCGPScanlinerBitmap::CBCGPScanlinerBitmap()
{
	Empty();
}

CBCGPScanlinerBitmap::~CBCGPScanlinerBitmap()
{

}

BOOL CBCGPScanlinerBitmap::Attach(HBITMAP bitmap, const CPoint& ptBegin/* = CPoint(0, 0)*/)
{
	if (bitmap == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CSize size (0, 0);
	int channels = 0;
	LPBYTE lpBits = NULL;

	DIBSECTION dib;
	if (::GetObject (bitmap, sizeof (DIBSECTION), &dib) == 0 || 
		dib.dsBm.bmBits == NULL || dib.dsBmih.biBitCount < 8)
	{
		BITMAP bmp;
		if (::GetObject (bitmap, sizeof (BITMAP), &bmp) == 0 ||
			bmp.bmBits == 0 || bmp.bmBitsPixel < 8)
		{
			return FALSE;
		}
		else
		{
			size.cx  = bmp.bmWidth;
			size.cy  = bmp.bmHeight;
			channels = bmp.bmBitsPixel / 8;
			lpBits   = (LPBYTE)bmp.bmBits;
		}
	}
	else
	{
		size.cx  = dib.dsBmih.biWidth;
		size.cy  = dib.dsBmih.biHeight;
		channels = dib.dsBmih.biBitCount / 8;
		lpBits   = (LPBYTE)dib.dsBm.bmBits;
	}

	BOOL bInvert = size.cy > 0;
	size.cy = abs (size.cy);
	CRect rect(CPoint(0, 0), size);
	rect.IntersectRect(CRect(ptBegin, size), rect);

	int pitch = channels * size.cx;
	if (pitch % 4)
	{
		pitch += 4 - (pitch % 4);
	}

	return CBCGPScanliner::Attach (lpBits, size, size.cy, pitch, (BYTE) channels, bInvert);
}


CBCGPTransferFunction::CBCGPTransferFunction()
	: m_InputValueMin (0.0)
	, m_InputValueMax (255.0)
	, m_InputValueStep(1.0)
	, m_OutputValueMin(0.0)
	, m_OutputValueMax(255.0)
{
}

CBCGPTransferFunction::~CBCGPTransferFunction()
{
}

void CBCGPTransferFunction::SetInput(double valueMin, double valueMax, double step)
{
	ASSERT(valueMin < valueMax);
	ASSERT(step != 0.0);

	m_InputValueMin  = valueMin;
	m_InputValueMax  = valueMax;
	m_InputValueStep = step;

	Clear();
}

void CBCGPTransferFunction::SetOutput(double valueMin, double valueMax)
{
	ASSERT(valueMin < valueMax);

	m_OutputValueMin = valueMin;
	m_OutputValueMax = valueMax;
}

void CBCGPTransferFunction::AddPoint(double point, double value)
{
	ASSERT(m_InputValueMin <= point && point <= m_InputValueMax);

	int index = (int)m_Points.GetSize();

	BOOL add = TRUE;
	if (index > 0)
	{
		for (int i = 0; i < index; i++)
		{
			if (point == m_Points[i].m_Point)
			{
				m_Points[i].m_Value = value;
				add = FALSE;
				break;
			}
			else if (point < m_Points[i].m_Point)
			{
				m_Points.InsertAt(i, XPoint(point, value));
				add = FALSE;
				break;
			}
		}
	}

	if (add)
	{
		m_Points.Add(XPoint(point, value));
	}
}

void CBCGPTransferFunction::Clear()
{
	m_Points.RemoveAll();
}

void CBCGPTransferFunction::Calculate(CArray<double, double>& output) const
{
	output.RemoveAll();

	int size = (int)m_Points.GetSize();
	if (size == 0)
	{
		return;
	}

	int count = (int)fabs((m_InputValueMax - m_InputValueMin) / m_InputValueStep) + 1;
	if (count == 1)
	{
		return;
	}

	output.SetSize(count);

	XPoint pt(m_Points[0]);
	if (m_InputValueMin < pt.m_Point)
	{
		for (int i = 0; i <= GetIndex(pt.m_Point); i++)
		{
			output[i] = pt.m_Value;
		}
	}

	if (size > 1)
	{
		pt = m_Points[size - 1];
	}

	if (pt.m_Point < m_InputValueMax)
	{
		for (int i = GetIndex(pt.m_Point); i < count; i++)
		{
			output[i] = pt.m_Value;
		}
	}

	// linear
	size = (int)m_Points.GetSize ();
	if (size < 2)
	{
		return;
	}

	for(long k = size - 1; k >= 1; k--)
	{
		CArray<double, double> points;

		XPoint pt1(m_Points[k - 1]);
		XPoint pt2(m_Points[k]);

		int index1 = GetIndex(pt1.m_Point);
		int index2 = GetIndex(pt2.m_Point);
		double dY = (pt2.m_Value - pt1.m_Value) / (double)(index2 - index1);
		double value = pt1.m_Value;
		for(int i = index1; i <= index2; i++)
		{
			points.Add(bcg_clamp(value, m_OutputValueMin, m_OutputValueMax));
			value += dY;
		}

		if(points.GetSize() <= 2)
		{
			continue;
		}

		int kInsert = index1;
		for(int kk = 0; kk <= points.GetSize() - 1; kk++)
		{
			output[kInsert++] = points[kk];
		}
	}
}


CBCGPColorTableFunction::CBCGPColorTableFunction(int size)
	: m_Colors(NULL)
{
	m_Size = bcg_clamp (size, 0, 256);
	ASSERT(m_Size > 1);

	m_R.SetInput (0, m_Size - 1, 1);
	m_G.SetInput (0, m_Size - 1, 1);
	m_B.SetInput (0, m_Size - 1, 1);
}

CBCGPColorTableFunction::~CBCGPColorTableFunction()
{
	if (m_Colors != NULL)
	{
		delete [] m_Colors;
	}
}

void CBCGPColorTableFunction::Calculate()
{
	if (m_Size == 0)
	{
		ASSERT(FALSE);
		return;
	}

	if (m_Colors != NULL)
	{
		delete [] m_Colors;
	}

	m_Colors = new RGBQUAD[m_Size];

	CArray<double, double> ar;
	m_R.Calculate(ar);
	CArray<double, double> ag;
	m_G.Calculate(ag);
	CArray<double, double> ab;
	m_B.Calculate(ab);

	for (int i = 0; i < m_Size; i++)
	{
		m_Colors[i].rgbRed      = (BYTE)ar[i];
		m_Colors[i].rgbGreen    = (BYTE)ag[i];
		m_Colors[i].rgbBlue     = (BYTE)ab[i];
		m_Colors[i].rgbReserved = 0;
	}
}


CBCGPColorLookupTable::CBCGPColorLookupTable()
{
	Initialize();
}

CBCGPColorLookupTable::CBCGPColorLookupTable(const CBCGPColorLookupTable& rLut)
{
	memcpy(Get(), rLut.Get(), 256 * 3);
}

CBCGPColorLookupTable::~CBCGPColorLookupTable()
{
}

CBCGPColorLookupTable& CBCGPColorLookupTable::operator = (const CBCGPColorLookupTable& rLut)
{
	memcpy(Get(), rLut.Get(), 256 * 3);
	return *this;
}

void CBCGPColorLookupTable::Initialize()
{
	for(int i = 0; i < 256; i ++)
	{
		m_Table[0][i] =(BYTE)i;
	}

	memcpy(Get(1), Get(0), 256);
	memcpy(Get(2), Get(1), 256);
}

void CBCGPColorLookupTable::Invert()
{
	for (int i = 0; i < 256; i ++)
	{
		m_Table[0][i] = (BYTE)(255 - m_Table[0][i]);
		m_Table[1][i] = (BYTE)(255 - m_Table[1][i]);
		m_Table[2][i] = (BYTE)(255 - m_Table[2][i]);
	}
}

void CBCGPColorLookupTable::SetBrightness(long value)
{
	value = bcg_clamp(value, -c_BrightnessLimit, c_BrightnessLimit);
	if (value == 0)
	{
		return;
	}

	for(int i = 0; i < 256; i ++)
	{
		m_Table[0][i] = (BYTE)bcg_clamp_to_byte(m_Table[0][i] + value);
		m_Table[1][i] = (BYTE)bcg_clamp_to_byte(m_Table[1][i] + value);
		m_Table[2][i] = (BYTE)bcg_clamp_to_byte(m_Table[2][i] + value);
	}
}

void CBCGPColorLookupTable::SetContrast(long value)
{
	SetContrast(value, 127.5, 127.5, 127.5);
}

void CBCGPColorLookupTable::SetContrast(long value, double midR, double midG, double midB)
{
	value = bcg_clamp(value, -c_ContrastLimit, c_ContrastLimit);
	if (value == 0)
	{
		return;
	}

	const double c_Value = 1.0 + bcg_sign(value) * bcg_sqr((double)value / (c_ContrastLimit * 0.5)) / 4.0;

	for (int i = 0; i < 256; i ++)
	{
		m_Table[0][i] = (BYTE)bcg_clamp_to_byte(bcg_round(midB + ((double)m_Table[0][i] - midB) * c_Value));
		m_Table[1][i] = (BYTE)bcg_clamp_to_byte(bcg_round(midG + ((double)m_Table[1][i] - midG) * c_Value));
		m_Table[2][i] = (BYTE)bcg_clamp_to_byte(bcg_round(midR + ((double)m_Table[2][i] - midR) * c_Value));
	}
}

void CBCGPColorLookupTable::SetGamma(double value)
{
	if (value == 0.0)
	{
		ASSERT(FALSE);
		return;
	}

	value = 1.0 / bcg_clamp(value, 0.5, 6.0);

	for(int i = 0; i < 256; i ++)
	{
		m_Table[0][i] = (BYTE)bcg_clamp_to_byte(bcg_round(pow(m_Table[0][i] / 255.0, value) * 255.0));
		m_Table[1][i] = (BYTE)bcg_clamp_to_byte(bcg_round(pow(m_Table[1][i] / 255.0, value) * 255.0));
		m_Table[2][i] = (BYTE)bcg_clamp_to_byte(bcg_round(pow(m_Table[2][i] / 255.0, value) * 255.0));
	}
}

void CBCGPColorLookupTable::SetColor(COLORREF clr, long opacity)
{
	opacity = bcg_clamp(opacity, 0, c_OpacityLimit);
	if (opacity == 0)
	{
		return;
	}

	double dFillH, dFillS, dFillL;
	CBCGPDrawManager::RGBtoHSL(clr, &dFillH, &dFillS, &dFillL);

	dFillL = RGB_LUM_COLOR_D(clr) / 255.0;
	dFillS *= opacity / 100.0;

	for(int i = 0; i < 256; i ++)
	{
		double dL = RGB_LUM_D(m_Table[2][i], m_Table[1][i], m_Table[0][i]) / 127.5;

		if (dL <= 1.0)
		{
			dL *= dFillL;
		}
		else
		{
			dL = (1.0 - dFillL) * (dL - 1.0) + dFillL;
		}

		clr = CBCGPDrawManager::HLStoRGB_ONE(dFillH, dL, dFillS);

		m_Table[2][i] = GetRValue(clr);
		m_Table[1][i] = GetGValue(clr);
		m_Table[0][i] = GetBValue(clr);
	}
}


CBCGPZoomKernel::XLPFilterProc CBCGPZoomKernel::FilterProc(CBCGPZoomKernel::XFilterType ft)
{
	return Filters[ft].Proc;
}

double CBCGPZoomKernel::FilterWidth(CBCGPZoomKernel::XFilterType ft)
{
	return Filters[ft].Width;
}

double CBCGPZoomKernel::Filter(CBCGPZoomKernel::XFilterType ft, double value)
{
	return Filters[ft].Proc(value);
}

void CBCGPZoomKernel::CorrectZoomSize(const CSize& sizeSrc, CSize& sizeDst, XZoomType zt)
{
	double ZoomX = (double)sizeDst.cx / (double)sizeSrc.cx;
	double ZoomY = (double)sizeDst.cy / (double)sizeSrc.cy;

	if(zt != e_ZoomTypeStretch)
	{
		switch(zt)
		{
		case e_ZoomTypeFitWidth:
			ZoomY = ZoomX;
			break;
		case e_ZoomTypeFitHeight:
			ZoomX = ZoomY;
			break;
		case e_ZoomTypeFitImage:
			ZoomX = min (ZoomX, ZoomY);
			ZoomY = ZoomX;
			break;
		}

		sizeDst.cx = (int)(sizeSrc.cx * ZoomX);
		sizeDst.cy = (int)(sizeSrc.cy * ZoomY);
	}
}

CBCGPZoomKernel::CBCGPZoomKernel()
	: m_Size (0)
	, m_List (NULL)
{
}

CBCGPZoomKernel::~CBCGPZoomKernel()
{
	Empty();
}

void CBCGPZoomKernel::Create(int sizeSrc, int sizeDst, int originSrc, int widthSrc, XFilterType ft)
{
	if(sizeSrc <= 0)
	{
		ASSERT(FALSE);
		return;
	}

	if(sizeDst <= 0)
	{
		ASSERT(FALSE);
		return;
	}

	Empty();

	m_Size = sizeDst;
	const double dScale = (double)(m_Size) / (double)(sizeSrc);

	const XLPFilterProc lpFilterProc = Filters[ft].Proc;
	const double dFilterWidth		 = Filters[ft].Width;

	m_List = new XKernelList[m_Size];

	double width = dFilterWidth;
	double scale = 1.0;
	double correction = -0.25;
	if (dScale < 1.0)
	{
		width /= dScale;
		scale = dScale;
		correction = -correction;
	}

	for (DWORD i = 0; i < m_Size; i++)
	{
		double center = i / dScale;

		int left  = (int)floor(center - width);
		int right = (int)ceil(center + width);

		const int c_Count = right - left + 1;

		m_List[i].count = 0;

		if (c_Count == 0)
		{
			continue;
		}

		m_List[i].stat	= new XKernel[c_Count];

		BOOL bCross = FALSE;
		DWORD index = 0;
		double weightSum = 0.0;

		XKernel* pStat = m_List[i].stat;

		BOOL bFirst = TRUE;
		for(int j = left; j <= right; j++)
		{
			double weight = lpFilterProc((center - (double)j + correction) * scale) * scale;
			if(weight == 0.0)
			{
				if (!bFirst)
				{
					break;
				}

				continue;
			}

			bFirst = FALSE;

			int pixel = j + originSrc;
			if (pixel < 0)
			{
				pixel = (int)(pixel / widthSrc) * widthSrc - pixel;
				bCross = TRUE;
			}
			else if (pixel >= widthSrc)
			{
				pixel = widthSrc - (pixel - (int)(pixel / widthSrc) * widthSrc) - 1;
				bCross = TRUE;
			}

			BOOL bFound = FALSE;
			if(bCross)
			{
				for(DWORD k = 0; k < index; k++)
				{
					if(pStat[k].pixel == pixel)
					{
						pStat[k].weight += weight;
						bFound = TRUE;
						break;
					}
				}
			}

			if(!bFound)
			{
				pStat[index].pixel	= pixel;
				pStat[index].weight = weight;
				index++;
				m_List[i].count = index;
			}

			weightSum += weight;
		}

		if(weightSum != 0.0)
		{
			for(DWORD j = 0; j < m_List[i].count; j++)
			{
				m_List[i].stat[j].weight /= weightSum;
			}
		}
	}
}

void CBCGPZoomKernel::Create(int sizeSrc, int sizeDst, XFilterType ft)
{
	Create(sizeSrc, sizeDst, 0, sizeSrc, ft);
}

void CBCGPZoomKernel::Empty()
{
	if (m_List != NULL)
	{
		for(DWORD i = 0; i < m_Size; i++)
		{
			if (m_List[i].count > 0)
			{
				delete [] m_List[i].stat;
			}
		}

		delete [] m_List;

		m_List = NULL;
		m_Size = 0;
	}
}


class CMemoryDC
{
public:
	CMemoryDC();
	virtual ~CMemoryDC();

	void CreateDC ();
	virtual void SetSize (const CSize& size);
	virtual void Update ();

	const CSize& GetSize () const
	{
		return m_Size;
	}

	CDC& GetDC ()
	{
		return m_DC;
	}
	const CDC& GetDC () const
	{
		return m_DC;
	}

	CBitmap& GetBitmap ()
	{
		return m_Bitmap;
	}
	const CBitmap& GetBitmap () const
	{
		return m_Bitmap;
	}

protected:
	CDC			m_DC;
	CBitmap		m_Bitmap;
	CBitmap*	m_pOldBitmap;
	
	CSize		m_Size;
};

CMemoryDC::CMemoryDC()
	: m_pOldBitmap (NULL)
	, m_Size       (0, 0)
{

}

CMemoryDC::~CMemoryDC()
{

}

void CMemoryDC::CreateDC ()
{
	if (m_DC.GetSafeHdc () != NULL)
	{
		return;
	}

	HDC hDC = ::GetDC (NULL);

	HDC hNewDC = ::CreateCompatibleDC (hDC);
	if (hNewDC != NULL)
	{
		m_DC.Attach (hNewDC);
	}

	::ReleaseDC (NULL, hDC);
}

void CMemoryDC::SetSize (const CSize& size)
{
	if (m_DC.GetSafeHdc () == NULL)
	{
		CreateDC ();
	}

	if (m_Bitmap.GetSafeHandle () != NULL)
	{
		if (m_Size.cx != size.cx || m_Size.cy != size.cy)
		{
			if (m_pOldBitmap != NULL)
			{
				m_DC.SelectObject (m_pOldBitmap);
			}

			m_Bitmap.DeleteObject ();
		}
	}

	m_Size = size;

	if (m_Bitmap.GetSafeHandle () == NULL)
	{
		HBITMAP hbmp = CBCGPDrawManager::CreateBitmap_32 (size, NULL);
		if (hbmp != NULL)
		{
			m_Bitmap.Attach (hbmp);
			m_pOldBitmap = (CBitmap*) m_DC.SelectObject (&m_Bitmap);
		}
	}

	if (m_Bitmap.GetSafeHandle () != NULL)
	{
		Update ();
	}
}

void CMemoryDC::Update ()
{
}


void BCGPResizeImage32
(
	CBCGPScanliner& md,
	int nXOriginDst,
	int nYOriginDst,
	int nWidthDst,
	int nHeightDst,
	const CBCGPScanliner& ms,
	const CBCGPZoomKernel& KernelX,
	const CBCGPZoomKernel& KernelY
)
{
	DWORD channel = ms.GetChannels ();
	if (channel < 4)
	{
		ASSERT(FALSE);
		return;
	}

	double* values  = new double[channel];
	double* values2 = new double[channel];

	const DWORD val_size   = sizeof(double) * channel;
	const DWORD offsetDstX = nXOriginDst * channel;

	for (DWORD dy = 0; dy < (DWORD)nHeightDst; dy++)
	{
		const CBCGPZoomKernel::XKernelList& listY = KernelY[dy];

		LPBYTE pRowDst = md[dy + nYOriginDst] + offsetDstX;

		for (DWORD dx = 0; dx < (DWORD)nWidthDst; dx++)
		{
			const CBCGPZoomKernel::XKernelList& listX = KernelX[dx];

			memset(values, 0, val_size);

			for (DWORD sy = 0; sy < listY.count; sy++)
			{
				const CBCGPZoomKernel::XKernel& statY = listY.stat[sy];

				const LPBYTE pRowSrc = ms[statY.pixel];
				double weight    = statY.weight;

				memset(values2, 0, val_size);

				for (DWORD sx = 0; sx < listX.count; sx++)
				{
					const CBCGPZoomKernel::XKernel& statX = listX.stat[sx];

					LPBYTE pRowSrc2 = pRowSrc + statX.pixel * channel;
					double weight2    = statX.weight;

					for(DWORD c = 0; c < channel; c++)
					{
						values2[c] += (double)(*pRowSrc2) * weight2;
						pRowSrc2++;
					}
				}

				for(DWORD c = 0; c < channel; c++)
				{
					values[c] += values2[c] * weight;
				}
			}

			for(DWORD c = 0; c < channel; c++)
			{
				*pRowDst = (BYTE)bcg_clamp_to_byte(values[c]);
				pRowDst++;
			}

			if (channel == 4)
			{
				BCGPCorrectAlpha(pRowDst - 4, *(pRowDst - 1));
			}
		}
	}

	delete [] values;
	delete [] values2;
}

void BCGPResizeImage32
(
	CBCGPScanliner& md,
	int nXOriginDst,
	int nYOriginDst,
	int nWidthDst,
	int nHeightDst,
	const CBCGPScanliner& ms,
	int nXOriginSrc,
	int nYOriginSrc,
	int nWidthSrc,
	int nHeightSrc,
	CBCGPZoomKernel::XFilterType ft
)
{
	DWORD channel = ms.GetChannels ();

	if ((ms.GetColumns() / channel) < (unsigned)(nXOriginSrc + nWidthSrc) || 
		 ms.GetRows() < (unsigned)(nYOriginSrc + nHeightSrc) || 
		(md.GetColumns() / channel) < (unsigned)(nXOriginDst + nWidthDst) || 
		 md.GetRows() < (unsigned)(nYOriginDst + nHeightDst))
	{
		ASSERT(FALSE);
		return;
	}

	CBCGPZoomKernel KernelX;
	KernelX.Create(nWidthSrc, nWidthDst, nXOriginSrc, ms.GetColumns() / channel, ft);

	CBCGPZoomKernel KernelY;
	KernelY.Create(nHeightSrc, nHeightDst, nYOriginSrc, ms.GetRows(), ft);

	BCGPResizeImage32
		(
			md, nXOriginDst, nYOriginDst, nWidthDst, nHeightDst, 
			ms,
			KernelX, KernelY
		);
}

void BCGPResizeImage32
(
	CBCGPScanliner& md,
	int nWidthDst,
	int nHeightDst,
	const CBCGPScanliner& ms,
	int nWidthSrc,
	int nHeightSrc,
	CBCGPZoomKernel::XFilterType ft
)
{
	BCGPResizeImage32
		(
			md, 0, 0, nWidthDst, nHeightDst, 
			ms, 0, 0, nWidthSrc, nHeightSrc, 
			ft
		);
}

void BCGPResizeImage32
(
#ifndef _BCGSUITE_
	CBCGPToolBarImages& imageSrc,
	CBCGPToolBarImages& imageDst,
#else
	CMFCToolBarImages& imageSrc,
	CMFCToolBarImages& imageDst,
#endif
	int index,
#ifndef _BCGSUITE_
	CBCGPToolBarImages::ImageAlignHorz horz,
	CBCGPToolBarImages::ImageAlignVert vert,
#else
	CMFCToolBarImages::ImageAlignHorz horz,
	CMFCToolBarImages::ImageAlignVert vert,
#endif
	CBCGPZoomKernel::XFilterType ft
)
{
	CSize sizeSrc (imageSrc.GetImageSize ());
	CSize sizeDst (imageDst.GetImageSize ());

	imageDst.Clear ();
	imageDst.SetPreMultiplyAutoCheck (TRUE);
	imageDst.SetTransparentColor ((COLORREF) -1);
	imageDst.SetImageSize (sizeDst);

	if (sizeSrc != sizeDst)
	{
		CSize size (sizeDst);

#ifndef _BCGSUITE_
		if (horz == CBCGPToolBarImages::ImageAlignHorzStretch)
#else
		if (horz == CMFCToolBarImages::ImageAlignHorzStretch)
#endif
		{
			size.cx = sizeSrc.cx;
		}

#ifndef _BCGSUITE_
		if (vert == CBCGPToolBarImages::ImageAlignVertStretch)
#else
		if (vert == CMFCToolBarImages::ImageAlignVertStretch)
#endif
		{
			size.cy = sizeSrc.cy;
		}

		int nStart = 0;
		int nEnd   = imageSrc.GetCount ();
		if (index != -1)
		{
			nStart = index;
			nEnd   = nStart + 1;
		}

		for (int i = nStart; i < nEnd; i++)
		{
			CMemoryDC dc;
			dc.SetSize (size);
			CBitmap* bmpImage = &dc.GetBitmap ();

			imageSrc.DrawEx (&dc.GetDC (), CRect (CPoint (0, 0), size), i, horz, vert);

			CMemoryDC dcImage;

#ifndef _BCGSUITE_
			if (horz == CBCGPToolBarImages::ImageAlignHorzStretch ||
				vert == CBCGPToolBarImages::ImageAlignVertStretch)
#else
			if (horz == CMFCToolBarImages::ImageAlignHorzStretch ||
				vert == CMFCToolBarImages::ImageAlignVertStretch)
#endif
			{
				dcImage.SetSize (sizeDst);
				bmpImage = &dcImage.GetBitmap ();

				CBCGPScanlinerBitmap scanSrc;
				scanSrc.Attach ((HBITMAP) dc.GetBitmap ());
				CBCGPScanlinerBitmap scanDst;
				scanDst.Attach ((HBITMAP) dcImage.GetBitmap ());

				BCGPResizeImage32 (scanDst, sizeDst.cx, sizeDst.cy, scanSrc, size.cx, size.cy, ft);
			}

			CBitmap bitmap;
			bitmap.Attach (CBCGPDrawManager::CreateBitmap_32 (*bmpImage));

			imageDst.AddImage (bitmap, TRUE);
		}
	}
	else
	{
		imageDst.AddImage (imageSrc, index);
	}
}


HBITMAP BCGPIconToBitmap32 (HICON hIcon)
{
	if (hIcon == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	ICONINFO ii;
	::GetIconInfo (hIcon, &ii);

	CSize size;
	{
		BITMAP bmp;
		if (::GetObject (ii.hbmColor, sizeof (BITMAP), &bmp) == 0)
		{
			ASSERT (FALSE);
			return NULL;
		}

		size.cx = bmp.bmWidth;
		size.cy = bmp.bmHeight;
	}

	CMemoryDC dcColor;
	dcColor.SetSize (size);
	::DrawIconEx (dcColor.GetDC ().GetSafeHdc (), 
		0, 0, 
		hIcon, 
		size.cx, size.cy, 0, NULL,
		DI_NORMAL);

	BITMAP bmpColor;
	dcColor.GetBitmap ().GetBitmap (&bmpColor);
	RGBQUAD* pColor = (RGBQUAD*) bmpColor.bmBits;

	BOOL bConvert = TRUE;
	for (int i = 0; i < size.cx * size.cy; i++)
	{
		if (pColor[i].rgbReserved != 0)
		{
			bConvert = FALSE;
			break;
		}
	}

	if (bConvert)
	{
		CMemoryDC dcMask;
		dcMask.SetSize (size);
		::DrawIconEx (dcMask.GetDC ().GetSafeHdc (), 
			0, 0, 
			hIcon, 
			size.cx, size.cy, 0, NULL,
			DI_MASK);


		BITMAP bmpMask;
		dcMask.GetBitmap ().GetBitmap (&bmpMask);
		RGBQUAD* pMask  = (RGBQUAD*) bmpMask.bmBits;

		if (pColor == NULL || pMask == NULL)
		{
			ASSERT (FALSE);
			return NULL;
		}

		// add alpha channel
		for (int i = 0; i < size.cx * size.cy; i++)
		{
			pColor->rgbReserved = (BYTE) (255 - pMask->rgbRed);
			pColor++;
			pMask++;
		}
	}

	HBITMAP bitmap = CBCGPDrawManager::CreateBitmap_32 (dcColor.GetBitmap ());
	ASSERT (bitmap != NULL);

	::DeleteObject (ii.hbmColor);
	::DeleteObject (ii.hbmMask);

	return bitmap;
}

void BCGPCorrectAlpha(BYTE* pRGB, BYTE a)
{
	if (a == 255 || (pRGB[0] <= a && pRGB[1] <= a && pRGB[2] <= a))
	{
		return;
	}

	const double coeff = (double)a / (double)max(pRGB[0], max(pRGB[1], pRGB[2]));

	pRGB[0] = min((BYTE)(pRGB[0] * coeff), a);
	pRGB[1] = min((BYTE)(pRGB[1] * coeff), a);
	pRGB[2] = min((BYTE)(pRGB[2] * coeff), a);
}

BOOL BCGPDesaturateBitmap(HBITMAP hBitmap, COLORREF clrTransparent, BOOL bIsIgnoreAlpha, double dblLumRatio)
{
	if (hBitmap == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BITMAP bmp = {0};
	if (::GetObject(hBitmap, sizeof(BITMAP), &bmp) != sizeof(BITMAP))
	{
		return FALSE;
	}

	if (bmp.bmBitsPixel < 4)
	{
		return FALSE;
	}

	double dblLum[3] = {0.299, 0.587, 0.114};
	dblLumRatio = bcg_clamp(dblLumRatio, 0.0, 2.0);
	if (dblLumRatio != 1.0)
	{
		dblLum[0] *= dblLumRatio;
		dblLum[1] *= dblLumRatio;
		dblLum[2] *= dblLumRatio;
	}

	// Try to use 24 or 32 BPP device independent bitmap
	if (bmp.bmBitsPixel >= 24)
	{
		CBCGPScanlinerBitmap scan;
		if (scan.Attach(hBitmap))
		{
			const BYTE channels = scan.GetChannels();
			for (LONG y = 0; y < bmp.bmHeight; y++)
			{
				LPBYTE pRow = scan.Get();

				for (LONG x = 0; x < bmp.bmWidth; x++)
				{
					COLORREF clr = RGB(pRow[2], pRow[1], pRow[0]);
					const BYTE gray = (BYTE)bcg_clamp_to_byte(pRow[2] * dblLum[0] + pRow[1] * dblLum[1] + pRow[0] * dblLum[2]);

					if (channels == 4 && !bIsIgnoreAlpha)
					{
						if (pRow[3] != 0)
						{
							pRow[0] = gray;
							pRow[1] = gray;
							pRow[2] = gray;

							BCGPCorrectAlpha(pRow, pRow[3]);
						}
					}
					else if (clr != clrTransparent)
					{
						pRow[0] = gray;
						pRow[1] = gray;
						pRow[2] = gray;
					}

					pRow += channels;
				}

				scan++;
			}

			return TRUE;
		}
	}

	HDC hMemDC = ::CreateCompatibleDC(NULL);
	if (hMemDC == NULL)
	{
		return FALSE;
	}

	HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hMemDC, hBitmap);
	if (hBitmapOld == NULL)
	{
		::DeleteDC(hMemDC);
		return FALSE;
	}

	for (LONG y = 0; y < bmp.bmHeight; y++)
	{
		for (LONG x = 0; x < bmp.bmWidth; x++)
		{
			COLORREF clr = ::GetPixel(hMemDC, x, y);
			if (clr == clrTransparent)
			{
				continue;
			}

			COLORREF clrNew = clr;

			if (bmp.bmBitsPixel <= 8)
			{
				double H, S, L;
				CBCGPDrawManager::RGBtoHSL(clr, &H, &S, &L);
				BYTE gray = (BYTE)bcg_clamp_to_byte (L * dblLumRatio * 255.0);

				if (gray < 64)
				{
					clrNew = RGB(0, 0, 0);
				}
				else if (gray < 128)
				{
					clrNew = RGB(128, 128, 128);
				}
				else if (gray < 255)
				{
					clrNew = RGB(192, 192, 192);
				}
				else
				{
					clrNew = RGB(255, 255, 255);
				}
			}
			else
			{
				BYTE gray = (BYTE)bcg_clamp_to_byte(GetRValue(clr) * dblLum[0] + GetGValue(clr) * dblLum[1] + GetBValue(clr) * dblLum[2]);
				clrNew = RGB(gray, gray, gray);
			}

			if (clr != clrNew)
			{
				::SetPixel(hMemDC, x, y, clrNew);
			}
		}
	}

	::SelectObject(hMemDC, hBitmapOld);
	::DeleteDC(hMemDC);

	return TRUE;
}

BOOL BCGPAdjustmentBitmap(HBITMAP hBitmap, COLORREF clrTransparent, BOOL bIsIgnoreAlpha, const CBCGPColorLookupTable& table, BOOL bOnGray)
{
	if (hBitmap == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BITMAP bmp = {0};
	if (::GetObject(hBitmap, sizeof(BITMAP), &bmp) != sizeof(BITMAP))
	{
		return FALSE;
	}

	if (bmp.bmBitsPixel < 16)
	{
		return FALSE;
	}

	// Try to use 24 or 32 BPP device independent bitmap
	if (bmp.bmBitsPixel >= 24)
	{
		CBCGPScanlinerBitmap scan;
		if (scan.Attach(hBitmap))
		{
			const BYTE channels = scan.GetChannels();
			for (LONG y = 0; y < bmp.bmHeight; y++)
			{
				LPBYTE pRow = scan.Get();

				for (LONG x = 0; x < bmp.bmWidth; x++)
				{
					BYTE r = pRow[2];
					BYTE g = pRow[1];
					BYTE b = pRow[0];
					if (bOnGray)
					{
						r = g = b = RGB_LUM(r, g, b);
					}

					if (channels == 4 && !bIsIgnoreAlpha)
					{
						if (pRow[3] != 0)
						{
							pRow[0] = table[0][b];
							pRow[1] = table[1][g];
							pRow[2] = table[2][r];

							BCGPCorrectAlpha(pRow, pRow[3]);
						}
					}
					else if (RGB(pRow[2], pRow[1], pRow[0]) != clrTransparent)
					{
						pRow[0] = table[0][b];
						pRow[1] = table[1][g];
						pRow[2] = table[2][r];
					}

					pRow += channels;
				}

				scan++;
			}

			return TRUE;
		}
	}

	HDC hMemDC = ::CreateCompatibleDC(NULL);
	if (hMemDC == NULL)
	{
		return FALSE;
	}

	HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hMemDC, hBitmap);
	if (hBitmapOld == NULL)
	{
		::DeleteDC(hMemDC);
		return FALSE;
	}

	for (LONG y = 0; y < bmp.bmHeight; y++)
	{
		for (LONG x = 0; x < bmp.bmWidth; x++)
		{
			COLORREF clr = ::GetPixel(hMemDC, x, y);
			if (clr == clrTransparent)
			{
				continue;
			}

			BYTE r = GetRValue(clr);
			BYTE g = GetGValue(clr);
			BYTE b = GetBValue(clr);
			if (bOnGray)
			{
				r = g = b = RGB_LUM(r, g, b);
			}

			COLORREF clrNew = RGB(table[2][r], table[1][g], table[0][b]);
			if (clr != clrNew)
			{
				::SetPixel(hMemDC, x, y, clrNew);
			}
		}
	}

	::SelectObject(hMemDC, hBitmapOld);
	::DeleteDC(hMemDC);

	return TRUE;
}

BOOL BCGPBlendBitmap(HBITMAP hBitmap, COLORREF clrTransparent, BOOL bIsIgnoreAlpha, double dblRatio)
{
	if (hBitmap == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BITMAP bmp = {0};
	if (::GetObject(hBitmap, sizeof(BITMAP), &bmp) != sizeof(BITMAP))
	{
		return FALSE;
	}

	if (bmp.bmBitsPixel < 16)
	{
		return FALSE;
	}

	COLORREF clr;

	dblRatio = bcg_clamp (dblRatio, 0.0, 2.0);

	COLORREF clrBlend = RGB(0, 0, 0);
	if (dblRatio > 1.0)
	{
		dblRatio -= 1.0;
		clrBlend = RGB(255, 255, 255);
	}

	int nRatio = bcg_clamp(bcg_round(dblRatio * 100.0), 0, 100);


	// Try to use 24 or 32 BPP device independent bitmap
	if (bmp.bmBitsPixel >= 24)
	{
		CBCGPScanlinerBitmap scan;
		if (scan.Attach(hBitmap))
		{
			const BYTE channels = scan.GetChannels();
			for (LONG y = 0; y < bmp.bmHeight; y++)
			{
				LPBYTE pRow = scan.Get();

				for (LONG x = 0; x < bmp.bmWidth; x++)
				{
					if (channels == 4 && !bIsIgnoreAlpha)
					{
						if (pRow[3] != 0)
						{
							clr = CBCGPDrawManager::PixelAlpha (clrBlend, RGB(pRow[2], pRow[1], pRow[0]), nRatio);

							pRow[2] = GetRValue(clr);
							pRow[1] = GetGValue(clr);
							pRow[0] = GetBValue(clr);

							BCGPCorrectAlpha(pRow, pRow[3]);
						}
					}
					else if (RGB(pRow[2], pRow[1], pRow[0]) != clrTransparent)
					{
						clr = CBCGPDrawManager::PixelAlpha (clrBlend, RGB(pRow[2], pRow[1], pRow[0]), nRatio);

						pRow[2] = GetRValue(clr);
						pRow[1] = GetGValue(clr);
						pRow[0] = GetBValue(clr);
					}

					pRow += channels;
				}

				scan++;
			}

			return TRUE;
		}
	}

	HDC hMemDC = ::CreateCompatibleDC(NULL);
	if (hMemDC == NULL)
	{
		return FALSE;
	}

	HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hMemDC, hBitmap);
	if (hBitmapOld == NULL)
	{
		::DeleteDC(hMemDC);
		return FALSE;
	}

	for (LONG y = 0; y < bmp.bmHeight; y++)
	{
		for (LONG x = 0; x < bmp.bmWidth; x++)
		{
			COLORREF clr = ::GetPixel(hMemDC, x, y);
			if (clr == clrTransparent)
			{
				continue;
			}

			COLORREF clrNew = CBCGPDrawManager::PixelAlpha (clrBlend, clr, nRatio);
			if (clr != clrNew)
			{
				::SetPixel(hMemDC, x, y, clrNew);
			}
		}
	}

	::SelectObject(hMemDC, hBitmapOld);
	::DeleteDC(hMemDC);

	return TRUE;
}

HBITMAP BCGPRotateBitmap(HBITMAP hBitmap, BOOL bCW)
{
	if (hBitmap == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	BITMAP bmp = {0};
	if (::GetObject(hBitmap, sizeof(BITMAP), &bmp) != sizeof(BITMAP))
	{
		return NULL;
	}

	LPBYTE lpBitsSrc = (LPBYTE)bmp.bmBits;

	HBITMAP hBitmapDst = NULL;

	if (bmp.bmBitsPixel >= 24 && lpBitsSrc != NULL)
	{
		LPBYTE lpBitsDst = NULL;
		int nChannels = bmp.bmBitsPixel / 8;
		if (bmp.bmBitsPixel == 32)
		{
			hBitmapDst = CBCGPDrawManager::CreateBitmap_32(CSize(bmp.bmHeight, bmp.bmWidth), (LPVOID*)&lpBitsDst);
		}
		else
		{
			hBitmapDst = CBCGPDrawManager::CreateBitmap_24(CSize(bmp.bmHeight, bmp.bmWidth), (LPVOID*)&lpBitsDst);
		}

		if (hBitmapDst == NULL)
		{
			return NULL;
		}

		int nPitchSrc = nChannels * bmp.bmWidth;
		if (nPitchSrc % 4)
		{
			nPitchSrc += 4 - (nPitchSrc % 4);
		}

		int nPitchDst = nChannels * bmp.bmHeight;
		if (nPitchDst % 4)
		{
			nPitchDst += 4 - (nPitchDst % 4);
		}

		if (bCW)
		{
			for (LONG y = 0; y < bmp.bmHeight; y++)
			{
				for (LONG x = 0; x < bmp.bmWidth; x++)
				{
					memcpy(lpBitsDst + (x * nPitchDst + y * nChannels), 
							lpBitsSrc + (y * nPitchSrc + (bmp.bmWidth - x - 1) * nChannels), nChannels);
				}
			}
		}
		else
		{
			for (LONG y = 0; y < bmp.bmHeight; y++)
			{
				for (LONG x = 0; x < bmp.bmWidth; x++)
				{
					memcpy(lpBitsDst + (x * nPitchDst + (bmp.bmHeight - y - 1) * nChannels), 
							lpBitsSrc + (y * nPitchSrc + x * nChannels), nChannels);
				}
			}
		}

		return hBitmapDst;
	}

	CDC dcSrc;
	dcSrc.CreateCompatibleDC (NULL);

	HBITMAP hOldBitmapSrc = (HBITMAP) dcSrc.SelectObject (hBitmap);
	if (hOldBitmapSrc == NULL)
	{
		return NULL;
	}

	CDC dcDst;
	dcDst.CreateCompatibleDC (NULL);

	hBitmapDst = CreateCompatibleBitmap(dcDst.GetSafeHdc(), bmp.bmHeight, bmp.bmWidth);
	if (hBitmapDst == NULL)
	{
		dcSrc.SelectObject (hOldBitmapSrc);
		return NULL;
	}

	HBITMAP hOldBitmapDst = (HBITMAP) dcDst.SelectObject (hBitmapDst);
	if (hOldBitmapDst == NULL)
	{
		dcSrc.SelectObject (hOldBitmapSrc);
		::DeleteObject(hBitmapDst);
		hBitmapDst = NULL;
		return NULL;
	}

	if (bCW)
	{
		for (LONG y = 0; y < bmp.bmHeight; y++)
		{
			for (LONG x = 0; x < bmp.bmWidth; x++)
			{
				dcDst.SetPixel(y, x, dcSrc.GetPixel(bmp.bmWidth - x - 1, y));
			}
		}
	}
	else
	{
		for (LONG y = 0; y < bmp.bmHeight; y++)
		{
			for (LONG x = 0; x < bmp.bmWidth; x++)
			{
				dcDst.SetPixel(bmp.bmHeight - y - 1, x, dcSrc.GetPixel(x, y));
			}
		}
	}

	dcSrc.SelectObject (hOldBitmapSrc);
	dcDst.SelectObject (hOldBitmapDst);

	return hBitmapDst;
}
