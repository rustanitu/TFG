/*!
\file SimpsonEvaluation.h
\brief Simpson Evaluation generic class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef SIMPSON_EVALUATOR_H
#define SIMPSON_EVALUATOR_H

//http://vanemden.wordpress.com/2014/07/30/recursion-versus-iteration/

template <class T, class P, class ER>
class AdaptiveSimpsonEvaluator
{
private:
  typedef void (*ReturnFunc)              (T, P, P, double, double, int, double, ER, void*);
  typedef void (*BeforeRecursionFunc)     (void*, double);
  typedef T    (*CompositionFunc)         (T, T, P, P, double);
  typedef void (*ErrorManipFunc)          (double*);

  typedef double (*DistanceFunc)          (P, P);
  typedef double (*ErrorEvaluationFunc)   (T, T, double, ER*);

  T AdaptiveSimpsonsRecursion (T (*f)(P, double, double, void*), P a, P b, double epsilon,
                               T S, T fa, T fb, T fc, int bottom, void* data)
  {
    P c = (a + b) / 2.0;
    double h = m_distance_func (a, b);
    P d = (a + c) / 2.0, e = (c + b) / 2.0;
    T fd = f (d, h / 4.0, epsilon/2, data);
    T fe = f (e, h / 4.0, epsilon/2, data);
    T Sleft = (h / 12.0) * (fa + (4.0 * fd) + fc);
    T Sright = (h / 12.0) * (fc + (4.0 * fe) + fb);
    T S2 = Sleft + Sright;
    ER erroeval;
    double erroreval = m_erroreval_func (S2, S, epsilon*15.0, &erroeval) / 15.0;
    if (bottom <= 0 || erroreval <= epsilon /*|| h <= max_interval*/)
    {
      //sum error eval
      estimated_cumulative_error += epsilon;
      evaluated_cumulative_error += erroreval * 15;
      evaluated_cumulative_error_ER += erroeval;

      T Sret;
      if (m_composition_func)
        Sret = m_composition_func (S, S2, a, b, epsilon);
      else
        Sret = S2 + (S2 - S) / 15.0;
      
      if (m_return_func)
        m_return_func (Sret, a, b, erroreval * 15, epsilon, bottom, h, erroeval, data);
      return Sret;
    }
    
    //ISOSUPERFÍCIES (a recursão vai até onde conseguir ir)
    //Manipulação do erro a cada recursão para um subintervalo
    double err = epsilon;
    if (m_func_error_manip)
      m_func_error_manip (&err);
    else
    {
      if (err > errordivthreshold)
        err /= 2.0;
      //else
      //  err = err;
    }

    if (m_before_call_recursion_func)
      m_before_call_recursion_func (data, epsilon);
    T Rleft = AdaptiveSimpsonsRecursion (f, a, c, err, Sleft, fa, fc, fd, bottom - 1, data);
    if (m_before_call_recursion_func)
      m_before_call_recursion_func (data, epsilon);
    T Rright = AdaptiveSimpsonsRecursion (f, c, b, err, Sright, fc, fb, fe, bottom - 1, data);
    return Rleft + Rright;
  }

public:
  double estimated_cumulative_error;
  double evaluated_cumulative_error;
  lqc::Vector3d evaluated_cumulative_error_ER;
  double errordivthreshold;
  AdaptiveSimpsonEvaluator ()
    : m_return_func (NULL),
    m_before_call_recursion_func (NULL),
    m_composition_func (NULL),
    m_func_error_manip (NULL),
    m_distance_func (NULL),
    m_erroreval_func (NULL)
  {
    estimated_cumulative_error = 0;
    evaluated_cumulative_error = 0;
    errordivthreshold = 1 / pow (10, 8);
  }

  ~AdaptiveSimpsonEvaluator () {}

  T AdaptiveSimpsons (T (*f)(P, double, double, void*), P a, P b, double epsilon, int maxRecursionDepth, void* data)
  {
    if (m_distance_func == NULL || m_erroreval_func == NULL) return T(0);
    double h = m_distance_func (a, b);
    if (h == 0) return T (0);
    P c = (a + b) / 2.0;
    T fa = f (a, h / 2.0, epsilon, data);
    T fc = f (c, h / 2.0, epsilon, data);
    T fb = f (b, h / 2.0, epsilon, data);
    T S = (h / 6.0)*(fa + (4.0 * fc) + fb);
    if (m_before_call_recursion_func)
      m_before_call_recursion_func (data, epsilon);
    return AdaptiveSimpsonsRecursion (f, a, b, epsilon, S, fa, fb, fc, maxRecursionDepth, data);
  }

  void SetReturnFunction (ReturnFunc r)
  {
    m_return_func = r;
  }
  
  void SetBeforeRecursionFunction (BeforeRecursionFunc r)
  {
    m_before_call_recursion_func = r;
  }

  void SetPrimaryFunctions (DistanceFunc distf, ErrorEvaluationFunc errorevalf)
  {
    m_distance_func = distf;
    m_erroreval_func = errorevalf;
  }

  void SetFunctions (DistanceFunc distf,
                     ErrorEvaluationFunc errorevalf,
                     BeforeRecursionFunc befrec = NULL,
                     ReturnFunc returnfunc = NULL,
                     CompositionFunc compositionfunc = NULL,
                     ErrorManipFunc errormanipfunc = NULL)
  {
    m_distance_func = distf;
    m_erroreval_func = errorevalf;

    m_before_call_recursion_func = befrec;
    m_return_func = returnfunc;
    m_composition_func = compositionfunc;
    m_func_error_manip = errormanipfunc;
  }
  
protected:

