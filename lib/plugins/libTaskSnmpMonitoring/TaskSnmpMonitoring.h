#ifndef SNMPMONITORING_GLOBAL_H
#define SNMPMONITORING_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SNMPMONITORING_LIBRARY)
#  define SNMPMONITORING_EXPORT Q_DECL_EXPORT
#else
#  define SNMPMONITORING_EXPORT Q_DECL_IMPORT
#endif

#endif // SNMPMONITORING_GLOBAL_H
