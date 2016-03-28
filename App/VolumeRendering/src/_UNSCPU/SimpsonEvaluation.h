/*!
\file SimpsonEvaluation.h
\brief Simpson Evaluation generic class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef SIMPSON_EVALUATOR_H
#define SIMPSON_EVALUATOR_H

template <class T, class P = T>
class SimpsonEvaluator
{
private:
  typedef void (*RetFunction) (T, P, P, void*);
  typedef void (*BeforeRecursionFunction) (void*, double);

  T AdaptiveSimpsonsRecursion (T (*f)(P, double, void*), P a, P b, double epsilon,
                               T S, T fa, T fb, T fc, int bottom, double (*ErrorEvaluation) (T, T), double (*Pdist) (P, P), void* data)
  {
    P c = (a + b) / 2.0;
    double h = Pdist (a, b);
    P d = (a + c) / 2.0, e = (c + b) / 2.0;
    T fd = f (d, h / 4.0, data);
    T fe = f (e, h / 4.0, data);
    T Sleft = (h / 12.0) * (fa + (4.0 * fd) + fc);
    T Sright = (h / 12.0) * (fc + (4.0 * fe) + fb);
    T S2 = Sleft + Sright;
    if (bottom <= 0 || ErrorEvaluation (S2, S) <= 15.0 * epsilon /*|| h <= max_interval*/)
    {
      cumulative_error += epsilon;
      T Sret = S2 + (S2 - S) / 15.0;
      if (m_function_ret)
        m_function_ret (Sret, a, b, data);
      return Sret;
    }
    if (m_before_call_recursion_func)
      m_before_call_recursion_func (data, epsilon);
    T Rleft  = AdaptiveSimpsonsRecursion (f, a, c, epsilon /*/ 2.0*/, Sleft , fa, fc, fd, bottom - 1, ErrorEvaluation, Pdist, data);
    if (m_before_call_recursion_func)
      m_before_call_recursion_func (data, epsilon);
    T Rright = AdaptiveSimpsonsRecursion (f, c, b, epsilon /*/ 2.0*/, Sright, fc, fb, fe, bottom - 1, ErrorEvaluation, Pdist, data);
    return Rleft + Rright;
  }

public:
  double cumulative_error;
  double max_interval;
  SimpsonEvaluator ()
    : m_function_ret (NULL),
    m_before_call_recursion_func (NULL)
  {
    cumulative_error = 0;
  }

  ~SimpsonEvaluator () {}

  T AdaptiveSimpsons (T (*f)(P, double, void*), P a, P b, double epsilon, int maxRecursionDepth, double (*ErrorEvaluation) (T, T), double (*Pdist) (P, P), void* data)
  {
    P c = (a + b) / 2.0;
    double h = Pdist (a, b);
    T fa = f (a, h / 2.0, data);
    T fc = f (c, h / 2.0, data);
    T fb = f (b, h / 2.0, data);
    T S = (h / 6.0)*(fa + (4.0 * fc) + fb);
    if (m_before_call_recursion_func)
      m_before_call_recursion_func (data, epsilon);
    return AdaptiveSimpsonsRecursion (f, a, b, epsilon, S, fa, fb, fc, maxRecursionDepth, ErrorEvaluation, Pdist, data);
  }

  void SetReturnFunction (RetFunction r)
  {
    m_function_ret = r;
  }
  
  void SetBeforeRecursionFunction (BeforeRecursionFunction r)
  {
    m_before_call_recursion_func = r;
  }

protected:

private:
  RetFunction m_function_ret;
  BeforeRecursionFunction m_before_call_recursion_func;
};

#endif