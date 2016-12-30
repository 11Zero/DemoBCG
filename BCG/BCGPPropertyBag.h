#if !defined(__BCGPPROPERTYBAG_H)
#define __BCGPPROPERTYBAG_H

//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2010 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include <comutil.h>

MIDL_INTERFACE("1259346D-92A5-4FF2-8227-8D5BAA8E9E5B")
IBcgpPropertyBag : public IUnknown
{
    virtual HRESULT     STDMETHODCALLTYPE SetProperty (const char* propertyName, VARIANT* pValue) = 0;
    virtual HRESULT     STDMETHODCALLTYPE GetProperty (const char* propertyName, VARIANT* pOut) = 0;
    virtual HRESULT     STDMETHODCALLTYPE ResetProperty (const char* propertyName) = 0;
    virtual VARTYPE     STDMETHODCALLTYPE GetPropertyType (const char* propertyName) = 0;
    virtual HRESULT     STDMETHODCALLTYPE RemoveAll () = 0;
    virtual HRESULT     STDMETHODCALLTYPE PropertyToString (const char* propertyName, BSTR* pOut) = 0;
    virtual HRESULT     STDMETHODCALLTYPE InitFromString (const char* propertyName, BSTR str) = 0;
    virtual int         STDMETHODCALLTYPE GetPropertyCount () = 0;
    virtual POSITION    STDMETHODCALLTYPE GetHead () = 0;
    virtual const char* STDMETHODCALLTYPE GetNext (POSITION* pPosition) = 0;
    virtual IBcgpPropertyBag* STDMETHODCALLTYPE Clone () = 0;
};

// CBCGPPropertyBag property container class

class BCGCBPRODLLEXPORT CBCGPPropertyBag : public CObject
{
public:
	CBCGPPropertyBag ();
    CBCGPPropertyBag (LPCWSTR initString);
    CBCGPPropertyBag (IBcgpPropertyBag* pPropertyBag);
    CBCGPPropertyBag (const CBCGPPropertyBag& propBag);

    CBCGPPropertyBag& operator = (const CBCGPPropertyBag& propBag);

    void SetProperty (const char* propertyName, LPCSTR value);
    void SetProperty (const char* propertyName, LPCWSTR value);
    void SetProperty (const char* propertyName, const _bstr_t& value);
    void SetProperty (const char* propertyName, int value);
    void SetProperty (const char* propertyName, unsigned int value);
    void SetProperty (const char* propertyName, long value);
    void SetProperty (const char* propertyName, unsigned long value);
    void SetProperty (const char* propertyName, double value);
    void SetProperty (const char* propertyName, float value);
    void SetProperty (const char* propertyName, bool value);

    bool ResetProperty (const char* propertyName);

    bool HasProperty (const char* propertyName) const;
    VARTYPE GetPropertyType (const char* propertyName) const;

    bool GetProperty (const char* propertyName, CString& outString) const;
    bool GetProperty (const char* propertyName, _bstr_t& outString) const;
    bool GetProperty (const char* propertyName, int& outInt) const;
    bool GetProperty (const char* propertyName, unsigned int& outUint) const;
    bool GetProperty (const char* propertyName, long& outLong) const;
    bool GetProperty (const char* propertyName, unsigned long& outUlong) const;
    bool GetProperty (const char* propertyName, double& outDouble) const;
    bool GetProperty (const char* propertyName, float& outFloat) const;
    bool GetProperty (const char* propertyName, bool& outBool) const;

    CString GetStringDefault    (const char* propertyName, const CString& defValue) const;
    _bstr_t GetOleStringDefault (const char* propertyName, const _bstr_t& defValue) const;
    int     GetIntDefault       (const char* propertyName, int defValue) const;
    UINT    GetUintDefault      (const char* propertyName, UINT defValue) const;
    long    GetLongDefault      (const char* propertyName, long defValue) const;
    ULONG   GetUlongDefault     (const char* propertyName, ULONG defValue) const;
    double  GetDoubleDefault    (const char* propertyName, double defValue) const;
    float   GetFloatDefault     (const char* propertyName, float defValue) const;
    bool    GetBoolDefault      (const char* propertyName, bool defValue) const;

    POSITION GetHead () const;
    const char* GetNext (POSITION& rPosition) const;
    int GetPropertyCount () const;

    IBcgpPropertyBag* GetIBcgpPropertyBag () { return m_pIPropBag; }
    
    static bool IsValidName (const char* propertyName);

    virtual void Serialize (CArchive& ar);
    _bstr_t AsOleString () const;
    
    static IBcgpPropertyBag* CreateSimplePropertyBag ();

#if defined(_DEBUG)
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    void CreateBagIfNeeded ();

public:
    virtual ~CBCGPPropertyBag ();
    DECLARE_SERIAL (CBCGPPropertyBag)

private:
    IBcgpPropertyBag* m_pIPropBag;
};

BCGCBPRODLLEXPORT _bstr_t QuoteOleString (BSTR string, OLECHAR quoteChar = '\"');
BCGCBPRODLLEXPORT _bstr_t UnquoteOleString (BSTR string, OLECHAR quoteChar = '\"');

BCGCBPRODLLEXPORT HRESULT VariantToString (const _variant_t& var, _bstr_t& str);
BCGCBPRODLLEXPORT HRESULT VariantFromString (BSTR str, _variant_t& var);

#endif // __BCGPPROPERTYBAG_H