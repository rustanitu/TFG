/// PredictionMap.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef PREDICTION_MAP_H
#define PREDICTION_MAP_H

#include "PMCell.h"
#include <forward_list>

template <class T, typename Cell = PMCell<T>>
class PredictionMap
{
public:
  PredictionMap(int coldim, int rowdim)
  : m_cells(NULL)
  , m_width(coldim)
  , m_height(rowdim)
  {
  }

  ~PredictionMap()
  {
    CleanUp();
  }

  bool Init()
  {
    if (m_width < 1 || m_height < 1)
      return false;

    m_cells = new Cell*[m_width];
    if (!m_cells)
      return false;

    int last = -1;
    for (int i = 0; i < m_width; ++i)
    {
      m_cells[i] = new Cell[m_height];
      if (!m_cells[i])
      {
        last = i - 1;
        break;
      }

      for (int j = 0; j < m_height; ++j)
      {
        m_undefined_cells.push_front(std::make_pair(i, j));
      }
    }

    if (last > -1)
    {
      for (int i = 0; i < last; ++i)
        delete[] m_cells[i];

      delete[] m_cells;
    }

    return true;
  }

  void CleanUp()
  {
    if (!m_cells)
      return;

    for (int i = 0; i < m_width; ++i)
    {
      for (int j = 0; j < m_height; ++j)
      {
        delete[] m_cells[i];
        m_cells[i] = NULL;
      }
    }

    delete[] m_cells;
    m_cells = NULL;
    m_defined_cells.clear();
    m_undefined_cells.clear();
  }

  void PredictWithInverseDistanceWeighting(const double& p, const double& d = 0)
  {
    while (!m_undefined_cells.empty())
    {
      const std::pair<int, int>& pair = m_undefined_cells.front();
      int i = pair.first;
      int j = pair.second;
      m_undefined_cells.pop_front();

      T val = 0;
      double wij = 0;
      for (auto it = m_defined_cells.begin(); it != m_defined_cells.end(); ++it)
      {
        int l = it->first;
        int m = it->second;

        double distance = sqrt(((i - l)*(i - l)) + ((j - m)*(j - m)));
        double w = (double)1.0f / pow(distance, p);
        val += (w * m_cells[l][m].GetValue());
        wij += w;
      }

      if (wij > 0)
        m_cells[i][j].SetValue(val / wij);
    }
    m_defined_cells.clear();
  }

  void SetValue(const T& value, const int& i, const int& j)
  {
    if (i > m_width || i > m_height || i < 0 || j < 0)
      return;

    m_cells[i][j].SetValue(value);
    m_cells[i][j].SetDefined(true);
    m_defined_cells.push_front(std::make_pair(i, j));
    m_undefined_cells.remove(std::make_pair(i, j));
  }

  T GetValue(const int& i, const int& j)
  {
    return m_cells[i][j].GetValue();
  }

  bool IsDefined(const int& i, const int& j)
  {
    return m_cells[i][j].IsDefined();
  }

private:
  int m_width;
  int m_height;
  Cell** m_cells;
  std::forward_list<std::pair<int, int>> m_defined_cells;
  std::forward_list<std::pair<int, int>> m_undefined_cells;
};

#endif