#include "TransferFunctionsViewer.h"

#include "../Viewer.h"
#include <volrend/TransferFunction.h>

#include <iup.h>
#include <iupcontrols.h>

#include <cstdio>
#include <cstdlib>

TransferFunctionsViewer *TransferFunctionsViewer::m_instance = 0;

///////////////
// CallBacks //
///////////////

int TransferFunctionsViewer::Action (Ihandle *ih, float posx, float posy)
{
  return IUP_DEFAULT;
}

int TransferFunctionsViewer::Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  TransferFunctionsViewer::Instance ()->Redraw ();
  return IUP_DEFAULT;
}

int TransferFunctionsViewer::Motion_CB (Ihandle *ih, int x, int y, char *status)
{
  return IUP_DEFAULT;
}

////////////////////
// Public Methods //
////////////////////

TransferFunctionsViewer* TransferFunctionsViewer::Instance ()
{
  if (!m_instance)
    m_instance = new TransferFunctionsViewer ();

  return m_instance;
}

bool TransferFunctionsViewer::Exists ()
{
  return (m_instance != NULL);
}

void TransferFunctionsViewer::DestroyInstance ()
{
  if (!TransferFunctionsViewer::Exists ())
    return;

  if (m_instance)
  {
    delete m_instance;
    m_instance = NULL;
  }
}

void TransferFunctionsViewer::Show ()
{
  IupShowXY (m_iup_main_dialog, IUP_CENTER, IUP_CENTER);
}

void TransferFunctionsViewer::BuildInterface ()
{
  m_iup_canvas = IupGLCanvas ("tfviewer_canvas");
  IupSetCallback (m_iup_canvas, "ACTION", (Icallback)TransferFunctionsViewer::Action);
  IupSetCallback (m_iup_canvas, "BUTTON_CB", (Icallback)TransferFunctionsViewer::Button_CB);
  IupSetCallback (m_iup_canvas, "MOTION_CB", (Icallback)TransferFunctionsViewer::Motion_CB);
  IupSetAttribute (m_iup_canvas, IUP_BUFFER, IUP_DOUBLE);
  IupSetAttribute (m_iup_canvas, IUP_RASTERSIZE, "258x50");
  IupSetAttribute (m_iup_canvas, IUP_RESIZE, IUP_NO);
  IupSetHandle ("tfviewer_canvas", m_iup_canvas);

  m_iup_sub_menu_file = IupSubmenu ("Arquivo", IupMenu(NULL));

  m_iup_menu = IupMenu (m_iup_sub_menu_file, NULL);
  IupSetHandle ("TransferFunctionMenu", m_iup_menu);

  m_iup_main_dialog = IupDialog (m_iup_canvas);
  //IupSetAttribute (m_iup_main_dialog, "MENU", "TransferFunctionMenu");
  IupSetAttribute (m_iup_main_dialog, "TITLE", "Transfer Function Visualization");
  IupSetAttribute (m_iup_main_dialog, "BORDER", "NO");
  IupSetAttribute (m_iup_main_dialog, "RESIZE", "NO");
  
  IupMap (m_iup_main_dialog);
  IupRefresh (m_iup_main_dialog);

  if (!m_pixels)
    m_pixels = new float[258 * 50 * 4];

  Redraw ();
}

void TransferFunctionsViewer::Redraw ()
{
  IupGLMakeCurrent (m_iup_canvas);
  
  vr::TransferFunction *tf = Viewer::Instance ()->m_transfer_function;
  if (tf)
  {
    for (int i = 0; i < 256; i++)
    {
      lqc::Vector4d c = tf->Get (i);
      for (int j = 0; j < 50; j++)
      {
        if (id < 2)
        {
          m_pixels[i * 4 + j * 258 * 4] = c.x;
          m_pixels[i * 4 + j * 258 * 4 + 1] = c.y;
          m_pixels[i * 4 + j * 258 * 4 + 2] = c.z;
          m_pixels[i * 4 + j * 258 * 4 + 3] = 1.0;
        }
        else
        {
          m_pixels[i * 4 + j * 258 * 4] = c.w;
          m_pixels[i * 4 + j * 258 * 4 + 1] = c.w;
          m_pixels[i * 4 + j * 258 * 4 + 2] = c.w;
          m_pixels[i * 4 + j * 258 * 4 + 3] = 1.0;
        }
      }
    }
  }
  
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawPixels (258, 50, GL_RGBA, GL_FLOAT, m_pixels);
  
  id = (id + 1) % 4;


  IupGLSwapBuffers (m_iup_canvas);
}

TransferFunctionsViewer::TransferFunctionsViewer ()
{
  m_pixels = NULL;
  id = 0;
  doubler = false;
}

TransferFunctionsViewer::~TransferFunctionsViewer ()
{
  if (m_pixels)
    delete m_pixels;
    m_pixels = NULL;
}