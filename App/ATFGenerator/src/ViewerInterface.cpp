#include "ViewerInterface.h"
#include "Viewer.h"

#include "TransferFunctionViewer/TransferFunctionsViewer.h"

#include <lqc/lqcdefines.h>

#include <cstdlib>
#include <string>
#include <vector>

//////////////////////
// Public Callbacks //
//////////////////////

int ViewerInterface::Keyboard_CB (Ihandle *ih, int c, int press)
{
	return Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->Keyboard_CB (ih, c, press);
}

int ViewerInterface::Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
	return Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->Button_CB (ih, button, pressed, x, y, status);
}

int ViewerInterface::Motion_CB (Ihandle *ih, int x, int y, char *status)
{
	return Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->Motion_CB (ih, x, y, status);
}

int ViewerInterface::Renderer_CB (Ihandle* handle)
{
	return Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->Idle_Action_CB (Viewer::Instance ()->GetCanvasRenderer ());
}

int ViewerInterface::Resize_CB (Ihandle *ih, int width, int height)
{
	Viewer::Instance ()->SetCanvasSizes (width, height);
	return Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->Resize_CB (ih, width, height);
}

int ViewerInterface::Close_CB (Ihandle* ih)
{
	return IUP_CLOSE;
}

int ViewerInterface::CPUVisualizationChange_CB (Ihandle* ih, char *text, int item, int state)
{
	Viewer::Instance ()->m_current_view = VRVIEWS::CPU;
	Viewer::Instance ()->DisactiveInterfaces ();
	Viewer::Instance ()->CleanViewers ();

	Viewer::Instance ()->m_viewmethods[VRVIEWS::CPU]->BuildViewer ();
	Viewer::Instance ()->m_viewmethods[VRVIEWS::CPU]->ActiveInterface ();
	
	Viewer::Instance ()->UpdateAdInterface ();

	return IUP_DEFAULT;
}

//int ViewerInterface::GLSLVisualizationChange_CB (Ihandle* ih, char *text, int item, int state)
//{
//  Viewer::Instance ()->m_current_view = VRVIEWS::GLSL;
//  Viewer::Instance ()->DisactiveInterfaces ();
//  Viewer::Instance ()->CleanViewers ();
//  
//  Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL]->BuildViewer ();
//  Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL]->ActiveInterface ();
//
//  Viewer::Instance ()->UpdateAdInterface ();
//
//  return IUP_DEFAULT;
//}

int ViewerInterface::ERN2DVisualizationChange_CB (Ihandle* ih, char *text, int item, int state)
{
	Viewer::Instance ()->m_current_view = VRVIEWS::ERN2D;
	Viewer::Instance ()->DisactiveInterfaces ();
	Viewer::Instance ()->CleanViewers ();

	Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D]->BuildViewer ();
	Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D]->ActiveInterface ();

	Viewer::Instance ()->UpdateAdInterface ();

	return IUP_DEFAULT;
}

int ViewerInterface::ERN3DVisualizationChange_CB (Ihandle* ih, char *text, int item, int state)
{
	Viewer::Instance ()->m_current_view = VRVIEWS::ERN;
	Viewer::Instance ()->DisactiveInterfaces ();
	Viewer::Instance ()->CleanViewers ();

	Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN]->BuildViewer ();
	Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN]->ActiveInterface ();

	Viewer::Instance ()->UpdateAdInterface ();

	return IUP_DEFAULT;
}

int ViewerInterface::GLSL2PVisualizationChange_CB (Ihandle* ih, char *text, int item, int state)
{
	Viewer::Instance ()->m_current_view = VRVIEWS::GLSL2P;
	Viewer::Instance ()->DisactiveInterfaces ();
	Viewer::Instance ()->CleanViewers ();

	Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P]->BuildViewer ();
	Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P]->ActiveInterface ();

	Viewer::Instance ()->UpdateAdInterface ();

	return IUP_DEFAULT;
}

int ViewerInterface::IASVisualizationChange_CB (Ihandle* ih, char *text, int item, int state)
{
	Viewer::Instance ()->m_current_view = VRVIEWS::IAS;
	Viewer::Instance ()->DisactiveInterfaces ();
	Viewer::Instance ()->CleanViewers ();

	Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS]->BuildViewer ();
	Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS]->ActiveInterface ();

	Viewer::Instance ()->UpdateAdInterface ();

	return IUP_DEFAULT;
}

//int ViewerInterface::UNSCPUVisualizationChange_CB (Ihandle* ih, char *text, int item, int state)
//{
//  Viewer::Instance ()->m_current_view = VRVIEWS::UNSCPU;
//  Viewer::Instance ()->DisactiveInterfaces ();
//  Viewer::Instance ()->CleanViewers ();
//
//  Viewer::Instance ()->m_viewmethods[VRVIEWS::UNSCPU]->BuildViewer ();
//  Viewer::Instance ()->m_viewmethods[VRVIEWS::UNSCPU]->ActiveInterface ();
//
//  Viewer::Instance ()->UpdateAdInterface ();
//
//  return IUP_DEFAULT;
//}