private:
  DistanceFunc m_distance_func;
  ErrorEvaluationFunc m_erroreval_func;

  ReturnFunc m_return_func;
  BeforeRecursionFunc m_before_call_recursion_func;
  CompositionFunc m_composition_func;
  ErrorManipFunc m_func_error_manip;
};

template <class T, class P = T>
class AdaptiveSimpsonOuterVersion
{
private:
  typedef void (*ReturnFunc)              (T, P, P, double, double, int, double, T, T, void*);
  typedef void (*BeforeRecursionFunc)     (void*, double);

  T AdaptiveSimpsonsRecursion (T (*f)(P, void*), P a, P b, double tol,
                               T S, T fa, T fb, T fc, int bottom, void* data, bool (*ErrorEvaluationFunc)(T, T, double))
  {
    P c = (a + b) / 2.0;
    double h = b - a;
    P d = (a + c) / 2.0, e = (c + b) / 2.0;
    T fd = f (d, data);
    T fe = f (e, data);
    T Sleft = (h / 12.0) * (fa + (4.0 * fd) + fc);
    T Sright = (h / 12.0) * (fc + (4.0 * fe) + fb);
    T S2 = Sleft + Sright;
    if (bottom <= 0 || ErrorEvaluationFunc(S2, S, tol * 15.0) /*|| b - a < 0.1*/ /*|| h <= max_interval*/)
    {
      T Sret = S2 + (S2 - S) / 15.0;
      if (m_return_func)
        m_return_func (Sret, a, b, tol, tol, bottom, h, S, S2, data);

      return Sret;
    }
    if (m_before_call_recursion_func)
      m_before_call_recursion_func (data, tol);
    T Rleft = AdaptiveSimpsonsRecursion (f, a, c, tol / 2.0, Sleft, fa, fc, fd, bottom - 1, data, ErrorEvaluationFunc);
    if (m_before_call_recursion_func)
      m_before_call_recursion_func (data, tol);
    T Rright = AdaptiveSimpsonsRecursion (f, c, b, tol / 2.0, Sright, fc, fb, fe, bottom - 1, data, ErrorEvaluationFunc);
    return Rleft + Rright;
  }

public:
  AdaptiveSimpsonOuterVersion ()
    : m_return_func (NULL),
    m_before_call_recursion_func (NULL) {}
  ~AdaptiveSimpsonOuterVersion () {}

  T AdaptiveSimpsons (T (*f)(P, void*), P a, P b, double tol, int maxRecursionDepth, void* data, bool (*ErrorEvaluationFunc)(T, T, double))
  {
    double h = b - a;
    if (h == 0) return T (0);
    P c = (a + b) / 2.0;
    T fa = f (a, data);
    T fc = f (c, data);
    T fb = f (b, data);
    T S = (h / 6.0)*(fa + (4.0 * fc) + fb);
    if (m_before_call_recursion_func)
      m_before_call_recursion_func (data, tol);
    return AdaptiveSimpsonsRecursion (f, a, b, tol, S, fa, fb, fc, maxRecursionDepth, data, ErrorEvaluationFunc);
  }

  void SetFunctions (BeforeRecursionFunc befrec = NULL,
                     ReturnFunc returnfunc = NULL)
  {
    m_before_call_recursion_func = befrec;
    m_return_func = returnfunc;
  }

protected:
private:
  ReturnFunc m_return_func;
  BeforeRecursionFunc m_before_call_recursion_func;
};


template <class T, class P>
class AdaptiveSimpsonMethod
{
private:
  typedef double (*DistanceFunc)        (P, P);
  typedef double (*ErrorEvaluationFunc) (T, T);

  T AdaptiveSimpsonsRecursion (T (*f)(P, void*), P a, P b, double epsilon,
    T S, T fa, T fb, T fc, int bottom, void* data)
  {
    P c = (a + b) / 2.0;
    double h = m_distance_func (a, b);
    P d = (a + c) / 2.0, e = (c + b) / 2.0;
    T fd = f (d, data);
    T fe = f (e, data);
    T Sleft = (h / 12.0) * (fa + (4.0 * fd) + fc);
    T Sright = (h / 12.0) * (fc + (4.0 * fe) + fb);
    T S2 = Sleft + Sright;
    if (bottom <= 0 || m_erroreval_func (S2, S) / 15.0 <= epsilon || h <= REFERENCE_MIN_STEP)
      return S2 + (S2 - S) / 15.0;
    return AdaptiveSimpsonsRecursion (f, a, c, epsilon / 2.0, Sleft, fa, fc, fd, bottom - 1, data)
        + AdaptiveSimpsonsRecursion (f, c, b, epsilon / 2.0, Sright, fc, fb, fe, bottom - 1, data);
  }

public:
  AdaptiveSimpsonMethod () : m_distance_func (NULL), m_erroreval_func (NULL) {}
  ~AdaptiveSimpsonMethod () {}

  T AdaptiveSimpsons (T (*f)(P, void*), P a, P b, double epsilon, T fa, T fc, T fb, int maxRecursionDepth, void* data)
  {
    if (m_distance_func == NULL || m_erroreval_func == NULL) return T (0);
    double h = m_distance_func (a, b);
    if (h == 0) return T (0);
    P c = (a + b) / 2.0;
    T S = (h / 6.0)*(fa + (4.0 * fc) + fb);
    return AdaptiveSimpsonsRecursion (f, a, b, epsilon, S, fa, fb, fc, maxRecursionDepth, data);
  }

  void SetFunctions (DistanceFunc distf,
    ErrorEvaluationFunc errorevalf)
  {
    m_distance_func = distf;
    m_erroreval_func = errorevalf;
  }

protected:

private:
  DistanceFunc m_distance_func;
  ErrorEvaluationFunc m_erroreval_func;
};

#endif