/////////////////////////////////////////////////////////////////////////////
// affine.cpp - affine transform class implementation
// Author: Doron Tal
// Date created: February, 1996

#include "wrap_dtlib_cpp/affine.h"

using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////

CAffine::CAffine()
{
    // identity transform
    m_rho =  m_rct = 1.0f;
    m_theta = m_rst = m_C = m_c = 0.0f;
}

/////////////////////////////////////////////////////////////////////////////
// PRECOND: (ox2 != ox1 || oy2 != oy1) && (ox2 != ox1 || oy2 != oy1)

void CAffine::Setup(const int& ix1, const int& iy1,
                    const int& ix2, const int& iy2,
                    const int& ox1, const int& oy1, 
                    const int& ox2, const int& oy2)
{
    const int idx = ix2-ix1;
    const int idy = iy2-iy1;
    const int odx = ox2-ox1;
    const int ody = oy2-oy1;
    const double idist = sqrt((double)(SQR(idx)+SQR(idy)));
    const double odist = sqrt((double)(SQR(odx)+SQR(ody)));
    m_rho = (float)(idist/odist);
    if (idx && odx && 
        ((float)idy/(float)idx) == ((float)ody/(float)odx)) m_theta = 0.0f;
    else m_theta = (float)(atan2((double)idy, (double)idx)-
                           atan2((double)ody, (double)odx));
    m_rct = (float)(m_rho*cos(m_theta));
    m_rst = (float)(m_rho*sin(m_theta));
    m_C = (float)ix1-m_rct*(float)ox1+m_rst*(float)oy1;
    m_c = (float)iy1-m_rst*(float)ox1-m_rct*(float)oy1;
}