int ViewerInterface::RiemannGLSLVisualizationChange_CB (Ihandle* ih, char *text, int item, int state)
{
	Viewer::Instance ()->m_current_view = VRVIEWS::EQUIDISTANT_GLSL;
	Viewer::Instance ()->DisactiveInterfaces ();
	Viewer::Instance ()->CleanViewers ();

	Viewer::Instance ()->m_viewmethods[VRVIEWS::EQUIDISTANT_GLSL]->BuildViewer ();
	Viewer::Instance ()->m_viewmethods[VRVIEWS::EQUIDISTANT_GLSL]->ActiveInterface ();

	Viewer::Instance ()->UpdateAdInterface ();

	return IUP_DEFAULT;
}

int ViewerInterface::SimpsonGLSLVisualizationChange_CB (Ihandle* ih, char *text, int item, int state)
{
	Viewer::Instance ()->m_current_view = VRVIEWS::ADAPTIVE_GLSL;
	Viewer::Instance ()->DisactiveInterfaces ();
	Viewer::Instance ()->CleanViewers ();

	Viewer::Instance ()->m_viewmethods[VRVIEWS::ADAPTIVE_GLSL]->BuildViewer ();
	Viewer::Instance ()->m_viewmethods[VRVIEWS::ADAPTIVE_GLSL]->ActiveInterface ();

	Viewer::Instance ()->UpdateAdInterface ();

	return IUP_DEFAULT;
}

int ViewerInterface::ClearColor_White_CB (Ihandle* ih)
{
	Viewer::Instance ()->SetClearColor (1.0f, 1.0f, 1.0f, 0.0f);
	return IUP_DEFAULT;
}

int ViewerInterface::ClearColor_Back_CB (Ihandle* ih)
{
	Viewer::Instance ()->SetClearColor (0.0f, 0.0f, 0.0f, 0.0f);
	return IUP_DEFAULT;
}

int ViewerInterface::ClearColor_Blue_CB (Ihandle* ih)
{
	Viewer::Instance ()->SetClearColor (0.0f, 0.0f, 1.0f, 0.0f);
	return IUP_DEFAULT;
}

int ViewerInterface::ButtonCB_ReloadTF (Ihandle* ih)
{
	Viewer::Instance ()->ReloadTransferFunction ();
	Viewer::Instance ()->MarkAllViewMethodsOutdated ();
	return IUP_DEFAULT;
}

int ViewerInterface::ButtonCB_Redisplay (Ihandle* ih)
{
	Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->SetRedisplay (true);
	return IUP_DEFAULT;
}

int ViewerInterface::ButtonCB_ResetRotation (Ihandle* ih)
{
	Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->ResetCamera ();
	return IUP_DEFAULT;
}

int ViewerInterface::btn_OpenVol_CB (Ihandle* ih)
{
	if (Viewer::Instance ()->FileDlg_VolumeModel ())
		Viewer::Instance ()->MarkAllViewMethodsOutdated ();
	return IUP_DEFAULT;
}

int ViewerInterface::btn_ViewVol_CB (Ihandle* ih)
{
	Viewer::Instance ()->m_gui.ShowTransferFunctionViewer ();
	return IUP_DEFAULT;
}

int ViewerInterface::btn_OpenTF_CB (Ihandle* ih)
{
	if (Viewer::Instance ()->FileDlg_TransferFunction ())
		Viewer::Instance ()->MarkAllViewMethodsOutdated ();
	return IUP_DEFAULT;
}

int ViewerInterface::btn_SaveState_cb (Ihandle* ih)
{
	Viewer::Instance ()->SaveViewerState ();
	return IUP_DEFAULT;
}

int ViewerInterface::btn_LoadState_cb (Ihandle* ih)
{
	Viewer::Instance ()->LoadViewerState ();
	Viewer::Instance ()->MarkAllViewMethodsOutdated ();
	return IUP_DEFAULT;
}

int ViewerInterface::btn_SaveCamState_cb (Ihandle* ih)
{
	printf ("Function Called: btn_SaveCamState_cb\n");

	Ihandle *dlg = IupFileDlg ();

	IupSetAttribute (dlg, "DIALOGTYPE", "SAVE");
	IupSetAttribute (dlg, "TITLE", "Save Camera State");
	IupSetAttributes (dlg, "FILTER = \"*.vrcam\", FILTERINFO = \"Camera states files (without the extension)\"");

	IupPopup (dlg, IUP_CURRENT, IUP_CURRENT);

	if (IupGetInt (dlg, "STATUS") != -1)
	{
		std::string file (IupGetAttribute (dlg, "VALUE"));
		std::cout << file << std::endl;
		
		Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->SaveCameraState (file);
	}
	IupDestroy (dlg);
	return IUP_DEFAULT;
}

int ViewerInterface::btn_LoadCamState_cb (Ihandle* ih)
{
	printf ("Function Called: btn_LoadCamState_cb\n");

	Ihandle *dlg = IupFileDlg ();

	IupSetAttribute (dlg, "DIALOGTYPE", "OPEN");
	IupSetAttribute (dlg, "TITLE", "Load Camera State");
	IupSetAttributes (dlg, "FILTER = \"*.vrcam\", FILTERINFO = \"Camera states files [.vrcam]\"");

	IupPopup (dlg, IUP_CURRENT, IUP_CURRENT);

	if (IupGetInt (dlg, "STATUS") != -1)
	{
		std::string file (IupGetAttribute (dlg, "VALUE"));
		std::cout << file << std::endl;

		Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->LoadCameraState (file);
	}
	IupDestroy (dlg);
	return IUP_DEFAULT;
}

