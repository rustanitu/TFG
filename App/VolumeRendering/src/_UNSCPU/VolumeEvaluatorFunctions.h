/*!
\file VolumeEvaluatorFunctions.h
\brief VolumeEvaluatorFunctions class with some functions for volume rendering.
\author Leonardo Quatrin Campagnolo
*/

#ifndef VOLUME_EVALUATOR_FUNCTIONS_H
#define VOLUME_EVALUATOR_FUNCTIONS_H

#include "../Viewer.h"

#include <lqc/Math/Vector3.h>
#include <cstring>
#include <iostream>
#include <vector>

class VolumeEvaluatorFunctions
{
public:
  typedef double (*VEFunction) (lqc::Vector3d pos, void* data);

private:
  static double Example_01_Function (lqc::Vector3d p, void* data)
  {
    lqc::Vector3f amin = Viewer::Instance ()->m_volume->GetAnchorMin ();
    lqc::Vector3f amax = Viewer::Instance ()->m_volume->GetAnchorMax ();

    return ((p.x - amin.x) / (amax.x - amin.x))*255.0;
  }

  static double Example_02_Function (lqc::Vector3d p, void* data)
  {
    lqc::Vector3f amin = Viewer::Instance ()->m_volume->GetAnchorMin ();
    lqc::Vector3f amax = Viewer::Instance ()->m_volume->GetAnchorMax ();

    float dist_x = amax.x - amin.x;
    float p_x = p.x - amin.x;

    if (p.x < amin.x / 2.0)
      return 0.0;
    else if (p.x > amax.x / 2.0)
      return 100.0;
    else
      return 50.0;
    return -1.0;
  }

  static double Example_03_Function (lqc::Vector3d p, void* data)
  {
    lqc::Vector3f amin = Viewer::Instance ()->m_volume->GetAnchorMin ();
    lqc::Vector3f amax = Viewer::Instance ()->m_volume->GetAnchorMax ();
    
    double x = p.x / (double)amin.x;
    double y = p.y / (double)amin.y;
    double z = p.z / (double)amin.z;

    double retdouble = ((x*x) + (y*y) + (z*z) - ((x*x*x*x) + (y*y*y*y) + (z*z*z*z))) * 255.0;
    return retdouble;
  }

  static double Example_04_Function (lqc::Vector3d p, void* data)
  {
    lqc::Vector3f amin = Viewer::Instance ()->m_volume->GetAnchorMin ();
    lqc::Vector3f amax = Viewer::Instance ()->m_volume->GetAnchorMax ();

    double x = p.x / (double)amin.x;
    double y = p.y / (double)amin.y;
    double z = p.z / (double)amin.z;

    double scale = 1.05;
    x *= scale;
    y *= scale;
    z *= scale;

    double retdouble = ((x*x) + (y*y) + (z*z) - ((x*x*x*x) + (y*y*y*y) + (z*z*z*z))) * 255.0;
    return retdouble;
  }

  static double Example_05_Function (lqc::Vector3d p, void* data)
  {
    if (p.x  < 0.0)
      return 0.0;
    else if (p.x == 0.0)
      return 50.0;
    else if (p.x  > 0.0)
      return 100.0;
    return  -1.0;
  }

  static double Example_06_Function (lqc::Vector3d p, void* data)
  {
    lqc::Vector3f amin = Viewer::Instance ()->m_volume->GetAnchorMin ();
    lqc::Vector3f amax = Viewer::Instance ()->m_volume->GetAnchorMax ();

    double x = p.x / (double)amax.x;
    double y = p.y / (double)amax.y;
    double z = p.z / (double)amax.z;

    return (x*x) + (y*y*y) + cos (z) * (255.0 / 3.0);
  }

public:
  VolumeEvaluatorFunctions ()
  {
    m_index_func = 0;
    AddFunction (Example_01_Function, std::string ("Example 01"));
    AddFunction (Example_02_Function, std::string ("Example 02"));
    AddFunction (Example_03_Function, std::string ("Example 03"));
    AddFunction (Example_04_Function, std::string ("Example 04"));
    AddFunction (Example_05_Function, std::string ("Example 05"));
    AddFunction (Example_06_Function, std::string ("Example 06"));
  }

  ~VolumeEvaluatorFunctions ()
  {
    m_functions.clear ();
  }

  float F (lqc::Vector3d p, void* data)
  {
    return m_functions[m_index_func].m_function (p, data);
  }

  void SetIndexFunc (int index)
  {
    m_index_func = index;
  }

  int GetNumberOfFunctions ()
  {
    return (int)m_functions.size();
  }

  VEFunction GetFunction (int i)
  {
    return m_functions[i].m_function;
  }

  std::string GetFunctionName (int i)
  {
    return m_functions[i].m_name;
  }

  std::vector<std::string> GetFunctionNames ()
  {
    std::vector<std::string> vret;
    for (int i = 0; i < (int)m_functions.size (); i++)
      vret.push_back (m_functions[i].m_name);

    return vret;
  }

  void AddFunction (VEFunction func, std::string name)
  {
    m_functions.push_back (VEEvalutionFunction(func, name));
  }

protected:
  class VEEvalutionFunction
  {
  public:
    VEEvalutionFunction (VEFunction func, std::string name)
    {
      m_function = func;
      m_name = name;
    }

    ~VEEvalutionFunction ()
    {}

    VEFunction m_function;
    std::string m_name;
  protected:
  private:
  };

private:
  int m_index_func;
  std::vector<VEEvalutionFunction> m_functions;

};

#endif