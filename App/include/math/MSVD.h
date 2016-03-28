#ifndef LQC_MATH_SVD_H
#define LQC_MATH_SVD_H

#include <limits>  // std::numeric_limits
#include <lqc/lqcdefines.h>
#include <math/Vector.h>
#include <math/Matrix.h>

namespace lqc
{
  //Taken from Numerical Recipes in C: The Art of Scientific Computing. Second Edition
  //William H. Press, Saul A. Teukolsky, William T. Vetterling, Brian P. Flannery
  void svdcmp (float **a, int m, int n, float w[], float **v);
  void svdbksb (float **u, float w[], float **v, int m, int n, float b[], float x[]);
  float pythag (float a, float b);
  void dsvdcmp (double **a, int m, int n, double w[], double **v);
  void dsvdbksb (double **u, double w[], double **v, int m, int n, double b[], double x[]);
  double dpythag (double a, double b);

  void dcksvdexample ();
  double dckPYTHAG (double a, double b);
  int dcksvd (float **a, int m, int n, float *w, float **v);

  void gdcsvdexample ();
  void gdcsvd (double **A, double *S2, int n);

  //Object for singular value decomposition of a matrix A, and related functions.
  //Numerical Recipes in C The Art of Scientific Computing (Third Edition)
  // William H. Press, Saul A. Teukolsky, William T. Vetterling, Brian P. Flannery
  struct SVD
  {
    int m, n;
    //The matrices U and V.
    Matrix<double> u, v;
    //The diagonal matrix W.
    Vector<double> w;
    double eps, tsh;

    //Constructor.The single argument is A.The SVD computation is done by decompose, and
    // the results are sorted by reorder.
    //Input: a
    SVD (Matrix<double> &a)
      : m (a.GetRows ()), n (a.GetCols ()), u (a), v (n, n), w (n)
    {
      eps = std::numeric_limits<double>::epsilon ();
      decompose ();
      reorder ();
      //Default threshold for nonzero singular values.
      tsh = 0.5*sqrt (m + n + 1.)*w[0] * eps;
    }

    //Solve A  x D b for a vector x using the pseudoinverse of A as obtained by SVD.If positive,
    // thresh is the threshold value below which singular values are considered as zero.If thresh is
    // negative, a default based on expected roundoff error is used.
    //Input: b, thresh
    //Output: x
    void solve (Vector<double> &b, Vector<double> &x, double thresh = -1.0)
    {
      int i, j, jj;
      double s;
      if (b.GetSize () != m || x.GetSize () != n) throw("SVD::solve bad sizes");
      Vector<double> tmp (n);
      tsh = (thresh >= 0.0 ? thresh : 0.5*sqrt (m + n + 1.0)*w[0] * eps);
      for (j = 0; j<n; j++)
      {
        s = 0.0;
        if (w[j] > tsh) {
          for (i = 0; i < m; i++) s += u[i][j] * b[i];
          s /= w[j];
        }
        tmp[j] = s;
      }
      for (j = 0; j < n; j++)
      {
        s = 0.0;
        for (jj = 0; jj < n; jj++) s += v[j][jj] * tmp[jj];
        x[j] = s;
      }
    }

    //Solves m sets of n equations AX D B using the pseudoinverse of A.The right - hand sides are
    // input as b[0..n - 1][0..m - 1], while x[0..n - 1][0..m - 1] returns the solutions.thresh as above.
    //Input: b, thresh
    //Output: x
    void solve (Matrix<double> &b, Matrix<double> &x, double thresh = -1.0)
    {
      int i, j, m = b.GetCols ();
      if (b.GetRows () != n || x.GetRows () != n || b.GetCols () != x.GetCols ())
        throw("SVD::solve bad sizes");
      Vector<double> xx (n);
      for (j = 0; j < m; j++)
      {
        for (i = 0; i < n; i++) xx[i] = b[i][j];
        solve (xx, xx, thresh);
        for (i = 0; i < n; i++) x[i][j] = xx[i];
      }
    }
    //Solve with (apply the pseudoinverse to) one or more right - hand sides.

