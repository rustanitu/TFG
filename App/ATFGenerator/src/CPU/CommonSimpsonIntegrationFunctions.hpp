#ifndef COMMON_SIMPSON_INTEGRATION_FUNCTIONS
#define COMMON_SIMPSON_INTEGRATION_FUNCTIONS

#include <math/Vector4.h>

namespace spf
{
  glm::dvec4 ExternalIntegration (glm::dvec4 Cinit, glm::dvec4 Cmid,glm::dvec4 C, double h, double pre_integrated)
  {
    double alphachannel = C.w*exp (-(pre_integrated + ((h / 6.0) * (Cinit.w + 4.0 * Cmid.w + C.w))));
    return glm::dvec4 (alphachannel * C.x, alphachannel * C.y, alphachannel * C.z, alphachannel);
  }
}

#endif