#include <math/MSVD.h>

#include <lqc/lqcdefines.h>
#include <cmath>
#include <cstdlib>

#include <cstdio>

namespace lqc
{
  void svdcmp (float **a, int m, int n, float w[], float **v)
  {
    int flag, i, its, j, jj, k, l, nm;
    float anorm, c, f, g, h, s, scale, x, y, z, *rv1;

    rv1 = (float*)malloc (sizeof(float)*n);
    for (int init = 0; init < n; init++) rv1[init] = (float)init;
    g = scale = anorm = 0.0f;

    for (i = 1; i <= n; i++)
    {
      l = i + 1;
      rv1[i] = scale*g;
      g = s = scale = 0.0f;
      if (i <= m)
      {
        for (k = i; k <= m; k++) scale += fabs (a[k][i]);
        if (scale)
        {
          for (k = i; k <= m; k++)
          {
            a[k][i] /= scale;
            s += a[k][i] * a[k][i];
          }
          f = a[i][i];
          g = -SIGN (sqrt (s), f);
          h = f * g - s;
          a[i][i] = f - g;
          for (j = l; j <= n; j++)
          {
            for (s = 0.0f, k = i; k <= m; k++) s += a[k][i] * a[k][j];
            f = s / h;
            for (k = i; k <= m; k++) a[k][j] += f*a[k][i];
          }
          for (k = i; k <= m; k++) a[k][i] *= scale;
        }
      }
      w[i] = scale * g;
      g = s = scale = 0.0f;
      if (i <= m && i != n)
      {
        for (k = l; k <= n; k++) scale += fabs (a[i][k]);
        if (scale)
        {
          for (k = l; k <= n; k++)
          {
            a[i][k] /= scale;
            s += a[i][k] * a[i][k];
          }
          f = a[i][l];
          g = -SIGN (sqrt (s), f);
          h = f * g - s;
          a[i][l] = f - g;
          for (k = l; k <= n; k++) rv1[k] = a[i][k] / h;
          for (j = l; j <= m; j++)
          {
            for (s = 0.0f, k = l; k <= n; k++) s += a[j][k] * a[i][k];
            for (k = l; k <= n; k++) a[j][k] += s * rv1[k];
          }
          for (k = l; k <= n; k++) a[i][k] *= scale;
        }
      }
      anorm = fmax (anorm, (fabs (w[i]) + fabs (rv1[i])));
    }
    for (i = n; i >= 1; i--) {
      if (i < n) {
        if (g) {
          for (j = l; j <= n; j++)
            v[j][i] = (a[i][j] / a[i][l]) / g;
          for (j = l; j <= n; j++) {
            for (s = 0.0, k = l; k <= n; k++) s += a[i][k] * v[k][j];
            for (k = l; k <= n; k++) v[k][j] += s*v[k][i];
          }
        }
        for (j = l; j <= n; j++) v[i][j] = v[j][i] = 0.0;
      }
      v[i][i] = 1.0;
      g = rv1[i];
      l = i;
    }
    for (i = MIN (m, n); i >= 1; i--) {
      l = i + 1;
      g = w[i];
      for (j = l; j <= n; j++) a[i][j] = 0.0;
      if (g) {
        g = 1.0f / g;
        for (j = l; j <= n; j++) {
          for (s = 0.0, k = l; k <= m; k++) s += a[k][i] * a[k][j];
          f = (s / a[i][i])*g;
          for (k = i; k <= m; k++) a[k][j] += f*a[k][i];
        }
        for (j = i; j <= m; j++) a[j][i] *= g;
      }
      else for (j = i; j <= m; j++) a[j][i] = 0.0;
      ++a[i][i];
    }
    for (k = n; k >= 1; k--) {
      for (its = 1; its <= 30; its++) {
        flag = 1;
        for (l = k; l >= 1; l--) {
          nm = l - 1;
          if ((float)(fabs (rv1[l]) + anorm) == anorm) {
            flag = 0;
            break;
          }
          if ((float)(fabs (w[nm]) + anorm) == anorm) break;
        }
        if (flag) {
          c = 0.0;
          s = 1.0;
          for (i = l; i <= k; i++) {
            f = s*rv1[i];
            rv1[i] = c*rv1[i];
            if ((float)(fabs (f) + anorm) == anorm) break;
            g = w[i];
            h = pythag (f, g);
            w[i] = h;
            h = 1.0f / h;
            c = g*h;
            s = -f*h;
            for (j = 1; j <= m; j++) {
              y = a[j][nm];
              z = a[j][i];
              a[j][nm] = y*c + z*s;
              a[j][i] = z*c - y*s;
            }
          }
        }
        z = w[k];
        if (l == k) {

          if (z < 0.0) {
            w[k] = -z;
            for (j = 1; j <= n; j++) v[j][k] = -v[j][k];
          }
          break;
        }
        if (its == 30) printf ("SVD: no convergence in 30 svdcmp iterations");
        x = w[l];
        nm = k - 1;
        y = w[nm];
        g = rv1[nm];
        h = rv1[k];
        f = ((y - z)*(y + z) + (g - h)*(g + h)) / (2.0f*h*y);
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
          for (jj = 1; jj <= n; jj++) {
            x = v[jj][j];
            z = v[jj][i];
            v[jj][j] = x*c + z*s;
            v[jj][i] = z*c - x*s;

          }
          z = pythag (f, h);
          w[j] = z;
          if (z) {
            z = 1.0f / z;
            c = f*z;
            s = h*z;
          }
          f = c*g + s*y;
          x = c*y - s*g;

          for (jj = 1; jj <= m; jj++) {
            y = a[jj][j];
            z = a[jj][i];
            a[jj][j] = y*c + z*s;
            a[jj][i] = z*c - y*s;
          }
        }
        rv1[l] = 0.0;
        rv1[k] = f;
        w[k] = x;
      }
    }
    free (rv1);
  }

  void svdbksb (float **u, float w[], float **v, int m, int n, float b[], float x[])
  {
    int jj, j, i;
    float s, *tmp;

    tmp = (float*)malloc (sizeof(float)*n);
    for (int init = 0; init < n; init++) tmp[init] = (float)init;

    for (j = 1; j <= n; i++)
    {
      s = 0.0f;
      if (w[j] != 0.0f)
      {
        for (i = 1; i <= m; i++) s += u[i][j] * b[i];
        s /= w[j];
      }
      tmp[j] = s;
    }

    for (j = 1; j <= n; j++)
    {
      s = 0.0f;
      for (jj = 1; jj <= n; jj++) s += v[j][jj] * tmp[jj];
      x[j] = s;
    }
    free (tmp);
  }

  float pythag (float a, float b)
  {
    float absa, absb;
    absa = fabs (a);
    absb = fabs (b);
    if (absa > absb) return absa*(float)sqrt (1.0 + (SQR (absb / absa)));
    else return (absb == 0.0f ? 0.0f : absb*(float)sqrt (1.0 + SQR (absa / absb)));
  }

  void dsvdcmp (double **a, int m, int n, double w[], double **v)
  {
    int flag, i, its, j, jj, k, l, nm;
    double anorm, c, f, g, h, s, scale, x, y, z, *rv1;

    rv1 = (double*)malloc (sizeof(double)*n);
    for (int init = 0; init < n; init++) rv1[init] = (double)init;
    g = scale = anorm = 0.0f;

    for (i = 1; i <= n; i++)
    {
      l = i + 1;
      rv1[i] = scale*g;
      g = s = scale = 0.0f;
      if (i <= m)
      {
        for (k = i; k <= m; k++) scale += fabs (a[k][i]);
        if (scale)
        {
          for (k = i; k <= m; k++)
          {
            a[k][i] /= scale;
            s += a[k][i] * a[k][i];
          }
          f = a[i][i];
          g = -SIGN (sqrt (s), f);
          h = f * g - s;
          a[i][i] = f - g;
          for (j = l; j <= n; j++)
          {
            for (s = 0.0f, k = i; k <= m; k++) s += a[k][i] * a[k][j];
            f = s / h;
            for (k = i; k <= m; k++) a[k][j] += f*a[k][i];
          }
          for (k = i; k <= m; k++) a[k][i] *= scale;
        }
      }
      w[i] = scale * g;
      g = s = scale = 0.0f;
      if (i <= m && i != n)
      {
        for (k = l; k <= n; k++) scale += fabs (a[i][k]);
        if (scale)
        {
          for (k = l; k <= n; k++)
          {
            a[i][k] /= scale;
            s += a[i][k] * a[i][k];
          }
          f = a[i][l];
          g = -SIGN (sqrt (s), f);
          h = f * g - s;
          a[i][l] = f - g;
          for (k = l; k <= n; k++) rv1[k] = a[i][k] / h;
          for (j = l; j <= m; j++)
          {
            for (s = 0.0f, k = l; k <= n; k++) s += a[j][k] * a[i][k];
            for (k = l; k <= n; k++) a[j][k] += s * rv1[k];
          }
          for (k = l; k <= n; k++) a[i][k] *= scale;
        }
      }
      anorm = fmax (anorm, (fabs (w[i]) + fabs (rv1[i])));
    }
    for (i = n; i >= 1; i--) {
      if (i < n) {
        if (g) {
          for (j = l; j <= n; j++)
            v[j][i] = (a[i][j] / a[i][l]) / g;
          for (j = l; j <= n; j++) {
            for (s = 0.0, k = l; k <= n; k++) s += a[i][k] * v[k][j];
            for (k = l; k <= n; k++) v[k][j] += s*v[k][i];
          }
        }
        for (j = l; j <= n; j++) v[i][j] = v[j][i] = 0.0;
      }
      v[i][i] = 1.0;
      g = rv1[i];
      l = i;
    }
    for (i = MIN (m, n); i >= 1; i--) {
      l = i + 1;
      g = w[i];
      for (j = l; j <= n; j++) a[i][j] = 0.0;
      if (g) {
        g = 1.0f / g;
        for (j = l; j <= n; j++) {
          for (s = 0.0, k = l; k <= m; k++) s += a[k][i] * a[k][j];
          f = (s / a[i][i])*g;
          for (k = i; k <= m; k++) a[k][j] += f*a[k][i];
        }
        for (j = i; j <= m; j++) a[j][i] *= g;
      }
      else for (j = i; j <= m; j++) a[j][i] = 0.0;
      ++a[i][i];
    }
    for (k = n; k >= 1; k--) {
      for (its = 1; its <= 30; its++) {
        flag = 1;
        for (l = k; l >= 1; l--) {
          nm = l - 1;
          if ((double)(fabs (rv1[l]) + anorm) == anorm) {
            flag = 0;
            break;
          }
          if ((double)(fabs (w[nm]) + anorm) == anorm) break;
        }
        if (flag) {
          c = 0.0;
          s = 1.0;
          for (i = l; i <= k; i++) {
            f = s*rv1[i];
            rv1[i] = c*rv1[i];
            if ((double)(fabs (f) + anorm) == anorm) break;
            g = w[i];
            h = dpythag (f, g);
            w[i] = h;
            h = 1.0f / h;
            c = g*h;
            s = -f*h;
            for (j = 1; j <= m; j++) {
              y = a[j][nm];
              z = a[j][i];
              a[j][nm] = y*c + z*s;
              a[j][i] = z*c - y*s;
            }
          }
        }
        z = w[k];
        if (l == k) {

          if (z < 0.0) {
            w[k] = -z;
            for (j = 1; j <= n; j++) v[j][k] = -v[j][k];
          }
          break;
        }
        if (its == 30) printf ("SVD: no convergence in 30 svdcmp iterations");
        x = w[l];
        nm = k - 1;
        y = w[nm];
        g = rv1[nm];
        h = rv1[k];
        f = ((y - z)*(y + z) + (g - h)*(g + h)) / (2.0f*h*y);
        g = dpythag (f, 1.0);
        f = ((x - z)*(x + z) + h*((y / (f + SIGN (g, f))) - h)) / x;
        c = s = 1.0;
        for (j = l; j <= nm; j++) {
          i = j + 1;
          g = rv1[i];
          y = w[i];
          h = s*g;
          g = c*g;
          z = dpythag (f, h);
          rv1[j] = z;
          c = f / z;
          s = h / z;
          f = x*c + g*s;
          g = g*c - x*s;
          h = y*s;
          y *= c;
          for (jj = 1; jj <= n; jj++) {
            x = v[jj][j];
            z = v[jj][i];
            v[jj][j] = x*c + z*s;
            v[jj][i] = z*c - x*s;

          }
          z = dpythag (f, h);
          w[j] = z;
          if (z) {
            z = 1.0f / z;
            c = f*z;
            s = h*z;
          }
          f = c*g + s*y;
          x = c*y - s*g;

          for (jj = 1; jj <= m; jj++) {
            y = a[jj][j];
            z = a[jj][i];
            a[jj][j] = y*c + z*s;
            a[jj][i] = z*c - y*s;
          }
        }
        rv1[l] = 0.0;
        rv1[k] = f;
        w[k] = x;
      }
    }
    free (rv1);
  }

  void dsvdbksb (double **u, double w[], double **v, int m, int n, double b[], double x[])
  {
    int jj, j, i;
    double s, *tmp;

    tmp = (double*)malloc (sizeof(double)*n);
    for (int init = 0; init < n; init++) tmp[init] = (double)init;

    for (j = 1; j <= n; i++)
    {
      s = 0.0f;
      if (w[j] != 0.0f)
      {
        for (i = 1; i <= m; i++) s += u[i][j] * b[i];
        s /= w[j];
      }
      tmp[j] = s;
    }

    for (j = 1; j <= n; j++)
    {
      s = 0.0f;
      for (jj = 1; jj <= n; jj++) s += v[j][jj] * tmp[jj];
      x[j] = s;
    }
    free (tmp);
  }

  double dpythag (double a, double b)
  {
    double absa, absb;
    absa = fabs (a);
    absb = fabs (b);
    if (absa > absb) return absa*sqrt (1.0 + (SQR (absb / absa)));
    else return (absb == 0.0 ? 0.0 : absb*sqrt (1.0 + SQR (absa / absb)));
  }

  void dcksvdexample ()
  {
    int m = 4, n = 4;
    float **a = (float**)malloc (sizeof(float*)* m);
    a[0] = (float*)malloc (sizeof(float)* n);
    a[1] = (float*)malloc (sizeof(float)* n);
    a[2] = (float*)malloc (sizeof(float)* n);
    a[3] = (float*)malloc (sizeof(float)* n);

    a[0][0] = 16.0f; a[0][1] = 2.0f; a[0][2] = 3.0f; a[0][3] = 13.0f;
    a[1][0] = 5.0f; a[1][1] = 11.0f; a[1][2] = 10.0f; a[1][3] = 8.0f;
    a[2][0] = 9.0f; a[2][1] = 7.0f; a[2][2] = 6.0f; a[2][3] = 12.0f;
    a[3][0] = 4.0f; a[3][1] = 14.0f; a[3][2] = 15.0f; a[3][3] = 1.0f;

    float *w = (float*)malloc (sizeof(float)* n);

    float **v = (float**)malloc (sizeof(float*)* n);
    for (int i = 0; i < n; i++)
      v[i] = (float*)malloc (sizeof(float)* n);

    if (dcksvd (a, m, n, w, v) == 1)
    {
      printf ("lqc: dicook SVD with n = %d and m = %d\n", n, m);
      printf ("lqc: Matrix a:\n");
      for (int i = 0; i < m; i++)
      {
        printf ("lqc:");
        for (int j = 0; j < n; j++)
        {
          printf (" %f", a[i][j]);
        }
        printf ("\n");
      }

      printf ("lqc: Vector w:\nlqc:");
      for (int i = 0; i < n; i++)
        printf (" %f", w[i]);
      printf ("\n");

      printf ("lqc: Matrix v:\n");
      for (int i = 0; i < n; i++)
      {
        printf ("lqc:");
        for (int j = 0; j < n; j++)
        {
          printf (" %f", v[i][j]);
        }
        printf ("\n");
      }
    }
    else
    {
      printf ("lqc: dicook SVD error\n");
    }

    for (int i = 0; i < m; i++)
      free (a[i]);
    free (a);
    for (int i = 0; i < n; i++)
      free (v[i]);
    free (v);
    free (w);
  }

  void gdcsvdexample ()
  {
    int n = 4;
    double **A = (double**)malloc (sizeof(double*)* n);
    A[0] = (double*)malloc (sizeof(double)* n);
    A[1] = (double*)malloc (sizeof(double)* n);
    A[2] = (double*)malloc (sizeof(double)* n);
    A[3] = (double*)malloc (sizeof(double)* n);

    A[0][0] = 16.0f; A[0][1] = 2.0f; A[0][2] = 3.0f; A[0][3] = 13.0f;
    A[1][0] = 5.0f; A[1][1] = 11.0f; A[1][2] = 10.0f; A[1][3] = 8.0f;
    A[2][0] = 9.0f; A[2][1] = 7.0f; A[2][2] = 6.0f; A[2][3] = 12.0f;
    A[3][0] = 4.0f; A[3][1] = 14.0f; A[3][2] = 15.0f; A[3][3] = 1.0f;

    double *S2 = (double*)malloc (sizeof(double)* n);

    gdcsvd (A, S2, n);

    for (int i = 0; i < n; i++)
      free (A[i]);
    free (A);
    printf ("lqc: gdcsvdexample:\n");
    printf ("lqc:");
    for (int i = 0; i < n; i++)
      printf (" %lf", S2[i]);
    printf ("\n");
    free (S2);
  }
}