#ifndef EXECUTECOMMAND_GLOBAL_H
#define EXECUTECOMMAND_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EXECUTECOMMAND_LIBRARY)
#  define EXECUTECOMMAND_EXPORT Q_DECL_EXPORT
#else
#  define EXECUTECOMMAND_EXPORT Q_DECL_IMPORT
#endif

#endif // EXECUTECOMMAND_GLOBAL_H