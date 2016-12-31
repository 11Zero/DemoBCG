// YahooQuote.cpp: implementation of the CYahooQuote class.
//
// --------------------------------------------------------------------------
// COPYRIGHT NOTICE:
// --------------------------------------------------------------------------
// 
// "Codejock Chart" is a MFC extension library for creating modern charting 
// applications. This file and the code herein are part of the Codejock Chart
// MFC extension library.
//
// This code is protected under U.S. Copyright Law and International treaties
// and is intended for registered and licensed use only. Any other use is 
// strictly prohibited. This code is subject to the terms and conditions 
// outlined in the "Codejock Chart" End User License Agreement (EULA).
//
// Copyright (c) 1998-2011 Codejock Technologies LLC, All Rights Reserved.
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "YahooQuote.h"

#include <afxinet.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CYahooQuote::CYahooQuote()
{
	
}

CYahooQuote::~CYahooQuote()
{
	
}

CStringArray& CYahooQuote::GetHistory(CString strSym, CTime timeStart, char ch /*='d'*/)
{
	m_strArray.RemoveAll();
	
	// format URL to pass to GetHttpConnection.
	CString strURL;
	
	strURL.Format(_T("http://ichart.finance.yahoo.com/table.csv?s=%s&a=%.2i&b=%i&c=%i&g=%c"),
		strSym,
		timeStart.GetMonth()-1,
		timeStart.GetDay(),
		timeStart.GetYear(),
		ch);
	
	CInternetSession session;
	CStdioFile* pFile = session.OpenURL(strURL);
	
	if (pFile)
	{
		CString strRead;
		while (pFile->ReadString(strRead))
		{
#ifdef _UNICODE
			strRead = CString((LPCSTR)(LPCTSTR)strRead);
#endif
			m_strArray.Add(strRead);
		}
		
		pFile->Close();
		delete pFile;
	}

	session.Close();
	return m_strArray;
}

CStringArray& CYahooQuote::GetHistory(CString strSym, CTime timeStart, CTime timeEnd, char ch /*='d'*/)
{
	m_strArray.RemoveAll();

	// format URL to pass to GetHttpConnection.
	CString strURL;

	strURL.Format(_T("http://ichart.finance.yahoo.com/table.csv?s=%s&a=%.2i&b=%i&c=%i&d=%.2i&e=%i&f=%i&g=%c"),
		strSym,
		timeStart.GetMonth()-1,
		timeStart.GetDay(),
		timeStart.GetYear(),
		timeEnd.GetMonth()-1,
		timeEnd.GetDay(),
		timeEnd.GetYear(),
		ch);

	CInternetSession session;
	CStdioFile* pFile = session.OpenURL(strURL);
	
	if (pFile)
	{
		CString strRead;
		while (pFile->ReadString(strRead))
		{
#ifdef _UNICODE
			strRead = CString((LPCSTR)(LPCTSTR)strRead);
#endif
			m_strArray.Add(strRead);
		}
		
		pFile->Close();
		delete pFile;
	}
	
	session.Close();
	return m_strArray;
}


double StringToDouble(LPCTSTR lpszStr)
{
	USES_CONVERSION;
	return atof(T2A((LPTSTR)lpszStr));
}

CString CYahooQuote::GetDate(CString arrQuote)
{
	CString strDate;
	AfxExtractSubString(strDate, arrQuote, 0, ',');
	return strDate;
}

double CYahooQuote::GetHigh(CString arrQuote)
{
	CString strHigh;
	AfxExtractSubString(strHigh, arrQuote, 2, ',');
	return StringToDouble(strHigh);
}

double CYahooQuote::GetLow(CString arrQuote)
{
	CString strLow;
	AfxExtractSubString(strLow, arrQuote, 3, ',');
	return StringToDouble(strLow);
}

double CYahooQuote::GetOpen(CString arrQuote)
{
	CString strOpen;
	AfxExtractSubString(strOpen, arrQuote, 1, ',');
	return StringToDouble(strOpen);
}

double CYahooQuote::GetClose(CString arrQuote)
{
	CString strClose;
	AfxExtractSubString(strClose, arrQuote, 4, ',');
	return StringToDouble(strClose);
}

CStringArray& CYahooQuote::GetQuote(CString strSym, CString strArgs /*=_T("nl1t1cpobat8mwva2j1")*/)
{
	m_strArray.RemoveAll();
	
	// format URL to pass to GetHttpConnection.
	CString strURL;
	strURL.Format(_T("http://download.finance.yahoo.com/d/quotes.csv?s=%s&f=%s"),
		strSym, strArgs);
	
	CInternetSession session;
	CStdioFile* pFile = session.OpenURL(strURL);
	
	if (pFile)
	{
		CString strRead;
		while (pFile->ReadString(strRead))
		{
#ifdef _UNICODE
			strRead = CString((LPCSTR)(LPCTSTR)strRead);
#endif
			strRead.Remove(_T('\"'));
			m_strArray.Add(strRead);
		}
		
		pFile->Close();
		delete pFile;
	}
	
	session.Close();
	return m_strArray;
}
