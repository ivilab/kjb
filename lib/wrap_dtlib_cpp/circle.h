/////////////////////////////////////////////////////////////////////////////
// circle.h - routine for fastest possible circle circumference traversal
// Author: Doron Tal
// Date created: January, 1993

#ifndef _CIRCLE_H
#define _CIRCLE_H

namespace DTLib {

    ///////////////////////////////////////////////////////////////////////////
    // Instantiates into an ImgVec of byte images containing white (255)
    // over black (0) circles.  Images are of variable size, always square,
    // alwas of odd width = (int)radius*2+1, so drawn circles are perfectly
    // centered.
    class CCircleMasks : public CImgVec<BYTE>
    {
    public:
        CCircleMasks(const int& MinRad, const int& MaxRad);
        ~CCircleMasks() { FreeMemory(); }

        inline int GetMinRad() { return m_MinRad; }
        inline int GetMaxRad() { return m_MinRad+m_nFrames-1; }

        BYTE* GetCircleMaskBuffer(const int& Rad);

    private:
        int m_MinRad;
    };

    ///////////////////////////////////////////////////////////////////////////
    // returns the number of pixels on the circumference of radius 'radius'
    int CircumLength(const int& radius);

    ///////////////////////////////////////////////////////////////////////////

    // routine for drawing/traversing circles - pre-computes traversal
    // of circle as chain code and returns that chain code as a
    // sequence of positive or negative pointer increments.  'radius'
    // is the radius of the wanted circle, 'Width' is the number of
    // columns in the image in which we would traverse the
    // circumference.  RETURNS a list of integers, first integer tells
    // how many pixels in the rest of the list; rest of list is a
    // sequence of offsets by which we need to increment the pointer
    // to the Img in order to traverse the entire circumference.  The
    // pointer to the Img which we need to increment according to the
    // returned list is at startring position = leftmost point on the
    // circle, i.e. if the circle's center is at (x,y) and its radius
    // is 'r', then the pointer to be incremented is (x-r, y).  The
    // circle is traversed CLOCKWISE.
    int* ComputeCircum(const int& radius, const int& Width);

} // namespace DTLib {

#endif /* #ifndef _CIRCUM_H */
