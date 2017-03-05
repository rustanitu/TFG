/// PredictionMap.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "PredictionMap.h"
#include <armadillo>

#define GAUSSIAN_SPREAD 16.0f

void PredictionMap<double, DoubleCell>::PredictWithRBF()
{
  const std::forward_list<std::pair<int, int>>::iterator first = m_defined_cells.begin();
  const std::forward_list<std::pair<int, int>>::iterator last = m_defined_cells.end();
  const int n = std::distance(first, last);

  arma::Mat<double> sigma_mat(n, n);
  arma::Col<double> p(n);

  int i = 0;
  for (std::forward_list<std::pair<int, int>>::iterator row = first; row != last; ++row)
  {
    p[i] = m_cells[row->first][row->second].GetValue();
    int j = i;

    for (std::forward_list<std::pair<int, int>>::iterator col = row; col != last; ++col)
    {
      if (i == j)
      {
        sigma_mat(i, j) = 1.0f;
        ++j;
        continue;
      }

      int xi = row->first;
      int yi = row->second;
      int xj = col->first;
      int yj = col->second;

      double h_sqr = (xi - xj) * (xi - xj) + (yi - yj) * (yi - yj);
      double s = GAUSSIAN_SPREAD / 3.0f;
      double sigma_ij = exp(-h_sqr / (2 * s * s));
      sigma_mat(i, j) = sigma_mat(j, i) = sigma_ij;

      ++j;
    }

    ++i;
  }

  arma::Mat<double> inverse_sigma_mat = arma::inv(sigma_mat);
  arma::Col<double> w = inverse_sigma_mat * p;

  while (!m_undefined_cells.empty())
  {
    const std::pair<int, int>& pair = m_undefined_cells.front();
    int i = pair.first;
    int j = pair.second;
    m_undefined_cells.pop_front();

    arma::Row<double> sigma(n);
    int count = 0;
    for (std::forward_list<std::pair<int, int>>::iterator it = first; it != last; ++it)
    {
      int x = it->first;
      int y = it->second;

      double h_sqr = (x - i) * (x - i) + (y - j) * (y - j);
      double s = GAUSSIAN_SPREAD / 3.0f;
      sigma(count++) = exp(-h_sqr / (2 * s * s));
    }

    m_cells[i][j].SetValue(arma::dot(sigma, w));
  }
  m_defined_cells.clear();
}