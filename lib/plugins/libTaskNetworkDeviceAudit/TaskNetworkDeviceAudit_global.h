#ifndef NETWORKDEVICEAUDIT_GLOBAL_H
#define NETWORKDEVICEAUDIT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(NETWORKDEVICEAUDIT_LIBRARY)
#  define NETWORKDEVICEAUDIT_EXPORT Q_DECL_EXPORT
#else
#  define NETWORKDEVICEAUDIT_EXPORT Q_DECL_IMPORT
#endif

#endif // NETWORKDEVICEAUDIT_GLOBAL_H
