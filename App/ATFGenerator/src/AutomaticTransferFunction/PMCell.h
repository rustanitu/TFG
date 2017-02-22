/// PMCell.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef PM_CELL_H
#define PM_CELL_H

#include <lqc/lqcdefines.h>

template <class T>
class PMCell
{
public:
  PMCell()
  {
    m_value = T(-DBL_MAX);
    m_defined = false;
  }

  void SetValue(const T& value)
  {
    m_value = value;
  }

  T GetValue()
  { 
    return m_value;
  }

  void SetDefined(bool defined)
  {
    m_defined = defined;
  }

  bool IsDefined()
  {
    return m_defined;
  }

  int GetRow()
  {
    return m_row;
  }
  
  int GetColumn()
  {
    return m_col;
  }
  
private:
  T m_value;
  bool m_defined;
};

#define DoubleCell PMCell<double>
#define RGBCell PMCell<glm::dvec3>

#endif