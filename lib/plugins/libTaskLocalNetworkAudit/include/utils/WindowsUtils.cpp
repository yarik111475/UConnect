#include <QRect>
#include <QString>
#include <QtGlobal>
#include <QJsonArray>
#include <QVersionNumber>

#include "include/Defines.h"
#include "include/wmi/Wmi.h"
#include "include/utils/Utils.h"
#include "include/utils/WindowsUtils.h"

#if defined(Q_OS_WIN)
#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <lm.h>
#include <sddl.h>

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "wbemuuid.lib")

struct MonitorSet
{
    enum { MaxMonitors = 8 };
    HMONITOR Monitors[MaxMonitors];
    int      MonitorCount;
};

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData)
{
    MonitorSet* monitorSet = (MonitorSet*)dwData;
    if (monitorSet->MonitorCount > MonitorSet::MaxMonitors)
        return FALSE;

    monitorSet->Monitors[monitorSet->MonitorCount] = hMonitor;
    monitorSet->MonitorCount++;
    return TRUE;
}

//QString UcCore::WindowsUtils::sidByName(const QString &userName) {

//    #define USERNAME_SIZE 512

//    TCHAR lpAccountName[USERNAME_SIZE] = {0};
//    userName.toWCharArray(lpAccountName);

//    BYTE sidbuf[SECURITY_MAX_SID_SIZE];
//    PSID sid { static_cast<PSID>(sidbuf) };
//    const DWORD SidSize { sizeof(sidbuf) };

//    SID_NAME_USE snu;

//    DWORD cbSid { SidSize };
//    DWORD cchRD { 0 };

//    LPTSTR rd = nullptr;
//    auto succ { LookupAccountName(nullptr, lpAccountName, sid, &cbSid, rd, &cchRD, &snu) };
//    if (!succ) {
//        const auto& lastError { GetLastError() };
//        if ( lastError != ERROR_INSUFFICIENT_BUFFER) {
////            qDebug() << "GetLastError" << lastError;
//            return QString();
//        }
//        rd = (LPTSTR) LocalAlloc(LPTR, cchRD * sizeof(*rd));
//        if (!rd) {
////            qDebug() << "!rd";
//            SetLastError(ERROR_OUTOFMEMORY);
//            return QString();
//        }
//        cbSid = SidSize;
//        /*succ = */LookupAccountName(nullptr, lpAccountName, sid, &cbSid, rd, &cchRD, &snu);
//        LocalFree(rd);
//    }

//    LPTSTR StringSid = nullptr;
//    ConvertSidToStringSid(sid,&StringSid);

//    return QString::fromWCharArray(StringSid);
//}

bool WindowsUtils::isHyperV() {

    if (!Utils::isElevated()) {
        return false;
    }

    std::unique_ptr<Wmi> wmi;
    wmi.reset(new Wmi());
    wmi->init((_bstr_t)"root\\virtualization\\v2");

    IEnumWbemClassObject* pEnumerator { nullptr };
    PointerRelaser<IEnumWbemClassObject> wrpEnumerator {&pEnumerator};

    QString query {
        "SELECT Caption, Name, ElementName"
        ", EnabledState, OnTimeInMilliseconds "
        "FROM Msvm_ComputerSystem "
        "WHERE Caption='Virtual Machine' or Caption='Виртуальная машина'"
    };

    const auto& bstrQwery { ::SysAllocString(query.toStdWString().c_str())};

    {//make query
        const HRESULT hr=wmi->wbemServices()->ExecQuery(bstr_t("WQL"), bstrQwery
                                                        , WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY
                                                        , NULL,
                                                        &pEnumerator);

        ::SysFreeString(bstrQwery);

        if (FAILED(hr)) {
            return false;
        }
    }

    {// parse result of query
        IWbemClassObject *pclsObj { nullptr };
        ULONG uReturn { 0 };

        while (pEnumerator) {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
                &pclsObj, &uReturn);
            if(0 == uReturn) {
                break;
            }
            else {
                return true; //found at least 1 VM
            }

            pclsObj->Release();
        }
    }

    return false;
}

