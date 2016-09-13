#include "Viewer.h"

#include "EquidistantEvaluation\ViewMethodEquidistant.h"
#include "AdaptiveEvaluation\ViewMethodAdaptive.h"

#include "GLSL2P\ViewMethodGLSL2P.h"
#include "IAS\ViewMethodIAS.h"

#include "CPU\ViewMethodCPU.h"

#include "ERN2D\ERN2DViewMethod.h"
#include "ERN\ERNViewMethod.h"

#include "volrend\Reader.h"
#include "AutomaticTransferFunction\TransferFunction.h"
#include "AutomaticTransferFunction\RAWFile.h"

#include <cstdlib>

#define ATFG
//#define FAST_TFG

Viewer *Viewer::m_instance = 0;

////////////////////
// Public Methods //
////////////////////

Viewer* Viewer::Instance ()
{
  if (!m_instance)
    m_instance = new Viewer ();

  return m_instance;
}

bool Viewer::Exists ()
{
  return (m_instance != NULL);
}

void Viewer::DestroyInstance ()
{
  if (!Viewer::Exists())
    return;

  for (int VRViewsInt = 0; VRViewsInt != VRVIEWS::_LastToIter; VRViewsInt++)
  {
    VRVIEWS vrview = static_cast<VRVIEWS>(VRViewsInt);
    if (m_instance->m_viewmethods[vrview])
    {
      ViewMethod* vm = m_instance->m_viewmethods[vrview];
      m_instance->m_viewmethods[vrview] = NULL;
      delete vm;
    }
  }
  m_instance->m_viewmethods.clear ();

  if (m_instance)
  {
    delete m_instance;
    m_instance = NULL;
  }
}

void Viewer::Init (int argc, char *argv[])
{
  LoadViewerState ();

  while (!m_volume)
    FileDlg_VolumeModel ();
  while (!m_transfer_function)
    FileDlg_TransferFunction ();

  m_instance->m_viewmethods[m_current_view]->BuildViewer ();
  UpdateIupUserInterfaces ();
}

void Viewer::Start ()
{
  IupShowXY (m_gui.m_iup_main_dialog, IUP_CENTER, IUP_CENTER);
  IupMainLoop ();
  IupClose ();
}

void Viewer::InitAndStart (int argc, char **argv)
{
  SetCanvasSizes (m_CurrentWidth, m_CurrentHeight);
  InitIup (argc, argv);
  InitGL (argc, argv);
  Init (argc, argv);

  //printf ("GL_ARB_gpu_shader_f64 %d\n", glewGetExtension ("GL_ARB_gpu_shader_fp64"));
  Start ();
}

void Viewer::ReloadTransferFunction ()
{
  if (m_transfer_function)
  {
    delete m_transfer_function;
    m_transfer_function = NULL;
    vr::TransferFunction* v = vr::ReadTransferFunction (m_transfer_function_file);
    SetTransferFunction (v, m_transfer_function_file);
  }
}

void Viewer::SetTransferFunction (vr::TransferFunction* tf, std::string file)
{
  if (tf)
  {
    if (m_transfer_function)
      delete m_transfer_function;

    m_transfer_function = tf;
    m_tf_name = tf->GetName ();
    m_transfer_function_file = file;
  }
}

void ConnectivityCleanup(vr::Volume* vol)
{
  bool* visited = new bool[vol->GetWidth() * vol->GetHeight() * vol->GetDepth()];
  
  for (int i = 0; i < vol->GetWidth(); ++i) {
    for (int j = 0; j < vol->GetHeight(); ++j) {
      for (int k = 0; k < vol->GetDepth(); ++k) {
      }
    }
  }

}

