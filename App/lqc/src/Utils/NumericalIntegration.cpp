#include <lqc/Utils/NumericalIntegration.h>

#include <lqc/lqcdefines.h>

#include <math/Vector3.h>
#include <math/MGeometry.h>

#include <math.h>  // include file for fabs and sin
#include <stdio.h> // include file for printf
#include <ctime>

namespace lqc
{
  void INAdaptiveSimpsonsAux (double (*f)(double), double a, double b, double epsilon,
    double S, double fa, double fb, double fc, int bottom, INode<double, double> *n)
  {
    double c = (a + b) / 2, h = b - a;
    double d = (a + c) / 2, e = (c + b) / 2;
    double fd = f (d), fe = f (e);
    double Sleft = (h / 12)*(fa + 4 * fd + fc);
    double Sright = (h / 12)*(fc + 4 * fe + fb);
    double S2 = Sleft + Sright;

    n->m_left = new INode<double, double> ();
    n->m_left->m_value = fd;
    n->m_left->m_pos = d;
    n->m_right = new INode<double, double> ();
    n->m_right->m_value = fe;
    n->m_right->m_pos = e;

    if (!(bottom <= 0 || fabs (S2 - S) <= 15 * epsilon))
    {
      INAdaptiveSimpsonsAux (f, a, c, epsilon / 2, Sleft, fa, fc, fd, bottom - 1, n->m_left);
      INAdaptiveSimpsonsAux (f, c, b, epsilon / 2, Sright, fc, fb, fe, bottom - 1, n->m_right);
    }
  }

  double INAdaptiveSimpsons (double (*f)(double), double a, double b,
    double epsilon, int maxRecursionDepth)
  {
    double c = (a + b) / 2, h = b - a;
    double fa = f (a), fb = f (b), fc = f (c);
    double S = (h / 6)*(fa + 4 * fc + fb);
    ITree<double, double>* t = new ITree<double, double> ();
    t->m_sum = 0.0;
    t->m_a = a; t->m_b = b;
    t->m_fa = fa; t->m_fb = fb;
    t->m_node = new INode<double, double> ();
    t->m_node->m_value = fc;
    t->m_node->m_pos = c;

    INAdaptiveSimpsonsAux (f, a, b, epsilon, S, fa, fb, fc, maxRecursionDepth, t->m_node);

    t->MakeSum ();
    double ret = t->m_sum;
    delete t;

    return ret;
  }

  double AdaptiveSimpsonsAux (double (*f)(double), double a, double b, double epsilon,                 
                              double S, double fa, double fb, double fc, int bottom)
  {                 
      double c = (a + b)/2, h = b - a;                                                                  
      double d = (a + c)/2, e = (c + b)/2;                                                              
      double fd = f(d), fe = f(e);                                                                      
      double Sleft = (h/12)*(fa + 4*fd + fc);                                                           
      double Sright = (h/12)*(fc + 4*fe + fb);                                                          
      double S2 = Sleft + Sright;                                                                       
      if (bottom <= 0 || fabs(S2 - S) <= 15*epsilon)                                                    
        return S2 + (S2 - S)/15;                                                                        
      return AdaptiveSimpsonsAux(f, a, c, epsilon/2, Sleft,  fa, fc, fd, bottom-1) +                    
        AdaptiveSimpsonsAux(f, c, b, epsilon/2, Sright, fc, fb, fe, bottom-1);                     
  }         

  double AdaptiveSimpsons (double (*f)(double), double a, double b, double epsilon, int maxRecursionDepth)    
  {   
      double c = (a + b)/2, h = b - a;                                                                  
      double fa = f(a), fb = f(b), fc = f(c);                                                           
      double S = (h/6)*(fa + 4*fc + fb);                                                                
      return AdaptiveSimpsonsAux(f, a, b, epsilon, S, fa, fb, fc, maxRecursionDepth);                   
  }

  double ZeroOrderIntegration (double (*f)(double), double a, double b, double epsilon)
  {
    double ZOA = 0.0;
    for (double i = a; i < b; i += epsilon)
      ZOA += ((f (i) + f (i + epsilon)) / 2.0) * epsilon;

    return ZOA;
  }
  
