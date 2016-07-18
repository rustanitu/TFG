/*!
\file ViewerInterface.h
\brief Main interface class of the volume viewer.
\author Leonardo Quatrin Campagnolo
*/

#ifndef VIEWERINTERFACE_H
#define VIEWERINTERFACE_H

#include "AutomaticTransferFunction/ATFInterface.h"

#include <iup.h>
#include <iupgl.h>

class TransferFunctionsViewer;

class ViewerInterface
{
public:
  static int Keyboard_CB (Ihandle *ih, int c, int press);
  static int Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status);
  static int Motion_CB (Ihandle *ih, int x, int y, char *status);
  static int Renderer_CB (Ihandle* handle);
  static int Resize_CB (Ihandle *ih, int width, int height);
  
  static int Close_CB (Ihandle *ih);

  static int CPUVisualizationChange_CB (Ihandle* ih, char *text, int item, int state);
  //static int GLSLVisualizationChange_CB (Ihandle* ih, char *text, int item, int state);

  static int GLSL2PVisualizationChange_CB (Ihandle* ih, char *text, int item, int state);
  static int IASVisualizationChange_CB (Ihandle* ih, char *text, int item, int state);
  //static int UNSCPUVisualizationChange_CB (Ihandle* ih, char *text, int item, int state);

  static int ERN2DVisualizationChange_CB (Ihandle* ih, char *text, int item, int state);
  static int ERN3DVisualizationChange_CB (Ihandle* ih, char *text, int item, int state);

  static int RiemannGLSLVisualizationChange_CB (Ihandle* ih, char *text, int item, int state);
  static int SimpsonGLSLVisualizationChange_CB (Ihandle* ih, char *text, int item, int state);

  static int ClearColor_White_CB (Ihandle* ih);
  static int ClearColor_Back_CB (Ihandle* ih);
  static int ClearColor_Blue_CB (Ihandle* ih);

  static int ButtonCB_ReloadTF (Ihandle* ih);
  static int ButtonCB_Redisplay (Ihandle* ih);
  static int ButtonCB_ResetRotation (Ihandle* ih);
  static int btn_OpenVol_CB (Ihandle* ih);
  static int btn_ViewVol_CB (Ihandle* ih);
  static int btn_OpenTF_CB (Ihandle* ih);

  static int btn_SaveState_cb (Ihandle* ih);
  static int btn_LoadState_cb (Ihandle* ih);

  static int btn_SaveCamState_cb (Ihandle* ih);
  static int btn_LoadCamState_cb (Ihandle* ih);

  static int BlendOption_GL_SRC_ALPHA_GL_ONE_MINUS_SRC_ALPHA (Ihandle* ih);
  static int BlendOption_GL_ONE_GL_ZERO (Ihandle* ih);
  static int BlendOption_GL_ONE_MINUS_SRC_ALPHA_GL_SRC_ALPHA (Ihandle* ih);
  static int BlendOption_GL_SRC_ALPHA_GL_SRC_COLOR (Ihandle* ih);
  static int BlendOption_GL_ZERO_GL_ONE (Ihandle* ih);
  static int BlendOption_GL_ZERO_GL_ZERO (Ihandle* ih);
  static int BlendOption_GL_ONE_GL_ONE (Ihandle* ih);
  static int BlendOption_GL_ONE_MINUS_DST_ALPHA_GL_DST_ALPHA (Ihandle* ih);
  static int BlendOption_GL_ONE_MINUS_DST_COLOR_GL_ONE_MINUS_SRC_COLOR (Ihandle* ih);
  static int BlendOption_GL_DST_ALPHA_GL_ONE_MINUS_DST_ALPHA (Ihandle* ih);
  static int BlendOption_GL_DST_COLOR_GL_SRC_COLOR (Ihandle* ih);
  static int BlendOption_GL_DST_COLOR_GL_SRC_ALPHA (Ihandle* ih);

  static int tgl_SetAutomaticTransferFunction_CB (Ihandle* ih);

public:
  ViewerInterface ();
  ~ViewerInterface ();
  
  void BuildInterface (int argc, char *argv[]);

  Ihandle* GetCanvasRenderer ();
  void UpdateAdInterface ();

  void ShowTransferFunctionViewer ();

  void SetViewer(Viewer* atg);

  Ihandle* m_iup_main_dialog;

  Ihandle* m_iup_hbox_dialog;

  Ihandle* m_iup_canvas_renderer;

  Ihandle* m_iup_frame_adinterface;

  Ihandle* m_iup_vbox_GUI;
  Ihandle* m_iup_vbox_commoninterface;
  Ihandle* m_iup_vbox_atf;

  Ihandle* m_iup_menu;

  Ihandle* m_iup_menu_file;
  Ihandle* m_iup_sub_menu_file;
  Ihandle* m_iup_menu_file_load_volmodel;
  Ihandle* m_iup_menu_file_load_TransFunct;
  Ihandle* m_iup_menu_file_reload_TransFunct;
  Ihandle* m_iup_menu_file_view_TransFunct;
  Ihandle* m_iup_menu_file_savestate;
  Ihandle* m_iup_menu_file_loadstate;

  Ihandle* m_iup_menu_file_savecampos;
  Ihandle* m_iup_menu_file_loadcampos;

  Ihandle* m_iup_menu_file_item_exit;

  Ihandle* m_iup_menu_visualizations;
  Ihandle* m_iup_sub_menu_visualizations;
  Ihandle* m_iup_item_vis_cpu;
  //Ihandle* m_iup_item_vis_glsl;
  Ihandle* m_iup_item_vis_glsl2p;
  Ihandle* m_iup_item_vis_ias;
  //Ihandle* m_iup_item_vis_unscpu;
  Ihandle* m_iup_item_vis_ern2D;
  Ihandle* m_iup_item_vis_ern3D;

  Ihandle* m_iup_item_vis_glslriemann;
  Ihandle* m_iup_item_vis_glslsimpson;

  Ihandle* m_iup_menu_clearcolors;
  Ihandle* m_iup_submenu_clearcolors;
  Ihandle* m_iup_item_clcol_white;
  Ihandle* m_iup_item_clcol_black;
  Ihandle* m_iup_item_clcol_blue;

  Ihandle* m_iup_menu_glblendoptions;
  Ihandle* m_iup_submenu_glblendoptions;
  
  Ihandle* m_iup_item_glblend[12];
  //http://wiki.delphigl.com/index.php/glBlendFunc
  //glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc (GL_ONE, GL_ZERO);
  //glBlendFunc (GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
  //glBlendFunc (GL_SRC_ALPHA, GL_SRC_COLOR);
  //glBlendFunc (GL_ZERO, GL_ONE);
  //glBlendFunc (GL_ZERO, GL_ZERO);
  //glBlendFunc (GL_ONE, GL_ONE);
  //glBlendFunc (GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
  //glBlendFunc (GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
  //glBlendFunc (GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
  //glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
  //glBlendFunc (GL_DST_COLOR, GL_SRC_ALPHA);

protected:
private:

  ATFInterface* m_atfg_interface;

};

#endif