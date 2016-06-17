#ifndef COMMON_SIMPSON_INTEGRATION_FUNCTIONS
#define COMMON_SIMPSON_INTEGRATION_FUNCTIONS

#include <math/Vector4.h>

namespace spf
{
  lqc::Vector4d ExternalIntegration (lqc::Vector4d Cinit, lqc::Vector4d Cmid,lqc::Vector4d C, double h, double pre_integrated)
  {
    double alphachannel = C.w*exp (-(pre_integrated + ((h / 6.0) * (Cinit.w + 4.0 * Cmid.w + C.w))));
    return lqc::Vector4d (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
  }
}

#endif