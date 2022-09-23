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

    bool init(const BSTR wmiNamespace);
    IWbemLocator *wbemLocator() const;
    IWbemServices *wbemServices() const;
    QVariant msVariantToQVariant(const VARIANT &msVariant, long type=0xfff, quint64 arraySize=-1);

    QVariant VARIANTToQVariant(const VARIANT &arg, const QByteArray &typeName, uint type);
    void release();
};
#endif

#endif // WMI_H
