#ifndef UPLOADFILE_GLOBAL_H
#define UPLOADFILE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(UPLOADFILE_LIBRARY)
#  define UPLOADFILE_EXPORT Q_DECL_EXPORT
#else
#  define UPLOADFILE_EXPORT Q_DECL_IMPORT
#endif

#endif // UPLOADFILE_GLOBAL_H
