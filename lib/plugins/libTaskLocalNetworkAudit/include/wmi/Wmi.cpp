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

bool Wmi::init(const BSTR wmiNamespace) {

    //CoInitializeEx(0, COINIT_MULTITHREADED)
    const HRESULT hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);

    if (FAILED(hr)) {
        release();
    }


//        {//CoInitializeSecurity
//             const HRESULT hr= CoInitializeSecurity(
//                            NULL,                        // Security descriptor
//                            -1,                          // COM negotiates authentication service
//                            NULL,                        // Authentication services
//                            NULL,                        // Reserved
//                            RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication level for proxies
//                            RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation level for proxies
//                            NULL,                        // Authentication info
//                            EOAC_NONE,                   // Additional capabilities of the client or server
//                            NULL);                        // Reserved

//            //Commented 27.10.20
//            if (FAILED(hr)) {
//                release();
//                const QString msg=QString { "%1. Failed to initialize security. Error code : '%2'" }
//                                        .arg(CODE_LOCATION_STRING)
//                                        .arg(hr);
//                THROW_EXCEPTION(msg);
//            }
//        }

    {//IWbemLocator
         const HRESULT hr=CoCreateInstance(CLSID_WbemLocator,
                                        0, CLSCTX_INPROC_SERVER,
                                        IID_IWbemLocator,
                                        (LPVOID *) &_wbemLocator);

        if (FAILED(hr)) {
            release();
        }
    }

    {//IWbemServices
        const HRESULT hr=_wbemLocator->ConnectServer(wmiNamespace, //namespace
                                                  NULL,       // User name
                                                  NULL,       // User password
                                                  0,         // Locale
                                                  NULL,     // Security flags
                                                  0,         // Authority
                                                  0,        // Context object
                                                  &_wbemServices);   // IWbemServices proxy

        if (FAILED(hr)) {
            release();
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

QVariant Wmi::msVariantToQVariant(const VARIANT &msVariant, long type, quint64 arraySize)
{
    QVariant returnValue;
    long variantType = type;

    if (variantType == 0xfff)
        variantType = msVariant.vt;

    switch (variantType) {
        case VT_I2:
        {
            QVariant var_int16{msVariant.iVal};
            returnValue = var_int16;
            break;
        }
        case VT_I4:
        {
            QVariant var_int32{msVariant.intVal};
            returnValue = var_int32;
            break;
        }
        case VT_UI2:
        {
            QVariant var_uint16{msVariant.uiVal};
            returnValue = var_uint16;
            break;
        }
        case VT_UI4:
        {
            QVariant var_uint32{msVariant.uintVal};
            returnValue = var_uint32;
            break;
        }
        case VT_BSTR:
        {
            QVariant var_s{QString::fromWCharArray(msVariant.bstrVal)};
            returnValue = var_s;
            break;
        }
//        case VT_ARRAY | VT_UI2:
//        {
//            SAFEARRAY *array = msVariant.parray;

////                quint16 * pchar;
////                SafeArrayAccessData(array, (void HUGEP **)&pchar);

//            const SAFEARRAYBOUND* psafearraybound = &((array -> rgsabound)[0]);
//            int maxLen = (arraySize == -1 ? psafearraybound->cElements : arraySize);

//            QString s;
//            int value;

//            for (long ul = 0; ul < maxLen - 2; ul++) {
//              value = 0;
//              SafeArrayGetElement(array, &ul, (void*)&value);
//              if (!value && arraySize == -1)
//                  break;
//              else
//                  s.append(value);
//            }

//            QVariant var_utf16_s{s};
//            returnValue = var_utf16_s;
//            break;
//        }
    case VT_ARRAY|VT_UI1:
    case VT_ARRAY|VT_UI2:
    case VT_ARRAY|VT_UI1|VT_BYREF:
    {
        SAFEARRAY *array = 0;
        if (msVariant.vt & VT_BYREF)
            array = *msVariant.pparray;
        else
            array = msVariant.parray;

        QByteArray bytes;
        if (!array || array->cDims != 1) {
            returnValue = bytes;
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

        returnValue = bytes;
    }
            break;

        //other types are developed, add is needed
        //...
    }

    return returnValue;
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