void WindowsUtils::checkKernelVersionForHyperv() {
    const auto& kernelType { QSysInfo::kernelType() };
    const auto& kernelVersion { QSysInfo::kernelVersion() };
    QVersionNumber kernelVersionQversion { QVersionNumber::fromString(kernelVersion) };
    QVersionNumber minimalVersion (6,2,0);

    if ("winnt" == kernelType.toLower()) { //only winNt kerenels
        if (kernelVersionQversion < minimalVersion) { //min version is 6.2 (Windows Server 2012)
            /*
            const auto msg {
               QString { "Current kernel version is '%1'. Minimal version: '%2'" }
                       .arg(kernelVersion)
                       .arg(minimalVersion.toString())
            };
            THROW_EXCEPTION(msg);*/
        }
    }
}

QString WindowsUtils::extractBstringValue(IWbemClassObject *pclsObj, LPCWSTR fieldName) {
    VARIANT vtProp;
    pclsObj->Get(fieldName, 0, &vtProp, 0, 0);
    const auto& result { QString::fromWCharArray(vtProp.bstrVal) };
    VariantClear(&vtProp);

    return result;
}

QString WindowsUtils::extractUint32Value(IWbemClassObject *pclsObj, LPCWSTR fieldName) {
    VARIANT vtProp;
    pclsObj->Get(fieldName, 0, &vtProp, 0, 0);
    const auto& result { QString::number(vtProp.ulVal) };
    VariantClear(&vtProp);

    return result;
}

WindowsUtils::WindowsUtils() {

}

//UcCore::WindowsUtils::UsersList UcCore::WindowsUtils::userList() {

//    UsersList userList;

//    LPUSER_INFO_3 pBuf = nullptr;
//    LPUSER_INFO_3 pTmpBuf;
//    DWORD dwLevel = 3;
//    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
//    DWORD dwEntriesRead = 0;
//    DWORD dwTotalEntries = 0;
//    DWORD dwResumeHandle = 0;
//    DWORD i;
//    DWORD dwTotalCount = 0;
//    NET_API_STATUS nStatus;
//    LPTSTR pszServerName = nullptr;

//    do {
//        nStatus = NetUserEnum(
//                              (LPCWSTR) pszServerName,
//                              dwLevel,
//                              FILTER_NORMAL_ACCOUNT, // global users
//                              (LPBYTE*)&pBuf,
//                              dwPrefMaxLen,
//                              &dwEntriesRead,
//                              &dwTotalEntries,
//                              &dwResumeHandle);

//        if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
//           if ((pTmpBuf = pBuf) != nullptr)
//           {
//              //
//              // Loop through the entries.
//              //
//              for (i = 0; (i < dwEntriesRead); i++) {

//                 if (pTmpBuf == nullptr)
//                 {
//                    fprintf(stderr, "An access violation has occurred\n");
//                    break;
//                 }
//                 //
//                 //  Print the name of the user account.
//                 //
////                 wprintf(L"\t-- %s\n", pTmpBuf->usri3_name);

//                 userList.push_back(QString::fromWCharArray(pTmpBuf->usri3_name).toStdString());

//                 pTmpBuf++;
//                 dwTotalCount++;
//              }
//           }
//        }
////        else {
////            fprintf(stderr, "A system error has occurred: %d\n", nStatus);
////        }

//        // Free the allocated buffer.
//        if (pBuf != nullptr) {
//           NetApiBufferFree(pBuf);
//           pBuf = nullptr;
//        }
//    }
//    while (nStatus == ERROR_MORE_DATA); // end do

//    if (pBuf != nullptr) {
//        NetApiBufferFree(pBuf);
//    }

//    return userList;
//}

QJsonArray WindowsUtils::userListWmi() {

    QJsonArray userList;

    QJsonObject result;
    std::unique_ptr<Wmi> wmi;
    wmi.reset(new Wmi());
    wmi->init((_bstr_t)"ROOT\\CIMV2");

    IEnumWbemClassObject* pEnumerator { nullptr };
    PointerRelaser<IEnumWbemClassObject> wrpEnumerator {&pEnumerator};

    {//make query
        const HRESULT hr=wmi->wbemServices()->ExecQuery(
                        bstr_t("WQL"),
                        bstr_t("SELECT AccountType, Domain, FullName, LocalAccount, Name, SID FROM Win32_UserAccount"),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &pEnumerator);

        if (FAILED(hr)) {
            /*
            const auto msg {
               QString { "%1. Query for operating system name failed. Error code : '%2'" }
               .arg(CODE_LOCATION_STRING)
                       .arg(hr)
            };
            THROW_EXCEPTION(msg);*/
        }
    }

    {// parse result of query
        IWbemClassObject *pclsObj { nullptr };
        ULONG uReturn { 0 };

        while (pEnumerator) {
            HRESULT hr {
                pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)
            };

            if (0 == uReturn) { return userList; }

            {
                const QString& domain { WindowsUtils::extractBstringValue(pclsObj, L"Domain") };
                const QString& name   { WindowsUtils::extractBstringValue(pclsObj, L"Name") };
                const QString& sid    { WindowsUtils::extractBstringValue(pclsObj, L"SID") };

                const QJsonObject& item {
                      { "domain", domain }
                    , { "name", name }
                    , { "sid", sid }
                };

                userList.append(item);
            }

            pclsObj->Release();
        }
    }

    return userList;
}

