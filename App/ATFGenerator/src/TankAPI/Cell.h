#ifndef ATFG_CELL_H
#define ATFG_CELL_H

#include <float.h>
#include <math/MUtils.h>

class Cell
{
public:
  Cell();

  void Init(const int& i, const int& j, const int& k, const bool& active, const int& nsteps);

  int GetI() const
  {
    return m_i;
  }

  int GetJ() const
  {
    return m_j;
  }

  int GetK() const
  {
    return m_k;
  }

  bool IsActive() const
  {
    return m_active;
  }

  int GetIthVertexIndex(const int& i) const;
  int GetAdjcentCellIndex(const int& face) const;
  double GetValue(const int& step) const;
  glm::vec3 GetCenter() const
  {
    return m_center;
  }

  glm::vec3 GetFaceCenter(const int& face) const
  {
    return m_adjface[face];
  }

  bool SetIthVertexIndex(const int& ith_vertex, const int& index);
  bool SetAdjcentCellIndex(const int& face, const int& index);
  bool SetValue(const int& step, const double& value);
  void SetCenter(glm::vec3 center)
  {
    m_center = center;
  }

  void SetFaceCenter(const int& face, glm::vec3 center)
  {
    m_adjface[face] = center;
  }

private:
  int m_i;
  int m_j;
  int m_k;
  bool m_active;
  int m_vertex_index[8];
  int m_adjcell_index[6];
  double m_value;
  glm::vec3 m_center;
  glm::vec3 m_adjface[6];
};

#endif