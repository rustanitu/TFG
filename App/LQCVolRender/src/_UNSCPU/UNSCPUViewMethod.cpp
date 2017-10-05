#include "UNSCPUViewMethod.h"

#include "../Viewer.h"

UNSCPUViewMethod::UNSCPUViewMethod ()
: ViewMethod (VRVIEWS::CPU)
{
  m_volumename = "";
  m_trasnferfunctionname = "";

  m_redisplay = true;
  m_show_redisplay_informations = true;

  m_canvaswidth = -1;
  m_canvasheight = -1;

  MarkOutdated ();
}

UNSCPUViewMethod::~UNSCPUViewMethod ()
{
}

int UNSCPUViewMethod::Idle_Action_CB (Ihandle* cnv_renderer)
{
  IupGLMakeCurrent (cnv_renderer);
  if (m_redisplay)
  {
    if (m_outdated)
    {
      if (Viewer::Instance ()->m_volumename.compare (m_volumename) != 0 ||
          Viewer::Instance ()->m_tf_name.compare (m_trasnferfunctionname) != 0)
      {
        m_volumename = Viewer::Instance ()->m_volumename;
        m_trasnferfunctionname = Viewer::Instance ()->m_tf_name;
      }
      m_viewer.SetModelVolumeAndTransferFunction (Viewer::Instance ()->m_volume, Viewer::Instance ()->m_transfer_function);
      if (m_canvaswidth != Viewer::Instance ()->GetCanvasWidth () || m_canvasheight != Viewer::Instance ()->GetCanvasHeight ())
        Resize (Viewer::Instance ()->GetCanvasWidth (), Viewer::Instance ()->GetCanvasHeight ());
      m_outdated = false;
    }

    if (m_show_redisplay_informations)
    {
      #ifdef CLEAR_CONSOLE_VIEWERVR
        #ifdef WIN32
          if (system ("CLS")) system ("clear");
        #endif
      #endif
      printf ("---------------------CPU ViewMethod Redisplay---------------------------\n");
      printf ("CPU ViewMethod Redisplay Settings:\n");
      printf ("  - Using[Outer, Inner] Integral Errors : [%lf, %lf]\n", m_viewer.m_voleval.m_integral_error, m_viewer.m_voleval.m_error_inner_integral);
      printf ("  - Interval Distance for Adaptive Method Set to : %lf\n", m_viewer.m_voleval.m_adaptive_distance_intervals);
      printf ("  - Interval Distance for Riemman Method Set to : %lf\n", m_viewer.m_voleval.m_distance_intervals);
      printf ("  - Opacity Correction : %lf\n", m_viewer.m_voleval.m_opacity_correction);
    }
    clock_t start_clock = clock ();
    m_viewer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
    clock_t end_clock = clock ();
    double time_elapsed = (double)(end_clock - start_clock) / (double)(CLOCKS_PER_SEC);

    m_gui.SetTimeElapsed (time_elapsed);
    m_gui.SetStatistics (m_viewer.m_voleval.m_statistics.n_gets_volume_values,
                         m_viewer.m_voleval.m_statistics.n_gets_tf_values,
                         m_viewer.m_voleval.m_statistics.n_compositions);
    printf ("CPU Redraw Done\n  - Time Elapsed: %Lf seconds\n", time_elapsed);
    printf ("------------------------------------------------------------------------\n");

    m_redisplay = false;
    IupGLSwapBuffers (cnv_renderer);
  }
  return IUP_DEFAULT;
}

