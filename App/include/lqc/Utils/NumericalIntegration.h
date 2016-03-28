//Adaptive Simpson's method
//Clenshaw–Curtis quadrature
//Gauss–Kronrod quadrature formula

#ifndef LQC_UTILS_NUMERICALINTEGRATION_H
#define LQC_UTILS_NUMERICALINTEGRATION_H

#include <math/Vector3.h>

namespace lqc
{
  template<typename T, typename U = T>
  class INode
  {
  public:
    INode ()
      : m_left (NULL), m_right (NULL)
    {}

    ~INode ()
    {
      if (m_left != NULL)
        delete m_left;
      if (m_right != NULL)
        delete m_right;
    }

    //Tem que modificar a função pra pega um nível a mais
    T MakeSum (U a, U b, T fa, T fb)
    {
      if (m_left == NULL && m_right == NULL)
      {
        U c = m_pos, h = b - a;
        T fc = m_value;
        T S = (h / 6.0)*(fa + 4.0 * fc + fb);
        return S;
      }
      else if (m_left != NULL && m_right != NULL)
      {
        T Sl = m_left->MakeSum (a, m_pos, fa, m_value);
        T Sr = m_right->MakeSum (m_pos, b, m_value, fb);
        return Sl + Sr;
      }
      else
      {
        printf ("INode:");
        if (!m_left)
          printf (" m_left NULL");
        if (!m_right)
          printf (" m_right NULL");
        printf (".\n");
        return -1000000;
      }
    }

    T m_value;
    U m_pos;
    INode *m_left, *m_right;
  private:
  };

  template<typename T, typename U = T>
  class ITree
  {
  public:
    ITree ()
      : m_node (NULL)
    {}

    ~ITree ()
    {
      delete m_node;
    }

    void MakeSum ()
    {
      m_sum = m_node->MakeSum (m_a, m_b, m_fa, m_fb);
    }

    T m_sum;
    T m_fa, m_fb;
    U m_a, m_b;
    INode<T, U> *m_node;
  private:
  };

  double INAdaptiveSimpsons (double (*f)(double), double a, double b,
                             double epsilon, int maxRecursionDepth);

  //From http://en.wikipedia.org/wiki/Adaptive_Simpson's_method
  // Adaptive Simpson's Rule
  // Parameters:
  // 1. ptr to function
  // 2|3. interval [a,b]
  // 4. error tolerance
  double AdaptiveSimpsons (double (*f)(double), double a, double b,  
                           double epsilon, int maxRecursionDepth);
  
  double ZeroOrderIntegration (double (*f)(double), double a,
                               double b, double epsilon);

  void TestAdaptiveSimpsonMethod (double (*f)(double), double a, double b,  
                                  double epsilon, int maxRecursionDepth);

  double AdaptiveSimpsons (double (*f)(double, double, double), lqc::Vector3d a,
                           lqc::Vector3d b, double epsilon, int maxRecursionDepth);

  double ZeroOrderIntegration (double (*f)(double, double, double), lqc::Vector3d a,
                               lqc::Vector3d b, double epsilon);
  
  void TestAdaptiveSimpsonMethod (double (*f)(double, double, double), lqc::Vector3d a,
                                  lqc::Vector3d b, double epsilon, int maxRecursionDepth);
}

#endif