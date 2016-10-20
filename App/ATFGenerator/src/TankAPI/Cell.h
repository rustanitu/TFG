#ifndef ATFG_CELL_H
#define ATFG_CELL_H

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

  int GetIthVertexIndex(const int& i);
  int GetAdjcentCellIndex(const int& face);
  int GetValue(const int& step);

  bool SetIthVertexIndex(const int& ith_vertex, const int& index);
  bool SetAdjcentCellIndex(const int& face, const int& index);
  bool SetValue(const int& step, const int& value);

private:
  int m_i;
  int m_j;
  int m_k;
  bool m_active;
  int m_vertex_index[8];
  int m_adjcell_index[6];
  int m_nsteps;
  int* m_values;
};

#endif