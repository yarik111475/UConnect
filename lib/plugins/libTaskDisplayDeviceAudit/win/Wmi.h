#ifndef WMI_H
#define WMI_H

#include <QtGlobal>
#include <QVariant>


#if defined(Q_OS_WIN)
#include <wtypes.h>

class IWbemLocator;
class IWbemServices;

class Wmi {

    IWbemLocator* _wbemLocator { nullptr };
    IWbemServices* _wbemServices { nullptr };

public:
    Wmi() = default;
    Wmi(const Wmi& ) = delete;
    ~Wmi();

    Wmi& operator=(const Wmi&) = delete;

    bool init(const BSTR wmi_namespace);
    IWbemLocator *wbemLocator() const;
    IWbemServices *wbemServices() const;
    QVariant ms_variant_to_qvariant(const VARIANT &ms_variant, long type=0xfff, quint64 array_size=-1);
    void release();
};
#endif

#endif // WMI_H
