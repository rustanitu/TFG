#include "Volume2D.h"

#include <glutils/GLTexture2D.h>

ERNVolume2D::ERNVolume2D ()
{
}

ERNVolume2D::~ERNVolume2D ()
{
}

gl::GLTexture2D* ERNVolume2D::GenerateRGBATexture2D (vr::Volume* volume, vr::TransferFunction* tfunction)
{
  int size_x = volume->GetWidth ();
  int size_y = volume->GetHeight ();
  GLfloat* scalar_values = new GLfloat[size_x * size_y * 4];
  for (int j = 0; j < size_y; j++)
  {
    for (int i = 0; i < size_x; i++)
    {
      double smp = (double)volume->SampleVolume(i, j, 0);
      if (smp == 1)
      {
        scalar_values[i * 4 + (j * size_x * 4) + 0] = 1;
        scalar_values[i * 4 + (j * size_x * 4) + 1] = 0;
        scalar_values[i * 4 + (j * size_x * 4) + 2] = 0;
        scalar_values[i * 4 + (j * size_x * 4) + 3] = 1;
      }
      else
      {
        scalar_values[i * 4 + (j * size_x * 4) + 0] = 1;
        scalar_values[i * 4 + (j * size_x * 4) + 1] = 1;
        scalar_values[i * 4 + (j * size_x * 4) + 2] = 1;
        scalar_values[i * 4 + (j * size_x * 4) + 3] = 1;
      }

      //lqc::Vector4d color = tfunction->Get ((double)volume->SampleVolume (i, j, 0));
      //scalar_values[i * 4 + (j * size_x * 4) + 0] = (GLfloat)color.x;
      //scalar_values[i * 4 + (j * size_x * 4) + 1] = (GLfloat)color.y;
      //scalar_values[i * 4 + (j * size_x * 4) + 2] = (GLfloat)color.z;
      //scalar_values[i * 4 + (j * size_x * 4) + 3] = (GLfloat)color.w;
    }
  }
  gl::GLTexture2D* tex2d = new gl::GLTexture2D (size_x, size_y);

  tex2d->GenerateTexture (GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
  tex2d->SetData (scalar_values, GL_RGBA, GL_RGBA, GL_FLOAT);

  delete[] scalar_values;
  
  return tex2d;
}

/*

*/