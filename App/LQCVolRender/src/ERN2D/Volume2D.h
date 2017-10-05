/*!
\file Volume2D.h
\brief Volume2D class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef ERN2D_VOLUME_H
#define ERN2D_VOLUME_H

#include <atfg/Volume.h>
#include <atfg/TransferFunction.h>
#include <glutils/GLTexture2D.h>

class ERNVolume2D
{
public:
  ERNVolume2D ();
  ~ERNVolume2D ();

  static gl::GLTexture2D* GenerateRGBATexture2D (vr::Volume* volume, vr::TransferFunction* tfunction);

protected:
private:
};

#endif