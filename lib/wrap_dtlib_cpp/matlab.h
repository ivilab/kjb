/////////////////////////////////////////////////////////////////////////////
// matlab.h - utilities to interface matlab and c/c++ via files
// Author: Doron Tal, based on code originally written by Bruce Fischl.
// Date created: June, 1995
//
// C++ header file with inlined code that reads and writes from C/C++
// Buffers (float or unigned char) into .mat files, which in turn can
// be read by Matlab.  This is the only file you need.

#ifndef _MATLAB_H
#define _MATLAB_H

/*
 * Kobus: We have run into trouble with 32 bit centric code in this
 * distribution. I have changed some long's to kjb_int32's.  The immediate
 * problem is that the segmentation maps can get written out as 64 bit integers. 
*/
#include "l/l_sys_def.h"

#include "wrap_dtlib_cpp/img.h"
#include <stdio.h>
#include <string.h>

// CONSTANTS

#define MAX_CHARS 2000 /* max characters allowed per filename/mat-variable */

// MATLAB CONSTANTS

// thousands digit
#define MACH_PC       0
#define MACH_MOTOROLA 1
#define MACH_VAXD     2
#define MACH_VADG     3
#define MACH_CRAY     4

// hundreds digit is always 0

// tens digit
#define TYPE_FLOAT    1
#define TYPE_LONG     2
#define TYPE_BYTE     5
#define TYPE_DOUBLE   0
#define TYPE_SHORT    3
#define TYPE_USHORT   4

// ones digit
#define MATRIX_NUM    0
#define MATRIX_TEXT   1
#define MATRIX_SPARSE 2


#warning "[Code police] Do not put 'using namespace' in global scope of header."
using namespace kjb_c;


namespace DTLib {

    ///////////////////////////////////////////////////////////////////////////
    // INTERFACE: prototypes
    ///////////////////////////////////////////////////////////////////////////

    // read/write buffer

    // ***TODO*** NEED COMMENTS HERE!
    template <class T>
    T* ReadBufferFromMatfile(const char* strFilename,
                             T* pBuffer, int& Width, int& Height);

    template <class T>
    bool WriteBufferToMatfile(T* pBuffer, const int Width,
                              const int Height, const char* strMatName,
                              const char* strFilename);

    // read/write image (CImg object)
    template <class T>
    bool ReadMatfile(CImg<T>* pInImg, const char* strFilename);

    void ReadMatheader(const char* strFilename);

    template <class T>
    bool WriteMatfile(CImg<T>& InImg,
                      const char* strMatname,
                      const char* strFilename);

    template <class T>
    bool WriteMatfile(CImg<T>& InImg, const char* strMatname);

    // read/write image-sequence (CImgVec object)
    template <class T>
    bool ReadMatfiles(CImgVec<T>& InVec, const char *strPrefix);
    template <class T>
    bool WriteMatfiles(CImgVec<T>& InVec, const char *strPrefix);
    template <class T>
    bool WriteMatfiles(const vector<CImg<T>*>& InVec, const char *strPrefix);

    ///////////////////////////////////////////////////////////////////////////
    // END OF INTERFACE
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    // matlab-specific definitions:
    ///////////////////////////////////////////////////////////////////////////

    typedef struct
    {
        int  type ;
        int  mrows ;
        int  ncols ;
        int  imagf ;
        int  namlen ;
    } MATFILE ;

    // overloaded 'type' field number parser acc. to input type

    inline int MatlabTypeID(float* pBuffer) { return TYPE_FLOAT; }
    inline int MatlabTypeID(unsigned char* pBuffer) { return TYPE_BYTE; }

    /*
    inline int MatlabTypeID(int* pBuffer) { return TYPE_LONG; }
    inline int MatlabTypeID(long* pBuffer) { return TYPE_LONG; }
    */
    inline int MatlabTypeID(int* pBuffer) { return TYPE_LONG; }

