#include "UserInterfaceCPU.h"

//#include <pthread.h>

#include <GL/glew.h>

#include <math/MGeometry.h>

#include "../Viewer.h"

#include "ViewMethodCPU.h"

int UserInterfaceCPU::ToggleCB_PThreadChange (Ihandle* ih, int state)
{
  ViewMethodCPU* cmethod = (ViewMethodCPU*)Viewer::Instance ()->m_viewmethods[VRVIEWS::CPU];
  cmethod->SetPThread(state == 1 ? true : false);
  
  return IUP_DEFAULT;
}

int UserInterfaceCPU::SpinCB_NumThreads (Ihandle* ih, int increment)
{
  ViewMethodCPU* cmethod = (ViewMethodCPU*)Viewer::Instance ()->m_viewmethods[VRVIEWS::CPU];

  if (increment == 1)
    cmethod->SetNumberOfThreads (cmethod->GetNumberOfThreads () + 2);
  else if (increment == -1)
    cmethod->SetNumberOfThreads (cmethod->GetNumberOfThreads () - 2);

  char s[20];
  sprintf (s, "%d", cmethod->GetNumberOfThreads ());
  IupSetAttribute (IupGetChild (ih, 1), "VALUE", s);

  return IUP_DEFAULT;
}

int UserInterfaceCPU::ListCB_RenderModes (Ihandle* ih, char *text, int item, int state)
{
  ViewMethodCPU* cmethod = (ViewMethodCPU*)Viewer::Instance ()->m_viewmethods[VRVIEWS::CPU];
  if (state == 1)
    cmethod->SetCurrentMethod (item - 1);
  return IUP_DEFAULT;
}

int UserInterfaceCPU::SpinCB_Samples (Ihandle* ih, int increment)
{
  ViewMethodCPU* cmethod = (ViewMethodCPU*)Viewer::Instance ()->m_viewmethods[VRVIEWS::CPU];

  if (increment == 1)
    cmethod->SetSamples (cmethod->GetSamples () * 2);
  else if (increment == -1)
    cmethod->SetSamples (cmethod->GetSamples () / 2);

  char s[20];
  sprintf (s, "%d", cmethod->GetSamples ());
  IupSetAttribute (IupGetChild (ih, 1), "VALUE", s);

  return IUP_DEFAULT;
}

int UserInterfaceCPU::SpinCB_IntegralSamples (Ihandle* ih, int increment)
{
  ViewMethodCPU* cmethod = (ViewMethodCPU*)Viewer::Instance ()->m_viewmethods[VRVIEWS::CPU];

  if (increment == 1)
    cmethod->SetAdaptiveSamples (cmethod->GetAdaptiveSamples () * 2);
  else if (increment == -1)
    cmethod->SetAdaptiveSamples (cmethod->GetAdaptiveSamples () / 2);

  char s[20];
  sprintf (s, "%d", cmethod->GetAdaptiveSamples ());
  IupSetAttribute (IupGetChild (ih, 1), "VALUE", s);

  return IUP_DEFAULT;
}

int UserInterfaceCPU::ValCB_button_press (Ihandle *c, double a)
{
  UserInterfaceCPU::ValCB_mousemove (c, a);
  return IUP_DEFAULT;
}

int UserInterfaceCPU::ValCB_button_release (Ihandle *c, double a)
{
  UserInterfaceCPU::ValCB_mousemove (c, a);
  return IUP_DEFAULT;
}

int UserInterfaceCPU::ValCB_mousemove (Ihandle *c, double a)
{
  ViewMethodCPU* cmethod = (ViewMethodCPU*)Viewer::Instance ()->m_viewmethods[VRVIEWS::CPU];
  cmethod->SetIntegralError (cmethod->GetMinError () + (cmethod->GetMaxError () - cmethod->GetMinError ())*a);
  
  return IUP_DEFAULT;
}

int UserInterfaceCPU::ToggleCB_AlphaTruncation (Ihandle* ih, int state)
{
  ViewMethodCPU* cmethod = (ViewMethodCPU*)Viewer::Instance ()->m_viewmethods[VRVIEWS::CPU];
  cmethod->SetUseAlphaTruncation (state == 1 ? true : false);

  return IUP_DEFAULT;
}


