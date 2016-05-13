#ifndef FUNCTION_DESIGNER_H
#define FUNCTION_DESIGNER_H

#include <iup.h>
#include <iupgl.h>

class FunctionDesigner
{
public:
  FunctionDesigner();
  ~FunctionDesigner();

private:
  Ihandle* m_dialog;
};

#endif