QJsonArray WindowsUtils::userProfileList() {
    QJsonArray resultArray;

    QJsonObject result;
    std::unique_ptr<Wmi> wmi;
    wmi.reset(new Wmi());
    wmi->init((_bstr_t)"ROOT\\CIMV2");

    IEnumWbemClassObject* pEnumerator { nullptr };
    PointerRelaser<IEnumWbemClassObject> wrpEnumerator {&pEnumerator};

    {//make query
        const HRESULT hr {
            wmi->wbemServices()->ExecQuery(
                        bstr_t("WQL"),
                        bstr_t("SELECT SID FROM Win32_UserProfile"),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &pEnumerator)
                        };
        if (FAILED(hr)) {
            /*
            const auto msg {
               QString { "%1. Query for operating system name failed. Error code : '%2'" }
               .arg(CODE_LOCATION_STRING)
                       .arg(hr)
            };
            THROW_EXCEPTION(msg);*/
        }
    }

    {// parse result of query
        IWbemClassObject *pclsObj { nullptr };
        ULONG uReturn { 0 };

        while (pEnumerator) {
            HRESULT hr {
                pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)
            };

            if (0 == uReturn) { return resultArray; }

            {
                const QString& sid { WindowsUtils::extractBstringValue(pclsObj, L"SID") };

                const QJsonObject& item {
                    { "sid", sid }
                };

                resultArray.append(item);
            }

            pclsObj->Release();
        }
    }

    return resultArray;
}

QJsonArray WindowsUtils::usersAtLeastEnteredList() {
    const auto& userFullList { WindowsUtils::userListWmi() };
    const auto& userProfileList { WindowsUtils::userProfileList() };

    QSet<QString> userProfileListSet;
    {//populate userProfileListSet
        for (const auto& item: userProfileList) {
            userProfileListSet.insert(item["sid"].toString());
        }
    }

    QJsonArray userWorkList; //used for get data only from user on local comp
    {//prepare set of userprofiles sid
        for (const auto& item: userFullList) {
            const auto& sid { item["sid"].toString() };

            if (userProfileListSet.contains(sid)) {
                userWorkList.append(item);
            }
        }
    }

    return userWorkList;
}

