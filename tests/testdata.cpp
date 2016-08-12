/*====================================================================================================
Copyright (c) 2016 Gdansk University of Technology

Unless otherwise indicated, Source Code is licensed under MIT license.
See further explanation attached in License Statement (distributed in the file LICENSE).

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
====================================================================================================*/
/*====================================================================================================
  Author: Piotr Sypek
====================================================================================================*/
#include "testdata.h"

/*
 * Raw code for reading and writing raw data (integer arrays) to a file in C.
 * Works in Linux and Windows. Designed for saving big data chunks.
 * Two functions form an interface:
 *  int readIntArrayFile(  const char* fileName, int ** ar, long long int *len );
 *  int writeIntArrayFile( const char* fileName, int ** ar, long long int  len );
 */


// saves data on disk
ssize_t writeAll (FILE * fd, const void* buffer, size_t count){
    ssize_t left = (ssize_t)count; // left_to_write
    ssize_t smallcount = 1024*1024;
    ssize_t written;
    char * data = (char*)buffer;
    while(  left > 0 ){
        if( left < smallcount )
            smallcount = left;
        written = fwrite( (void*)data, 1, (size_t)smallcount, fd );
        if( written <= 0 )
            /* An error occurred; bail.  */
            return -1;
        else{
            /* Keep count of how much more we need to write.  */
            left -= written;
            data += written;
        }
    }
    if( left != 0 )
        return -1;
    /* The number of bytes written is exactly COUNT.  */
    return (ssize_t) count;
}

// reads data from disk
size_t readAll( FILE * fd, void* buffer, size_t count ){
    ssize_t left = count; // left_to_read
    ssize_t smallcount = 1024*1024;
    ssize_t dread;
    //off_t of;
    char * data = (char*)buffer;
    while(  left > 0 ){
        if( left < smallcount )
            smallcount = left;
        dread = fread( (void*)data, 1, (size_t) smallcount, fd );
        if( dread <= 0 )
            /* An error occurred; bail.  */
            //fread() does not distinguish between end-of-file and error, and callers must use feof(3) and ferror(3) to determine which occurred.
            return -1;
        else
            /* Keep count of how much more we need to write.  */
            left -= dread;
            data += dread;
    }
    if( left != 0 )
        return -1;
    /* The number of bytes read is exactly COUNT.  */
    return (ssize_t) count;
}

int checkTransfer( const char * desc, size_t si, size_t sr, int printOk ){
    if( sr!=si ){
        printf( "Data transfer failed [%s] (si=%d,sr=%d)\n", desc, (int)si, (int)sr );
        return 1;
    }
    if( printOk==1 )
        printf( "Data transfer complited [%s] (si=%d,sr=%d)\n", desc, (int)si, (int)sr );

    return 0;
}


int readInt( FILE * matrixFile, int * l ){
    size_t sr;

    size_t si;

    si =  sizeof(int); sr = readAll( matrixFile, (void *)l, si );
    if( checkTransfer( "readInt", si, sr, dPRINTOK ) ) return 1;
    //fflush( matrixFile );

    return 0;
}

int writeInt( FILE * matrixFile, int l ){
    size_t sr;  // ret
    size_t si;  // in

    si = sizeof(int); sr = writeAll( matrixFile, (void *)&l, si );
    if( checkTransfer( "writeInt", si, sr, dPRINTOK ) ) return 1;

    return 0;
}

int writeIntArray( FILE * matrixFile, int * ar, size_t len ){
    size_t sr;  // ret
    size_t si;  // in

    si = len * sizeof(int); sr = writeAll( matrixFile, (void *) ar, si );
    if( checkTransfer( "writeIntArray", si, sr, dPRINTOK ) ) return 1;

    return 0;
}

int readIntArray( FILE * matrixFile,  int ** ar, size_t len ){
    size_t sr;
    size_t si;

    *ar = new int[len];
    if( ar==NULL ){
        printf( "[readIntArray] Out of memory.\n" );
        return 1;
    }

    si = len * sizeof(int);  sr = readAll( matrixFile, (void *) *ar, si );
    if( checkTransfer( "readIntArray", si, sr, dPRINTOK ) ) return 1;

    return 0;
}

