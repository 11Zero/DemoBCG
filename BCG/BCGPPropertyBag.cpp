// BCGPPropertyBag.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPPropertyBag.h"

#error "This file has been excluded from project. Please get the latest version from SourceSafe."

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL (CBCGPPropertyBag, CObject, 1)

// CBCGPPropertyBag

CBCGPPropertyBag::CBCGPPropertyBag ()
    : m_pIPropBag (NULL)
{
}

CBCGPPropertyBag::CBCGPPropertyBag (LPCWSTR /*initString*/)
    : m_pIPropBag (CreateSimplePropertyBag ())
{
    m_pIPropBag->AddRef ();

    // TODO: init from string
}

CBCGPPropertyBag::CBCGPPropertyBag (IBcgpPropertyBag* pPropertyBag)
    : m_pIPropBag (pPropertyBag)
{
    if (m_pIPropBag != NULL)
    {
        m_pIPropBag->AddRef ();
    }
}

CBCGPPropertyBag::CBCGPPropertyBag (const CBCGPPropertyBag& propBag)
    : m_pIPropBag (NULL)
{
    if (propBag.m_pIPropBag != NULL)
    {
        m_pIPropBag = propBag.m_pIPropBag->Clone ();
        if (m_pIPropBag != NULL)
        {
            m_pIPropBag->AddRef ();
        }
    }
}

CBCGPPropertyBag& CBCGPPropertyBag::operator = (const CBCGPPropertyBag& propBag)
{
    CBCGPPropertyBag temp (propBag);
    IBcgpPropertyBag* p = m_pIPropBag;
    m_pIPropBag = temp.m_pIPropBag;
    temp.m_pIPropBag = p;
    return *this;
}

CBCGPPropertyBag::~CBCGPPropertyBag()
{
    if (m_pIPropBag != NULL)
    {
        m_pIPropBag->Release ();
    }
}

void CBCGPPropertyBag::CreateBagIfNeeded ()
{
    if (m_pIPropBag == NULL)
    {
        m_pIPropBag = CreateSimplePropertyBag ();
        m_pIPropBag->AddRef ();
    }
}


// CBCGPPropertyBag member functions