QJsonObject WindowsUtils::wmiResultMemory() {
    QJsonObject result;
    std::unique_ptr<Wmi> wmi;
    wmi.reset(new Wmi());
    wmi->init((_bstr_t)"ROOT\\CIMV2");

    IEnumWbemClassObject* pEnumerator { nullptr };
    PointerRelaser<IEnumWbemClassObject> wrpEnumerator {&pEnumerator};

    {//make query
        const HRESULT hr {
            wmi->wbemServices()->ExecQuery(
                        bstr_t("WQL"),
                        bstr_t("SELECT FreePhysicalMemory, TotalVisibleMemorySize FROM Win32_OperatingSystem"),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &pEnumerator)
                        };
        if (FAILED(hr)) {
            /*
            const auto msg {
               QString { "%1. Query for operating system name failed. Error code : '%2'" }
               .arg(CODE_LOCATION_STRING)
                       .arg(hr)
            };
            THROW_EXCEPTION(msg);*/
        }
    }

    {// parse result of query
        IWbemClassObject *pclsObj { nullptr };
        ULONG uReturn { 0 };

        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn) { return QJsonObject(); }

        {//total ram
            const QString& value { extractBstringValue(pclsObj, L"TotalVisibleMemorySize") };
            result.insert("totalRam", value.toLongLong() * 1'024);
        }

        {//free ram
            const QString& value { extractBstringValue(pclsObj, L"FreePhysicalMemory") };
            result.insert("freeRam", value.toLongLong() * 1'024);
        }

        pclsObj->Release();
    }

    return result;
}

QJsonObject WindowsUtils::biosInfo() {

    QJsonObject result;
    std::unique_ptr<Wmi> wmi;
    wmi.reset(new Wmi());
    wmi->init((_bstr_t)"ROOT\\CIMV2");

    IEnumWbemClassObject* pEnumerator { nullptr };
    PointerRelaser<IEnumWbemClassObject> wrpEnumerator {&pEnumerator};

    {//make query
        const HRESULT hr {
            wmi->wbemServices()->ExecQuery(
                        bstr_t("WQL"),
                        bstr_t("SELECT SerialNumber, Manufacturer, Name, SMBIOSBIOSVersion FROM Win32_BIOS"),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &pEnumerator)
                        };
        if (FAILED(hr)) {
            /*
            const auto msg {
               QString { "%1. Query for operating system name failed. Error code : '%2'" }
               .arg(CODE_LOCATION_STRING)
                       .arg(hr)
            };
            THROW_EXCEPTION(msg);*/
        }
    }

    {// parse result of query
        IWbemClassObject *pclsObj { nullptr };
        ULONG uReturn { 0 };



        HRESULT hr {
            pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)
        };

        if (0 == uReturn) { return result; }

        {
            const QString& value { WindowsUtils::extractBstringValue(pclsObj, L"SerialNumber") };
            result.insert("serialNumber", value);
        }
        {
            const QString& value { extractBstringValue(pclsObj, L"Manufacturer") };
            result.insert("manufacturer", value);
        }
        {
            const QString& value { extractBstringValue(pclsObj, L"Name") };
            result.insert("productName", value);
        }
        {
            const QString& value { extractBstringValue(pclsObj, L"SMBIOSBIOSVersion") };
            result.insert("version", value);
        }

        pclsObj->Release();
    }

    return result;
}

QString WindowsUtils::buildUserDomainName(const QString &userName, const QString &domainName) {
    return domainName + "\\" + userName;
}

int WindowsUtils::displaysCount()
{
    return GetSystemMetrics(SM_CMONITORS);
}

QJsonArray WindowsUtils::displaysWmi()
{
    QJsonArray displaysJSON;

    std::unique_ptr<Wmi> wmi;
    wmi.reset(new Wmi());
    wmi->init((_bstr_t)"ROOT\\WMI");

    IEnumWbemClassObject* pEnumerator { nullptr };
    PointerRelaser<IEnumWbemClassObject> wrpEnumerator {&pEnumerator};

    {//make query
        const HRESULT hr {
            wmi->wbemServices()->ExecQuery(
                        bstr_t("WQL"),
                        bstr_t("SELECT * FROM WMIMonitorID"),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &pEnumerator)
                        };
        if (FAILED(hr)) {
            /*
            const auto msg {
               QString { "%1. Query for operating system name failed. Error code : '%2'" }
               .arg(CODE_LOCATION_STRING)
                       .arg(hr)
            };
            THROW_EXCEPTION(msg); */
        }
    }

    {// parse result of query
        IWbemClassObject *pclsObj { nullptr };
        ULONG uReturn { 0 };

        while (pEnumerator) {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
                &pclsObj, &uReturn);

            if (0 == uReturn)
                break;

            VARIANT vtProp;
            CIMTYPE vtType;

            HRESULT hrGet = pclsObj->Get(L"InstanceName", 0, &vtProp, &vtType, 0);
            QVariant InstanceName;
            if (hrGet == ERROR_SUCCESS) {
                InstanceName = wmi->msVariantToQVariant(vtProp);
                VariantClear(&vtProp);
            }

            hrGet = pclsObj->Get(L"UserFriendlyNameLength", 0, &vtProp, &vtType, 0);
            QVariant UserFriendlyNameLength{-1};
            if (hrGet == ERROR_SUCCESS) {
                UserFriendlyNameLength = wmi->msVariantToQVariant(vtProp);
                VariantClear(&vtProp);
            }

            hrGet = pclsObj->Get(L"UserFriendlyName", 0, &vtProp, &vtType, 0);
            QVariant UserFriendlyName;
            if (hrGet == ERROR_SUCCESS) {
//                UserFriendlyName = ucWmi->msVariantToQVariant(vtProp, vtType, UserFriendlyNameLength.toInt());
                UserFriendlyName = wmi->msVariantToQVariant(vtProp, vtType);
                VariantClear(&vtProp);
            }

            hrGet = pclsObj->Get(L"ManufacturerNameLength", 0, &vtProp, &vtType, 0);
            QVariant ManufacturerNameLength{-1};
            if (hrGet == ERROR_SUCCESS) {
                ManufacturerNameLength = wmi->msVariantToQVariant(vtProp);
                VariantClear(&vtProp);
            }

            hrGet = pclsObj->Get(L"ManufacturerName", 0, &vtProp, &vtType, 0);
            QVariant ManufacturerName;
            if (hrGet == ERROR_SUCCESS) {
//                ManufacturerName = ucWmi->msVariantToQVariant(vtProp, vtType, ManufacturerNameLength.toInt());
                ManufacturerName = wmi->msVariantToQVariant(vtProp, vtType);
                VariantClear(&vtProp);
            }

            hrGet = pclsObj->Get(L"ProductCodeID", 0, &vtProp, &vtType, 0);
            QVariant ProductCodeID;
            if (hrGet == ERROR_SUCCESS) {
                ProductCodeID= wmi->msVariantToQVariant(vtProp, vtType);
                VariantClear(&vtProp);
            }

            hrGet = pclsObj->Get(L"SerialNumberID", 0, &vtProp, &vtType, 0);
            QVariant SerialNumberID;
            if (hrGet == ERROR_SUCCESS) {
                SerialNumberID = wmi->msVariantToQVariant(vtProp, vtType);
                VariantClear(&vtProp);
            }

            pclsObj->Release();

            QJsonObject display;

            display.insert("instanceName", InstanceName.toString());
            display.insert("model", UserFriendlyName.toString());
            display.insert("manufacturerName", ManufacturerName.toString());
            display.insert("serialNumber", SerialNumberID.toString());
            display.insert("productCode", ProductCodeID.toString());

            displaysJSON.append(display);
        }
    }

    return displaysJSON;
}