void Viewer::SetVolumeModel (vr::Volume* vol, std::string file)
{
  if (vol)
  {
    if (m_volume)
      delete m_volume;

    m_volume = vol;
    m_volumename = vol->GetName();
    m_volume_file = file;

    delete m_atfg;
    delete m_fast_tfg;

    ConnectivityCleanup(vol);
    
#ifdef ATFG
    try
    {
      m_atfg = new ATFGenerator(m_volume);
      m_atfg->SetMainPlot(Viewer::Instance()->m_gui.m_iup_main_plot_dialog);
      m_atfg->SetTransferFunctionPlot(Viewer::Instance()->m_gui.m_iup_tf_plot_dialog);
      m_atfg->SetBoundaryFunctionPlot(Viewer::Instance()->m_gui.m_iup_bx_plot_dialog);
      if (m_atfg->Init())
      {
        //m_atfg->GenerateVolumeSlices();
        //m_atfg->GenerateGradientSlices();
        //m_atfg->GenerateLaplacianSlices();
        //m_atfg->GenerateHistogramSlices();
        m_atfg->GenerateGradientSummedHistogram();
        m_atfg->GenerateLaplacianSummedHistogram();

        if (m_atfg->ExtractTransferFunction())
        {
          ITransferFunction* tf = m_atfg->GetTransferFunction();
          tf->SetValueColor(0, 255, 255, 255);
          tf->SetValueColor(32, 255, 0, 0);
          tf->SetValueColor(64, 0, 255, 0);
          tf->SetValueColor(96, 0, 0, 255);
          tf->SetValueColor(128, 127, 127, 0);
          tf->SetValueColor(160, 127, 0, 127);
          tf->SetValueColor(192, 0, 127, 127);
          tf->SetValueColor(224, 84, 84, 85);
          tf->SetValueColor(255, 0, 0, 0);

          if (tf->Generate()) {
            char* tf_file = tf->GetPath();
            vr::TransferFunction* tfr = vr::ReadTransferFunction(tf_file);
            SetTransferFunction(tfr, tf_file);
          }
        }
      }
    }
    catch (const std::out_of_range& e)
    {
      printf("\nEste volume nao pode ser processado, devido a suas dimensoes!\n\n");
    }
    catch (const std::exception&)
    {
      printf("\nOcorreu um erro inesperado!\n\n");
    }
#elif defined(FAST_TFG)
    m_fast_tfg = new FastTFGenerator(m_volume);
    m_fast_tfg->SetMainPlot(Viewer::Instance()->m_gui.m_iup_main_plot_dialog);
    m_fast_tfg->SetBoundaryFunctionPlot(Viewer::Instance()->m_gui.m_iup_bx_plot_dialog);
    if (m_fast_tfg->Init())
    {
      m_fast_tfg->ExtractTransferFunction();
    }
#endif
  }
}

int Viewer::SetBoundaryThickness(Ihandle* ih)
{
#ifdef ATFG
  char *val = IupGetAttribute(ih, "VALUE");
  std::string::size_type size;
  int scale = std::stoi(val, &size);
  if (scale != Viewer::Instance()->m_boundary_thickness)
  {
    Viewer::Instance()->m_boundary_thickness = scale;
    Viewer::Instance()->m_gui.UpdateBThickLabel(scale);
    TransferFunction* tf = (TransferFunction*)Viewer::Instance()->m_atfg->GetTransferFunction();
    tf->SetBoundaryThickness(scale);
    if (tf->Generate())
    {
      char* tf_file = tf->GetPath();
      vr::TransferFunction* tfr = vr::ReadTransferFunction(tf_file);
      Viewer::Instance()->SetTransferFunction(tfr, tf_file);
      Viewer::Instance()->m_viewmethods[GLSL2P]->ReloadTransferFunction();
      Viewer::Instance()->m_viewmethods[IAS]->ReloadTransferFunction();
      Viewer::Instance()->m_viewmethods[EQUIDISTANT_GLSL]->ReloadTransferFunction();
      Viewer::Instance()->m_viewmethods[ADAPTIVE_GLSL]->ReloadTransferFunction();
    }
  }
#endif
  return IUP_DEFAULT;
}