bool CBCGPPropertyBag::IsValidName (const char* propertyName)
{
    if (propertyName == NULL || *propertyName == 0)
    {
        return FALSE;
    }

    while (*propertyName != 0)
    {
        char c = *propertyName;
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
        {
            propertyName ++;
        }
        else
        {
            return false;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
// GetProperty functions

bool CBCGPPropertyBag::GetProperty (const char* propertyName, CString& outString) const
{
    _bstr_t bstr;
    if (GetProperty (propertyName, bstr))
    {
        outString = (const wchar_t*)bstr;
        return true;
    }

    return false;
}

bool CBCGPPropertyBag::GetProperty (const char* propertyName, _bstr_t& outString) const
{
    if (m_pIPropBag == NULL)
    {
        return false;
    }

    _variant_t var;
    if (SUCCEEDED (m_pIPropBag->GetProperty (propertyName, &var)))
    {
        if (SUCCEEDED (::VariantChangeType (&var, &var, VARIANT_NOVALUEPROP, VT_BSTR)))
        {
            outString = V_BSTR(&var);
            return true;
        }
    }
    return false;
}

bool CBCGPPropertyBag::GetProperty (const char* propertyName, int& outInt) const
{
    if (m_pIPropBag == NULL)
    {
        return false;
    }

    _variant_t var;
    if (SUCCEEDED (m_pIPropBag->GetProperty (propertyName, &var)))
    {
        if (SUCCEEDED (::VariantChangeType (&var, &var, VARIANT_NOVALUEPROP, VT_I4)))
        {
            outInt = V_I4(&var);
            return true;
        }
    }
    return false;
}

bool CBCGPPropertyBag::GetProperty (const char* propertyName, unsigned int& outUint) const
{
    if (m_pIPropBag == NULL)
    {
        return false;
    }

    _variant_t var;
    if (SUCCEEDED (m_pIPropBag->GetProperty (propertyName, &var)))
    {
        if (SUCCEEDED (::VariantChangeType (&var, &var, VARIANT_NOVALUEPROP, VT_UI4)))
        {
            outUint = V_UI4(&var);
            return true;
        }
    }
    return false;
}

bool CBCGPPropertyBag::GetProperty (const char* propertyName, long& outLong) const
{
    if (m_pIPropBag == NULL)
    {
        return false;
    }

    _variant_t var;
    if (SUCCEEDED (m_pIPropBag->GetProperty (propertyName, &var)))
    {
        if (SUCCEEDED (::VariantChangeType (&var, &var, VARIANT_NOVALUEPROP, VT_I4)))
        {
            outLong = V_I4(&var);
            return true;
        }
    }
    return false;
}

bool CBCGPPropertyBag::GetProperty (const char* propertyName, unsigned long& outUlong) const
{
    if (m_pIPropBag == NULL)
    {
        return false;
    }

    _variant_t var;
    if (SUCCEEDED (m_pIPropBag->GetProperty (propertyName, &var)))
    {
        if (SUCCEEDED (::VariantChangeType (&var, &var, VARIANT_NOVALUEPROP, VT_UI4)))
        {
            outUlong = V_UI4(&var);
            return true;
        }
    }
    return false;
}

bool CBCGPPropertyBag::GetProperty (const char* propertyName, double& outDouble) const
{
    if (m_pIPropBag == NULL)
    {
        return false;
    }

    _variant_t var;
    if (SUCCEEDED (m_pIPropBag->GetProperty (propertyName, &var)))
    {
        if (SUCCEEDED (::VariantChangeType (&var, &var, VARIANT_NOVALUEPROP, VT_R8)))
        {
            outDouble = V_R8(&var);
            return true;
        }
    }
    return false;
}

bool CBCGPPropertyBag::GetProperty (const char* propertyName, float& outFloat) const
{
    if (m_pIPropBag == NULL)
    {
        return false;
    }

    _variant_t var;
    if (SUCCEEDED (m_pIPropBag->GetProperty (propertyName, &var)))
    {
        if (SUCCEEDED (::VariantChangeType (&var, &var, VARIANT_NOVALUEPROP, VT_R4)))
        {
            outFloat = V_R4(&var);
            return true;
        }
    }
    return false;
}

bool CBCGPPropertyBag::GetProperty (const char* propertyName, bool& outBool) const
{
    if (m_pIPropBag == NULL)
    {
        return false;
    }

    _variant_t var;
    if (SUCCEEDED (m_pIPropBag->GetProperty (propertyName, &var)))
    {
        if (SUCCEEDED (::VariantChangeType (&var, &var, VARIANT_NOVALUEPROP, VT_BOOL)))
        {
            outBool = V_BOOL(&var) != 0;
            return true;
        }
    }
    return false;
}

CString CBCGPPropertyBag::GetStringDefault (const char* propertyName, const CString& defValue) const
{
    CString value = defValue;
    GetProperty(propertyName, value);
    return value;
}

_bstr_t CBCGPPropertyBag::GetOleStringDefault (const char* propertyName, const _bstr_t& defValue) const
{
    _bstr_t value = defValue;
    GetProperty(propertyName, value);
    return value;
}

int CBCGPPropertyBag::GetIntDefault (const char* propertyName, int defValue) const
{
    int value = defValue;
    GetProperty(propertyName, value);
    return value;
}

UINT CBCGPPropertyBag::GetUintDefault (const char* propertyName, UINT defValue) const
{
    UINT value = defValue;
    GetProperty(propertyName, value);
    return value;
}

long CBCGPPropertyBag::GetLongDefault (const char* propertyName, long defValue) const
{
    long value = defValue;
    GetProperty(propertyName, value);
    return value;
}

ULONG CBCGPPropertyBag::GetUlongDefault (const char* propertyName, ULONG defValue) const
{
    ULONG value = defValue;
    GetProperty(propertyName, value);
    return value;
}

double CBCGPPropertyBag::GetDoubleDefault (const char* propertyName, double defValue) const
{
    double value = defValue;
    GetProperty(propertyName, value);
    return value;
}

float CBCGPPropertyBag::GetFloatDefault (const char* propertyName, float defValue) const
{
    float value = defValue;
    GetProperty(propertyName, value);
    return value;
}

bool CBCGPPropertyBag::GetBoolDefault (const char* propertyName, bool defValue) const
{
    bool value = defValue;
    GetProperty(propertyName, value);
    return value;
}

//////////////////////////////////////////////////////////////////////////
// SetProperty functions

void CBCGPPropertyBag::SetProperty (const char* propertyName, LPCSTR value)
{
    CreateBagIfNeeded ();
    _variant_t v(value);
    m_pIPropBag->SetProperty (propertyName, &v);
}

void CBCGPPropertyBag::SetProperty (const char* propertyName, LPCWSTR value)
{
    CreateBagIfNeeded ();
    _variant_t v(value);
    m_pIPropBag->SetProperty (propertyName, &v);
}

void CBCGPPropertyBag::SetProperty (const char* propertyName, const _bstr_t& value)
{
    CreateBagIfNeeded ();
    _variant_t v(value);
    m_pIPropBag->SetProperty (propertyName, &v);
}

void CBCGPPropertyBag::SetProperty (const char* propertyName, int value)
{
    CreateBagIfNeeded ();
    _variant_t v((long)value);
    m_pIPropBag->SetProperty (propertyName, &v);
}

void CBCGPPropertyBag::SetProperty (const char* propertyName, unsigned int value)
{
    CreateBagIfNeeded ();
    _variant_t v;
	v.vt = VT_UI4;
	v.uintVal = value;
    m_pIPropBag->SetProperty (propertyName, &v);
}

void CBCGPPropertyBag::SetProperty (const char* propertyName, long value)
{
    CreateBagIfNeeded ();
    _variant_t v(value);
    m_pIPropBag->SetProperty (propertyName, &v);
}

void CBCGPPropertyBag::SetProperty (const char* propertyName, unsigned long value)
{
    CreateBagIfNeeded ();
    _variant_t v;
	v.vt = VT_UI4;
	v.uintVal = value;
    m_pIPropBag->SetProperty (propertyName, &v);
}

void CBCGPPropertyBag::SetProperty (const char* propertyName, double value)
{
    CreateBagIfNeeded ();
    _variant_t v(value);
    m_pIPropBag->SetProperty (propertyName, &v);
}

void CBCGPPropertyBag::SetProperty (const char* propertyName, float value)
{
    CreateBagIfNeeded ();
    _variant_t v(value);
    m_pIPropBag->SetProperty (propertyName, &v);
}

void CBCGPPropertyBag::SetProperty (const char* propertyName, bool value)
{
    CreateBagIfNeeded ();
    _variant_t v(value);
    m_pIPropBag->SetProperty (propertyName, &v);
}

//////////////////////////////////////////////////////////////////////////
// Other Methods

bool CBCGPPropertyBag::HasProperty (const char* propertyName) const
{
    return (m_pIPropBag != NULL && m_pIPropBag->GetPropertyType (propertyName) != VT_EMPTY);
}

VARTYPE CBCGPPropertyBag::GetPropertyType (const char* propertyName) const
{
    return m_pIPropBag == NULL ? (VARTYPE)VT_EMPTY : m_pIPropBag->GetPropertyType (propertyName);
}

bool CBCGPPropertyBag::ResetProperty (const char* propertyName)
{
    if (m_pIPropBag != NULL)
    {
        return SUCCEEDED (m_pIPropBag->ResetProperty (propertyName));
    }

    return false;
}

POSITION CBCGPPropertyBag::GetHead () const
{
    if (m_pIPropBag == NULL)
    {
        return NULL;
    }

    return m_pIPropBag->GetHead ();
}

const char* CBCGPPropertyBag::GetNext (POSITION& rPosition) const
{
    if (m_pIPropBag == NULL)
    {
        return NULL;
    }

    return m_pIPropBag->GetNext (&rPosition);
}

int CBCGPPropertyBag::GetPropertyCount () const
{
    if (m_pIPropBag == NULL)
    {
        return 0;
    }

    return m_pIPropBag->GetPropertyCount ();
}

_bstr_t CBCGPPropertyBag::AsOleString () const
{
    // TODO: implement
    return _bstr_t();
}

void CBCGPPropertyBag::Serialize (CArchive& ar)
{
    CObject::Serialize (ar);

    if (ar.IsLoading ())
    {
        // Prepare property bag for new data
        if (m_pIPropBag == NULL)
        {
            m_pIPropBag = CreateSimplePropertyBag ();
            m_pIPropBag->AddRef ();
        }
        else
        {
            m_pIPropBag->RemoveAll ();
        }

        int nProps = (int)ar.ReadCount ();
        for (; nProps > 0; nProps --)
        {
            _bstr_t strName, strValue;

            int nChars = (int)ar.ReadCount ();
            if (nChars > 0)
            {
                int nBytesToRead = nChars * sizeof (wchar_t);
                BSTR bstr = ::SysAllocStringByteLen (NULL, (nChars + 1)* sizeof (wchar_t));
                if (bstr != NULL)
                {
                    ZeroMemory (bstr, (nChars + 1)* sizeof (wchar_t));
                    if (ar.Read (bstr, nBytesToRead) == (UINT)nBytesToRead)
                    {
                        strName = _bstr_t (bstr, false); // OK
                    }
                }
                else
                {
                    // skip (nChars) in archive
                    for (int i = 0; i < nBytesToRead; ++i)
                    {
                        char chDummy;
                        ar >> chDummy;
                    }
                }
            }

            // Reading property value
            nChars = (int)ar.ReadCount ();
            if (nChars > 0)
            {
                int nBytesToRead = nChars * sizeof (wchar_t);
                BSTR bstr = ::SysAllocStringByteLen (NULL, (nChars + 1)* sizeof (wchar_t));
                if (bstr != NULL)
                {
                    ZeroMemory (bstr, (nChars + 1)* sizeof (wchar_t));
                    if (ar.Read (bstr, nBytesToRead) == (UINT)nBytesToRead)
                    {
                        strValue = _bstr_t (bstr, false);
                        if (strName.length () > 0)
                        {
                            const char* name = (const char*)strName; // cast to ANSI string
                            if (CBCGPPropertyBag::IsValidName (name))
                            {
                                if (FAILED (m_pIPropBag->InitFromString(name, strValue)))
                                {
                                    TRACE (" Syntax error occured while parsing \"%hs\" property.\n", name);
                                }
                            }
                            else
                            {
                                TRACE (" \"%hs\" is not a valid property name.\n", name);
                            }
                        }
                    }
                }
                else
                {
                    // skip (nChars) in archive
                    for (int i = 0; i < nBytesToRead; ++i)
                    {
                        char chDummy;
                        ar >> chDummy;
                    }
                }
            }
        }
    }
    else // Storing
    {
        if (m_pIPropBag == NULL)
        {
            return;
        }

        int nProps = m_pIPropBag->GetPropertyCount ();
        ar.WriteCount (nProps);

        POSITION pos = m_pIPropBag->GetHead ();
        while (pos != NULL && nProps > 0)
        {
            const char* propName = m_pIPropBag->GetNext (&pos);
            if (propName == NULL) break;
            
            bstr_t strName (propName);
            BSTR bstrValue;
            if (SUCCEEDED (m_pIPropBag->PropertyToString (propName, &bstrValue)))
            {
                int nChars = strName.length ();
                ar.WriteCount (nChars);
                ar.Write ((const wchar_t*)strName, nChars * sizeof (wchar_t));
                nChars = ::SysStringLen (bstrValue);
                ar.WriteCount (nChars);
                ar.Write ((const wchar_t*)bstrValue, nChars * sizeof (wchar_t));
				::SysFreeString (bstrValue);
            }

            nProps --;
        }

        while (nProps > 0)
        {
            // Write the stubs
            ar.WriteCount (0);
            ar.WriteCount (0);
            nProps --;
        }
    }
}

#if defined(_DEBUG)
void CBCGPPropertyBag::AssertValid() const
{
    CObject::AssertValid ();
}

void CBCGPPropertyBag::Dump (CDumpContext& dc) const
{
    CObject::Dump (dc);

    if (m_pIPropBag == NULL)
    {
        dc << "m_pIPropBag is NULL\n";
        return;
    }

    dc << "Property Count: " <<  m_pIPropBag->GetPropertyCount () << "\n";

    POSITION pos = m_pIPropBag->GetHead ();
    while (pos != NULL)
    {
        const char* propName = m_pIPropBag->GetNext (&pos);
        if (propName == NULL) break;

        dc << propName << " : "; 
        BSTR bstrValue;
        if (SUCCEEDED (m_pIPropBag->PropertyToString (propName, &bstrValue)))
        {
            dc << "{" << (LPCWSTR)bstrValue << "}\n";
			::SysFreeString (bstrValue);
        }
        else
        {
            dc << "ERROR\n";
        }
    }
}
#endif

//////////////////////////////////////////////////////////////////////////
// CBCGPSimplePropertyBag class

class CBCGPSimplePropertyBag : public IBcgpPropertyBag
{
public:
    CBCGPSimplePropertyBag ();
    virtual ~CBCGPSimplePropertyBag ();

    virtual HRESULT     STDMETHODCALLTYPE QueryInterface (REFIID riid, void **ppvObject);
    virtual ULONG       STDMETHODCALLTYPE AddRef ();
    virtual ULONG       STDMETHODCALLTYPE Release ();

    virtual HRESULT     STDMETHODCALLTYPE SetProperty (const char* propertyName, VARIANT* pValue);
    virtual HRESULT     STDMETHODCALLTYPE GetProperty (const char* propertyName, VARIANT* pOut);
    virtual HRESULT     STDMETHODCALLTYPE ResetProperty (const char* propertyName);
    virtual VARTYPE     STDMETHODCALLTYPE GetPropertyType (const char* propertyName);
    virtual HRESULT     STDMETHODCALLTYPE RemoveAll ();
    virtual HRESULT     STDMETHODCALLTYPE PropertyToString (const char* propertyName, BSTR* pOut);
    virtual HRESULT     STDMETHODCALLTYPE InitFromString (const char* propertyName, BSTR str);
    virtual int         STDMETHODCALLTYPE GetPropertyCount ();
    virtual POSITION    STDMETHODCALLTYPE GetHead ();
    virtual const char* STDMETHODCALLTYPE GetNext (POSITION* pPosition);
    virtual IBcgpPropertyBag* STDMETHODCALLTYPE Clone ();

private:
    LONG m_lRefCount;
    CMap <const char*, const char*, _variant_t, const _variant_t&> m_map;
};

CBCGPSimplePropertyBag::CBCGPSimplePropertyBag ()
    : m_lRefCount (0)
{
}

CBCGPSimplePropertyBag::~CBCGPSimplePropertyBag ()
{
}

//////////////////////////////////////////////////////////////////////////
// IUnknown interface implementation

STDMETHODIMP CBCGPSimplePropertyBag::QueryInterface (REFIID riid, void** ppv) 
{ 
    if (ppv == NULL)
    {
        return E_POINTER;
    }

    if (riid == IID_IUnknown) 
    {
        *ppv = static_cast<IUnknown*> (this);
        AddRef ();
        return S_OK;
    }
    if (riid == __uuidof (IBcgpPropertyBag)) 
    {
        *ppv = static_cast<IBcgpPropertyBag*> (this);
        AddRef ();
        return S_OK;
    }

    *ppv = NULL; 
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CBCGPSimplePropertyBag::AddRef ()
{
    InterlockedIncrement (&m_lRefCount);
    return m_lRefCount;
}

STDMETHODIMP_(ULONG) CBCGPSimplePropertyBag::Release() 
{ 
    InterlockedDecrement (&m_lRefCount);

    if (m_lRefCount > 0) 
    {
        return m_lRefCount; 
    }

    delete this; 
    return 0; 
}

//////////////////////////////////////////////////////////////////////////
// IBcgpPropertyBag interface implementation

HRESULT STDMETHODCALLTYPE CBCGPSimplePropertyBag::SetProperty (const char* propertyName, VARIANT* pValue)
{
    if (!CBCGPPropertyBag::IsValidName (propertyName))
    {
        return E_INVALIDARG;
    }

    if (pValue == NULL)
    {
        return E_POINTER;
    }
    
    m_map[propertyName] = *pValue;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CBCGPSimplePropertyBag::GetProperty (const char* propertyName, VARIANT* pOut)
{
    if (pOut == NULL)
    {
        return E_POINTER;
    }

    _variant_t v;
    if (m_map.Lookup (propertyName, v))
    {
        *pOut = v.Detach ();
        return S_OK;
    }

    return E_FAIL; // Not found
}

HRESULT STDMETHODCALLTYPE CBCGPSimplePropertyBag::ResetProperty (const char* propertyName)
{
    return m_map.RemoveKey(propertyName) ? S_OK : S_FALSE;
}

VARTYPE STDMETHODCALLTYPE CBCGPSimplePropertyBag::GetPropertyType (const char* propertyName)
{
    _variant_t var;
    if (!m_map.Lookup (propertyName, var))
    {
        return VT_EMPTY;
    }
    return var.vt;
}

HRESULT STDMETHODCALLTYPE CBCGPSimplePropertyBag::RemoveAll ()
{
    m_map.RemoveAll ();
    return S_OK;
}

IBcgpPropertyBag* STDMETHODCALLTYPE CBCGPSimplePropertyBag::Clone ()
{
    CBCGPSimplePropertyBag* pClone = new CBCGPSimplePropertyBag;
    POSITION pos = m_map.GetStartPosition ();
    while (pos != NULL)
    {
        const char* strName = 0;
        _variant_t v;
        m_map.GetNextAssoc (pos, strName, v);

        pClone->m_map[strName] = v;
    }

    return pClone;
}

HRESULT STDMETHODCALLTYPE CBCGPSimplePropertyBag::PropertyToString (const char* propertyName, BSTR* pOut)
{
    if (pOut == NULL)
    {
        return E_POINTER;
    }

    _variant_t var;
    if (m_map.Lookup (propertyName, var))
    {
        _bstr_t bstr;
        if (SUCCEEDED (VariantToString(var, bstr)))
        {
            *pOut = bstr.copy ();
            return S_OK;
        }
    }

    pOut = NULL;
    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CBCGPSimplePropertyBag::InitFromString (const char* propertyName, BSTR str)
{
    _variant_t var;
    if (CBCGPPropertyBag::IsValidName (propertyName) && SUCCEEDED (VariantFromString (str, var)))
    {
        m_map[propertyName] = var;
    }

    return E_FAIL;
}

int STDMETHODCALLTYPE CBCGPSimplePropertyBag::GetPropertyCount ()
{
    return m_map.GetCount ();
}

POSITION STDMETHODCALLTYPE CBCGPSimplePropertyBag::GetHead ()
{
    return m_map.GetStartPosition ();
}

const char* STDMETHODCALLTYPE CBCGPSimplePropertyBag::GetNext (POSITION* pPosition)
{
    if (pPosition == NULL)
    {
        return NULL;
    }

    const char* strName = NULL;
    _variant_t v;
    m_map.GetNextAssoc (*pPosition, strName, v);
    return strName;
}

IBcgpPropertyBag* CBCGPPropertyBag::CreateSimplePropertyBag ()
{
    return new CBCGPSimplePropertyBag;
}

//////////////////////////////////////////////////////////////////////////
// Helper functions

_bstr_t QuoteOleString (BSTR string, OLECHAR quoteChar)
{
    int nChars = ::SysStringLen(string);
    int i, nQuotes = 2;
    for (i = 0; i < nChars; ++i)
    {
        if (string[i] == quoteChar) nQuotes ++;
    }

    BSTR str = ::SysAllocStringLen (NULL, nChars + nQuotes);
    ZeroMemory (str, (nChars + nQuotes + 1) * sizeof (wchar_t));
    if (str != NULL)
    {
        wchar_t* pBuffer = str;
        *(pBuffer ++) = quoteChar;
        for (i = 0; i < nChars; ++i)
        {
            *(pBuffer ++) = string[i];
            if (string[i] == quoteChar)
            {
                *(pBuffer ++) = string[i];
            }
        }
        *pBuffer = quoteChar;
    }

    return _bstr_t (str, false); // attach the new string
}

_bstr_t UnquoteOleString (BSTR string, OLECHAR quoteChar)
{
    int len = ::SysStringLen(string);
    if (len <= 2 || string[0] != quoteChar || string[len - 1] != quoteChar)
    {
        return _bstr_t();
    }
    
    int i, nChars = 0;
    bool bPrevIsQuote = false;
    for (i = 1; i < len - 1; ++i)
    {
        if (string[i] == quoteChar && !bPrevIsQuote)
        { 
            bPrevIsQuote = true;
            continue;
        }

        bPrevIsQuote = false;
        nChars ++;
    }

    BSTR str = ::SysAllocStringLen (NULL, nChars);
    ZeroMemory (str, (nChars + 1) * sizeof (wchar_t));
    if (str != NULL)
    {
        wchar_t* pBuffer = str;
        bPrevIsQuote = false;
        for (i = 1; i < len - 1; ++i)
        {
            if (string[i] == quoteChar && !bPrevIsQuote)
            { 
                bPrevIsQuote = true;
                continue;
            }

            *(pBuffer ++) = string[i];
            bPrevIsQuote = false;
        }
    }

    return _bstr_t (str, false); // attach the new string
}

HRESULT VariantToString (const _variant_t& var, _bstr_t& str)
{
    switch (var.vt)
    {
    case VT_EMPTY:
        str = _bstr_t();
        return S_OK;
    case VT_NULL:
        str = _bstr_t(L"NULL");
        return S_OK;
    case VT_BOOL:
        str = _bstr_t(var.boolVal != 0 ? L"true" : L"false");
        return S_OK;
    case VT_BSTR:
        str = QuoteOleString (var.bstrVal);
        return S_OK;
    case VT_I1: case VT_I2: case VT_I4: case VT_I8:
    case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
    case VT_INT: case VT_UINT:
    case VT_R4: case VT_R8:
        {
            _variant_t vTemp;
            if (SUCCEEDED (::VariantChangeType (&vTemp, (VARIANTARG*)&var, VARIANT_NOVALUEPROP, VT_BSTR)))
            {
                str = vTemp.bstrVal;
                return S_OK;
            }
        }
        return E_FAIL;
    }

    return E_FAIL;
}

HRESULT VariantFromString (BSTR str, _variant_t& var)
{
    _bstr_t bstr = str;
    if (bstr.length () == 0)
    {
        var = _variant_t(); // VT_EMPTY
        return S_OK;
    }

    wchar_t wFirst = str[0];
    if (wFirst == '\"')
    {
        var = UnquoteOleString (str, L'\"');
        return S_OK;
    }
    if (_wcsicmp (str, L"null") == 0)
    {
        var = _variant_t();
        var.vt = VT_NULL;
        return S_OK;
    }

    if (_wcsicmp (str, L"true") == 0)
    {
        var = _variant_t(true);
        return S_OK;
    }

    if (_wcsicmp (str, L"false") == 0)
    {
        var = _variant_t(false);
        return S_OK;
    }

    if ((wFirst >= L'0' && wFirst <= L'9') || wFirst == L'.' || wFirst == L'-' || wFirst == L'+')
    {
        bool bSigned = (wFirst == '-' || wFirst == '+');
        bool bReal = (wcschr (str, L',') != NULL) || (wcschr (str, L'.') != NULL) || (wcschr (str + 1, L'E') != NULL);
        VARTYPE numType = (VARTYPE)(bReal ? VT_R8 : (bSigned ? VT_I4 : VT_UI4));

        _variant_t vTemp (str);
        return ::VariantChangeType (&var, &vTemp, VARIANT_NOVALUEPROP, numType);
    }

    return E_FAIL;
}