    //Quantities associated with the range and nullspace of A.
    //
    //Return the rank of A, after zeroing any singular values smaller than thresh.If thresh is
    // negative, a default value based on estimated roundoff is used.
    int rank (double thresh = -1.0)
    {
      int j, nr = 0;
      tsh = (thresh >= 0. ? thresh : 0.5*sqrt (m + n + 1.)*w[0] * eps);
      for (j = 0; j<n; j++) if (w[j] > tsh) nr++;
      return nr;
    }
    //Return the nullity of A, after zeroing any singular values smaller than thresh.Default value as above.
    int nullity (double thresh = -1.0)
    {
      int j, nn = 0;
      tsh = (thresh >= 0. ? thresh : 0.5*sqrt (m + n + 1.)*w[0] * eps);
      for (j = 0; j < n; j++) if (w[j] <= tsh) nn++;
      return nn;
    }
    //Give an orthonormal basis for the range of A as the columns of a returned matrix.thresh as above.
    Matrix<double> range (double thresh = -1.0)
    {
      int i, j, nr = 0;
      Matrix<double> rnge (m, rank (thresh));
      for (j = 0; j<n; j++) {
        if (w[j] > tsh) {
          for (i = 0; i < m; i++) rnge[i][nr] = u[i][j];
          nr++;
        }
      }
      return rnge;
    }
    //Give an orthonormal basis for the nullspace of A as the columns of a returned matrix.thresh as above.
    Matrix<double> nullspace (double thresh = -1.0)
    {
      int j, jj, nn = 0;
      Matrix<double> nullsp (n, nullity (thresh));
      for (j = 0; j < n; j++) {
        if (w[j] <= tsh) {
          for (jj = 0; jj < n; jj++) nullsp[jj][nn] = v[jj][j];
          nn++;
        }
      }
      return nullsp;
    }
    //

    //Return reciprocal of the condition number of A.
    double inv_condition ()
    {
      return (w[0] <= 0. || w[n - 1] <= 0.) ? 0. : w[n - 1] / w[0];
    }

    //Functions used by the constructor.
    void decompose ()
    {
      bool flag;
      int i, its, j, jj, k, l, nm;
      double anorm, c, f, g, h, s, scale, x, y, z;
      Vector<double> rv1 (n);
      g = scale = anorm = 0.0;
      for (i = 0; i < n; i++) {
        l = i + 2;
        rv1[i] = scale*g;
        g = s = scale = 0.0;
        if (i < m) {
          for (k = i; k < m; k++) scale += abs (u[k][i]);
          if (scale != 0.0) {
            for (k = i; k < m; k++) {
              u[k][i] /= scale;
              s += u[k][i] * u[k][i];
            }
            f = u[i][i];
            g = -SIGN (sqrt (s), f);
            h = f*g - s;
            u[i][i] = f - g;
            for (j = l - 1; j < n; j++) {
              for (s = 0.0, k = i; k < m; k++) s += u[k][i] * u[k][j];
              f = s / h;
              for (k = i; k < m; k++) u[k][j] += f*u[k][i];
            }
            for (k = i; k < m; k++) u[k][i] *= scale;
          }
        }
        w[i] = scale *g;
        g = s = scale = 0.0;
        if (i + 1 <= m && i + 1 != n) {
          for (k = l - 1; k < n; k++) scale += abs (u[i][k]);
          if (scale != 0.0) {
            for (k = l - 1; k < n; k++) {
              u[i][k] /= scale;
              s += u[i][k] * u[i][k];

            }
            f = u[i][l - 1];
            g = -SIGN (sqrt (s), f);
            h = f*g - s;
            u[i][l - 1] = f - g;
            for (k = l - 1; k < n; k++) rv1[k] = u[i][k] / h;
            for (j = l - 1; j < m; j++) {
              for (s = 0.0, k = l - 1; k < n; k++) s += u[j][k] * u[i][k];
              for (k = l - 1; k < n; k++) u[j][k] += s*rv1[k];
            }
            for (k = l - 1; k < n; k++) u[i][k] *= scale;
          }
        }
        anorm = MAX (anorm, (abs (w[i]) + abs (rv1[i])));
      }
      for (i = n - 1; i >= 0; i--) {
        if (i < n - 1) {
          if (g != 0.0) {
            for (j = l; j < n; j++)
              v[j][i] = (u[i][j] / u[i][l]) / g;
            for (j = l; j < n; j++) {
              for (s = 0.0, k = l; k < n; k++) s += u[i][k] * v[k][j];
              for (k = l; k < n; k++) v[k][j] += s*v[k][i];
            }
          }
          for (j = l; j < n; j++) v[i][j] = v[j][i] = 0.0;
        }
        v[i][i] = 1.0;
        g = rv1[i];
        l = i;
      }
      for (i = MIN (m, n) - 1; i >= 0; i--) {
        l = i + 1;
        g = w[i];
        for (j = l; j < n; j++) u[i][j] = 0.0;
        if (g != 0.0) {
          g = 1.0 / g;
          for (j = l; j < n; j++) {
            for (s = 0.0, k = l; k < m; k++) s += u[k][i] * u[k][j];
            f = (s / u[i][i])*g;
            for (k = i; k < m; k++) u[k][j] += f*u[k][i];
          }
          for (j = i; j < m; j++) u[j][i] *= g;
        }
        else for (j = i; j < m; j++) u[j][i] = 0.0;
        ++u[i][i];
      }
      for (k = n - 1; k >= 0; k--) {

        for (its = 0; its < 30; its++) {
          flag = true;
          for (l = k; l >= 0; l--) {

            nm = l - 1;
            if (l == 0 || abs (rv1[l]) <= eps*anorm) {
              flag = false;
              break;
            }
            if (abs (w[nm]) <= eps*anorm) break;
          }
          if (flag) {
            c = 0.0;
            s = 1.0;
            for (i = l; i < k + 1; i++) {
              f = s*rv1[i];
              rv1[i] = c*rv1[i];
              if (abs (f) <= eps*anorm) break;
              g = w[i];
              h = pythag (f, g);
              w[i] = h;
              h = 1.0 / h;
              c = g*h;
              s = -f*h;
              for (j = 0; j < m; j++) {
                y = u[j][nm];
                z = u[j][i];
                u[j][nm] = y*c + z*s;
                u[j][i] = z*c - y*s;
              }
            }
          }
          z = w[k];
          if (l == k) {
            if (z < 0.0) {
              w[k] = -z;
              for (j = 0; j < n; j++) v[j][k] = -v[j][k];
            }
            break;
          }
          if (its == 29) throw("no convergence in 30 svdcmp iterations");
          x = w[l];
          nm = k - 1;
          y = w[nm];
          g = rv1[nm];
          h = rv1[k];
          f = ((y - z)*(y + z) + (g - h)*(g + h)) / (2.0*h*y);
          g = pythag (f, 1.0);
          f = ((x - z)*(x + z) + h*((y / (f + SIGN (g, f))) - h)) / x;
          c = s = 1.0;

          for (j = l; j <= nm; j++) {
            i = j + 1;
            g = rv1[i];
            y = w[i];
            h = s*g;
            g = c*g;
            z = pythag (f, h);
            rv1[j] = z;
            c = f / z;
            s = h / z;
            f = x*c + g*s;
            g = g*c - x*s;
            h = y*s;
            y *= c;
            for (jj = 0; jj < n; jj++) {
              x = v[jj][j];
              z = v[jj][i];
              v[jj][j] = x*c + z*s;
              v[jj][i] = z*c - x*s;
            }
            z = pythag (f, h);
            w[j] = z;

            if (z) {
              z = 1.0 / z;
              c = f*z;
              s = h*z;
            }
            f = c*g + s*y;
            x = c*y - s*g;
            for (jj = 0; jj < m; jj++) {
              y = u[jj][j];
              z = u[jj][i];

              u[jj][j] = y*c + z*s;
              u[jj][i] = z*c - y*s;
            }
          }
          rv1[l] = 0.0;
          rv1[k] = f;
          w[k] = x;
        }
      }
    }

