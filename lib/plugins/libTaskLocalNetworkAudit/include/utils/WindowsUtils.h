#ifndef WINDOWSUTILS_H
#define WINDOWSUTILS_H

#include <QJsonObject>

#if defined(Q_OS_WIN)
#include <list>
#include <string>
#define _WIN32_DCOM
#include <wbemidl.h>
#include <comdef.h>

class WindowsUtils {

    using UsersList = std::list<std::string>;

public:
    WindowsUtils();
//        static UsersList userList();
    static QJsonArray userListWmi();
    static QJsonArray userProfileList();
//        static QString sidByName(const QString& userName);
    static QJsonArray usersAtLeastEnteredList();

    static bool isHyperV();
    static void checkKernelVersionForHyperv();

    static QString extractBstringValue(IWbemClassObject *pclsObj, LPCWSTR fieldName);
    static QString extractUint32Value(IWbemClassObject *pclsObj, LPCWSTR fieldName);

    static QJsonObject wmiResultMemory();
    static QJsonObject biosInfo();

    static QString buildUserDomainName(const QString& userName, const QString& domainName);

    static int displaysCount();
    static QJsonArray displaysWmi();
    static QJsonArray displays();

};

template<typename Pointer>
class PointerRelaser {
    Pointer** _ptr;
public:
    PointerRelaser(Pointer** ptr): _ptr { ptr } {

    }

    ~PointerRelaser() {
        if (*_ptr) {
            (&**_ptr)->Release();
        }
    }
};
#endif


#endif // WINDOWSUTILS_H