int ViewerInterface::BlendOption_GL_SRC_ALPHA_GL_ONE_MINUS_SRC_ALPHA (Ihandle* ih)
{
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	return IUP_DEFAULT;
}

int ViewerInterface::BlendOption_GL_ONE_GL_ZERO (Ihandle* ih)
{
	glBlendFunc (GL_ONE, GL_ZERO);
	return IUP_DEFAULT;
}

int ViewerInterface::BlendOption_GL_ONE_MINUS_SRC_ALPHA_GL_SRC_ALPHA (Ihandle* ih)
{
	glBlendFunc (GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	return IUP_DEFAULT;
}

int ViewerInterface::BlendOption_GL_SRC_ALPHA_GL_SRC_COLOR (Ihandle* ih)
{
	glBlendFunc (GL_SRC_ALPHA, GL_SRC_COLOR);
	return IUP_DEFAULT;
}

int ViewerInterface::BlendOption_GL_ZERO_GL_ONE (Ihandle* ih)
{
	glBlendFunc (GL_ZERO, GL_ONE);
	return IUP_DEFAULT;
}

int ViewerInterface::BlendOption_GL_ZERO_GL_ZERO (Ihandle* ih)
{
	glBlendFunc (GL_ZERO, GL_ZERO);
	return IUP_DEFAULT;
}

int ViewerInterface::BlendOption_GL_ONE_GL_ONE (Ihandle* ih)
{
	glBlendFunc (GL_ONE, GL_ONE);
	return IUP_DEFAULT;
}

int ViewerInterface::BlendOption_GL_ONE_MINUS_DST_ALPHA_GL_DST_ALPHA (Ihandle* ih)
{
	glBlendFunc (GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
	return IUP_DEFAULT;
}

int ViewerInterface::BlendOption_GL_ONE_MINUS_DST_COLOR_GL_ONE_MINUS_SRC_COLOR (Ihandle* ih)
{
	glBlendFunc (GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
	return IUP_DEFAULT;
}

int ViewerInterface::BlendOption_GL_DST_ALPHA_GL_ONE_MINUS_DST_ALPHA (Ihandle* ih)
{
	glBlendFunc (GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	return IUP_DEFAULT;
}

int ViewerInterface::BlendOption_GL_DST_COLOR_GL_SRC_COLOR (Ihandle* ih)
{
	glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
	return IUP_DEFAULT;
}

int ViewerInterface::BlendOption_GL_DST_COLOR_GL_SRC_ALPHA (Ihandle* ih)
{
	glBlendFunc (GL_DST_COLOR, GL_SRC_ALPHA);
	return IUP_DEFAULT;
}

int ViewerInterface::tgl_SetAutomaticTransferFunction_CB(Ihandle* ih)
{
	return 0;
}

////////////////////
// Public Methods //
////////////////////

ViewerInterface::ViewerInterface ()
:m_iup_vbox_atf(NULL)
{
}

ViewerInterface::~ViewerInterface ()
{
}

void ViewerInterface::BuildInterface (int argc, char *argv[])
{
	m_iup_canvas_renderer = IupGLCanvas ("renderCanvas");
	IupSetCallback (m_iup_canvas_renderer, "KEYPRESS_CB", (Icallback)ViewerInterface::Keyboard_CB);
	IupSetCallback (m_iup_canvas_renderer, "BUTTON_CB", (Icallback)ViewerInterface::Button_CB);
	IupSetCallback (m_iup_canvas_renderer, "MOTION_CB", (Icallback)ViewerInterface::Motion_CB);
	IupSetCallback (m_iup_canvas_renderer, "RESIZE_CB", (Icallback)ViewerInterface::Resize_CB);
	//IupSetCallback (m_iup_canvas_renderer, "ACTION", (Icallback)ViewerInterface::Renderer_CB);
	IupSetAttribute (m_iup_canvas_renderer, IUP_RASTERSIZE, "600x400");
	IupSetAttribute (m_iup_canvas_renderer, IUP_RESIZE, IUP_YES);
	IupSetAttribute (m_iup_canvas_renderer, IUP_BUFFER, IUP_DOUBLE);
	IupSetAttribute (m_iup_canvas_renderer, IUP_TITLE, "renderCanvas");
	IupSetFunction (IUP_IDLE_ACTION, (Icallback)ViewerInterface::Renderer_CB);
	IupSetHandle ("renderCanvas", m_iup_canvas_renderer);

	Ihandle* common_int_label = IupLabel ("Interface comum");
	IupSetAttribute (common_int_label, "EXPAND", "HORIZONTAL");
	IupSetAttribute (common_int_label, "ALIGNMENT", "ACENTER");

	Ihandle* buttonOpenVol = IupButton ("Carregar modelo", NULL);
	IupSetAttribute (buttonOpenVol, "EXPAND", "HORIZONTAL");
	IupSetCallback (buttonOpenVol, "ACTION", (Icallback)ViewerInterface::btn_OpenVol_CB);

	Ihandle* buttonTransferFunction = IupButton ("Carregar função de transferência", NULL);
	IupSetAttribute (buttonTransferFunction, "EXPAND", "HORIZONTAL");
	IupSetCallback (buttonTransferFunction, "ACTION", (Icallback)ViewerInterface::btn_OpenTF_CB);

	Ihandle* buttonCameraState = IupButton ("Carregar estado de câmera", NULL);
	IupSetAttribute (buttonCameraState, "EXPAND", "HORIZONTAL");
	IupSetCallback (buttonCameraState, "ACTION", (Icallback)ViewerInterface::btn_LoadCamState_cb);

	m_iup_vbox_commoninterface = IupVbox (
		buttonOpenVol,
		buttonTransferFunction,
		buttonCameraState,
		NULL);
	Viewer::Instance ()->CreateAllViewMethodIupInterfaces ();

	Ihandle* vbox_sep = IupVbox (NULL);
	IupSetAttribute (vbox_sep, "RASTERSIZE", "0x10");

	Ihandle* sgima_bar = IupVal("VERTICAL");
	IupSetAttribute(sgima_bar, "ACTIVE", "YES");
	IupSetAttribute(sgima_bar, "SHOWTICKS", "48");
	IupSetAttribute(sgima_bar, "MAX", "50");
	IupSetAttribute(sgima_bar, "MIN", "1");
	IupSetAttribute(sgima_bar, "VALUE", "1");
	IupSetAttribute(sgima_bar, "EXPAND", "HORIZONTAL");
	IupSetAttribute(sgima_bar, "ALIGNMENT", "ACENTER");
	IupSetAttribute(sgima_bar, "RASTERSIZE", "0x200");
	IupSetCallback(sgima_bar, "VALUECHANGED_CB", (Icallback)Viewer::SetBoundaryThickness);
	IupSetCallback(sgima_bar, "LEAVEWINDOW_CB", (Icallback)Viewer::MarkOutdated);

	m_gtresh_bar = IupVal("VERTICAL");
	IupSetAttribute(m_gtresh_bar, "ACTIVE", "YES");
	IupSetAttribute(m_gtresh_bar, "SHOWTICKS", "101");
	IupSetAttribute(m_gtresh_bar, "MAX", "100");
	IupSetAttribute(m_gtresh_bar, "VALUE", "0.0");
	IupSetAttribute(m_gtresh_bar, "EXPAND", "HORIZONTAL");
	IupSetAttribute(m_gtresh_bar, "ALIGNMENT", "ACENTER");
	IupSetAttribute(m_gtresh_bar, "RASTERSIZE", "0x200");
	IupSetCallback(m_gtresh_bar, "VALUECHANGED_CB", (Icallback)Viewer::SetGTresh);
	IupSetCallback(m_gtresh_bar, "LEAVEWINDOW_CB", (Icallback)Viewer::MarkOutdated);

	Ihandle* atfg_label = IupLabel("BThick    GTresh");
	Ihandle* atfg_boundary_label = IupLabel("Boundary");
	IupSetAttribute(atfg_boundary_label, "ALIGNMENT", "ACENTER");

	//Ihandle* spinbox_boundary = IupText("0");
	//IupSetAttribute(spinbox_boundary, "ALIGNMENT", "ACENTER");
	//IupSetAttribute(spinbox_boundary, "SPIN", "YES");
	//IupSetCallback(spinbox_boundary, "SPIN_CB", (Icallback)Viewer::SetBoundary);

	Ihandle* atfg_set_label = IupLabel("Set");
	IupSetAttribute(atfg_set_label, "ALIGNMENT", "ACENTER");

	Ihandle* spinbox_set = IupText("0");
	IupSetAttribute(spinbox_set, "ALIGNMENT", "ACENTER");
	IupSetAttribute(spinbox_set, "SPIN", "YES");
	IupSetAttribute(spinbox_set, "SPINMAX", "999999999");
	IupSetCallback(spinbox_set, "SPIN_CB", (Icallback)Viewer::SetVisibleSet);

	m_bthick_label = IupLabel("BThick: 1   ");
	m_gtresh_label = IupLabel("GTresh: 0.0f");

	//Ihandle* vbox_atfg = IupVbox(atfg_boundary_label, spinbox_boundary, atfg_set_label, spinbox_set, m_bthick_label, m_gtresh_label, NULL);
	Ihandle* vbox_atfg = IupVbox(atfg_boundary_label, atfg_set_label, spinbox_set, m_bthick_label, m_gtresh_label, NULL);
	Ihandle* hbox_atfg = IupHbox(sgima_bar, m_gtresh_bar, vbox_atfg, NULL);

	Ihandle* selected_int_label = IupLabel ("Interface do método ativo");
	IupSetAttribute (selected_int_label, "EXPAND", "HORIZONTAL");
	IupSetAttribute (selected_int_label, "ALIGNMENT", "ACENTER");
	
	m_iup_frame_adinterface = Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->GetIupUserInterface ();
	m_iup_vbox_GUI = IupVbox(common_int_label, m_iup_vbox_commoninterface, vbox_sep, atfg_label, hbox_atfg, selected_int_label, m_iup_frame_adinterface, m_iup_vbox_atf, NULL);
	//IupSetAttribute (m_iup_vbox_GUI, "NGAP", "20");
	IupSetAttribute (m_iup_vbox_GUI, "NMARGIN", "1x1");
	m_iup_hbox_dialog = 
		//IupVbox (
		IupHbox (m_iup_vbox_GUI, m_iup_canvas_renderer, NULL)
		//, IupText ("") , NULL)
		;

	m_iup_menu_file_load_volmodel = IupItem ("Carregar Modelo", NULL);
	IupSetAttribute (m_iup_menu_file_load_volmodel, "ACTIVE", "YES");
	IupSetCallback (m_iup_menu_file_load_volmodel, "ACTION", (Icallback)ViewerInterface::btn_OpenVol_CB);
	
	m_iup_menu_file_load_TransFunct = IupItem ("Carregar Função de Transferência", NULL);
	IupSetAttribute (m_iup_menu_file_load_TransFunct, "ACTIVE", "YES");
	IupSetCallback (m_iup_menu_file_load_TransFunct, "ACTION", (Icallback)ViewerInterface::btn_OpenTF_CB);

	m_iup_menu_file_reload_TransFunct = IupItem ("Recarregar Função de Transferência", NULL);
	IupSetAttribute (m_iup_menu_file_reload_TransFunct, "ACTIVE", "YES");
	IupSetCallback (m_iup_menu_file_reload_TransFunct, "ACTION", (Icallback)ViewerInterface::ButtonCB_ReloadTF);

	m_iup_menu_file_view_TransFunct = IupItem ("Ver Função de Transferência", NULL);
	IupSetAttribute (m_iup_menu_file_view_TransFunct, "ACTIVE", "YES");
	IupSetCallback (m_iup_menu_file_view_TransFunct, "ACTION", (Icallback)ViewerInterface::btn_ViewVol_CB);
	
	m_iup_menu_file_savestate = IupItem ("Salvar Estado", NULL);
	IupSetAttribute (m_iup_menu_file_savestate, "ACTIVE", "NO");
	IupSetCallback (m_iup_menu_file_savestate, "ACTION", (Icallback)ViewerInterface::btn_SaveState_cb);
	
	m_iup_menu_file_loadstate = IupItem ("Carregar Estado", NULL);
	IupSetAttribute (m_iup_menu_file_loadstate, "ACTIVE", "NO");
	IupSetCallback (m_iup_menu_file_loadstate, "ACTION", (Icallback)ViewerInterface::btn_LoadState_cb);

	m_iup_menu_file_savecampos = IupItem ("Salvar estado da câmera", NULL);
	IupSetAttribute (m_iup_menu_file_savecampos, "ACTIVE", "NO");
	IupSetCallback (m_iup_menu_file_savecampos, "ACTION", (Icallback)ViewerInterface::btn_SaveCamState_cb);
	
	m_iup_menu_file_loadcampos = IupItem ("Carregar estado da câmera", NULL);
	IupSetAttribute (m_iup_menu_file_loadcampos, "ACTIVE", "NO");
	IupSetCallback (m_iup_menu_file_loadcampos, "ACTION", (Icallback)ViewerInterface::btn_LoadCamState_cb);

	m_iup_menu_file_item_exit = IupItem ("Fechar", NULL);
	IupSetAttribute (m_iup_menu_file_item_exit, "ACTIVE", "YES");
	IupSetCallback (m_iup_menu_file_item_exit, "ACTION", (Icallback)ViewerInterface::Close_CB);

	m_iup_menu_file = IupMenu (
		m_iup_menu_file_load_volmodel, 
		IupSeparator (),
		m_iup_menu_file_load_TransFunct,
		m_iup_menu_file_reload_TransFunct,
		m_iup_menu_file_view_TransFunct,
		IupSeparator (),
		m_iup_menu_file_savestate,
		m_iup_menu_file_loadstate,
		IupSeparator (),
		m_iup_menu_file_savecampos,
		m_iup_menu_file_loadcampos,
		IupSeparator (),
		m_iup_menu_file_item_exit,
		NULL);
	m_iup_sub_menu_file = IupSubmenu ("Arquivo", m_iup_menu_file);

	m_iup_item_vis_cpu         = IupItem ("Malha Estruturada - CPU", NULL);
	//m_iup_item_vis_glsl = IupItem ("Malha Estruturada - GLSL", NULL);
	m_iup_item_vis_glsl2p      = IupItem ("Malha Estruturada - GLSL2P", NULL);
	m_iup_item_vis_ias         = IupItem ("Malha Estruturada - IAS", NULL);
	//m_iup_item_vis_unscpu = IupItem ("Malha Não Estruturada - CPU", NULL);
	m_iup_item_vis_ern2D       = IupItem ("Malha Estruturada - 2D ERN", NULL);
	m_iup_item_vis_ern3D       = IupItem ("Malha Estruturada - 3D ERN", NULL);
	m_iup_item_vis_glslriemann = IupItem ("GLSL - Equidistante", NULL);
	m_iup_item_vis_glslsimpson = IupItem ("GLSL - Simpson Adaptativo", NULL);

	IupSetAttribute(m_iup_item_vis_glslriemann, "ACTIVE", "YES");
	if (!Viewer::Instance()->m_viewmethods[VRVIEWS::EQUIDISTANT_GLSL])
		IupSetAttribute(m_iup_item_vis_glslriemann, "ACTIVE", "YES");

	IupSetAttribute(m_iup_item_vis_glslsimpson, "ACTIVE", "YES");
	if (!Viewer::Instance()->m_viewmethods[VRVIEWS::ADAPTIVE_GLSL])
		IupSetAttribute(m_iup_item_vis_glslsimpson, "ACTIVE", "YES");

	IupSetAttribute (m_iup_item_vis_cpu, "ACTIVE", "YES");
	if (!Viewer::Instance ()->m_viewmethods[VRVIEWS::CPU])
		IupSetAttribute (m_iup_item_vis_cpu, "ACTIVE", "YES");
	
	IupSetAttribute (m_iup_item_vis_glsl2p, "ACTIVE", "YES");
	if (!Viewer::Instance ()->m_viewmethods[VRVIEWS::GLSL2P])
		IupSetAttribute (m_iup_item_vis_glsl2p, "ACTIVE", "YES");

	IupSetAttribute (m_iup_item_vis_ias, "ACTIVE", "YES");
	if (!Viewer::Instance ()->m_viewmethods[VRVIEWS::IAS])
		IupSetAttribute (m_iup_item_vis_ias, "ACTIVE", "YES");

	IupSetAttribute (m_iup_item_vis_ern2D, "ACTIVE", "YES");
	if (!Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN2D])
		IupSetAttribute (m_iup_item_vis_ern2D, "ACTIVE", "NO");

	IupSetAttribute (m_iup_item_vis_ern3D, "ACTIVE", "YES");
	if (!Viewer::Instance ()->m_viewmethods[VRVIEWS::ERN])
		IupSetAttribute (m_iup_item_vis_ern3D, "ACTIVE", "NO");

	IupSetCallback (m_iup_item_vis_cpu, "ACTION", (Icallback)ViewerInterface::CPUVisualizationChange_CB);
	//IupSetCallback (m_iup_item_vis_glsl, "ACTION", (Icallback)ViewerInterface::GLSLVisualizationChange_CB);
	IupSetCallback (m_iup_item_vis_glsl2p, "ACTION", (Icallback)ViewerInterface::GLSL2PVisualizationChange_CB);
	IupSetCallback (m_iup_item_vis_ias, "ACTION", (Icallback)ViewerInterface::IASVisualizationChange_CB);
	//IupSetCallback (m_iup_item_vis_unscpu, "ACTION", (Icallback)ViewerInterface::UNSCPUVisualizationChange_CB);
	IupSetCallback (m_iup_item_vis_ern2D, "ACTION", (Icallback)ViewerInterface::ERN2DVisualizationChange_CB);
	IupSetCallback (m_iup_item_vis_ern3D, "ACTION", (Icallback)ViewerInterface::ERN3DVisualizationChange_CB);
	IupSetCallback (m_iup_item_vis_glslriemann, "ACTION", (Icallback)ViewerInterface::RiemannGLSLVisualizationChange_CB);
	IupSetCallback (m_iup_item_vis_glslsimpson, "ACTION", (Icallback)ViewerInterface::SimpsonGLSLVisualizationChange_CB);

	m_iup_menu_visualizations = IupMenu (
		//m_iup_item_vis_unscpu,
		//m_iup_item_vis_glsl,
		m_iup_item_vis_glsl2p,
		m_iup_item_vis_ias,
		IupSeparator (), 
		m_iup_item_vis_glslriemann,
		m_iup_item_vis_glslsimpson,
		IupSeparator (),
		m_iup_item_vis_cpu,
		IupSeparator(),
		m_iup_item_vis_ern2D,
		m_iup_item_vis_ern3D,
		NULL);
	m_iup_sub_menu_visualizations = IupSubmenu ("Visualizações", m_iup_menu_visualizations);

	m_iup_item_clcol_white = IupItem ("Branco", NULL);
	IupSetAttribute (m_iup_item_clcol_white, "ACTIVE", "YES");
	IupSetCallback (m_iup_item_clcol_white, "ACTION", (Icallback)ViewerInterface::ClearColor_White_CB);
	m_iup_item_clcol_black = IupItem ("Preto", NULL);
	IupSetAttribute (m_iup_item_clcol_black, "ACTIVE", "YES");
	IupSetCallback (m_iup_item_clcol_black, "ACTION", (Icallback)ViewerInterface::ClearColor_Back_CB);
	m_iup_item_clcol_blue = IupItem ("Azul", NULL);
	IupSetAttribute (m_iup_item_clcol_blue, "ACTIVE", "YES");
	IupSetCallback (m_iup_item_clcol_blue, "ACTION", (Icallback)ViewerInterface::ClearColor_Blue_CB);
	m_iup_menu_clearcolors = IupMenu (m_iup_item_clcol_white, m_iup_item_clcol_black, m_iup_item_clcol_blue, NULL);
	m_iup_submenu_clearcolors = IupSubmenu ("Cor de fundo", m_iup_menu_clearcolors);

	//http://www.andersriggelsen.dk/glblendfunc.php
	m_iup_item_glblend[0] = IupItem ("s*sA + d*(1-sA)", NULL);
	IupSetAttribute (m_iup_item_glblend[0], "ACTIVE", "YES");
	IupSetCallback (m_iup_item_glblend[0], "ACTION", (Icallback)ViewerInterface::BlendOption_GL_SRC_ALPHA_GL_ONE_MINUS_SRC_ALPHA);
	m_iup_item_glblend[1] = IupItem ("s*1 + d*0", NULL);
	IupSetAttribute (m_iup_item_glblend[1], "ACTIVE", "YES");
	IupSetCallback (m_iup_item_glblend[1], "ACTION", (Icallback)ViewerInterface::BlendOption_GL_ONE_GL_ZERO);
	//m_iup_item_glblend[2] = IupItem ("[GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA]", NULL);
	//IupSetAttribute (m_iup_item_glblend[2], "ACTIVE", "YES");
	//IupSetCallback (m_iup_item_glblend[2], "ACTION", (Icallback)ViewerInterface::BlendOption_GL_ONE_MINUS_SRC_ALPHA_GL_SRC_ALPHA);
	//m_iup_item_glblend[3] = IupItem ("[GL_SRC_ALPHA, GL_SRC_COLOR]", NULL);
	//IupSetAttribute (m_iup_item_glblend[3], "ACTIVE", "YES");
	//IupSetCallback (m_iup_item_glblend[3], "ACTION", (Icallback)ViewerInterface::BlendOption_GL_SRC_ALPHA_GL_SRC_COLOR);
	//m_iup_item_glblend[4] = IupItem ("[0 * src, dst]", NULL);
	//IupSetAttribute (m_iup_item_glblend[4], "ACTIVE", "YES");
	//IupSetCallback (m_iup_item_glblend[4], "ACTION", (Icallback)ViewerInterface::BlendOption_GL_ZERO_GL_ONE);
	//m_iup_item_glblend[5] = IupItem ("[0 * src, 0 * dst]", NULL);
	//IupSetAttribute (m_iup_item_glblend[5], "ACTIVE", "YES");
	//IupSetCallback (m_iup_item_glblend[5], "ACTION", (Icallback)ViewerInterface::BlendOption_GL_ZERO_GL_ZERO);
	//m_iup_item_glblend[6] = IupItem ("[src, dst]", NULL);
	//IupSetAttribute (m_iup_item_glblend[6], "ACTIVE", "YES");
	//IupSetCallback (m_iup_item_glblend[6], "ACTION", (Icallback)ViewerInterface::BlendOption_GL_ONE_GL_ONE);
	//m_iup_item_glblend[7] = IupItem ("[GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA]", NULL);
	//IupSetAttribute (m_iup_item_glblend[7], "ACTIVE", "YES");
	//IupSetCallback (m_iup_item_glblend[7], "ACTION", (Icallback)ViewerInterface::BlendOption_GL_ONE_MINUS_DST_ALPHA_GL_DST_ALPHA);
	//m_iup_item_glblend[8] = IupItem ("[GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR]", NULL);
	//IupSetAttribute (m_iup_item_glblend[8], "ACTIVE", "YES");
	//IupSetCallback (m_iup_item_glblend[8], "ACTION", (Icallback)ViewerInterface::BlendOption_GL_ONE_MINUS_DST_COLOR_GL_ONE_MINUS_SRC_COLOR);
	//m_iup_item_glblend[9] = IupItem ("[GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA]", NULL);
	//IupSetAttribute (m_iup_item_glblend[9], "ACTIVE", "YES");
	//IupSetCallback (m_iup_item_glblend[9], "ACTION", (Icallback)ViewerInterface::BlendOption_GL_DST_ALPHA_GL_ONE_MINUS_DST_ALPHA);
	//m_iup_item_glblend[10] = IupItem ("[GL_DST_COLOR, GL_SRC_COLOR]", NULL);
	//IupSetAttribute (m_iup_item_glblend[10], "ACTIVE", "YES");
	//IupSetCallback (m_iup_item_glblend[10], "ACTION", (Icallback)ViewerInterface::BlendOption_GL_DST_COLOR_GL_SRC_COLOR);
	//m_iup_item_glblend[11] = IupItem ("[GL_DST_COLOR, GL_SRC_ALPHA]", NULL);
	//IupSetAttribute (m_iup_item_glblend[11], "ACTIVE", "YES");
	//IupSetCallback (m_iup_item_glblend[11], "ACTION", (Icallback)ViewerInterface::BlendOption_GL_DST_COLOR_GL_SRC_ALPHA);
	m_iup_menu_glblendoptions = IupMenu (
		m_iup_item_glblend[0],
		m_iup_item_glblend[1],
		//m_iup_item_glblend[2],
		//m_iup_item_glblend[3],
		//m_iup_item_glblend[4],
		//m_iup_item_glblend[5],
		//m_iup_item_glblend[6],
		//m_iup_item_glblend[7],
		//m_iup_item_glblend[8],
		//m_iup_item_glblend[9],
		//m_iup_item_glblend[10],
		//m_iup_item_glblend[11],
		NULL);
	m_iup_submenu_glblendoptions = IupSubmenu ("Blending", m_iup_menu_glblendoptions);

	m_iup_main_dialog = IupDialog (m_iup_hbox_dialog);
	IupSetAttribute (m_iup_main_dialog, "MENU", "ViewerInterfaceIupMenu");
	IupSetAttribute (m_iup_main_dialog, "TITLE", "Volume Visualization");
	
	m_tf_plot = IupPlot();
	IupSetAttribute(m_tf_plot, "SYNCVIEW", "YES");
	IupSetAttribute(m_tf_plot, "LEGEND", "YES");
	IupSetAttribute(m_tf_plot, "LEGENDBOX", "NO");
	IupSetAttribute(m_tf_plot, "VIEWPORTSQUARE", "YES");

	m_tf_plot_dialog = IupDialog(IupVbox(m_tf_plot, NULL));
	IupSetAttribute(m_tf_plot_dialog, "TITLE", "Transfer Function");
	IupSetAttribute(m_tf_plot_dialog, "SIZE", "QUARTERxHALF");
	
	m_deriv_plot = IupPlot();
	IupSetAttribute(m_deriv_plot, "SYNCVIEW", "YES");
	IupSetAttribute(m_deriv_plot, "LEGEND", "YES");
	IupSetAttribute(m_deriv_plot, "LEGENDBOX", "NO");
	IupSetAttribute(m_deriv_plot, "VIEWPORTSQUARE", "YES");

	m_deriv_plot_dialog = IupDialog(IupVbox(m_deriv_plot, NULL));
	IupSetAttribute(m_deriv_plot_dialog, "TITLE", "Derivative Functions");
	IupSetAttribute(m_deriv_plot_dialog, "SIZE", "QUARTERxHALF");

	m_dist_plot = IupPlot();
	IupSetAttribute(m_dist_plot, "SYNCVIEW", "YES");
	IupSetAttribute(m_dist_plot, "LEGEND", "YES");
	IupSetAttribute(m_dist_plot, "LEGENDBOX", "NO");
	IupSetAttribute(m_dist_plot, "VIEWPORTSQUARE", "YES");

	m_dist_plot_dialog = IupDialog(IupVbox(m_dist_plot, NULL));
	IupSetAttribute(m_dist_plot_dialog, "TITLE", "Boundary Distance Function");
	IupSetAttribute(m_dist_plot_dialog, "SIZE", "QUARTERxHALF");

	Ihandle* m_show_plots = IupItem("Exibir Gráficos", NULL);
	IupSetAttribute(m_show_plots, "ACTIVE", "YES");

	IupSetCallback(m_show_plots, "ACTION", (Icallback)ShowPlots);

	Ihandle* m_atfg_menu = IupSubmenu("ATFG", IupMenu(m_show_plots, NULL));

	m_iup_menu = IupMenu(m_iup_sub_menu_file, m_iup_sub_menu_visualizations, m_iup_submenu_clearcolors, m_iup_submenu_glblendoptions, m_atfg_menu, NULL);
	IupSetHandle ("ViewerInterfaceIupMenu", m_iup_menu);
	
	TransferFunctionsViewer::Instance()->BuildInterface ();

	IupMap (m_iup_main_dialog);
	IupRefresh (m_iup_main_dialog);
}

int ViewerInterface::ShowPlots()
{
	IupShow(Viewer::Instance()->m_gui.m_deriv_plot_dialog);
	IupShow(Viewer::Instance()->m_gui.m_dist_plot_dialog);
	IupShow(Viewer::Instance()->m_gui.m_tf_plot_dialog);
	return IUP_DEFAULT;
}

void ViewerInterface::UpdateBThickLabel(int bthick)
{
	char *val = new char[13];
	sprintf(val, "BThick: %d .", bthick);
	IupSetAttribute(m_bthick_label, "TITLE", val);
}

void ViewerInterface::UpdateGTreshLabel(float gtresh)
{
	char *val = new char[15];
	sprintf(val, "%.2f ", gtresh);
	IupSetAttribute(m_gtresh_bar, "VALUE", val);
	sprintf(val, "GTresh: %.2f ", gtresh);
	IupSetAttribute(m_gtresh_label, "TITLE", val);
}

Ihandle* ViewerInterface::GetCanvasRenderer ()
{
	return m_iup_canvas_renderer;
}

void ViewerInterface::UpdateAdInterface ()
{
	IupDestroy (m_iup_frame_adinterface);
	Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->CreateIupUserInterface ();
	m_iup_frame_adinterface = Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->GetIupUserInterface ();
	IupAppend (m_iup_vbox_GUI, m_iup_frame_adinterface);
	IupMap (m_iup_frame_adinterface);
	Viewer::Instance ()->m_viewmethods[Viewer::Instance ()->m_current_view]->UpdateIupUserInterface ();
	IupRefresh (m_iup_frame_adinterface);
}

void ViewerInterface::ShowTransferFunctionViewer ()
{
	TransferFunctionsViewer::Instance ()->Show ();
}

///////////////////////
// Protected Methods //
///////////////////////

/////////////////////
// Private Methods //
/////////////////////

