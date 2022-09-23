#include <QJsonArray>
#include <QJsonObject>

#if defined(Q_OS_WIN)

#include <windows.h>
#include <versionhelpers.h>
#include "atlbase.h"
#include "include/wmi/Wmi.h"
#include "include/utils/WindowsUtils.h"

#define _WIN32_DCOM
#include <wbemidl.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")

#endif

#if defined(Q_OS_MACOS)
#include <unistd.h>
#endif

#if defined(Q_OS_LINUX)
#include <unistd.h>
#endif

#include "Utils.h"

bool Utils::isElevated()
{
#if defined(Q_OS_WIN)

        HANDLE hToken { nullptr };

        BOOL fRet { FALSE };
        if ( OpenProcessToken( GetCurrentProcess( ), TOKEN_QUERY, &hToken ) ) {
            TOKEN_ELEVATION Elevation;
            DWORD cbSize { sizeof( TOKEN_ELEVATION ) };
            if( GetTokenInformation( hToken, TokenElevation, &Elevation, sizeof( Elevation ), &cbSize ) ) {
                fRet = Elevation.TokenIsElevated;
            }
        }
        else {
            if( hToken ) { CloseHandle( hToken ); }
            const auto& lastError { GetLastError() };
            /*
            THROW_EXCEPTION(QString { "Failed to OpenProcessToken lastError: '%1'"}
                            .arg(lastError));*/
        }

        if( hToken ) { CloseHandle( hToken ); }

        return fRet;
#else
    return false;
#endif
}

QJsonArray Utils::string_list_to_json_array(const QStringList &string_list)
{
    QJsonArray json_array;
    for (const auto& item: string_list) {
        json_array.append(item);
    }
    return json_array;
}

void Utils::append_json_object_to_json_object(const QJsonObject &source, QJsonObject &target)
{
    for (auto&& it=source.begin(); it != source.end(); it++  ) {
        if (!target.contains(it.key())) { //prevent replace exists item
            target.insert(it.key(), it.value());
        }
    }
}
