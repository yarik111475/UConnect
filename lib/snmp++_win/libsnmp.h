/*_############################################################################
  _## 
  _##  cmake_libsnmp.h.in  
  _##
  _##  SNMP++ v3.4
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2021 Jochen Katz, Frank Fock
  _##
  _##  This software is based on SNMP++2.6 from Hewlett Packard:
  _##  
  _##    Copyright (c) 1996
  _##    Hewlett-Packard Company
  _##  
  _##  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  _##  Permission to use, copy, modify, distribute and/or sell this software 
  _##  and/or its documentation is hereby granted without fee. User agrees 
  _##  to display the above copyright notice and this license notice in all 
  _##  copies of the software and any documentation of the software. User 
  _##  agrees to assume all liability for the use of the software; 
  _##  Hewlett-Packard, Frank Fock, and Jochen Katz make no representations 
  _##  about the suitability of this software for any purpose. It is provided 
  _##  "AS-IS" without warranty of any kind, either express or implied. User 
  _##  hereby grants a royalty-free license to any and all derivatives based
  _##  upon this software code base. 
  _##  
  _##########################################################################*/
#ifndef __LIBSNMP_H_INCLUDED__
#define __LIBSNMP_H_INCLUDED__

/*
#define _WIN32_WINNT_NT4                    0x0400 // Windows NT 4.0
#define _WIN32_WINNT_WIN2K                  0x0500 // Windows 2000
#define _WIN32_WINNT_WINXP                  0x0501 // Windows XP
#define _WIN32_WINNT_WS03                   0x0502 // Windows Server 2003
#define _WIN32_WINNT_WIN6                   0x0600 // Windows Vista
#define _WIN32_WINNT_VISTA                  0x0600 // Windows Vista
#define _WIN32_WINNT_WS08                   0x0600 // Windows Server 2008
#define _WIN32_WINNT_LONGHORN               0x0600 // Windows Vista
#define _WIN32_WINNT_WIN7                   0x0601 // Windows 7
#define _WIN32_WINNT_WIN8                   0x0602 // Windows 8
#define _WIN32_WINNT_WINBLUE                0x0603 // Windows 8.1
#define _WIN32_WINNT_WINTHRESHOLD           0x0A00 // Windows 10
#define _WIN32_WINNT_WIN10                  0x0A00 // Windows 10
*/

//#define _WIN32_WINNT 0x0601 // Windows 7
//#define _WIN32_WINNT 0x0602 // Windows 8
#define _WIN32_WINNT 0x0A00 // Windows 10

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* #undef _DEBUG */

#define CNF_HAVE_SYS_TYPES_H
#ifdef CNF_HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#define CNF_HAVE_SYS_STAT_H
#ifdef CNF_HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#define CNF_STDCXX_98_HEADERS
#define CNF_STDC_HEADERS
#define CNF_HAVE_STDLIB_H
#define CNF_HAVE_STRING_H
#define CNF_HAVE_MEMORY_H
/* #undef CNF_HAVE_STRINGS_H */
#define CNF_HAVE_INTTYPES_H
#define CNF_HAVE_CTYPE_H
#define CNF_HAVE_SIGNAL_H
#define CNF_HAVE_ERRNO_H
#define CNF_HAVE_TIME_H
#define HAVE_STDIO_H
#define CNF_HAVE_IOSTREAM

#ifdef CNF_STDCXX_98_HEADERS
# include <cctype>
# include <cerrno>
# include <climits>
# include <csignal>
# include <cstddef>
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <ctime>
#else
# ifdef HAVE_STDIO_H
#  include <stdio.h>
# endif
# ifdef CNF_STDC_HEADERS
#  include <stdlib.h>
#  include <stddef.h>
# else
#  ifdef CNF_HAVE_STDLIB_H
#   include <stdlib.h>
#  endif
# endif
# ifdef CNF_HAVE_STRING_H
#  if !defined(CNF_STDC_HEADERS) && defined(CNF_HAVE_MEMORY_H)
#   include <memory.h>
#  endif
#  include <string.h>
# endif
# ifdef CNF_HAVE_STRINGS_H
#  include <strings.h>
# endif
# ifdef CNF_HAVE_INTTYPES_H
#  include <inttypes.h>
# endif
# ifdef CNF_HAVE_CTYPE_H
#  include <ctype.h>
# endif
# ifdef CNF_HAVE_SIGNAL_H
#  include <signal.h>
# endif
# ifdef CNF_HAVE_ERRNO_H
#  include <errno.h>
# endif
# ifdef CNF_HAVE_TIME_H
#  include <time.h>
#endif
#endif