  void TestAdaptiveSimpsonMethod (double (*f)(double), double a, double b, double epsilon, int maxRecursionDepth)
  {
    printf ("lqc: Numerical Integration Methods of function\n");
    std::clock_t s1,s2,s3;
    double d1,d2,d3;

    s1 = std::clock ();
    double FLA = ZeroOrderIntegration (f, a, b, NANO10);
    d1 = (std::clock () - s1) / (double)CLOCKS_PER_SEC;

    s2 = std::clock ();
    double ZOA = ZeroOrderIntegration (f, a, b, epsilon);
    d2 = (std::clock () - s2) / (double)CLOCKS_PER_SEC;

    s3 = std::clock ();
    double ASM = AdaptiveSimpsons (f, a, b, epsilon, maxRecursionDepth);
    d3 = (std::clock () - s3) / (double)CLOCKS_PER_SEC;
        
    printf ("  - With low epsilon and sum = %lf (%lf s)\n", FLA, d1);
    printf ("  - Zero Order Approximation = %lf (%lf s)\n", ZOA, d2);
    printf ("  - Adaptive Simpson Method  = %lf (%lf s)\n", ASM, d3);
  }

  double AdaptiveSimpsonsAux (double (*f)(double, double, double), lqc::Vector3d a, lqc::Vector3d b, double epsilon,
                              double S, double fa, double fb, double fc, int bottom)
  {
    lqc::Vector3d c = (a + b) / 2.0;
    double h = lqc::Distance (a, b);
    lqc::Vector3d d = (a + c) / 2.0, e = (c + b) / 2.0;
    double fd = f ((double)d.x, (double)d.y, (double)d.z);
    double fe = f ((double)e.x, (double)e.y, (double)e.z);
    double Sleft = (h / 12.0) * (fa + 4.0 * fd + fc);
    double Sright = (h / 12.0) * (fc + 4.0 * fe + fb);
    double S2 = Sleft + Sright;
    if (bottom <= 0 || fabs (S2 - S) <= 15.0 * epsilon)
      return S2 + (S2 - S) / 15.0;
    return AdaptiveSimpsonsAux (f, a, c, epsilon / 2.0, Sleft, fa, fc, fd, bottom - 1)
      + AdaptiveSimpsonsAux (f, c, b, epsilon / 2.0, Sright, fc, fb, fe, bottom - 1);
  }

  double AdaptiveSimpsons (double (*f)(double, double, double), lqc::Vector3d a, lqc::Vector3d b, double epsilon, int maxRecursionDepth)
  {
    lqc::Vector3d c = (a + b) / 2.0;
    double h = lqc::Distance(a, b);
    double fa = f ((double)a.x, (double)a.y, (double)a.z);
    double fb = f ((double)b.x, (double)b.y, (double)b.z);
    double fc = f ((double)c.x, (double)c.y, (double)c.z);
    double S = (lqc::Distance (a, b) / 6.0)*(fa + 4.0 * fc + fb);
    return AdaptiveSimpsonsAux (f, a, b, epsilon, S, fa, fb, fc, maxRecursionDepth);
  }
  
  double ZeroOrderIntegration (double (*f)(double, double, double), lqc::Vector3d a, lqc::Vector3d b, double epsilon)
  {
    lqc::Vector3d atob = lqc::Vector3d::Normalize(b - a);
    atob = atob*epsilon;

    double ZOA = 0.0;
    for (lqc::Vector3d s = a; lqc::Distance (a, s) < lqc::Distance (a, b); s += atob)
      ZOA += ((f (s.x, s.y, s.z) + f (s.x + atob.x, s.y + atob.y, s.z + atob.z)) / 2.0) * epsilon;

    return ZOA;
  }
  
  void TestAdaptiveSimpsonMethod (double (*f)(double, double, double), lqc::Vector3d a,
                                  lqc::Vector3d b, double epsilon, int maxRecursionDepth)
  {
    printf ("lqc: Numerical Integration Methods\n");
    std::clock_t s1, s2, s3;
    double d1, d2, d3;

    s1 = std::clock ();
    double FLA = ZeroOrderIntegration (f, a, b, NANO10);
    d1 = (std::clock () - s1) / (double)CLOCKS_PER_SEC;

    s2 = std::clock ();
    double ZOA = ZeroOrderIntegration (f, a, b, epsilon);
    d2 = (std::clock () - s2) / (double)CLOCKS_PER_SEC;

    s3 = std::clock ();
    double ASM = AdaptiveSimpsons (f, a, b, epsilon, maxRecursionDepth);
    d3 = (std::clock () - s3) / (double)CLOCKS_PER_SEC;

    printf ("  - With low epsilon and sum = %lf (%lf s)\n", FLA, d1);
    printf ("  - Zero Order Approximation = %lf (%lf s)\n", ZOA, d2);
    printf ("  - Adaptive Simpson Method  = %lf (%lf s)\n", ASM, d3);

  }
}