UserInterfaceCPU::UserInterfaceCPU ()
: m_iup_val_minerror (0.000001),
m_iup_val_maxerror (0.001)
{
}

UserInterfaceCPU::~UserInterfaceCPU ()
{
}

void UserInterfaceCPU::CreateIupInterface ()
{
  ViewMethodCPU* cmethod = (ViewMethodCPU*)Viewer::Instance ()->m_viewmethods[VRVIEWS::CPU];

  m_iup_toggle_ptrhread = IupToggle ("Pthread", NULL);
  IupSetAttribute (m_iup_toggle_ptrhread, "VALUE", cmethod->IsUsingPThread () ? "YES" : "NO");
  IupSetCallback (m_iup_toggle_ptrhread, "ACTION", (Icallback)UserInterfaceCPU::ToggleCB_PThreadChange);

  m_iup_label_nofthreads = IupLabel ("Threads: ");
  m_iup_spinbox_nofthreads = IupSpinbox (IupText (NULL));
  char st[20];
  sprintf (st, "%d", cmethod->GetNumberOfThreads ());
  IupSetAttribute (IupGetChild (m_iup_spinbox_nofthreads, 1), "VALUE", st);
  IupSetCallback (m_iup_spinbox_nofthreads, "SPIN_CB", (Icallback)UserInterfaceCPU::SpinCB_NumThreads);

  m_iup_hbox_nofthreads = IupHbox (
    m_iup_label_nofthreads,
    m_iup_spinbox_nofthreads,
    NULL);
  
  m_iup_text_redrawtime = IupText (NULL);
  IupSetAttribute (m_iup_text_redrawtime, "BORDER", "NO");
  IupSetAttribute (m_iup_text_redrawtime, "READONLY", "YES");
  m_iup_hbox_time = IupHbox (
    IupLabel ("Tempo de desenho: "),
    m_iup_text_redrawtime,
    NULL);

  m_iup_list_rendermodes = IupList (NULL);
  //Riemman Summation
  //IupSetAttribute (m_iup_list_rendermodes, "DROPDOWN", "YES");
  IupSetAttribute (m_iup_list_rendermodes,  "1" , "(S) Riemann Dir");
  IupSetAttribute (m_iup_list_rendermodes,  "2" , "(S) Riemann Dir exp");
  IupSetAttribute (m_iup_list_rendermodes,  "3" , "(S) Regra de Simpson");
  //IupSetAttribute (m_iup_list_rendermodes,  "4", "(A) Simpson Adaptativo");
  //IupSetAttribute (m_iup_list_rendermodes,  "5", "(A) Simpson Adaptativo Par");
  //IupSetAttribute (m_iup_list_rendermodes,  "6", "(A) Simpson Half Half");
  //IupSetAttribute (m_iup_list_rendermodes,  "7", "(A) Simpson Err");
  //IupSetAttribute (m_iup_list_rendermodes,  "8", "(A) Simpson Err Proj");
  //IupSetAttribute (m_iup_list_rendermodes,  "9", "(A) Simpson Err Half Proj");
  //IupSetAttribute (m_iup_list_rendermodes, "10", "(A) Simpson Half Queue");
  //IupSetAttribute (m_iup_list_rendermodes, "11", "(A) Simpson Half Queue Pj");
  //IupSetAttribute (m_iup_list_rendermodes, "12", "(A) Simpson Half Iteration");
  //IupSetAttribute (m_iup_list_rendermodes, "13", "(A) Simpson Err Half");
  //IupSetAttribute (m_iup_list_rendermodes, "14", "(A) Simpson Err Quadratic");
  //IupSetAttribute (m_iup_list_rendermodes, "15", "(A) Simpson Half Iter RiEr");
  IupSetAttribute (m_iup_list_rendermodes, "4" , "(A) Simpson HI S");
  IupSetAttribute (m_iup_list_rendermodes, "5" , "Adap. Simp. Coupled");
  IupSetAttribute (m_iup_list_rendermodes, "6" , "(A) Simpson HI Scount");
  IupSetAttribute (m_iup_list_rendermodes, "7" , "(A) Simpson HI C ExtStep");
  IupSetAttribute (m_iup_list_rendermodes, "8" , "(A) Simpson Half Iterate Exp");
  IupSetAttribute (m_iup_list_rendermodes, "9" , "(A) Simpson Half Iterate Sep");
  IupSetAttribute (m_iup_list_rendermodes, "10", "Adap. Simp. Decoupled");

  IupSetAttribute (m_iup_list_rendermodes, "VALUE", "1");
  IupSetCallback (m_iup_list_rendermodes, "ACTION", (Icallback)UserInterfaceCPU::ListCB_RenderModes);
  IupSetAttribute (m_iup_list_rendermodes, "EXPAND", "HORIZONTAL");

  m_iup_label_samples = IupLabel ("Amostras:");
  m_iup_spinbox_samples = IupSpinbox (IupText (NULL));
  char s[20];
  sprintf (s, "%d", cmethod->GetSamples ());
  IupSetAttribute (IupGetChild (m_iup_spinbox_samples, 1), "VALUE", s);
  IupSetCallback (m_iup_spinbox_samples, "SPIN_CB", (Icallback)UserInterfaceCPU::SpinCB_Samples);

  m_iup_label_adaptivesamples = IupLabel ("Amostras para o método adaptativo:");
  m_iup_spinbox_adaptivesamples = IupSpinbox (IupText (NULL));
  char c[20];
  sprintf (c, "%d", cmethod->GetAdaptiveSamples ());
  IupSetAttribute (IupGetChild (m_iup_spinbox_adaptivesamples, 1), "VALUE", c);
  IupSetCallback (m_iup_spinbox_adaptivesamples, "SPIN_CB", (Icallback)UserInterfaceCPU::SpinCB_IntegralSamples);
  
  m_iup_label_errorvalue = IupLabel ("Erro: ");
  m_iup_text_errorvalue = IupText (NULL);
  IupSetAttribute (m_iup_text_errorvalue, "BORDER", "NO");
  char errtval[128];
  sprintf (errtval, "%.10lf", cmethod->GetIntegralError ());
  IupSetAttribute (m_iup_text_errorvalue, "VALUE", errtval);
  IupSetAttribute (m_iup_text_errorvalue, "READONLY", "YES");
  m_iup_hbox_errorvalue = IupHbox (m_iup_label_errorvalue,
                                   m_iup_text_errorvalue, NULL);

  m_iup_val_errorvalue = IupVal (NULL);
  IupSetAttribute (m_iup_val_errorvalue, "EXPAND", "HORIZONTAL");
  IupSetAttribute (m_iup_val_errorvalue, "ORIENTATION", "HORIZONTAL");
  //IupSetAttribute (m_iup_val_errorvalue, "SHOWTICKS", "10");
  IupSetAttribute (m_iup_val_errorvalue, "TICKSPOS", "BOTH");
  IupSetAttribute (m_iup_val_errorvalue, "VALUE", "1");
  IupSetCallback (m_iup_val_errorvalue, "BUTTON_PRESS_CB", (Icallback)UserInterfaceCPU::ValCB_button_press);
  IupSetCallback (m_iup_val_errorvalue, "BUTTON_RELEASE_CB", (Icallback)UserInterfaceCPU::ValCB_button_release);
  IupSetCallback (m_iup_val_errorvalue, "MOUSEMOVE_CB", (Icallback)UserInterfaceCPU::ValCB_mousemove);
  m_iup_vbox_errorcontrol = IupVbox (m_iup_hbox_errorvalue,
                                     //m_iup_val_errorvalue,
                                     NULL);


  m_iup_label_statistics = IupLabel ("Estatísticas");

  m_iup_text_stattextar = IupText (NULL);
  IupSetAttribute (m_iup_text_stattextar, "BORDER", "NO");
  IupSetAttribute (m_iup_text_stattextar, "READONLY", "YES");
  IupSetAttribute (m_iup_text_stattextar, "EXPAND", "HORIZONTAL");

  m_iup_text_stattexttfr = IupText (NULL);
  IupSetAttribute (m_iup_text_stattexttfr, "BORDER", "NO");
  IupSetAttribute (m_iup_text_stattexttfr, "READONLY", "YES");

  m_iup_text_stattextc = IupText (NULL);
  IupSetAttribute (m_iup_text_stattextc, "BORDER", "NO");
  IupSetAttribute (m_iup_text_stattextc, "READONLY", "YES");

  char ar[128]; sprintf (ar, "%020d", 0);
  char ctf[128]; sprintf (ctf, "%020d", 0);
  char cf[128]; sprintf (cf, "%020d", 0);

  IupSetAttribute (m_iup_text_stattextar, "VALUE", ar);
  IupSetAttribute (m_iup_text_stattexttfr, "VALUE", ctf);
  IupSetAttribute (m_iup_text_stattextc, "VALUE", cf);
  
  m_iup_vbox_statistics = IupVbox (
    IupHbox (
    //IupLabel ("Amostras:"),
    m_iup_text_stattextar, 
    NULL),
    IupHbox (IupLabel ("Requisições ft:"), m_iup_text_stattexttfr, NULL),
    IupHbox (IupLabel ("Composições:"), m_iup_text_stattextc, NULL),
    NULL);

  m_iup_toggle_alphatruncation = IupToggle ("Alpha Truncation", NULL);
  IupSetAttribute (m_iup_toggle_alphatruncation, "VALUE", cmethod->IsUsingAlphaTruncation () ? "YES" : "NO");
  IupSetCallback (m_iup_toggle_alphatruncation, "ACTION", (Icallback)UserInterfaceCPU::ToggleCB_AlphaTruncation);

  cmethod->SetIupUserInterface (
    IupVbox (
    m_iup_toggle_ptrhread,
    m_iup_hbox_nofthreads,
    m_iup_hbox_time,
    IupLabel ("Método de Integração: "),
    m_iup_list_rendermodes,
    //m_iup_label_samples,
    //m_iup_spinbox_samples,
    //m_iup_label_adaptivesamples,
    //m_iup_spinbox_adaptivesamples,
    //m_iup_vbox_errorcontrol,
    //m_iup_label_statistics,
    //m_iup_vbox_statistics,
    //m_iup_toggle_alphatruncation,
    NULL));
}

