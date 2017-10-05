/// PMCell.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef PM_CELL_H
#define PM_CELL_H

#include <lqc/lqcdefines.h>
#include <float.h>

class PMCell
{
public:
  PMCell(const int& x, const int& y, const int& base)
  : m_x(x), m_y(y), m_base(base)
  {
    m_value = -DBL_MAX;
    m_defined = false;
  }

  void SetValue(const double& value)
  {
    m_value = value;
    m_defined = true;
  }

  double GetValue() const
  { 
    return m_value;
  }

  //void SetDefined(bool defined)
  //{
  //  m_defined = defined;
  //}

  bool IsDefined() const
  {
    return m_defined;
  }

  int GetX() const
  {
    return m_x;
  }
  
  int GetY() const
  {
    return m_y;
  }
  
  int GetBase() const
  {
    return m_base;
  }
  
private:
  int m_x;
  int m_y;
  int m_base;
  double m_value;
  bool m_defined;
};

struct PMCellHash
{
  size_t operator()(PMCell* p) const {
    return p->GetX() + p->GetY() * p->GetBase();
  }
};

struct PMCellComparator
{
  size_t operator()(PMCell* p1, PMCell* p2) const {
    return p1->GetX() == p2->GetX() && p1->GetY() == p2->GetY();
  }
};

#endif