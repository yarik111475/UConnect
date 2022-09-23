#ifndef UPDATE_GLOBAL_H
#define UPDATE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(UPDATE_LIBRARY)
#  define UPDATE_EXPORT Q_DECL_EXPORT
#else
#  define UPDATE_EXPORT Q_DECL_IMPORT
#endif

#endif // UPDATE_GLOBAL_H