QJsonArray WindowsUtils::displays()
{
    QJsonArray displaysJSON;

    MonitorSet monitors;
    monitors.MonitorCount = 0;

    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors);

    MONITORINFOEX info;
    DEVMODE devMode;

    QJsonArray displaysWmi = WindowsUtils::displaysWmi();

    for (auto monitorIndex = 0; monitorIndex < monitors.MonitorCount; monitorIndex++)
    {
        QVariantMap display;

        info = {};
        info.cbSize = sizeof(info);

        const int infoResult = GetMonitorInfo(monitors.Monitors[monitorIndex], (LPMONITORINFO)&info);

        if (infoResult) {
            const RECT &rcMonitor = info.rcMonitor;
            const RECT &rcWork = (info.rcWork);
            const bool is_primary = ((info.dwFlags & MONITORINFOF_PRIMARY) != 0);
            const auto szDevice = QString::fromStdWString(info.szDevice);

            const QRect rect(rcMonitor.left, rcMonitor.top, rcMonitor.right - rcMonitor.left, rcMonitor.bottom - rcMonitor.top);

            display.insert("left", rect.left());
            display.insert("top", rect.top());
            display.insert("width", rect.width());
            display.insert("height", rect.height());
            display.insert("name", szDevice);

            if (is_primary)
                display.insert("is_primary", true);
        }

        devMode = {};
        devMode.dmSize = sizeof(DEVMODE);
        devMode.dmDriverExtra = 0;
        int settingsResult = EnumDisplaySettingsEx(info.szDevice, ENUM_CURRENT_SETTINGS, &devMode, 0);

        //TODO DMDO_DEFAULT привести к виду Qt::ScreenOrientations
        if (infoResult && settingsResult) {
            DWORD orientation = devMode.dmDisplayOrientation;
            display.insert("orientation", (devMode.dmFields & DM_DISPLAYORIENTATION ? (qint64)orientation : QJsonValue::Null) );
        }

        if (displaysWmi.count()) {
            auto wmiInfo = displaysWmi.takeAt(0);
            display.insert(wmiInfo.toObject().toVariantMap());
        }

        displaysJSON.append(QJsonObject::fromVariantMap(display));
    }

//not need because UcCore::UcWmi work have destructor
//    pEnumerator->Release();

    return displaysJSON;
}

#endif