    ///////////////////////////////////////////////////////////////////////////
    // IMPLEMENTATION:
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    // allocates new Buffer only if pBuffer is null, and in that case, it
    // writes into Width & Height the correct Values.
    // if pBuffer is not null,
    template <class T>
    T* ReadBufferFromMatfile(const char* strFilename,
                             T* pBuffer, int& Width, int& Height)
    {
        FILE *pF;
        if ((pF = fopen(strFilename, "rb")) == NULL) {
            return NULL;
        }

        MATFILE matfile;
        if (fread(&matfile, 1, sizeof(MATFILE), pF) !=
            (unsigned int)sizeof(MATFILE)) {
            fclose(pF);
            return NULL;
        }

        char name[MAX_CHARS];
        if (fread(name, sizeof(char), matfile.namlen, pF) !=
            (unsigned int)matfile.namlen) {
            fclose(pF);
            return NULL;
        }

        const int W = matfile.ncols;
        const int H = matfile.mrows;

        if (pBuffer == NULL) {
            Width = W; // modify for output
            Height = H; // modify for output
            pBuffer = new T[Width*Height];
        }
        // read matrix
        const int EltSize = sizeof(T);
        for (int x = 0; x < W; x++)
            for (int y = 0; y < H; y++) {
                T Val;
                if (fread(&Val, 1, EltSize, pF) != (unsigned int)EltSize)
                    return NULL;
                *(pBuffer+y*W+x) = Val;
            } // for y
        fclose(pF);
        return pBuffer;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    bool WriteBufferToMatfile(T* pBuffer, const int Width,
                              const int Height, const char* strMatName,
                              const char* strFilename)
    {
        MATFILE matfile;
        char strType[32];
        sprintf(strType, "%d0%d%d", MACH_PC, MatlabTypeID(pBuffer),MATRIX_NUM);
        matfile.type = atoi(strType);
        matfile.ncols = Width;
        matfile.mrows = Height;
        matfile.imagf = 0 ;     // no complex for now
        matfile.namlen = strlen(strMatName)+1 ;

        FILE *pF;
        if ((pF = fopen(strFilename, "wb")) == NULL)
            return false;

        if(fwrite(&matfile, 1, sizeof(MATFILE), pF) !=
           (unsigned int)sizeof(MATFILE)) {
            fclose(pF);
            return false;
        }
        if (fwrite(strMatName, 1, (int)matfile.namlen, pF) !=
            (unsigned int)matfile.namlen) {
            fclose(pF);
            return false;
        }
        // write matllab matrix in column order
        const int EltSize = sizeof(T);
        for (int x = 0; x < Width; x++)
            for (int y = 0; y < Height; y++)
                if (fwrite(pBuffer+y*Width+x, 1, EltSize, pF) !=
                    (unsigned int)EltSize)
                    return false;
        fclose(pF);
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    // assumes you know the type of the file you're reading and that the
    // current Img is preallocated

    // float version
    template <class T>
    bool ReadMatfile(CImg<T>& InImg, const char* strFilename)
    {
        if (strlen(strFilename) == 0) return false;

        char* strFopenName = NULL;
        if (!strstr(strFilename, ".mat")) {
            // doesn't contain extension, add it
            char strFullFilename[MAX_CHARS];
            sprintf(strFullFilename, "%s.mat", strFilename);
            strFopenName = (char*)strFullFilename;
        }
        else strFopenName = (char*)strFilename;

        CImg<T>* pImg = &InImg;
        if (pImg == NULL) return false;

        int Width = 0, Height = 0;
        T *pBuffer =
            ReadBufferFromMatfile(strFopenName, InImg.pBuffer(), Width,Height);
        if (pBuffer == NULL) return false;

        if (InImg.IsEmpty()) {
            InImg.Set_pBuffer(pBuffer, Width, Height);
        }

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    bool WriteMatfile(CImg<T>& InImg,
                      const char* strMatname,
                      const char* strFilename)
    {
        return WriteBufferToMatfile(InImg.pBuffer(),
                                    InImg.Width(), InImg.Height(),
                                    strMatname, strFilename);
    }

    // same as WriteMatfile above, but no need to supply two strings

    template <class T>
    bool WriteMatfile(CImg<T>& InImg, const char* strMatname)
    {
        char strFilename[MAX_CHARS];
        sprintf(strFilename, "%s.mat", strMatname);
        return WriteMatfile(InImg, strMatname, strFilename);
    }

    ///////////////////////////////////////////////////////////////////////////

    // ignores ROI

    template <class T>
    bool ReadMatfiles(CImgVec<T>& InVec, const char *strPrefix)
    {
        char strFname[MAX_CHARS];

        // count how many files
        FILE *pF;
        int nFrames;

        for (nFrames = 0; ; nFrames++) {
            if (nFrames < 10) sprintf(strFname, "%s_00%d.mat", strPrefix,
                                      nFrames);
            else if (nFrames < 100) sprintf(strFname, "%s_0%d.mat",
                                            strPrefix, nFrames);
            else sprintf(strFname, "%s_%d.mat", strPrefix, nFrames);
            if ((pF = fopen(strFname, "r")) == NULL) break;
            else fclose(pF);
        } // for (int nFrames = 0; ; i++) {
        if (nFrames == 0) return false;

        // read in first file to get Width & Height
        CImg<T> tmpImg;
        sprintf(strFname, "%s_000.mat", strPrefix);
        if (!ReadMatfile(tmpImg, strFname)) return false;

        if (InVec.IsEmpty()) {
            InVec.Allocate(nFrames, tmpImg.Width(), tmpImg.Height());
        }

        for (int i = 0; i < InVec.nFrames(); i++) {
            if (i < 10) sprintf(strFname, "%s_00%d.mat", strPrefix, i);
            else if (i < 100) sprintf(strFname, "%s_0%d.mat", strPrefix, i);
            else sprintf(strFname, "%s_%d.mat", strPrefix, i);
            if (!ReadMatfile(*(InVec.GetImg(i)), strFname)) return false;
        }
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    bool WriteMatfiles(CImgVec<T>& InVec, const char *strPrefix)
    {
        char strFilename[MAX_CHARS];
        char strMatName[MAX_CHARS];
        for (int i = 0; i < InVec.nFrames(); i++) {
            if (i < 10) sprintf(strMatName, "%s_00%d", strPrefix, i);
            else if (i < 100) sprintf(strMatName, "%s_0%d", strPrefix, i);
            else sprintf(strMatName, "%s_%d", strPrefix, i);
            sprintf(strFilename, "%s.mat", strMatName);
            if (!WriteMatfile(*(InVec.GetImg(i)), strMatName, strFilename))
                return false;
        }
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    bool WriteMatfiles(const vector<CImg<T>*>& InVec, const char *strPrefix)
    {
        char strFilename[MAX_CHARS];
        char strMatName[MAX_CHARS];
        const int nFrames = InVec.size();
        for (int i = 0; i < nFrames; i++) {
            if (i < 10) sprintf(strMatName, "%s_00%d", strPrefix, i);
            else if (i < 100) sprintf(strMatName, "%s_0%d", strPrefix, i);
            else sprintf(strMatName, "%s_%d", strPrefix, i);
            sprintf(strFilename, "%s.mat", strMatName);
            if (!WriteMatfile(*(InVec[i]), strMatName, strFilename))
                return false;
        }
        return true;
    }

} // namespace DTLib;

#endif /* #ifndef _MATLAB_H */
