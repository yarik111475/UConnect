#include <QString>

#include "Wmi.h"

#if defined(Q_OS_WIN)
#define _WIN32_DCOM
#include <wbemidl.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")

Wmi::~Wmi() {
    release();
}

bool Wmi::init(const BSTR wmi_namespace) {

    //CoInitializeEx(0, COINIT_MULTITHREADED)
    const HRESULT hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);

    if (FAILED(hr)) {
        release();
        return false;
    }

    {//IWbemLocator
        const HRESULT hr=CoCreateInstance(CLSID_WbemLocator,
                                          0, CLSCTX_INPROC_SERVER,
                                          IID_IWbemLocator,
                                          (LPVOID *) &_wbemLocator);

        if (FAILED(hr)) {
            release();
            return false;
        }
    }

    {//IWbemServices
        const HRESULT hr=_wbemLocator->ConnectServer(wmi_namespace, //namespace
                                                     NULL,       // User name
                                                     NULL,       // User password
                                                     0,         // Locale
                                                     NULL,     // Security flags
                                                     0,         // Authority
                                                     0,        // Context object
                                                     &_wbemServices);   // IWbemServices proxy

        if (FAILED(hr)) {
            release();
            return false;
        }
    }

    {//CoSetProxyBlanket
        const HRESULT hr=CoSetProxyBlanket(_wbemServices,
                                           RPC_C_AUTHN_WINNT,
                                           RPC_C_AUTHZ_NONE,
                                           NULL,
                                           RPC_C_AUTHN_LEVEL_CALL,
                                           RPC_C_IMP_LEVEL_IMPERSONATE,
                                           NULL,
                                           EOAC_NONE);

        if (FAILED(hr)) {
            release();
            return false;
        }
    }
    return true;
}

IWbemLocator *Wmi::wbemLocator() const {
    return _wbemLocator;
}

IWbemServices *Wmi::wbemServices() const {
    return _wbemServices;
}

QVariant Wmi::ms_variant_to_qvariant(const VARIANT &ms_variant, long type, quint64 array_size)
{
    QVariant ret_value {};
    long var_type {type};

    if (var_type == 0xfff)
        var_type = ms_variant.vt;

    switch (var_type) {
    case VT_I2:{
        QVariant var_int16{ms_variant.iVal};
        ret_value = var_int16;
        break;
    }
    case VT_I4:{
        QVariant var_int32{ms_variant.intVal};
        ret_value = var_int32;
        break;
    }
    case VT_UI2:{
        QVariant var_uint16{ms_variant.uiVal};
        ret_value = var_uint16;
        break;
    }
    case VT_UI4:{
        QVariant var_uint32{ms_variant.uintVal};
        ret_value = var_uint32;
        break;
    }
    case VT_BSTR:{
        QVariant var_s{QString::fromWCharArray(ms_variant.bstrVal)};
        ret_value = var_s;
        break;
    }
    case VT_ARRAY|VT_UI1:
    case VT_ARRAY|VT_UI2:
    case VT_ARRAY|VT_UI1|VT_BYREF:{
        SAFEARRAY *array = 0;
        if (ms_variant.vt & VT_BYREF)
            array = *ms_variant.pparray;
        else
            array = ms_variant.parray;

        QByteArray bytes;
        if (!array || array->cDims != 1) {
            ret_value = bytes;
            break;
        }

        long lBound, uBound;
        SafeArrayGetLBound(array, 1, &lBound);
        SafeArrayGetUBound(array, 1, &uBound);

        if (uBound != -1) { // non-empty array
            bytes.resize(uBound - lBound + 1);
            char *data = bytes.data();
            char *src;
            SafeArrayAccessData(array, (void**)&src);
            memcpy(data, src, bytes.size());
            SafeArrayUnaccessData(array);
        }

        ret_value = bytes;
        break;
    }
    }

    return ret_value;
}

void Wmi::release() {

    if (_wbemServices) {
        _wbemServices->Release();
        _wbemServices = nullptr; //added 14 jan 22
    }

    if (_wbemLocator) {
        _wbemLocator->Release();
        _wbemLocator = nullptr;//added 14 jan 22
    }
    CoUninitialize();
}

#endif


