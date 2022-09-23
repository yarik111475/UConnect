#include <QRect>
#include <QtGlobal>
#include <QString>
#include <QJsonArray>
#include <QVersionNumber>

#include "WinUtils.h"

#ifdef Q_OS_WINDOWS
#include "Wmi.h"
#include <windows.h>
#include <lm.h>
#include <sddl.h>

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "wbemuuid.lib")
#ifndef UNICODE
#define UNICODE
#endif

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

QJsonObject WinUtils::display_wmi()
{
    QJsonObject display_json {};

    std::unique_ptr<Wmi> wmi;
    wmi.reset(new Wmi());
    wmi->init((BSTR)"ROOT\\WMI");

    IEnumWbemClassObject* pEnumerator { nullptr };
    PointerRelaser<IEnumWbemClassObject> wrpEnumerator {&pEnumerator};

    {//make query
        const HRESULT hr {
            wmi->wbemServices()->ExecQuery(
                        BSTR ("WQL"),
                        BSTR ("SELECT * FROM WMIMonitorID"),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &pEnumerator)
                        };
        if (FAILED(hr)) {
            return display_json;
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
                InstanceName = wmi->ms_variant_to_qvariant(vtProp);
                VariantClear(&vtProp);
            }

            hrGet = pclsObj->Get(L"UserFriendlyNameLength", 0, &vtProp, &vtType, 0);
            QVariant UserFriendlyNameLength{-1};
            if (hrGet == ERROR_SUCCESS) {
                UserFriendlyNameLength = wmi->ms_variant_to_qvariant(vtProp);
                VariantClear(&vtProp);
            }

            hrGet = pclsObj->Get(L"UserFriendlyName", 0, &vtProp, &vtType, 0);
            QVariant UserFriendlyName;
            if (hrGet == ERROR_SUCCESS) {
//                UserFriendlyName = ucWmi->msVariantToQVariant(vtProp, vtType, UserFriendlyNameLength.toInt());
                UserFriendlyName = wmi->ms_variant_to_qvariant(vtProp, vtType);
                VariantClear(&vtProp);
            }

            hrGet = pclsObj->Get(L"ManufacturerNameLength", 0, &vtProp, &vtType, 0);
            QVariant ManufacturerNameLength{-1};
            if (hrGet == ERROR_SUCCESS) {
                ManufacturerNameLength = wmi->ms_variant_to_qvariant(vtProp);
                VariantClear(&vtProp);
            }

            hrGet = pclsObj->Get(L"ManufacturerName", 0, &vtProp, &vtType, 0);
            QVariant ManufacturerName;
            if (hrGet == ERROR_SUCCESS) {
//                ManufacturerName = ucWmi->msVariantToQVariant(vtProp, vtType, ManufacturerNameLength.toInt());
                ManufacturerName = wmi->ms_variant_to_qvariant(vtProp, vtType);
                VariantClear(&vtProp);
            }

            hrGet = pclsObj->Get(L"ProductCodeID", 0, &vtProp, &vtType, 0);
            QVariant ProductCodeID;
            if (hrGet == ERROR_SUCCESS) {
                ProductCodeID= wmi->ms_variant_to_qvariant(vtProp, vtType);
                VariantClear(&vtProp);
            }

            hrGet = pclsObj->Get(L"SerialNumberID", 0, &vtProp, &vtType, 0);
            QVariant SerialNumberID;
            if (hrGet == ERROR_SUCCESS) {
                SerialNumberID = wmi->ms_variant_to_qvariant(vtProp, vtType);
                VariantClear(&vtProp);
            }

            pclsObj->Release();

            QJsonObject display;

            display_json.insert("instanceName", InstanceName.toString());
            display_json.insert("model", UserFriendlyName.toString());
            display_json.insert("manufacturer", ManufacturerName.toString());
            display_json.insert("serial", SerialNumberID.toString());
            display_json.insert("productCode", ProductCodeID.toString());
        }
    }

    return display_json;
}

QJsonArray WinUtils::displays_win()
{
    QJsonArray displays_array_json {};

    MONITORINFOEX info;
    DEVMODE devMode;
    MonitorSet monitors;
    monitors.MonitorCount = 0;

    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors);
    for (auto monitorIndex = 0; monitorIndex < monitors.MonitorCount; monitorIndex++)
    {
        QJsonObject display_json;

        info = {};
        info.cbSize = sizeof(info);

        const int infoResult = GetMonitorInfo(monitors.Monitors[monitorIndex], (LPMONITORINFO)&info);

        if (infoResult) {
            const RECT &rcMonitor = info.rcMonitor;
            const RECT &rcWork = (info.rcWork);
            const bool is_primary = ((info.dwFlags & MONITORINFOF_PRIMARY) != 0);
            const auto szDevice = QString::fromStdWString(info.szDevice);

            const QRect rect(rcMonitor.left, rcMonitor.top, rcMonitor.right - rcMonitor.left, rcMonitor.bottom - rcMonitor.top);
            /* not used yet
            display_json.insert("left", rect.left());
            display_json.insert("top", rect.top());
            */
            display_json.insert("width", rect.width());
            display_json.insert("height", rect.height());
            display_json.insert("name", szDevice);

            if (is_primary)
                display_json.insert("is_primary", true);
        }

        devMode = {};
        devMode.dmSize = sizeof(DEVMODE);
        devMode.dmDriverExtra = 0;
        int settingsResult = EnumDisplaySettingsEx(info.szDevice, ENUM_CURRENT_SETTINGS, &devMode, 0);

        //TODO DMDO_DEFAULT привести к виду Qt::ScreenOrientations
        if (infoResult && settingsResult) {
            DWORD orientation = devMode.dmDisplayOrientation;
            display_json.insert("orientation", (devMode.dmFields & DM_DISPLAYORIENTATION ? (qint64)orientation : QJsonValue::Null) );
        }

        QJsonObject display_wmi_json = WinUtils::display_wmi();
        if (!display_wmi_json.isEmpty()) {
            display_json["model"]=display_json["model"].toString();
            display_json["serial"]=display_json["serial"].toString();
            display_json["manufacturer"]=display_json["manufacturer"].toString();
        }

        displays_array_json.append(display_json);
    }
    return displays_array_json;
}
#endif
