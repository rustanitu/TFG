#ifndef CPU_VOLUME_EVALUATOR_DEFINES_H
#define CPU_VOLUME_EVALUATOR_DEFINES_H

template <class T>
T SimpsonRule (T fa, T fc, T fb, double h)
{
  return (h / 6.0) * (fa + 4.0 * fc + fb);
}

//#define ANALYSIS__RGBA_ALONG_THE_RAY



//#define STATISTICS

//#define EVALUATIONINTEGRAL_CALCULATE_MINOR_STEP

//#define ANALYSIS__MINMAX_STEPS



//#define ANALYSIS__ERROR_ALONG_THE_RAY

//#define ANALYSIS__STORE_INTEGRATE_INTERVAL_TIMES

//Acessos ao volume e à Função de transferência
#define STATISTICS_GETS

#define REFERENCE_MIN_STEP 0.01
#define REFERENCE_MAX_STEP 8.0
#define RESHAPE_TRUTH_FACTOR 0.8

#define STACK_LEGNTH 12

//#define COMPUTE_STEPS_ALONG_EVALUATION

#define INTEGRATOR__STEP_CONTROLLED
#ifdef INTEGRATOR__STEP_CONTROLLED
  #define INTEGRATOR__MAX_STEP_BOUNDARY 10
#endif

#define INTEGRATOR_INITIAL_STEP 0.5

#define MAX_ERROR_FACTOR 15.0

//definições de hmin e hmax
#define USE_HMAX


#endif