int UNSCPUViewMethod::Keyboard_CB (Ihandle *ih, int c, int press)
{
  if (press == 0)
  {
    switch (c)
    {
      case 'w':
        m_viewer.m_camera.Keyboard_cb (c, 0, 0);
        break;
      case 'a':
        m_viewer.m_camera.Keyboard_cb (c, 0, 0);
        break;
      case 's':
        m_viewer.m_camera.Keyboard_cb (c, 0, 0);
        break;
      case 'd':
        m_viewer.m_camera.Keyboard_cb (c, 0, 0);
        break;
      case 'e':
        m_viewer.m_camera.Keyboard_cb (c, 0, 0);
        break;
      case 'q':
        m_viewer.m_camera.Keyboard_cb (c, 0, 0);
        break;
      case K_C:
      case K_c:
        if (m_viewer.m_num_of_threads == 2)
          m_viewer.m_num_of_threads = 4;
        else if (m_viewer.m_num_of_threads == 4)
          m_viewer.m_num_of_threads = 2;
        printf ("Set Number of Threads equal to %d\n", m_viewer.m_num_of_threads);
        break;
      case K_V:
      case K_v:
        RecalculateAABBSizes ();
        ResetCamera ();
        break;
      case K_B:
      case K_b:
        m_viewer.m_camera.SetMovementSpeed (m_viewer.m_camera.GetMovementSpeed () - 1.0f);
        break;
      case K_N:
      case K_n:
        m_viewer.m_camera.SetMovementSpeed (m_viewer.m_camera.GetMovementSpeed () + 1.0f);
        break;
      case K_Y:
      case K_y:
        std::cout << "Set the new integral error value: ";
        {
          double ierror;
          std::cin >> ierror;
          SetIntegralError (ierror);
          std::cout << "Integral Error set to: " << ierror << "." << std::endl;
        }
        break;
      case K_U:
      case K_u:
        std::cout << "Set the new inner integral error value: ";
        {
          double ierror;
          std::cin >> ierror;
          SetInnerIntegralError (ierror);
          std::cout << "Inner Integral Error set to: " << ierror << "." << std::endl;
        }
        break;
      case K_H:
      case K_h:
        std::cout << "Set the new interval distance evaluation: ";
        {
          double idistance;
          std::cin >> idistance;
          SetIntervalDistance (idistance);
          std::cout << "interval distance evaluation set to: " << idistance << "." << std::endl;
        }
        break;
      case K_J:
      case K_j:
        std::cout << "Set the new opacity correction value: ";
        {
          double opcorrection;
          std::cin >> opcorrection;
          m_viewer.m_voleval.m_opacity_correction = opcorrection;
          std::cout << "Opacity Correction set to: " << opcorrection << "." << std::endl;
        }
        break;
      case K_K:
      case K_k:
        std::cout << "Set the new interval distance: ";
        {
          double distintervals;
          std::cin >> distintervals;
          m_viewer.m_voleval.m_distance_intervals = distintervals;
          std::cout << "Opacity Correction set to: " << distintervals << "." << std::endl;
        }
        break;
      case K_R:
      case K_r:
        m_redisplay = true;
        break;
      case K_M:
      case K_m:
        m_viewer.ResetCamera (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
        break;
      case K_ESC:
        exit (EXIT_SUCCESS);
        break;
      case K_UP:
        m_viewer.m_camera.RotateX (-.1f);
        break;
      case K_DOWN:
        m_viewer.m_camera.RotateX (.1f);
        break;
      case K_LEFT:
        m_viewer.m_camera.RotateY (.1f);
        break;
      case K_RIGHT:
        m_viewer.m_camera.RotateY (-.1f);
        break;
      default:
        break;
    }
    m_redisplay = true;
  }
  return IUP_DEFAULT;
}

int UNSCPUViewMethod::Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  return IUP_DEFAULT;
}

int UNSCPUViewMethod::Motion_CB (Ihandle *ih, int x, int y, char *status)
{
  return IUP_DEFAULT;
}

int UNSCPUViewMethod::Resize_CB (Ihandle *ih, int width, int height)
{
  Resize (width, height);
  return IUP_DEFAULT;
}

void UNSCPUViewMethod::CreateIupUserInterface ()
{
  m_gui.CreateIupInterface ();
}

void UNSCPUViewMethod::UpdateIupUserInterface ()
{
  m_gui.SetUseFunctionEvaluation (m_viewer.m_voleval.m_use_evaluate_function);
  m_gui.SetAllFunctions (m_viewer.m_voleval.m_voleval_functions.GetFunctionNames());
  
  m_gui.SetUsingPThreads (m_viewer.m_pthread_on);
  m_gui.SetNumberOfThreads (GetNumberOfThreads());
  
  m_gui.SetTimeElapsed (0.0);
  m_gui.SetStatistics (m_viewer.m_voleval.m_statistics.n_gets_volume_values,
                       m_viewer.m_voleval.m_statistics.n_gets_tf_values,
                       m_viewer.m_voleval.m_statistics.n_compositions);
  m_gui.SetErrorValue (GetIntegralError());

  m_gui.SetSamples (m_viewer.m_voleval.m_samples);
  m_gui.SetAdaptiveSamples (m_viewer.m_voleval.m_adaptive_samples);
}

