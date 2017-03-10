/*!
\file CPUViewMethod.h
\brief CPU ViewMethod class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef CPUVIEWMETHOD_H
#define CPUVIEWMETHOD_H

#include "../ViewMethod/ViewMethod.h"

#include "CPUUserInterface.h"
#include "CPURenderer.h"
#include "../CPUTimer.h"
/*! Class associated with the CPU view method.
\ingroup views
*/

typedef float err_type;

class CPUViewMethod : public ViewMethod
{
public:
  /*! Constructor*/
  CPUViewMethod ();
  /*! Destructor*/
  ~CPUViewMethod ();

  virtual int Idle_Action_CB (Ihandle* cnv_renderer);
  virtual int Keyboard_CB (Ihandle *ih, int c, int press);
  virtual int Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status);
  virtual int Motion_CB (Ihandle *ih, int x, int y, char *status);
  virtual int Resize_CB (Ihandle *ih, int width, int height);
  virtual void CreateIupUserInterface ();
  virtual void UpdateIupUserInterface ();
  virtual void ResetCamera ();

  virtual void SaveCameraState (std::string filename);
  virtual void LoadCameraState (std::string filename);

  virtual void BuildViewer ();
  virtual void CleanViewer ();

  /*! Set the use of pthread in Renderer.
  \param state use or not use pthreads.
  */
  void SetPThread (bool state);
  /*! Return if the renderer are using pthreads or not.
  \return state of using or not pthreads.
  */
  bool IsUsingPThread ();
  
  /*! Set the evaluation method to be used.
  \param item index of the type of evaluation.
  */
  void SetCurrentMethod (int item);

  /*! Set the number of samples used in normal methods.
  \param samples the number of samples.
  */
  void SetSamples (int samples);
  /*! Return the number of samples.*/
  int GetSamples ();
  
  /*! Set the number of samples used in adaptive particioned methods.
  \param adap_samples the number of intervals in adaptive particioned methods.
  */
  void SetAdaptiveSamples (int adap_samples);
  /*! Return the number of samples in adaptive particioned methods.*/
  int GetAdaptiveSamples ();
  
  /*! Set the integral error bound of integral evaluations.*/
  void SetIntegralError (double error);
  /*! Return the integral error bound of integral evaluations.*/
  double GetIntegralError ();

  void SetInnerIntegralError (double error);
  double GetInnerIntegralError ();

  void SetIntervalDistance (double distance);
  double GetIntervalDistance ();
 
  /*! Return the minimum integral error bound value available by CPUInterface.*/
  double GetMinError ();
  /*! Return the maximum integral error bound value available by CPUInterface.*/
  double GetMaxError ();

  /*! Set Redisplay boolean value.*/
  virtual void SetRedisplay (bool vredisplay){ m_redisplay = vredisplay; }

  void Resize (int width, int height);

  int GetNumberOfThreads ();
  void SetNumberOfThreads (int nthreads);

  void SetUseAlphaTruncation (bool state);
  bool IsUsingAlphaTruncation ();

  VolumeEvaluator* GetVolumeEvaluator ();

private:
  void RecalculateAABBSizes ();
  void ErrorTests ();
  void HistogramGeneration ();
  err_type* RenderGroundTruth ();
  err_type* GetErrorFromVolEvalMethod (err_type* GroundTruth, std::string feedback_message, err_type* maxerror);
  void GetTimeFromVolEvalMethod (std::string feedback_message);
  void GenerateHistogramErrorControlled (std::string filename, err_type* p_errors);
  void GenerateExtrapolatedError (std::string filename, err_type* p_errors, float error);
  void RunMethod (std::string feedback_message);
  void TimeGeneration ();
  void HistogramGeneration_minmax ();
  void RiemannComparison ();
  err_type* RenderAndComparison (err_type* GroundTruth, std::string feedback_message, float* pixelcolor);

  void StepChangeSimpsonTest ();
  void MapErrorTests ();
  void GenerateErrorMap (char* filename, char* filename_alpha, err_type* gt_pixels, int width, int height);
  void RenderErrorTest (CPUTimer* timeclock, lqc::Vector4f* accumerror, lqc::Vector4f* maxerror);
  void GenerateHistogramError (std::string filename, err_type* p_errors);
  int raystraced;
  err_type maxerror[4];
  err_type hist_error_value_step[4];


  std::string m_volumename;
  std::string m_trasnferfunctionname;

  bool m_redisplay;
  bool m_show_redisplay_informations;

  int m_canvaswidth;
  int m_canvasheight;

  CPURenderer m_viewer;
  CPUUserInterface m_gui;
};

#endif