int Viewer::SetGTresh(Ihandle* ih)
{
#ifdef ATFG
  char *val = IupGetAttribute(ih, "VALUE");
  std::string::size_type size;
  float scale = std::stof(val, &size);
  Viewer::Instance()->m_gtresh = scale;
  Viewer::Instance()->m_gui.UpdateGTreshLabel(scale);
  Viewer::Instance()->m_atfg->SetGTresh(scale);
  if (!Viewer::Instance()->m_atfg->ExtractTransferFunction())
    return IUP_DEFAULT;

  TransferFunction* tf = (TransferFunction*)Viewer::Instance()->m_atfg->GetTransferFunction();
  tf->SetValueColor(0, 255, 255, 255);
  tf->SetValueColor(32, 255, 0, 0);
  tf->SetValueColor(64, 0, 255, 0);
  tf->SetValueColor(96, 0, 0, 255);
  tf->SetValueColor(128, 127, 127, 0);
  tf->SetValueColor(160, 127, 0, 127);
  tf->SetValueColor(192, 0, 127, 127);
  tf->SetValueColor(224, 84, 84, 85);
  tf->SetValueColor(255, 0, 0, 0);
  tf->SetBoundaryThickness(Viewer::Instance()->m_boundary_thickness);
  tf->SetBoundary(Viewer::Instance()->m_boundary);
  if (tf->Generate())
  {
    char* tf_file = tf->GetPath();
    vr::TransferFunction* tfr = vr::ReadTransferFunction(tf_file);
    Viewer::Instance()->SetTransferFunction(tfr, tf_file);
    Viewer::Instance()->m_viewmethods[GLSL2P]->ReloadTransferFunction();
    Viewer::Instance()->m_viewmethods[IAS]->ReloadTransferFunction();
    Viewer::Instance()->m_viewmethods[EQUIDISTANT_GLSL]->ReloadTransferFunction();
    Viewer::Instance()->m_viewmethods[ADAPTIVE_GLSL]->ReloadTransferFunction();
  }
#endif
  return IUP_DEFAULT;
}

int Viewer::SetMinHistogramValue(Ihandle* ih, int min)
{
#ifdef ATFG
  Viewer::Instance()->m_atfg->SetMinimumHistogramValue(min);
  Viewer::Instance()->m_atfg->GenerateGradientSummedHistogram();
  Viewer::Instance()->m_atfg->GenerateLaplacianSummedHistogram();
  if (!Viewer::Instance()->m_atfg->ExtractTransferFunction())
    return IUP_DEFAULT;

  TransferFunction* tf = (TransferFunction*)Viewer::Instance()->m_atfg->GetTransferFunction();
  tf->SetValueColor(0, 255, 255, 255);
  tf->SetValueColor(32, 255, 0, 0);
  tf->SetValueColor(64, 0, 255, 0);
  tf->SetValueColor(96, 0, 0, 255);
  tf->SetValueColor(128, 127, 127, 0);
  tf->SetValueColor(160, 127, 0, 127);
  tf->SetValueColor(192, 0, 127, 127);
  tf->SetValueColor(224, 84, 84, 85);
  tf->SetValueColor(255, 0, 0, 0);
  tf->SetBoundaryThickness(Viewer::Instance()->m_boundary_thickness);
  tf->SetBoundary(Viewer::Instance()->m_boundary);
  if (tf->Generate())
  {
    char* tf_file = tf->GetPath();
    vr::TransferFunction* tfr = vr::ReadTransferFunction(tf_file);
    Viewer::Instance()->SetTransferFunction(tfr, tf_file);
    Viewer::Instance()->m_viewmethods[GLSL2P]->ReloadTransferFunction();
    Viewer::Instance()->m_viewmethods[IAS]->ReloadTransferFunction();
    Viewer::Instance()->m_viewmethods[EQUIDISTANT_GLSL]->ReloadTransferFunction();
    Viewer::Instance()->m_viewmethods[ADAPTIVE_GLSL]->ReloadTransferFunction();
  }
#endif
  return IUP_DEFAULT;
}

int Viewer::SetBoundary(Ihandle* ih, int boundary)
{
#ifdef ATFG
  TransferFunction* tf = (TransferFunction*)Viewer::Instance()->m_atfg->GetTransferFunction();
  Viewer::Instance()->m_boundary = boundary;
  tf->SetBoundary(boundary);
  if (tf->Generate()) {
    char* tf_file = tf->GetPath();
    vr::TransferFunction* tfr = vr::ReadTransferFunction(tf_file);
    Viewer::Instance()->SetTransferFunction(tfr, tf_file);
    Viewer::Instance()->m_viewmethods[GLSL2P]->ReloadTransferFunction();
    Viewer::Instance()->m_viewmethods[IAS]->ReloadTransferFunction();
    Viewer::Instance()->m_viewmethods[EQUIDISTANT_GLSL]->ReloadTransferFunction();
    Viewer::Instance()->m_viewmethods[ADAPTIVE_GLSL]->ReloadTransferFunction();
  }
#endif
  return IUP_DEFAULT;
}