void UNSCPUViewMethod::ResetCamera ()
{
  m_viewer.ResetCamera (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
  m_redisplay = true;
}

void UNSCPUViewMethod::BuildViewer ()
{
  if (!m_built)
  {
    m_viewer.ResetAABB (Viewer::Instance ()->m_volume);

    Resize (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);

    SetAdaptiveSamples (4);
    SetSamples (64);

    m_viewer.m_voleval.m_opacity_correction = 1.0f;
    m_viewer.m_voleval.m_integral_error = GetMaxError ();

    SetIntegralError (1.0);

    printf ("CPUViewMethod Built\n");
    m_built = true;
  }
}

void UNSCPUViewMethod::CleanViewer ()
{
  if (m_built)
  {

    printf ("CPUViewMethod Cleaned\n");
    m_built = false;
  }
}

void UNSCPUViewMethod::SetPThread (bool state)
{
  m_viewer.m_pthread_on = state;
}

bool UNSCPUViewMethod::IsUsingPThread ()
{
  return m_viewer.m_pthread_on;
}

void UNSCPUViewMethod::SetCurrentMethod (int item)
{
  m_viewer.m_voleval.m_current_method = item;
}

void UNSCPUViewMethod::SetSamples (int samples)
{
  if (samples >= 1)
    m_viewer.m_voleval.m_samples = samples;
}

int UNSCPUViewMethod::GetSamples ()
{
  return m_viewer.m_voleval.m_samples;
}

void UNSCPUViewMethod::SetAdaptiveSamples (int adap_samples)
{
  if (adap_samples >= 1)
    m_viewer.m_voleval.m_adaptive_samples = adap_samples;
}

int UNSCPUViewMethod::GetAdaptiveSamples ()
{
  return m_viewer.m_voleval.m_adaptive_samples;
}

void UNSCPUViewMethod::SetIntegralError (double error)
{
  m_viewer.m_voleval.m_integral_error = error;
  m_gui.SetErrorValue (error);
}

double UNSCPUViewMethod::GetIntegralError ()
{
  return m_viewer.m_voleval.m_integral_error;
}

void UNSCPUViewMethod::SetInnerIntegralError (double error)
{
  m_viewer.m_voleval.m_error_inner_integral = error;
}

double UNSCPUViewMethod::GetInnerIntegralError ()
{
  return m_viewer.m_voleval.m_error_inner_integral;
}

void UNSCPUViewMethod::SetIntervalDistance (double distance)
{
  m_viewer.m_voleval.m_adaptive_distance_intervals = distance;
}

double UNSCPUViewMethod::GetIntervalDistance ()
{
  return m_viewer.m_voleval.m_adaptive_distance_intervals;
}

double UNSCPUViewMethod::GetMinError ()
{
  return m_gui.GetMinError ();
}

double UNSCPUViewMethod::GetMaxError ()
{
  return m_gui.GetMaxError ();
}

void UNSCPUViewMethod::Resize (int width, int height)
{
  m_canvaswidth = width;
  m_canvasheight = height;

  if (m_viewer.m_pixels)
    delete[] m_viewer.m_pixels;
  m_viewer.m_pixels = new float[width*height * 4];
  m_viewer.ResetCamera (width, height);
}

int UNSCPUViewMethod::GetNumberOfThreads ()
{
  return m_viewer.m_num_of_threads;
}

void UNSCPUViewMethod::SetNumberOfThreads (int nthreads)
{
  if (nthreads == 2 || nthreads == 4)
    m_viewer.m_num_of_threads = nthreads;
}

void UNSCPUViewMethod::SetUseFunctionEvaluation (bool usefunctionevaluation)
{
  m_viewer.m_voleval.m_use_evaluate_function = usefunctionevaluation;
  m_gui.SetUseFunctionEvaluation (usefunctionevaluation);
}

void UNSCPUViewMethod::SetFunctionEvaluationIndex (int index)
{
  m_viewer.m_voleval.SetFunctionEvaluationIndex (index);
}

void UNSCPUViewMethod::SetUseAlphaTruncation (bool state)
{
  m_viewer.m_voleval.m_alpha_truncation = state;
}

bool UNSCPUViewMethod::IsUsingAlphaTruncation ()
{
  return m_viewer.m_voleval.m_alpha_truncation;
}

VolumeEvaluator* UNSCPUViewMethod::GetVolumeEvaluator ()
{
  return &m_viewer.m_voleval;
}

void UNSCPUViewMethod::RecalculateAABBSizes ()
{
  float w = (float)Viewer::Instance ()->m_volume->GetWidth ();
  float h = (float)Viewer::Instance ()->m_volume->GetHeight ();
  float d = (float)Viewer::Instance ()->m_volume->GetDepth ();
  
  float x, y, z;
  
  if (m_viewer.m_scaletomaxanchorvalue)
  {
    float max = std::max (w, std::max (h, d));

    x = ((w / max) * m_viewer.m_max_aabb_anchor_value) / 2.0f;
    y = ((h / max) * m_viewer.m_max_aabb_anchor_value) / 2.0f;
    z = ((d / max) * m_viewer.m_max_aabb_anchor_value) / 2.0f;
  }
  else
  {
    x = w / 2.0f;
    y = h / 2.0f;
    z = d / 2.0f;
  }

  m_viewer.SetAABB (Viewer::Instance ()->m_volume, -x, -y, -z, x, y, z);
}