int writeFileStart( FILE * matrixFile ){
    int ret;
    ret = writeInt( matrixFile, (int)FIRST_INT     );   if(ret) return ret;
    ret = writeInt( matrixFile, (int)FILE_VERSION  );   if(ret) return ret;
    ret = writeInt( matrixFile, (int)DATATYPE      );   if(ret) return ret;
    return 0;
}

int readFileStart( FILE * matrixFile, int * datatype ){
    int first,
        version;
    int ret;
    ret = readInt( matrixFile, &first    );     if(ret) return ret;
    ret = readInt( matrixFile, &version  );     if(ret) return ret;
    ret = readInt( matrixFile,  datatype );     if(ret) return ret;
    if( first != (int)FIRST_INT ){
        printf( "Data read failed: unknown data format\n" );
        return -1;
    }
    //if( version != (int)FILE_VERSION ){
    if( (version != 0x1) &&  (version != 0x2) ){
        printf( "Data read failed: Inconsistent version of data format"
                " (file: %d, lib: %d)\n", version, FILE_VERSION );
        return -2;
    }
    if( *datatype != (int)DATATYPE ){
        printf( "Data read failed: Inconsistent data size int/long  "
                "file=%d, lib=%d\n", *datatype, DATATYPE );
        return -3;
    }
    return version;
}

/*
 * Writes an integer array of len length to a file fileName.
 * Returns 0 if successful, 1 if not.
 */
int writeIntArrayFile( const char* fileName, int * ar, size_t len ){
    FILE * matrixFile;
#ifdef OS_LINUX
    matrixFile = fopen( fileName, "w" );
    if( matrixFile==NULL ){
        printf("Error in attempt to write \"%s\" file\n",fileName);
        return 1;
    }
#endif
#ifdef OS_WIN
    errno_t err;
    err = fopen_s( &matrixFile, fileName, "wb" );
    if( err!=0 ){
        printf("Error in attempt to write \"%s\" file\n",fileName);
        return 1;
    }
#endif
    fseek(matrixFile,0,SEEK_SET);       // go to the top of file

    int ret = 0;
    ret = writeFileStart( matrixFile );      if(ret<0) goto writeIntArrayFileLabel;

    size_t sr;
    size_t si;
    si = sizeof(long long int); sr = writeAll( matrixFile, (void *)&len, si );
    if( checkTransfer( "writeCMatrix/1", si, sr, dPRINTOK ) ) goto writeIntArrayFileLabel;

    writeIntArray( matrixFile, ar, len );

    fflush( matrixFile );
writeIntArrayFileLabel:
    fclose(matrixFile);
    return ret;
}

/*
 * Reads an integer array of len length from a file fileName.
 * Returns 0 if successful, 1 if not.
 */
int readIntArrayFile( const char* fileName, int ** ar, size_t * len ){
    FILE * matrixFile;
    int ret;
#ifdef OS_LINUX
    matrixFile = fopen( fileName, "rb" );
    if( matrixFile==NULL ){
        printf("Error in attempt to read \"%s\" file\n",fileName);
        return 1;
    }
#endif
#ifdef OS_WIN
    errno_t err;
    err = fopen_s( &matrixFile, fileName, "rb" );
    if( err!=0 ){
        printf("Error in attempt to read \"%s\" file\n",fileName);
        return 1;
    }
#endif
    fseek( matrixFile, 0, SEEK_SET);

    int dataType;
    ret = readFileStart( matrixFile, &dataType ); if(ret<0) goto readIntArrayFileLabel;

    size_t sr;
    size_t si;
    si = sizeof(long long int); sr = readAll( matrixFile, (void *)len, si );
    if( checkTransfer( "readCMatrix/1", si, sr, dPRINTOK ) )  goto readIntArrayFileLabel;

    if( readIntArray( matrixFile, ar, *len ) )
        goto readIntArrayFileLabel;

readIntArrayFileLabel:

    ret = fclose(matrixFile);

    return ret;
}


