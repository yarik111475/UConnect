#ifndef SOFTAUDIT_GLOBAL_H
#define SOFTAUDIT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SOFTAUDIT_LIBRARY)
#  define SOFTAUDIT_EXPORT Q_DECL_EXPORT
#else
#  define SOFTAUDIT_EXPORT Q_DECL_IMPORT
#endif

#endif // SOFTAUDIT_GLOBAL_H
