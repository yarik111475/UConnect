#ifndef WINUTILS_H
#define WINUTILS_H

#include <QJsonObject>

#if defined(Q_OS_WIN)
#include <list>
#include <string>
#define _WIN32_DCOM
#include <wbemidl.h>
#include <comdef.h>
#endif

class WinUtils
{
public:
    WinUtils()=default;
    static QJsonObject display_wmi();
    static QJsonArray displays_win();
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


#endif // WINUTILS_H
