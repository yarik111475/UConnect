#ifndef SERVICEAUDIT_GLOBAL_H
#define SERVICEAUDIT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SERVICEAUDIT_LIBRARY)
#  define SERVICEAUDIT_EXPORT Q_DECL_EXPORT
#else
#  define SERVICEAUDIT_EXPORT Q_DECL_IMPORT
#endif

#endif // SERVICEAUDIT_GLOBAL_H
