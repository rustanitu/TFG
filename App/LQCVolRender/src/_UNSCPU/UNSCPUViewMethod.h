/*!
\file CPUViewMethod.h
\brief CPU ViewMethod class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef UNSCPUVIEWMETHOD_H
#define UNSCPUVIEWMETHOD_H

#include "../ViewMethod/ViewMethod.h"

#include "UNSCPUUserInterface.h"
#include "UNSCPURenderer.h"

/*! Class associated with the CPU view method.
\ingroup views
*/
class UNSCPUViewMethod : public ViewMethod
{
public:
  /*! Constructor*/
  UNSCPUViewMethod ();
  /*! Destructor*/
  ~UNSCPUViewMethod ();

  virtual int Idle_Action_CB (Ihandle* cnv_renderer);
  virtual int Keyboard_CB (Ihandle *ih, int c, int press);
  virtual int Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status);
  virtual int Motion_CB (Ihandle *ih, int x, int y, char *status);
  virtual int Resize_CB (Ihandle *ih, int width, int height);
  virtual void CreateIupUserInterface ();
  virtual void UpdateIupUserInterface ();
  virtual void ResetCamera ();

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

  void SetUseFunctionEvaluation (bool usefunctionevaluation);
  void SetFunctionEvaluationIndex (int index);

  void SetUseAlphaTruncation (bool state);
  bool IsUsingAlphaTruncation ();

  VolumeEvaluator* GetVolumeEvaluator ();

private:
  void RecalculateAABBSizes ();

  std::string m_volumename;
  std::string m_trasnferfunctionname;

  bool m_redisplay;
  bool m_show_redisplay_informations;

  int m_canvaswidth;
  int m_canvasheight;

  UNSCPURenderer m_viewer;
  UNSCPUUserInterface m_gui;
};

#endif