void UserInterfaceCPU::SetUsingPThreads (bool usepthreads)
{
  IupSetAttribute (m_iup_toggle_ptrhread, "VALUE", usepthreads ? "YES" : "NO");
}

void UserInterfaceCPU::SetNumberOfThreads (int threads)
{
  char c[20];
  sprintf (c, "%d", threads);
  IupSetAttribute (IupGetChild (m_iup_spinbox_nofthreads, 1), "VALUE", c);
}

void UserInterfaceCPU::SetTimeElapsed (double time_elapsed)
{
  char te[128];
  sprintf (te, "%lf", time_elapsed);
  IupSetAttribute (m_iup_text_redrawtime, "VALUE", te);
}

void UserInterfaceCPU::SetStatistics (int volumeR, int transferfunctionR, int compositions)
{
  char ar[128];
  sprintf (ar, "%020d", volumeR);
  char ctf[128];
  sprintf (ctf, "%020d", transferfunctionR);
  char cf[128];
  sprintf (cf, "%020d", compositions);
  IupSetAttribute (m_iup_text_stattextar, "VALUE", ar);
  IupSetAttribute (m_iup_text_stattexttfr, "VALUE", ctf);
  IupSetAttribute (m_iup_text_stattextc, "VALUE", cf);
}

void UserInterfaceCPU::SetErrorValue (double error)
{
  char buffer[40];
  sprintf (buffer, "%.10lf", error);
  IupSetAttribute (m_iup_text_errorvalue, "VALUE", buffer);
}

void UserInterfaceCPU::SetSamples (int samples)
{
  char s[20];
  sprintf (s, "%d", samples);
  IupSetAttribute (IupGetChild (m_iup_spinbox_samples, 1), "VALUE", s);
}

void UserInterfaceCPU::SetAdaptiveSamples (int adaptive_samples)
{
  char c[20];
  sprintf (c, "%d", adaptive_samples);
  IupSetAttribute (IupGetChild (m_iup_spinbox_adaptivesamples, 1), "VALUE", c);
}

double UserInterfaceCPU::GetMaxError ()
{
  return m_iup_val_maxerror;
}

double UserInterfaceCPU::GetMinError ()
{
  return m_iup_val_minerror;
}