/* #undef CNF_HAVE_UNISTD_H */
/* #undef CNF_HAVE_SYS_UNISTD_H */
#ifdef CNF_HAVE_UNISTD_H
# include <unistd.h>
#else
# ifdef CNF_HAVE_SYS_UNISTD_H
#  include <sys/unistd.h>
# endif
#endif

#define CNF_HAVE_STDINT_H
#ifdef CNF_HAVE_STDINT_H
# include <stdint.h>
#endif

/* #undef CNF_HAVE_SYS_TIME_H */
#ifdef CNF_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
/* #undef CNF_HAVE_SYS_PARAM_H */
#ifdef CNF_HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#define CNF_HAVE_SYS_TIMEB_H
#ifdef CNF_HAVE_SYS_TIMEB_H
#include <sys/timeb.h> // and _ftime
#endif

#define CNF_HAVE_WINSOCK2_H
#define CNF_HAVE_WS2TCPIP_H
/* #undef CNF_HAVE_WSPIAPI_H */
#define CNF_HAVE_WINSOCK_H
/* #undef CNF_HAVE_NETDB_H */
/* #undef CNF_HAVE_SYS_SOCKET_H */
/* #undef CNF_HAVE_ARPA_INET_H */
/* #undef CNF_HAVE_NETINET_IN_H */

#ifdef CNF_HAVE_WINSOCK2_H
# include <winsock2.h>
# ifdef CNF_HAVE_WS2TCPIP_H
#  include <ws2tcpip.h>
# endif
# ifdef CNF_HAVE_WSPIAPI_H
#  include <wspiapi.h>
# endif
#elif defined(CNF_HAVE_WINSOCK_H)
  /* IIRC winsock.h must be included before windows.h */
# include <winsock.h>
#else
# ifdef CNF_HAVE_NETDB_H
#  include <netdb.h>
# endif
# ifdef CNF_HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
# endif
# ifdef CNF_HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
# ifdef CNF_HAVE_NETINET_IN_H
#  include <netinet/in.h>
# endif
#endif

/* #undef CNF_HAVE_POLL_H */
/* #undef CNF_HAVE_SYS_SELECT_H */
#ifdef CNF_HAVE_POLL_H
# include <poll.h>
#endif
#ifdef CNF_HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif

#define CNF_HAVE_IO_H
#define CNF_HAVE_PROCESS_H
#ifdef _WIN32
# ifdef CNF_HAVE_IO_H
#  include <io.h>
# endif
# ifdef CNF_HAVE_PROCESS_H
#  include <process.h>
# endif
# include <windows.h>
#endif

/* #undef CNF_HAVE_STRCASECMP */
#define CNF_HAVE_STRICMP
#if !defined(CNF_HAVE_STRCASECMP)
# ifdef CNF_HAVE_STRICMP
#  define strcasecmp stricmp
# else
extern "C" int strcasecmp(const char *s1, const char *s2);
# endif
#endif

#define CNF_HAVE_GETPID
#define CNF_HAVE__GETPID
#if !defined(CNF_HAVE_GETPID)
# ifdef CNF_HAVE__GETPID
#  define getpid _getpid
# endif
#endif

#define CNF_HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY
/* #undef CNF_HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY */
#if !defined(CNF_HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY)
# ifdef CNF_HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY
#  define ss_family __ss_family
# endif
#endif

typedef int64_t pp_type_int64;
typedef uint64_t pp_type_uint64;

/* Minimum of signed integral types.  */
#ifndef INT8_MIN
# define INT8_MIN               (-128)
#endif
#ifndef INT16_MIN
# define INT16_MIN              (-32767-1)
#endif
#ifndef INT32_MIN
# define INT32_MIN              (-2147483647-1)
#endif
#ifndef INT64_MIN
# define INT64_MIN              (-pp_type_int64(9223372036854775807)-1)
#endif
/* Maximum of signed integral types.  */
#ifndef INT8_MAX
# define INT8_MAX               (127)
#endif
#ifndef INT16_MAX
# define INT16_MAX              (32767)
#endif
#ifndef INT32_MAX
# define INT32_MAX              (2147483647)
#endif
#ifndef INT64_MAX
# define INT64_MAX              (pp_type_int64(9223372036854775807))
#endif

/* Maximum of unsigned integral types.  */
#ifndef UINT8_MAX
# define UINT8_MAX              (255)
#endif
#ifndef UINT16_MAX
# define UINT16_MAX             (65535)
#endif
#ifndef UINT32_MAX
# define UINT32_MAX             (4294967295U)
#endif
#ifndef UINT64_MAX
# define UINT64_MAX             (pp_type_uint64(18446744073709551615))
#endif

#ifndef NULL
#define NULL    0
#endif

#ifdef CNF_HAVE_IOSTREAM
# include <iostream>
#else
# include <iostream.h>
#endif

#endif /* ?__LIBSNMP_H_INCLUDED__ */