void Viewer::BuildViewers ()
{
  for (int VRViewsInt = 0; VRViewsInt != VRVIEWS::_LastToIter; VRViewsInt++)
  {
    VRVIEWS vrview = static_cast<VRVIEWS>(VRViewsInt);
    if (m_viewmethods[vrview])
      m_viewmethods[vrview]->BuildViewer ();
  }
}

void Viewer::CleanViewers ()
{
  for (int VRViewsInt = 0; VRViewsInt != VRVIEWS::_LastToIter; VRViewsInt++)
  {
    VRVIEWS vrview = static_cast<VRVIEWS>(VRViewsInt);
    if (m_current_view != vrview && m_viewmethods[vrview])
      m_viewmethods[vrview]->CleanViewer ();
  }
}

void Viewer::UpdateIupUserInterfaces ()
{
  for (int VRViewsInt = 0; VRViewsInt != VRVIEWS::_LastToIter; VRViewsInt++)
  {
    VRVIEWS vrview = static_cast<VRVIEWS>(VRViewsInt);
    if (m_viewmethods[vrview])
      m_viewmethods[vrview]->UpdateIupUserInterface ();
  }
}

void Viewer::GenerateVolHistogram ()
{
  if (!m_volume) return;

  int h[256];
  for (int i = 0; i < 256; i++)
    h[i] = 0;

  int out = 0;
  for (int x = 0; x < m_volume->GetWidth(); x++)
  {
    for (int y = 0; y < m_volume->GetHeight(); y++)
    {
      for (int z = 0; z < m_volume->GetDepth(); z++)
      {
        int v = m_volume->SampleVolume(x, y, z);
        if (v < 256)
          h[v]++;
        else
          ++out;
      }
    }
  }

  if (out > 0)
    printf ("%d values > 255.\n", out);

  std::ofstream state_file;
  state_file.open ("histogram.txt");
  for (int i = 0; i < 256; i++)
    state_file << i << ": " << h[i] << "\n";
  state_file.close ();
}

int Viewer::GetCanvasWidth ()
{
  return m_CurrentWidth;
}

int Viewer::GetCanvasHeight ()
{
  return m_CurrentHeight;
}

void Viewer::SetCanvasSizes (int width, int height)
{
  m_CurrentWidth = width;
  m_CurrentHeight = height;
}

int Viewer::Redraw (Ihandle* handle, Ihandle* canvas)
{
  return m_viewmethods[Viewer::Instance ()->m_current_view]->Idle_Action_CB (canvas);
}

int Viewer::Keyboard (Ihandle *ih, int c, int press)
{
  return m_viewmethods[Viewer::Instance ()->m_current_view]->Keyboard_CB (ih, c, press);
}

int Viewer::MouseButton (Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  return m_viewmethods[Viewer::Instance ()->m_current_view]->Button_CB (ih, button, pressed, x, y, status);
}

int Viewer::MouseMotion (Ihandle *ih, int x, int y, char *status)
{
  return m_viewmethods[Viewer::Instance ()->m_current_view]->Motion_CB (ih, x, y, status);
}

void Viewer::DisactiveInterfaces ()
{
  for (int VRViewsInt = 0; VRViewsInt != VRVIEWS::_LastToIter; VRViewsInt++)
  {
    VRVIEWS vrview = static_cast<VRVIEWS>(VRViewsInt);
    if (m_viewmethods[vrview])
      m_viewmethods[vrview]->DisactiveInterface ();
  }
}

void Viewer::MarkAllViewMethodsOutdated ()
{
  for (int VRViewsInt = 0; VRViewsInt != VRVIEWS::_LastToIter; VRViewsInt++)
  {
    VRVIEWS vrview = static_cast<VRVIEWS>(VRViewsInt);
    if (m_viewmethods[vrview])
      m_viewmethods[vrview]->MarkOutdated ();
  }
}

void Viewer::CreateAllViewMethodIupInterfaces ()
{
  for (int VRViewsInt = 0; VRViewsInt != VRVIEWS::_LastToIter; VRViewsInt++)
  {
    VRVIEWS vrview = static_cast<VRVIEWS>(VRViewsInt);
    if (m_viewmethods[vrview])
      m_viewmethods[vrview]->CreateIupUserInterface ();
  }
}

Ihandle* Viewer::GetCanvasRenderer ()
{
  return m_gui.GetCanvasRenderer ();
}