    void reorder ()
    {
      int i, j, k, s, inc = 1;
      double sw;
      Vector<double> su (m), sv (n);
      do { inc *= 3; inc++; } while (inc <= n);
      do {
        inc /= 3;
        for (i = inc; i < n; i++) {
          sw = w[i];
          for (k = 0; k < m; k++) su[k] = u[k][i];
          for (k = 0; k < n; k++) sv[k] = v[k][i];
          j = i;
          while (w[j - inc] < sw) {
            w[j] = w[j - inc];
            for (k = 0; k < m; k++) u[k][j] = u[k][j - inc];
            for (k = 0; k < n; k++) v[k][j] = v[k][j - inc];
            j -= inc;
            if (j < inc) break;
          }
          w[j] = sw;
          for (k = 0; k < m; k++) u[k][j] = su[k];
          for (k = 0; k<n; k++) v[k][j] = sv[k];
        }
      } while (inc > 1);
      for (k = 0; k < n; k++)
      {
        s = 0;
        for (i = 0; i < m; i++) if (u[i][k] < 0.) s++;
        for (j = 0; j < n; j++) if (v[j][k] < 0.) s++;
        if (s > (m + n) / 2) {
          for (i = 0; i < m; i++) u[i][k] = -u[i][k];
          for (j = 0; j<n; j++) v[j][k] = -v[j][k];
        }
      }
    }

    double pythag (const double a, const double b)
    {
      double absa = abs (a), absb = abs (b);
      return (absa > absb ? absa*sqrt (1.0 + SQR (absb / absa)) :
        (absb == 0.0 ? 0.0 : absb*sqrt (1.0 + SQR (absa / absb))));
    }
  };
}

#endif