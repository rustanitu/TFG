/*!
\file ViewerInterface.h
\brief Main interface class of the volume viewer.
\author Leonardo Quatrin Campagnolo
*/

#ifndef TRANSFERFUCNTIONVIEWER_H
#define TRANSFERFUCNTIONVIEWER_H

#include <volrend/TransferFunction.h>

#include <iup.h>
#include <iupgl.h>
#include <iupcontrols.h>

class TransferFunctionsViewer
{
public:
  static int Action (Ihandle *ih, float posx, float posy);
  static int Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status);
  static int Motion_CB (Ihandle *ih, int x, int y, char *status);

public:
  static TransferFunctionsViewer* Instance ();
  static bool Exists ();
  static void DestroyInstance ();

  
  void Show ();
  void BuildInterface ();

  void Redraw ();

protected:
  vr::TransferFunction* m_transfer_function;

private:
  float* m_pixels;
  int id;
  bool doubler;

  Ihandle* m_iup_sub_menu_file;
  Ihandle* m_iup_menu;
  
  Ihandle* m_iup_canvas;
  
  Ihandle* m_iup_main_dialog;
protected:
  TransferFunctionsViewer ();
  ~TransferFunctionsViewer ();

private:
  /*! the pointer to the singleton instance.*/
  static TransferFunctionsViewer* m_instance;
};

#endif