#ifndef ATFG_CELL_H
#define ATFG_CELL_H

#include <float.h>
#include <math/MUtils.h>

class Cell
{

  //midle edge definition
  //
  //         6____ub____7              5   1
  //        /|         /               ^  /!
  //      ul |       ur|               |  /
  //      /  lb      / rb              | /
  //    2/___|uf____/3 |       2 ------|-------> 3
  //    |   4|___db|___|5             /|
  //    |   /      |   /             / |
  //    lf dl      rf dr            /  |
  //    | /        | /             0   4
  //    |/____df___|/
  //    0          1
  //

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
  double GetValue() const;
  double GetGradient() const;
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
  void SetValue(const double& value);
  void SetGradient(const double& value);
  void SetCenter(glm::vec3 center)
  {
    m_center = center;
  }

  void SetFaceCenter(const int& face, glm::vec3 center)
  {
    m_adjface[face] = center;
  }

  int* GetFaceVertices(const int& face) const;

private:
  int m_i;
  int m_j;
  int m_k;
  bool m_active;
  int m_vertex_index[8];
  int m_adjcell_index[6];
  double m_value;
  double m_gradient;
  glm::vec3 m_center;
  glm::vec3 m_adjface[6];
};

#endif