void Viewer::UpdateAdInterface ()
{
  m_gui.UpdateAdInterface ();
}

void Viewer::SetClearColor (float r, float g, float b, float a)
{
  m_rendererClearColor = lqc::Vector4f (r, g, b, a);
  glClearColor (r, g, b, a);
}

lqc::Vector4f Viewer::GetClearColor ()
{
  return m_rendererClearColor;
}

void Viewer::SaveViewerState ()
{
  std::ofstream state_file;
  state_file.open ("iup_volume_redering_state.vrstate");
  state_file << m_volume_file << '\n';
  state_file << m_transfer_function_file << '\n';
  state_file << (int)m_current_view;
  state_file.close ();
}

void Viewer::LoadViewerState ()
{
  std::ifstream state_file;
  state_file.open ("iup_volume_redering_state.vrstate");
  if (state_file.is_open ())
  {
    std::string view_method_number;
    std::getline (state_file, m_volume_file);
    std::getline (state_file, m_transfer_function_file);
    std::getline (state_file, view_method_number);

    vr::Volume* v = vr::ReadFromVolMod (m_volume_file);
    Viewer::Instance ()->SetVolumeModel (v, m_volume_file);

    vr::TransferFunction* tf = vr::ReadTransferFunction (m_transfer_function_file);
    Viewer::Instance ()->SetTransferFunction (tf, m_transfer_function_file);

    m_current_view = static_cast<VRVIEWS>(atoi (view_method_number.c_str ()));

    state_file.close ();

    UpdateAdInterface ();
  }
}

bool Viewer::FileDlg_VolumeModel ()
{
  bool ret = false;

  Ihandle *dlg = IupFileDlg ();

  IupSetAttribute(dlg, "DIRECTORY", "../../Modelos/VolumeModels");
  IupSetAttribute (dlg, "DIALOGTYPE", "OPEN");
  IupSetAttribute (dlg, "TITLE", "Load Volume Model");
  IupSetAttributes (dlg, "FILTER = \"*.vol;*.ele;*.node;*.raw;*.med\", FILTERINFO = \"Volume Files [.vol, .ele, .node, .raw, .med]\"");

  IupPopup (dlg, IUP_CURRENT, IUP_CURRENT);

  if (IupGetInt (dlg, "STATUS") != -1)
  {
    std::string file (IupGetAttribute (dlg, "VALUE"));

    vr::Volume* v = vr::ReadFromVolMod (file);
    if (v)
    {
      Viewer::Instance ()->SetVolumeModel (v, file);
      ret = true;
    }
  }

  IupDestroy (dlg);

  return ret;
}

bool Viewer::FileDlg_TransferFunction ()
{
  bool ret = false;

  Ihandle *dlg = IupFileDlg ();

  IupSetAttribute(dlg, "DIRECTORY", "../../Modelos/TransferFunctions");
  IupSetAttribute (dlg, "DIALOGTYPE", "OPEN");
  IupSetAttribute (dlg, "TITLE", "Load Transfer Function");
  IupSetAttributes (dlg, "FILTER = \"*.tf1d;*.tf;*.tfg1d;*.tfgersa\", FILTERINFO = \"TF1D Files;TF Files\"");

  IupPopup (dlg, IUP_CURRENT, IUP_CURRENT);

  if (IupGetInt (dlg, "STATUS") != -1)
  {
    std::string file (IupGetAttribute (dlg, "VALUE"));

    vr::TransferFunction* v = vr::ReadTransferFunction (file);
    if (v)
    {
      Viewer::Instance ()->SetTransferFunction (v, file);
      ret = true;
    }
  }

  IupDestroy (dlg);

  return ret;
}

bool Viewer::SaveSnapshot (char* filename)
{
  int width = Viewer::Instance ()->m_CurrentWidth;
  int height = Viewer::Instance ()->m_CurrentHeight;
  unsigned char *gl_data = new unsigned char[3 * width*height];
  glPushClientAttrib (GL_CLIENT_PIXEL_STORE_BIT); // pixelstore
  glPushAttrib (GL_PIXEL_MODE_BIT); // readbuffer
  glReadBuffer (GL_FRONT);
  glFlush ();
  //glPixelStorei (GL_PACK_ALIGNMENT, width % 4 ? 1 : (width % 8 ? 4 : 8));
  glPixelStorei (GL_PACK_ALIGNMENT, 1); /* data alignment must be 1 */
  glReadPixels (0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, gl_data);
  glPopAttrib ();
  glPopClientAttrib ();
  int error;
  imFile* ifile = imFileNew (filename, "PNG", &error);
  error = imFileWriteImageInfo (ifile, width, height, IM_RGB | IM_PACKED, IM_BYTE);
  error = imFileWriteImageData (ifile, gl_data);
  imFileClose (ifile);
  delete[] gl_data;

  return true;
}

