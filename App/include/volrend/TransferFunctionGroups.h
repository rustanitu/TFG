#ifndef VOLREND_TRANSFERFUNCTION_GROUPS_H
#define VOLREND_TRANSFERFUNCTION_GROUPS_H

#include <volrend/TransferFunction.h>

#include <vector>
#include <iostream>

namespace vr
{
  class TransferFunctionGroups : public TransferFunction
  {
  public:
    class TFGroups_ControlPoint
    {
    public:
      TFGroups_ControlPoint (int vle, lqc::Vector4d clr)
        : value (vle), color (clr) {}
      ~TFGroups_ControlPoint (){}
      lqc::Vector4d color;
      int value;
    };

  public:
    TransferFunctionGroups ();
    ~TransferFunctionGroups ();

    virtual const char* GetNameClass ();
    virtual lqc::Vector4d Get (double value);

    virtual gl::GLTexture1D* GenerateTexture_1D_RGBA ();

    bool AddGroup (int value, lqc::Vector4d color);

    void SetLimits (int minvalue, int maxvalue)
    {
      m_minvalue = minvalue;
      m_maxvalue = maxvalue;
    }

  private:
    std::vector<TFGroups_ControlPoint> m_groups;
    int m_minvalue, m_maxvalue;
  };

}

#endif