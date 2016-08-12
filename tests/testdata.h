#ifndef TESTDATA_INCLUDE
#define TESTDATA_INCLUDE

#include <stdio.h>
#include <stdlib.h>

#define FIRST_INT       0xabcd1234 
#define FILE_VERSION    0x2
#define DATATYPE        8
#define dPRINTOK        0

#ifdef _WIN32
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#define OS_WIN
#else
#define OS_LINUX
#endif

int readIntArrayFile(  const char* fileName, int ** ar, size_t *len );
int writeIntArrayFile( const char* fileName, int ** ar, size_t  len );

#endif
