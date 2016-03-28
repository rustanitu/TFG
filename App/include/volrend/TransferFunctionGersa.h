#ifndef VOLREND_TRANSFERFUNCTIONGERSA_H
#define VOLREND_TRANSFERFUNCTIONGERSA_H

#include <volrend/TransferFunction.h>

#include <vector>
#include <iostream>

namespace vr
{
  class TransferFunctionGersa : public TransferFunction
  {
  public:
    TransferFunctionGersa (double v0 = 0.0, double v1 = 256.0);
    ~TransferFunctionGersa ();

    virtual const char* GetNameClass ();
    virtual lqc::Vector4d Get (double value);

    virtual gl::GLTexture1D* GenerateTexture_1D_RGBA ();

    void AddRGBControlPoint (TransferControlPoint rgb);
    void AddAlphaControlPoint (TransferControlPoint alpha);
    void ClearControlPoints ();

    lqc::Vector3f GetRGBPointOnSpline (float s);
    lqc::Vector3f GetAlphaPointOnSpline (float s);

    //If we don't have a file with the values of the TF, we need to compute the TF
    void Build (TFInterpolationType type);

    void PrintControlPoints ();
    void PrintTransferFunction ();

    TFInterpolationType m_interpolation_type;
    bool m_built;
  private:
    std::vector<TransferControlPoint> m_cpt_rgb;
    std::vector<TransferControlPoint> m_cpt_alpha;
    lqc::Vector4d* m_transferfunction;
    int m_tflenght;
    lqc::Vector3f* m_gradients;

    double m_v0, m_v1;
  };

}

#endif