/*!
\file CPUUserInterface.h
\brief CPU ViewMethod GUI.
\author Leonardo Quatrin Campagnolo
*/

#ifndef CPUUSERINTERFACE_H
#define CPUUSERINTERFACE_H

#include <GL/glew.h>

#include <iup.h>
#include <iupgl.h>

#include <cstring>
#include <iostream>
#include <vector>


class UserInterfaceCPU
{
public:
  static int ToggleCB_PThreadChange (Ihandle* ih, int state);
  static int SpinCB_NumThreads (Ihandle* ih, int increment);
  static int ListCB_RenderModes (Ihandle* ih, char *text, int item, int state);
  static int SpinCB_Samples (Ihandle* ih, int increment);
  static int SpinCB_IntegralSamples (Ihandle* ih, int increment);
  static int ValCB_button_press (Ihandle *c, double a);
  static int ValCB_button_release (Ihandle *c, double a);
  static int ValCB_mousemove (Ihandle *c, double a);
  static int ToggleCB_AlphaTruncation (Ihandle* ih, int state);

public:
  /*! Constructor*/
  UserInterfaceCPU ();
  /*! Destructor*/
  ~UserInterfaceCPU ();

  /*! Generate all iup elements of the gui.*/
  void CreateIupInterface ();

  void SetUsingPThreads (bool usepthreads);
  void SetNumberOfThreads (int threads);

  /*! set iup_text_redrawtime*/
  void SetTimeElapsed (double time_elapsed);

  /*! set iup_text_stattextar, iup_text_stattexttfr, iup_text_stattextc*/
  void SetStatistics (int volumeR, int transferfunctionR, int compositions);
  /*! set iup_text_errorvalue*/
  void SetErrorValue (double error);
  /*! set iup_spinbox_samples*/
  void SetSamples (int samples);
  /*! set iup_spinbox_adaptivesamples*/
  void SetAdaptiveSamples (int adaptive_samples);

  /*! get the max error value available by the GUI (iup_val_errorvalue max bound).*/
  double GetMaxError ();
  /*! get the min error value stored by GUI (iup_val_errorvalue min bound).*/
  double GetMinError ();

private:
  
  Ihandle* m_iup_toggle_ptrhread;
  Ihandle* m_iup_label_nofthreads;
  Ihandle* m_iup_spinbox_nofthreads;
  Ihandle* m_iup_hbox_nofthreads;
  Ihandle* m_iup_text_redrawtime;
  Ihandle* m_iup_hbox_time;
  Ihandle* m_iup_list_rendermodes;
  Ihandle* m_iup_label_samples;
  Ihandle* m_iup_spinbox_samples;
  Ihandle* m_iup_label_adaptivesamples;
  Ihandle* m_iup_spinbox_adaptivesamples;
  Ihandle* m_iup_label_errorvalue;
  Ihandle* m_iup_text_errorvalue;
  Ihandle* m_iup_hbox_errorvalue;
  Ihandle* m_iup_val_errorvalue;
  Ihandle* m_iup_vbox_errorcontrol;
  Ihandle* m_iup_label_statistics;
  Ihandle* m_iup_text_stattextar;
  Ihandle* m_iup_text_stattexttfr;
  Ihandle* m_iup_text_stattextc;
  Ihandle* m_iup_vbox_statistics;
  Ihandle* m_iup_toggle_alphatruncation;

  double m_iup_val_maxerror;
  double m_iup_val_minerror;
};


#endif