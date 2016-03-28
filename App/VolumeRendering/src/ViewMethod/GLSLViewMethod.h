/*!
\file ViewMethod.h
\brief Template class for view methods.
\author Leonardo Quatrin Campagnolo
*/

#ifndef TEMPLATE_GLSL_VIEWMETHOD_H
#define TEMPLATE_GLSL_VIEWMETHOD_H

#include "ViewMethod.h"

#include <iup.h>
#include <iupgl.h>
#include <string>

class GLSLViewMethod : public ViewMethod
{
public:
  /*! Constructor.
  \param type VRVIEWS type of the ViewMethod.
  */
  GLSLViewMethod (VRVIEWS t) : ViewMethod(t)
  {
    m_redisplay = true;
    m_auto_redisplay = true;
  }

  /*! Destructor.*/
  ~GLSLViewMethod() {}

  virtual char* GetViewMethodName () { return "GLSLViewMethod"; }

protected:
  bool m_redisplay;
  bool m_auto_redisplay;

private:
};

#endif