///////////////////////
// Protected Methods //
///////////////////////

Viewer::Viewer ()
{
  m_boundary = 0;
  m_boundary_thickness = 1;
  m_gtresh = 0.0f;
  m_min_hist = 0;
  m_CurrentWidth = 800;
  m_CurrentHeight = 600;

  m_rendererClearColor = lqc::Vector4f (1.0f, 1.0f, 1.0f, 1.0f);

  for (int VRViewsInt = 0; VRViewsInt != VRVIEWS::_LastToIter; VRViewsInt++)
    m_viewmethods[static_cast<VRVIEWS>(VRViewsInt)] = NULL;
  

  m_viewmethods[VRVIEWS::CPU] = new ViewMethodCPU ();
  m_viewmethods[VRVIEWS::GLSL2P]           = new ViewMethodGLSL2P();
  m_viewmethods[VRVIEWS::IAS]              = new ViewMethodIAS ();
  //m_viewmethods[VRVIEWS::ERN2D]         = new ERN2DViewMethod ();
  //m_viewmethods[VRVIEWS::ERN]           = new ERNViewMethod ();
  m_viewmethods[VRVIEWS::EQUIDISTANT_GLSL] = new ViewMethodEquidistant();
  m_viewmethods[VRVIEWS::ADAPTIVE_GLSL]    = new ViewMethodAdaptive();

  bool choosen = false;
  for (int VRViewsInt = 0; VRViewsInt != VRVIEWS::_LastToIter; VRViewsInt++)
  {
    VRVIEWS vrview = static_cast<VRVIEWS>(VRViewsInt);
    if (m_viewmethods[vrview])
    {
      m_current_view = vrview;
      choosen = true;
      break;
    }
  }

  if (!choosen)
  {
    printf("No ViewMethod Found. Closing Application.\n");
    getchar ();
    exit (1);
  }

  m_transfer_function_file = "NULL";
  m_volume_file = "NULL";

  m_transfer_function = NULL;
  m_tf_name = "NULL";
  
  m_volume = NULL;
  m_volumename = "NULL";

  m_atfg = NULL;
  m_fast_tfg = NULL;
}

Viewer::~Viewer ()
{
  SaveViewerState ();

  if (m_transfer_function)
    delete m_transfer_function;
  m_transfer_function = NULL;
  if (m_volume)
    delete m_volume;
  m_volume = NULL;
}

void Viewer::InitIup (int argc, char *argv[])
{
  IupOpen (&argc, &argv);
  IupPlotOpen();
  IupGLCanvasOpen ();

  m_gui.BuildInterface (argc, argv);
}

void Viewer::InitGL (int argc, char *argv[])
{
  IupGLMakeCurrent (GetCanvasRenderer());

  glewExperimental = GL_TRUE;
  GLenum GlewInitResult = glewInit ();

  if (GLEW_OK != GlewInitResult)
  {
    fprintf (stderr, "ERROR: %s\n", glewGetErrorString (GlewInitResult));
    exit (EXIT_FAILURE);
  }

  fprintf (stdout, "INFO: OpenGL Version: %s\n", glGetString (GL_VERSION));

  glGetError ();

  lqc::Vector4f a = m_rendererClearColor;
  glClearColor (a.x, a.y, a.z, a.w);
  //gluOrtho2D (0, m_CurrentWidth, 0, m_CurrentHeight);
  
  glEnable (GL_CULL_FACE);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LESS);
  gl::ExitOnGLError ("ERROR: Could not set OpenGL depth testing options");

  glEnable (GL_ALPHA_TEST);
  glEnable (GL_BLEND);

  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable (GL_TEXTURE_3D);
  glEnable (GL_TEXTURE_2D);
  glEnable (GL_TEXTURE_1D);
  gl::ExitOnGLError ("ERROR: OpenGL error");
}